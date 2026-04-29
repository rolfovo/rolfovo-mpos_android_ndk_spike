# MicroPythonOS Android NDK Spike

Minimal Android host for the first MicroPythonOS Android port milestone:
native graphics and touch input.

What works:

- Java `Activity` with a fullscreen `SurfaceView`
- JNI bridge into a native C++ shared library
- LVGL 9.3 sources compiled from `../_tmp_mpos/lvgl_micropython/lib/lvgl`
- MicroPython core compiled from `../_tmp_mpos/lvgl_micropython/lib/micropython`
- embedded MicroPython runtime started from the native Android shared library
- a temporary built-in `mpos` module for Python-to-LVGL UI updates
- native LVGL display driver flushing to `ANativeWindow`
- Android touch events bridged to an LVGL pointer input device
- a small LVGL screen whose title/status text is changed by MicroPython
- a tappable button that calls back into MicroPython and live touch coordinates

What is intentionally not wired yet:

- MicroPythonOS Python boot scripts
- full `lvgl` Python bindings
- filesystem/assets mounting
- network, camera, GPS, audio and other Android hardware APIs

Build:

```powershell
.\gradlew.bat assembleDebug --console=plain
```

APK output:

```text
app/build/outputs/apk/debug/app-debug.apk
```

The CMake project expects the MicroPythonOS checkout to be present at
`../_tmp_mpos`. If that path changes, update `LVGL_DIR` and `MICROPY_DIR` in
`app/src/main/cpp/CMakeLists.txt`.
