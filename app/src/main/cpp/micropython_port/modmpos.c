#include "py/obj.h"
#include "py/runtime.h"
#include "mpos_bridge.h"

static mp_obj_t mpos_set_title(mp_obj_t text_obj) {
    mpos_bridge_set_title(mp_obj_str_get_str(text_obj));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mpos_set_title_obj, mpos_set_title);

static mp_obj_t mpos_set_subtitle(mp_obj_t text_obj) {
    mpos_bridge_set_subtitle(mp_obj_str_get_str(text_obj));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mpos_set_subtitle_obj, mpos_set_subtitle);

static mp_obj_t mpos_set_status(mp_obj_t text_obj) {
    mpos_bridge_set_status(mp_obj_str_get_str(text_obj));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mpos_set_status_obj, mpos_set_status);

static mp_obj_t mpos_clicks(void) {
    return mp_obj_new_int_from_uint(mpos_bridge_click_count());
}
static MP_DEFINE_CONST_FUN_OBJ_0(mpos_clicks_obj, mpos_clicks);

static const mp_rom_map_elem_t mpos_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mpos) },
    { MP_ROM_QSTR(MP_QSTR_set_title), MP_ROM_PTR(&mpos_set_title_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_subtitle), MP_ROM_PTR(&mpos_set_subtitle_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_status), MP_ROM_PTR(&mpos_set_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_clicks), MP_ROM_PTR(&mpos_clicks_obj) },
};
static MP_DEFINE_CONST_DICT(mpos_module_globals, mpos_module_globals_table);

const mp_obj_module_t mp_module_mpos = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mpos_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mpos, mp_module_mpos);
