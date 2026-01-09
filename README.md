# serio_hook_for_keyboard_recording

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
sudo insmod kbd_sim.ko
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
