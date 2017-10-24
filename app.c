#include <pebble.h>

#define BATTERY 0
#define ALTITUDE 1
#define SPEED 2
#define GPS 3
#define STATE 4

static Window *s_main_window;
static TextLayer *s_battery_layer;
static TextLayer *s_altitude_layer;
static TextLayer *s_speed_layer;
static TextLayer *s_gps_layer;
static TextLayer *s_state_layer;
static ActionBarLayer *s_action_bar;
static GBitmap *s_gps_bitmap, *s_photo_bitmap, *s_record_bitmap;

static char batteryLevel[30];
static char altitudeValue[30];
static char speedValue[30];
static char gpsValue[30];
static char state[30];


static void timer_callback(struct tm *tick_time, TimeUnits changed) {
  // Status
  DictionaryIterator *iter;

  app_message_outbox_begin(&iter);
  dict_write_cstring(iter, 7, "STATUS");
  app_message_outbox_send();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  Tuple *battery_tuple = dict_find(iterator, BATTERY);
  Tuple *altitude_tuple = dict_find(iterator, ALTITUDE);
  Tuple *speed_tuple = dict_find(iterator, SPEED);
  Tuple *gps_tuple = dict_find(iterator, GPS);
  Tuple *state_tuple = dict_find(iterator, STATE);


  if(battery_tuple && altitude_tuple && speed_tuple && gps_tuple && state_tuple) {

    int battery = battery_tuple->value->int32;

    snprintf(altitudeValue, sizeof(altitudeValue), "%s", altitude_tuple->value->cstring);
    snprintf(speedValue, sizeof(speedValue), "%s", speed_tuple->value->cstring);
    snprintf(gpsValue, sizeof(gpsValue), "%s", gps_tuple->value->cstring);
    snprintf(state, sizeof(state), "%s", state_tuple->value->cstring);

    snprintf(batteryLevel, 5, "%d %%", battery);

    text_layer_set_text(s_battery_layer, batteryLevel);
    text_layer_set_text(s_altitude_layer, altitudeValue);
    text_layer_set_text(s_speed_layer, speedValue);
    text_layer_set_text(s_gps_layer, gpsValue);
    text_layer_set_text(s_state_layer, state);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Updating data!");
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_gps_bitmap = gbitmap_create_with_resource(RESOURCE_ID_GPS_ICON);
  s_record_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RECORD_ICON);
  s_photo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PHOTO_ICON);

  // Set the icons
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_gps_bitmap);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_photo_bitmap);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_record_bitmap);

  // Add to Window
  action_bar_layer_add_to_window(s_action_bar, window);

  s_battery_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(-20, -14), PBL_IF_ROUND_ELSE(10,4), bounds.size.w, 50));

  s_altitude_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(-20, -14), PBL_IF_ROUND_ELSE(50, 44), bounds.size.w, 50));

  s_speed_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(-20, -14), PBL_IF_ROUND_ELSE(100, 94), bounds.size.w, 60));

  s_gps_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(-60, -54), PBL_IF_ROUND_ELSE(10,4), bounds.size.w, 60));

  s_state_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(-20, -14), PBL_IF_ROUND_ELSE(150,144), bounds.size.w, 50));

  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);

  text_layer_set_background_color(s_altitude_layer, GColorClear);
  text_layer_set_text_color(s_altitude_layer, GColorBlack);
  text_layer_set_font(s_altitude_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_altitude_layer, GTextAlignmentCenter);

  text_layer_set_background_color(s_speed_layer, GColorClear);
  text_layer_set_text_color(s_speed_layer, GColorBlack);
  text_layer_set_text(s_speed_layer, "Connecting");
  text_layer_set_font(s_speed_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_speed_layer, GTextAlignmentCenter);

  text_layer_set_background_color(s_gps_layer, GColorClear);
  text_layer_set_text_color(s_gps_layer, GColorBlack);
  text_layer_set_text_alignment(s_gps_layer, GTextAlignmentCenter);

  text_layer_set_background_color(s_state_layer, GColorClear);
  text_layer_set_text_color(s_state_layer, GColorBlack);
  text_layer_set_text_alignment(s_state_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_altitude_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_speed_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_gps_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_state_layer));

  tick_timer_service_subscribe(SECOND_UNIT, timer_callback);

}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Record
  DictionaryIterator *dict;

  app_message_outbox_begin(&dict);
  dict_write_cstring (dict, 6, "Recording");
  app_message_outbox_send();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
// GPS
  DictionaryIterator *iter;

  app_message_outbox_begin(&iter);
  dict_write_cstring(iter, 3, "GPS");
  app_message_outbox_send();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
// Photo
  DictionaryIterator *iter;

  app_message_outbox_begin(&iter);
  dict_write_cstring(iter, 5, "Photo");
  app_message_outbox_send();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_speed_layer);
  text_layer_destroy(s_altitude_layer);
}

static void init() {
  // Create ActionBarLayer
  s_action_bar = action_bar_layer_create();
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);

  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

}

static void deinit() {
  // Destroy the ActionBarLayer
  action_bar_layer_destroy(s_action_bar);

  // Destroy the icon GBitmaps
  gbitmap_destroy(s_gps_bitmap);
  gbitmap_destroy(s_record_bitmap);
  gbitmap_destroy(s_photo_bitmap);
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
