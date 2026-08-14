/*
 * Command recording and replay (cmclient port, batch 5).
 *
 * Records every executed command (cmd, tile, payload, company) into a
 * binary file, and replays the file back by re-executing the commands.
 * Uses jrpm's built-in DynBaseCommandContainer serialisation so payloads
 * round-trip losslessly.
 */

#ifndef CM_COMMAND_RECORD_H
#define CM_COMMAND_RECORD_H

#include "command_type.h"
#include "company_type.h"
#include "tile_type.h"

/** Record a command execution (called from DoCommandPInternal). */
void CommandRecordLog(Commands cmd, TileIndex tile, const CommandPayloadBase &payload, StringID error_msg, CompanyID company);

/** Per-tick maintenance (called from StateGameLoop); handles deferred stop. */
void CommandRecordTick();

/** Register the cmdrecord/cmdreplay console commands. */
void RegisterJRPMCommandRecordCommands();

#endif /* CM_COMMAND_RECORD_H */
