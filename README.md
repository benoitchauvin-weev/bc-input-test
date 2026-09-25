# bc-input-test

A sandbox for the Weev engine's input plan, outside the engine: a small
Windows game loop with mock gameplay, physics and animation systems,
driven by the planned input system. Written to the engine's coding
standards and clang-format, so the code reads like engine code.

## Build and run

Uses the Weev engine's pinned toolchain (cmake, ninja, clang) from
`C:/weev/weev`; nothing to install. Set `WEEV_TOOLCHAIN_ENV` if the weev
checkout lives elsewhere.

```
./build.sh debug                          # or release
./build.sh debug -- --agent               # build, then run
./build/debug/bc_input_test.exe --test    # input self-tests
```

| Flag | Does |
|------|------|
| `--test` | run the input self-tests and exit |
| `--agent` | a scripted agent plays player 2 through `PushAtFrame` |
| `--fixed_dt[=us]` | every frame advances by a constant step (16667 us) |
| `--frames=N` | quit after N frames |
| `--record=PATH` | record events, steps and state hashes per frame |
| `--replay=PATH` | replay a recording, ignoring live input; reports the first diverging frame |
| `--headless` | no window; needs `--fixed_dt` or `--replay` |

Controls: P1 A/D and Space or W, gamepad slot 0. P2 arrows and Up or
RCtrl, gamepad slot 1. R resets, Esc quits.

A record and replay round trip:

```
./build/release/bc_input_test.exe --headless --fixed_dt --agent --frames=1200 --record=build/agent.wvir
./build/release/bc_input_test.exe --headless --replay=build/agent.wvir
```

## Where things are

| Path | Holds |
|------|-------|
| `src/app/App.cpp` | the frame loop: pump, timer, poll, drain, build, update, record, render |
| `src/input/Input*.h` | `WvInput` (static: queue, `Push`, backend), `WvInputQueue`, `WvInputFrame` (the snapshot), `WvInputFrameBuilder` (double buffer, held state), key maps, recording |
| `src/input/Input_Win.cpp` | the Windows backend: keys, mouse, raw input, XInput |
| `src/game/` | the world and the gameplay, physics and animation systems, and the scripted agent |
| `src/test/InputTest.cpp` | the self-tests, on a standalone builder and queue |

Not covered here: the web backend (the DOM key table exists and is
tested), actions and contexts, Apple and Android.
