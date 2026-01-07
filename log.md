# Log

Keep this file updated with notable build/runtime errors and how they were resolved.

## 项目概述

本项目是一个安全的键盘扫描码演示系统：内核模块提供模拟扫描码字符设备（/dev/kbd），
用户态 C 库负责扫描码解析与统计，Qt 前端读取设备显示最后 200 个字符并统计总数和日计数。
项目使用 CMake 构建、CTest 测试，要求先 `make test` 再构建。内核模块使用 kfifo 缓冲。

## 2025-01-07

- `ctest` 运行失败：找不到 `test_scancode`/`test_stats`（使用 `cmake --build build --target test` 时）。
  - 解决：新增顶层 `Makefile`，先构建测试目标再调用 `ctest`。
- Qt 构建错误：`no matching function for call to 'MainWindow::close(int&)'`。
  - 解决：在 `app/mainwindow.cpp` 使用 `::close(m_fd)`，避免与 `QWidget::close()` 冲突。
- 内核模块构建错误：`no_llseek` 未声明。
  - 解决：在 `kernel/kbd_sim.c` 改用 `noop_llseek`。
- 内核模块构建错误：`del_timer_sync` 未声明。
  - 解决：在 `kernel/kbd_sim.c` 改用 `timer_delete_sync`。
- 缺少 Clang 导致构建使用 GCC（`clang: command not found`）。
  - 解决：在 `CMakeLists.txt` 强制要求 Clang，构建前需安装 Clang。
