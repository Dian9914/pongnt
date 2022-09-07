#include <pebble.h>

// DECLARATION OF APP ELEMENTS
// window element
static Window *s_main_window;
// general layers
static Layer *s_player_layer;
// general variables
static int s_player_pose;
static bool s_pause_flag = false;
// timers
static AppTimer *s_main_timer;

//BUTTON RELATED CALLS
//center
static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Pauses and unpauses the game
  if (s_pause_flag){
    s_pause_flag = false;
  }
  else{
    s_pause_flag = true;
    }
}

//up
static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Moves the player position up
  s_player_pose = s_player_pose - 2;
}

//down
static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Moves the player position down
  s_player_pose = s_player_pose + 2;
}

//BUTTON RELATED CALLBACKS CONFIGURATION
static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 50, prv_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 50, prv_down_click_handler);
}

//PLAYER PAD MANAGEMENT
static void player_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Erase the previous bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the new bar
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, s_player_pose, 3, 18), 0, GCornerNone);

}

// TIME RELATED CALLS
static void update_screen(void *data){
  // Resets timer
  s_main_timer = app_timer_register(50, update_screen, false);
  // Update screen
  layer_mark_dirty(window_get_root_layer(s_main_window));
}

//WINDOW INIT
static void prv_window_load(Window *window) {
  // Get information about the window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //BACKGROUND

  
  //PLAYER
  //pose initialization
  s_player_pose = 60;
  //create a layer where the player can move
  s_player_layer = layer_create(GRect(5, 25, 3, 120));
  //set a callback that will be called each time the player is redrawn
  layer_set_update_proc(s_player_layer, player_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_player_layer);

}

// WINDOW DEINIT
static void prv_window_unload(Window *window) {
}

// APP INIT
static void prv_init(void) {
  s_main_window = window_create();

  // Configuracion de eventos, callbacks y timers
  window_set_click_config_provider(s_main_window, prv_click_config_provider);
  s_main_timer = app_timer_register(50, update_screen, false);
  // Configuracion de handlers de la ventana
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });


  const bool animated = true;
  window_stack_push(s_main_window, animated);
}

// APP DEINIT
static void prv_deinit(void) {
  // destroy all layers
  layer_destroy(s_player_layer);
  // destroy window
  window_destroy(s_main_window);
}

// MAIN
int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_main_window);

  app_event_loop();
  prv_deinit();
}
