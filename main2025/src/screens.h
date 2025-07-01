#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *settings;
    lv_obj_t *select_asset_screen;
    lv_obj_t *select_inspection_type;
    lv_obj_t *inspection_form;
    lv_obj_t *inspection_zones;
    lv_obj_t *obj0;
    lv_obj_t *logo1;
    lv_obj_t *do_settings;
    lv_obj_t *do_inspect_button;
    lv_obj_t *do_sync;
    lv_obj_t *do_power;
    lv_obj_t *help1;
    lv_obj_t *obj1;
    lv_obj_t *logo1_2;
    lv_obj_t *back_from_settings;
    lv_obj_t *help2_1;
    lv_obj_t *setting_wifi_name;
    lv_obj_t *setting_wifi_password;
    lv_obj_t *setting_server_url;
    lv_obj_t *setting_company;
    lv_obj_t *setting_division;
    lv_obj_t *obj2;
    lv_obj_t *logo1_1;
    lv_obj_t *do_select_inspection_type;
    lv_obj_t *back_from_select_asset;
    lv_obj_t *asset_list;
    lv_obj_t *obj3;
    lv_obj_t *selected_asset_list;
    lv_obj_t *obj4;
    lv_obj_t *de_select_asset;
    lv_obj_t *select_asset;
    lv_obj_t *read_asset_tag;
    lv_obj_t *help2;
    lv_obj_t *temp_asset;
    lv_obj_t *de_select_asset_1;
    lv_obj_t *obj5;
    lv_obj_t *logo1_3;
    lv_obj_t *do_inspection_form;
    lv_obj_t *back_from_select_insp;
    lv_obj_t *inspection_types;
    lv_obj_t *obj6;
    lv_obj_t *help2_2;
    lv_obj_t *obj7;
    lv_obj_t *logo1_4;
    lv_obj_t *help2_3;
    lv_obj_t *do_zones;
    lv_obj_t *back_from_form_fields;
    lv_obj_t *back_from_form;
    lv_obj_t *inspection_types_1;
    lv_obj_t *obj8;
    lv_obj_t *inspection_type_name;
    lv_obj_t *obj9;
    lv_obj_t *logo1_5;
    lv_obj_t *help2_4;
    lv_obj_t *submit;
    lv_obj_t *back_from_form_zones;
    lv_obj_t *back_from_form_1;
    lv_obj_t *read_zone;
    lv_obj_t *all_ok_button;
    lv_obj_t *comp_ok_button;
    lv_obj_t *defect_button;
    lv_obj_t *zone_asset_list;
    lv_obj_t *asset_template;
    lv_obj_t *zone_list;
    lv_obj_t *zone_template;
    lv_obj_t *zone_component_list;
    lv_obj_t *compo_template;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SETTINGS = 2,
    SCREEN_ID_SELECT_ASSET_SCREEN = 3,
    SCREEN_ID_SELECT_INSPECTION_TYPE = 4,
    SCREEN_ID_INSPECTION_FORM = 5,
    SCREEN_ID_INSPECTION_ZONES = 6,
};

void create_screen_main();
void tick_screen_main();

void create_screen_settings();
void tick_screen_settings();

void create_screen_select_asset_screen();
void tick_screen_select_asset_screen();

void create_screen_select_inspection_type();
void tick_screen_select_inspection_type();

void create_screen_inspection_form();
void tick_screen_inspection_form();

void create_screen_inspection_zones();
void tick_screen_inspection_zones();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/