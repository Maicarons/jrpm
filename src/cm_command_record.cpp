/*
 * Command recording and replay - implementation (cmclient port, batch 5).
 */

#include "stdafx.h"

#include "cm_command_record.h"

#include "command_func.h"
#include "console_func.h"
#include "console_internal.h"
#include "core/serialisation.hpp"
#include "core/string_consumer.hpp"
#include "company_base.h"
#include "core/backup_type.hpp"
#include "company_func.h"
#include "fileio_func.h"
#include "network/network_type.h"
#include "network/network_internal.h"

#include <fstream>

#include "safeguards.h"

/* Record file format:
 *   "JRCM" magic (4 bytes), u8 version (1), u32 entry count,
 *   then per entry:
 *     u8 company, u32 length, length bytes (DynBaseCommandContainer::Serialise output).
 */

static const char *RECORD_MAGIC = "JRCM";
static const uint8_t RECORD_VERSION = 1;

static std::ofstream _record_file;
static uint32_t _record_count = 0;
static uint32_t _stop_at_frame = UINT32_MAX;

void CommandRecordLog(Commands cmd, TileIndex tile, const CommandPayloadBase &payload, StringID error_msg, CompanyID company)
{
	if (!_record_file.is_open()) return;

	DynBaseCommandContainer container(cmd, error_msg, tile, payload.Clone());

	std::vector<uint8_t> buf;
	BufferSerialisationRef sbuf(buf);
	container.Serialise(sbuf);

	uint32_t len = static_cast<uint32_t>(buf.size());
	uint8_t company_raw = static_cast<uint8_t>(company.base());
	_record_file.write(reinterpret_cast<const char *>(&company_raw), 1);
	_record_file.write(reinterpret_cast<const char *>(&len), 4);
	_record_file.write(reinterpret_cast<const char *>(buf.data()), buf.size());
	_record_count++;
}

static void StopRecording()
{
	if (!_record_file.is_open()) return;

	/* Patch in the entry count: count was written at offset 5. */
	_record_file.seekp(5, std::ios::beg);
	_record_file.write(reinterpret_cast<const char *>(&_record_count), 4);
	_record_file.close();

	IConsolePrint(CC_DEFAULT, "Command recording stopped ({} commands).", _record_count);
	_record_count = 0;
}

void CommandRecordTick()
{
	if (_record_file.is_open() && _stop_at_frame != UINT32_MAX && _frame_counter >= _stop_at_frame) {
		_stop_at_frame = UINT32_MAX;
		StopRecording();
	}
}

static bool ConCmdRecord(std::span<std::string_view> argv)
{
	/* argv[0] is the command name; parameters start at argv[1]. */
	if (argv.size() <= 1) {
		IConsolePrint(CC_HELP, "Record executed commands. Usage: 'cmdrecord [start [file]]' / 'cmdrecord stop'.");
		return true;
	}

	std::string_view sub = argv[1];
	if (sub == "stop") {
		/* Defer the actual flush until the command queue has been drained,
		 * so commands posted just before the stop are still recorded. */
		_stop_at_frame = _frame_counter + 10;
		IConsolePrint(CC_DEFAULT, "Stopping recording after the pending commands.");
		return true;
	}

	/* Optional "start" subcommand, then the file name. */
	if (sub == "start") argv = argv.subspan(2); else argv = argv.subspan(1);

	if (_record_file.is_open()) {
		IConsolePrint(CC_ERROR, "Already recording. Use 'cmdrecord stop' first.");
		return false;
	}

	/* Determine the output file. */
	std::string filename = "cmdrecord.jrcm";
	if (!argv.empty()) filename = std::string(argv[0]);

	/* Create the file in the save directory so the path is always writable. */
	std::string full_path = _personal_dir + filename;

	_record_file.open(full_path, std::ios::binary | std::ios::out | std::ios::trunc);
	if (!_record_file.is_open()) {
		IConsolePrint(CC_ERROR, "Cannot open '{}' for writing.", full_path);
		return false;
	}

	_record_file.write(RECORD_MAGIC, 4);
	_record_file.write(reinterpret_cast<const char *>(&RECORD_VERSION), 1);
	uint32_t zero = 0;
	_record_file.write(reinterpret_cast<const char *>(&zero), 4); // count placeholder

	IConsolePrint(CC_DEFAULT, "Recording commands to '{}'.", full_path);
	return true;
}

static void ReplayExecute(DynBaseCommandContainer &container, CompanyID company)
{
	extern ClientID _cmd_client_id;

	Backup<CompanyID> cur_company(_current_company, FILE_LINE);
	cur_company.Change(company);
	_cmd_client_id = CLIENT_ID_SERVER;

	DoCommandPImplementation(container.cmd, container.tile, *container.payload, container.error_msg, CommandCallback::None, 0,
			DCIF_NETWORK_COMMAND | DCIF_TYPE_CHECKED | DCIF_NOT_MY_CMD);

	cur_company.Restore();
	_cmd_client_id = INVALID_CLIENT_ID;
}

static bool ConCmdReplay(std::span<std::string_view> argv)
{
	if (argv.size() <= 1) {
		IConsolePrint(CC_HELP, "Replay a recorded command file. Usage: 'cmdreplay <file>'.");
		return true;
	}
	if (_record_file.is_open()) {
		IConsolePrint(CC_ERROR, "Stop recording first ('cmdrecord stop').");
		return false;
	}

	std::string full_path = _personal_dir + std::string(argv[1]);

	std::ifstream file(full_path, std::ios::binary | std::ios::in);
	if (!file.is_open()) {
		IConsolePrint(CC_ERROR, "Cannot open '{}'.", full_path);
		return false;
	}

	char magic[4];
	file.read(magic, 4);
	if (file.gcount() != 4 || memcmp(magic, RECORD_MAGIC, 4) != 0) {
		IConsolePrint(CC_ERROR, "'{}' is not a command record file.", full_path);
		return false;
	}

	uint8_t version = 0;
	file.read(reinterpret_cast<char *>(&version), 1);
	if (version != RECORD_VERSION) {
		IConsolePrint(CC_ERROR, "Unsupported record version {}.", version);
		return false;
	}

	uint32_t count = 0;
	file.read(reinterpret_cast<char *>(&count), 4);

	uint32_t replayed = 0;
	uint32_t failed = 0;
	for (uint32_t i = 0; i < count && file.good() && file.peek() != EOF; i++) {
		uint8_t company_raw = 0;
		uint32_t len = 0;
		file.read(reinterpret_cast<char *>(&company_raw), 1);
		file.read(reinterpret_cast<char *>(&len), 4);
		if (file.gcount() != 4 || file.eof() || len > 1 << 20) {
			IConsolePrint(CC_ERROR, "Corrupt record entry {} (bad length).", i);
			break;
		}

		std::vector<uint8_t> data(len);
		file.read(reinterpret_cast<char *>(data.data()), len);
		if (file.gcount() != static_cast<std::streamsize>(len)) {
			IConsolePrint(CC_ERROR, "Corrupt record entry {} (truncated).", i);
			break;
		}

		DeserialisationBuffer read_buffer(data.data(), data.size());
		DynBaseCommandContainer container;
		const char *error = container.Deserialise(read_buffer);
		if (error != nullptr) {
			IConsolePrint(CC_ERROR, "Cannot deserialise command {}: {}", i, error);
			failed++;
			continue;
		}

		ReplayExecute(container, static_cast<CompanyID>(company_raw));
		replayed++;
	}

	IConsolePrint(CC_DEFAULT, "Replay finished: {} executed, {} failed.", replayed, failed);
	return true;
}

void RegisterJRPMCommandRecordCommands()
{
	IConsole::CmdRegister("cmdrecord", ConCmdRecord);
	IConsole::CmdRegister("cmdreplay", ConCmdReplay);
}
