#include <pebble.h>

// DECLARATION OF APP ELEMENTS
// window element
static Window *s_main_window;
// general layers
static Layer *s_player_layer;
static Layer *s_enemy_layer;
static Layer *s_ball_layer;
// layers for images
static BitmapLayer *s_background_layer;
// images
static GBitmap *s_background_bitmap;
// general variables
static int s_total_time = 0;
static int s_player_points = 0;
static int s_enemy_points = 0;
static int s_player_pose;
static int s_enemy_pose;
static int s_ball_y_pose;
static int s_ball_y_speed;
static int s_ball_x_pose;
static int s_ball_x_speed;
static bool s_pause_flag = false;
// timers
static AppTimer *s_main_timer;

// constants
#define PAD_LENGTH 22

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
    if (s_player_pose>=0){
      s_player_pose = s_player_pose - 4;
    }
  }
}

//down
static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Moves the player position down only if unpaused
  if (s_pause_flag){
    if (s_player_pose<=120-PAD_LENGTH){
      s_player_pose = s_player_pose + 4;  
    }
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

//enemy movement management
static void enemy_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Erase the previous bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the new bar
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, s_enemy_pose, 3, PAD_LENGTH), 0, GCornerNone);

}

//ball movement management
static void ball_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int bounce_point;

  // Calculate the ball speed
  // Wall bouncing
  if (s_ball_x_pose <= 0){
    s_ball_x_pose = bounds.size.w / 2;
    s_ball_y_pose = bounds.size.h / 2;
    s_player_pose = (bounds.size.h - PAD_LENGTH) / 2;
    s_enemy_pose = (bounds.size.h - PAD_LENGTH) / 2;
    s_ball_x_speed = 2;
    s_ball_y_speed = (s_total_time % 5) - 2;
    s_enemy_points++;
    s_pause_flag = false;
  }
  else if (s_ball_x_pose >= bounds.size.w-3){
    s_ball_x_pose = bounds.size.w / 2;
    s_ball_y_pose = bounds.size.h / 2;
    s_player_pose = (bounds.size.h - PAD_LENGTH) / 2;
    s_enemy_pose = (bounds.size.h - PAD_LENGTH) / 2;
    s_ball_x_speed = -2;
    s_ball_y_speed = (s_total_time % 5) - 2;
    s_player_points++;
    s_pause_flag = false;
  }

  if (s_ball_y_pose <= 0){
    s_ball_y_pose = 0;
    s_ball_y_speed = -s_ball_y_speed;
  }
  else if (s_ball_y_pose >= bounds.size.h-3){
    s_ball_y_pose = bounds.size.h-3;
    s_ball_y_speed = -s_ball_y_speed;
  }

  // Player bouncing
  if (s_ball_x_pose <= 8 && s_ball_y_pose>=s_player_pose-2 && s_ball_y_pose<=s_player_pose+PAD_LENGTH){
    s_ball_x_pose = 8;
    // when the ball bounces on the player, it will always have a constant x_speed towards the enemy
    s_ball_x_speed = 4;

    //the y speed is calculated depending on where the ball hits the player
    bounce_point = (s_ball_y_pose + 1 - s_player_pose);
    switch (bounce_point)
    {
    case -1:
      s_ball_y_speed = -5;
      break;

    case PAD_LENGTH:
      s_ball_y_speed = 5;
      break;

    default:
      s_ball_y_speed = ((bounce_point * 8 ) / PAD_LENGTH) - 4;
      break;
    }
  }

// enemy bouncing
  if (s_ball_x_pose >= bounds.size.w-8 && s_ball_y_pose>=s_enemy_pose-2 && s_ball_y_pose<=s_enemy_pose+PAD_LENGTH){
    s_ball_x_pose = bounds.size.w-8;
    // when the ball bounces on the enemy, it will always have a constant x_speed towards the player
    s_ball_x_speed = -4;

    //the y speed is calculated depending on where the ball hits the enemy
    bounce_point = (s_ball_y_pose + 1 - s_enemy_pose);
    switch (bounce_point)
    {
    case -1:
      s_ball_y_speed = -5;
      break;

    case PAD_LENGTH:
      s_ball_y_speed = 5;
      break;

    default:
      s_ball_y_speed = ((bounce_point * 8 ) / PAD_LENGTH) - 4;
      break;
    }
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

    // Move the enemy towards the ball
    if (s_ball_y_pose > s_enemy_pose){ 
      if (s_enemy_pose<=120-PAD_LENGTH){
        s_enemy_pose = s_enemy_pose + 4;  
      }
    }
    else{ 
      if (s_enemy_pose>=0){
        s_enemy_pose = s_enemy_pose - 4;
      }
    }
    
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
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);

  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(GRect(0,0,bounds.size.w,bounds.size.h));

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  
  //PLAYER
  //pose initialization
  s_player_pose = 60 - (PAD_LENGTH / 2);
  //create a layer where the player can move
  s_player_layer = layer_create(GRect(5, 24, 3, 120));
  //set a callback that will be called each time the player is redrawn
  layer_set_update_proc(s_player_layer, player_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_player_layer);


  //ENEMY
  //pose initialization
  s_enemy_pose = 60 - (PAD_LENGTH / 2);
  //create a layer where the player can move
  s_enemy_layer = layer_create(GRect(bounds.size.w-8, 24, 3, 120));
  //set a callback that will be called each time the player is redrawn
  layer_set_update_proc(s_enemy_layer, enemy_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_enemy_layer);

  //BALL
  //pose initialization. Initial ball movement is random, giving priority to player
  s_ball_y_pose = 60;
  s_ball_x_pose = 60;
  s_ball_x_speed = (s_total_time % 3 * 2) - 2;
  if (s_ball_x_speed==0){
    s_ball_x_speed = -2;
  }
  s_ball_y_speed = (s_total_time % 5) - 2;
  //create a layer where the ball can move
  s_ball_layer = layer_create(GRect(0, 24, bounds.size.w, 120));
  //set a callback that will be called each time the ball is redrawn
  layer_set_update_proc(s_ball_layer, ball_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_ball_layer);

}

// WINDOW DEINIT
static void prv_window_unload(Window *window) {
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
    
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
    
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
  layer_destroy(s_enemy_layer);
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
