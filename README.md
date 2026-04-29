# MicroPythonOS Android NDK Spike

Minimal Android host for the first MicroPythonOS Android port milestone:
native graphics and touch input.

What works:

- Java `Activity` with a fullscreen `SurfaceView`
- JNI bridge into a native C++ shared library
- LVGL 9.3 sources compiled from `../_tmp_mpos/lvgl_micropython/lib/lvgl`
- native LVGL display driver flushing to `ANativeWindow`
- Android touch events bridged to an LVGL pointer input device
- a small native LVGL screen with a tappable button and live touch coordinates

What is intentionally not wired yet:

- MicroPython interpreter startup
- MicroPythonOS Python boot scripts
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
`../_tmp_mpos`. If that path changes, update `LVGL_DIR` in
`app/src/main/cpp/CMakeLists.txt`.
