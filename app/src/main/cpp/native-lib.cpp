#include <jni.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>

#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

extern "C" {
#include "lvgl.h"
}

#define LOG_TAG "MPOSAndroidSpike"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {

std::mutex g_lock;
ANativeWindow *g_window = nullptr;
lv_display_t *g_display = nullptr;
lv_indev_t *g_pointer = nullptr;
uint8_t *g_draw_buf = nullptr;
uint32_t g_draw_buf_size = 0;
int32_t g_width = 0;
int32_t g_height = 0;
bool g_lvgl_started = false;
bool g_paused = false;
bool g_touch_pressed = false;
int32_t g_touch_x = 0;
int32_t g_touch_y = 0;
uint32_t g_click_count = 0;
lv_obj_t *g_count_label = nullptr;
lv_obj_t *g_touch_label = nullptr;

uint32_t clamp_u32(int32_t value, int32_t lower, int32_t upper) {
    return static_cast<uint32_t>(std::max(lower, std::min(value, upper)));
}

void update_count_label() {
    if (g_count_label != nullptr) {
        lv_label_set_text_fmt(g_count_label, "Native LVGL button clicks: %u", g_click_count);
    }
}

void update_touch_label() {
    if (g_touch_label != nullptr) {
        if (g_touch_pressed) {
            lv_label_set_text_fmt(g_touch_label, "Touch: %ld, %ld",
                                  static_cast<long>(g_touch_x),
                                  static_cast<long>(g_touch_y));
        } else {
            lv_label_set_text(g_touch_label, "Touch: released");
        }
    }
}

void button_event_cb(lv_event_t *event) {
    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
        g_click_count++;
        update_count_label();
    }
}

void pointer_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    (void)indev;
    data->point.x = static_cast<lv_coord_t>(g_touch_x);
    data->point.y = static_cast<lv_coord_t>(g_touch_y);
    data->state = g_touch_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map) {
    ANativeWindow *window = g_window;
    if (window == nullptr || g_width <= 0 || g_height <= 0) {
        lv_display_flush_ready(display);
        return;
    }

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(window, &buffer, nullptr) != 0) {
        lv_display_flush_ready(display);
        return;
    }

    const int32_t x1 = static_cast<int32_t>(clamp_u32(area->x1, 0, g_width - 1));
    const int32_t y1 = static_cast<int32_t>(clamp_u32(area->y1, 0, g_height - 1));
    const int32_t x2 = static_cast<int32_t>(clamp_u32(area->x2, 0, g_width - 1));
    const int32_t y2 = static_cast<int32_t>(clamp_u32(area->y2, 0, g_height - 1));
    const int32_t src_width = area->x2 - area->x1 + 1;
    auto *dst_base = static_cast<uint8_t *>(buffer.bits);

    for (int32_t y = y1; y <= y2; y++) {
        const int32_t src_y = y - area->y1;
        const int32_t src_x_offset = x1 - area->x1;
        const uint8_t *src = px_map + ((src_y * src_width + src_x_offset) * 4);
        uint8_t *dst = dst_base + ((y * buffer.stride + x1) * 4);

        for (int32_t x = x1; x <= x2; x++) {
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
            dst[3] = 0xff;
            src += 4;
            dst += 4;
        }
    }

    ANativeWindow_unlockAndPost(window);
    lv_display_flush_ready(display);
}

bool allocate_draw_buffer(int32_t width, int32_t height) {
    const int32_t rows = std::min(height, 80);
    const uint32_t next_size = static_cast<uint32_t>(width * rows * 4);
    if (next_size == 0) {
        return false;
    }

    auto *next = static_cast<uint8_t *>(std::malloc(next_size));
    if (next == nullptr) {
        LOGE("Failed to allocate LVGL draw buffer: %u bytes", next_size);
        return false;
    }

    std::free(g_draw_buf);
    g_draw_buf = next;
    g_draw_buf_size = next_size;
    return true;
}

void create_demo_screen() {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "MicroPythonOS Android NDK spike");
    lv_obj_set_style_text_color(title, lv_color_hex(0xf7f7f7), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 24);

    lv_obj_t *subtitle = lv_label_create(screen);
    lv_label_set_text(subtitle, "LVGL from MicroPythonOS submodule, rendered through ANativeWindow");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x9fb3c8), 0);
    lv_obj_set_width(subtitle, LV_PCT(90));
    lv_obj_set_style_text_align(subtitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);

    lv_obj_t *button = lv_button_create(screen);
    lv_obj_set_size(button, 260, 72);
    lv_obj_align(button, LV_ALIGN_CENTER, 0, -18);
    lv_obj_add_event_cb(button, button_event_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *button_label = lv_label_create(button);
    lv_label_set_text(button_label, "Tap native button");
    lv_obj_center(button_label);

    g_count_label = lv_label_create(screen);
    lv_obj_set_style_text_color(g_count_label, lv_color_hex(0xf7f7f7), 0);
    update_count_label();
    lv_obj_align_to(g_count_label, button, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);

    lv_obj_t *slider = lv_slider_create(screen);
    lv_obj_set_width(slider, LV_PCT(78));
    lv_slider_set_value(slider, 35, LV_ANIM_OFF);
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, -76);

    g_touch_label = lv_label_create(screen);
    lv_obj_set_style_text_color(g_touch_label, lv_color_hex(0x9fe7df), 0);
    update_touch_label();
    lv_obj_align(g_touch_label, LV_ALIGN_BOTTOM_MID, 0, -34);
}

void ensure_lvgl(int32_t width, int32_t height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    if (!g_lvgl_started) {
        lv_init();
        g_lvgl_started = true;
    }

    if (g_display == nullptr) {
        if (!allocate_draw_buffer(width, height)) {
            return;
        }

        g_display = lv_display_create(width, height);
        lv_display_set_color_format(g_display, LV_COLOR_FORMAT_XRGB8888);
        lv_display_set_flush_cb(g_display, flush_cb);
        lv_display_set_buffers(g_display, g_draw_buf, nullptr, g_draw_buf_size,
                               LV_DISPLAY_RENDER_MODE_PARTIAL);

        g_pointer = lv_indev_create();
        lv_indev_set_type(g_pointer, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(g_pointer, pointer_read_cb);

        create_demo_screen();
    } else if (width != g_width || height != g_height) {
        if (!allocate_draw_buffer(width, height)) {
            return;
        }

        lv_display_set_resolution(g_display, width, height);
        lv_display_set_buffers(g_display, g_draw_buf, nullptr, g_draw_buf_size,
                               LV_DISPLAY_RENDER_MODE_PARTIAL);
        lv_obj_invalidate(lv_screen_active());
    }

    g_width = width;
    g_height = height;
}

void set_window(ANativeWindow *window, int32_t width, int32_t height) {
    if (g_window != nullptr) {
        ANativeWindow_release(g_window);
        g_window = nullptr;
    }

    if (window != nullptr) {
        g_window = window;
        ANativeWindow_acquire(g_window);
        g_paused = false;
        if (width > 0 && height > 0) {
            ANativeWindow_setBuffersGeometry(g_window, width, height, WINDOW_FORMAT_RGBA_8888);
            ensure_lvgl(width, height);
            if (g_display != nullptr) {
                lv_obj_invalidate(lv_screen_active());
            }
        }
    }
}

} // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_example_mposandroid_MainActivity_nativeSurfaceCreated(JNIEnv *env, jclass, jobject surface) {
    std::lock_guard<std::mutex> guard(g_lock);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    set_window(window, g_width, g_height);
    if (window != nullptr) {
        ANativeWindow_release(window);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_mposandroid_MainActivity_nativeSurfaceChanged(JNIEnv *env, jclass, jobject surface,
                                                               jint width, jint height) {
    std::lock_guard<std::mutex> guard(g_lock);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    set_window(window, width, height);
    if (window != nullptr) {
        ANativeWindow_release(window);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_mposandroid_MainActivity_nativeSurfaceDestroyed(JNIEnv *, jclass) {
    std::lock_guard<std::mutex> guard(g_lock);
    set_window(nullptr, 0, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_mposandroid_MainActivity_nativeTouch(JNIEnv *, jclass, jfloat x, jfloat y, jboolean pressed) {
    std::lock_guard<std::mutex> guard(g_lock);
    g_touch_pressed = pressed == JNI_TRUE;
    if (g_touch_pressed) {
        g_touch_x = static_cast<int32_t>(x);
        g_touch_y = static_cast<int32_t>(y);
    }
    update_touch_label();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_mposandroid_MainActivity_nativeTick(JNIEnv *, jclass) {
    std::lock_guard<std::mutex> guard(g_lock);
    if (!g_lvgl_started || g_display == nullptr || g_window == nullptr || g_paused) {
        return;
    }

    lv_tick_inc(16);
    lv_timer_handler();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_mposandroid_MainActivity_nativeResume(JNIEnv *, jclass) {
    std::lock_guard<std::mutex> guard(g_lock);
    g_paused = false;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_mposandroid_MainActivity_nativePause(JNIEnv *, jclass) {
    std::lock_guard<std::mutex> guard(g_lock);
    g_paused = true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_mposandroid_MainActivity_nativeShutdown(JNIEnv *, jclass) {
    std::lock_guard<std::mutex> guard(g_lock);
    set_window(nullptr, 0, 0);
    std::free(g_draw_buf);
    g_draw_buf = nullptr;
    g_draw_buf_size = 0;
    g_display = nullptr;
    g_pointer = nullptr;
    g_count_label = nullptr;
    g_touch_label = nullptr;
    if (g_lvgl_started) {
        lv_deinit();
        g_lvgl_started = false;
    }
}
