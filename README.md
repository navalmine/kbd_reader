# codex_demo_kbd_sim

This project demonstrates a safe, simulated keyboard scancode device and a Qt UI that reads it.
It does **not** hook keyboard interrupts or capture real input.
It requires Clang for both user-space and kernel module builds.

## Layout

- `kernel/` simulated scancode kernel module (`/dev/kbd`)
- `lib/` C helpers (scancode mapping + stats)
- `app/` Qt Widgets UI
- `tests/` C tests for mapping and stats

## Build (CMake)

```bash
make test
```

This runs tests first, then builds the targets if tests pass.

## Kernel module (simulated device)

```bash
cd kernel
make
sudo insmod kbd_sim.ko interval_ms=120
ls -l /dev/kbd
```

To unload:

```bash
sudo rmmod kbd_sim
```

## Run UI

```bash
./build/app/kbd_ui
```

Optional device override:

```bash
DEVICE_PATH=/dev/kbd ./build/app/kbd_ui
```

Stats are stored at `~/.local/share/codex_demo/kbd_sim_ui/stats.txt`.

## Notes

- The device emits a repeating simulated scancode sequence. It does not read hardware input.
- "Total count" is persisted to the stats file. To approximate "since boot", run the UI at boot or clear the stats file at boot.
