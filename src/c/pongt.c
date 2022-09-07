#include <pebble.h>

// DECLARATION OF APP ELEMENTS
// window element
static Window *s_main_window;
// general layers
static Layer *s_player_layer;
static Layer *s_ball_layer;
// general variables
static int s_total_time = 0;
static int s_player_pose;
static int s_ball_y_pose;
static int s_ball_y_speed;
static int s_ball_x_pose;
static int s_ball_x_speed;
static bool s_pause_flag = false;
// timers
static AppTimer *s_main_timer;

// constants
#define PAD_LENGTH 18

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
  // Moves the player position up only if unpaused
  if (s_pause_flag){
    s_player_pose = s_player_pose - 2;
  }
}

//down
static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Moves the player position down only if unpaused
  if (s_pause_flag){
    s_player_pose = s_player_pose + 2;  
  }
}

//BUTTON RELATED CALLBACKS CONFIGURATION
static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 50, prv_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 50, prv_down_click_handler);
}

//MOVING ITEMS MANAGEMENT
//player movement management
static void player_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Erase the previous bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the new bar
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, s_player_pose, 3, PAD_LENGTH), 0, GCornerNone);

}

//ball movement management
static void ball_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Calculate the ball speed
  // Wall bouncing
  if (s_ball_x_pose <= 0){
    s_ball_x_pose = 0;
    s_ball_x_speed = -s_ball_x_speed;
  }
  else if (s_ball_x_pose >= bounds.size.w-3){
    s_ball_x_pose = bounds.size.w-3;
    s_ball_x_speed = -s_ball_x_speed;
  }
  // Player bouncing
  if (s_ball_x_pose <= 8 && s_ball_y_pose>=s_player_pose-2 && s_ball_y_pose<=s_player_pose+PAD_LENGTH){
    s_ball_x_pose = 8;
    s_ball_x_speed = -s_ball_x_speed;
  }
  // Draw the new ball
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(s_ball_x_pose, s_ball_y_pose, 3, 3), 0, GCornerNone);

}

// TIME RELATED CALLS
static void update_screen(void *data){
  s_total_time++;
  // Resets timer
  s_main_timer = app_timer_register(50, update_screen, false);
  // Calculate the new ball position only if unpaused
  if (s_pause_flag){
    s_ball_x_pose = s_ball_x_pose + s_ball_x_speed;
    s_ball_y_pose = s_ball_y_pose + s_ball_y_speed;
    
    // Update screen only if unpaused
    layer_mark_dirty(window_get_root_layer(s_main_window));
  }
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

  //BALL
  //pose initialization. Initial ball movement is random, giving priority to player
  s_ball_y_pose = 60;
  s_ball_x_pose = 60;
  s_ball_x_speed = (s_total_time%3 * 2) - 2;
  if (s_ball_x_speed==0){
    s_ball_x_speed = -2;
  }
  s_ball_y_speed = 0;
  //create a layer where the ball can move
  s_ball_layer = layer_create(GRect(0, 25, bounds.size.w, 120));
  //set a callback that will be called each time the ball is redrawn
  layer_set_update_proc(s_ball_layer, ball_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_ball_layer);

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
  layer_destroy(s_ball_layer);
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
