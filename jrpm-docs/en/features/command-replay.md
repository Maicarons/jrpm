---
title: Command Recording & Replay
---

# Command Record / Replay

cmclient command replay port (Batch 5, commit `f113acce28`). **Does not port cmclient's command object layer** — instead uses jrpm's built-in command serialization infrastructure (`DynBaseCommandContainer`), completely bypassing the "command object layer."

## Console Commands

```
cmdrecord [start [file]]    # Start recording (default file cmdrecord.jrcm, stored in personal directory)
cmdrecord stop              # Stop (waits 10 ticks for the command queue to drain before writing to disk)
cmdreplay <file>            # Replay: deserialize and immediately execute each command
```

## Implementation Highlights

- **Recording hook**: `CommandRecordLog` is attached after command execution in `DoCommandPInternal` — this is the **single real execution point** for local, network, and replay commands, ensuring no duplicate recording.
- **Serialization**: `cmd / tile / error_msg / payload / company` are losslessly serialized using `DynBaseCommandContainer::Serialise`; file format is `JRCM` magic + version + count + entries.
- **Delayed stop**: `cmdrecord stop` sets a delayed flag of 10 ticks; `StateGameLoop` checks each frame (`CommandRecordTick`) to ensure queued commands are captured before flushing.
- **Replay execution**: Each command is executed immediately via the server command path (`DCIF_NETWORK_COMMAND`); the replay file can be read based on actual data even if the count header is not updated.

## Verification

Dedicated server E2E: record `pause` → 26-byte record file → replay on a new map → `Game paused (manual)` + `Replay finished: 1 executed, 0 failed`.

## Typical Usage

```
cmdrecord start build1     # Start recording
# ... build rails, stations in the game ...
cmdrecord stop             # Stop and write to disk
# After switching maps or crash recovery:
cmdreplay build1           # One-click rebuild of all operations
```