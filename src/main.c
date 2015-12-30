#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer; // 12 hour time layer
static TextLayer *time_shadow_layer;
static TextLayer *battery_layer; //Battery display 
static TextLayer *date_layer; //Current date display
static TextLayer *inner_rect;
static TextLayer *underline;

// The layers for the background
static TextLayer *background;
static int colour_state = -1;

/*
Main_colour = Time, outer border colours
Accent_colour = Time shadow, battery colours
Underline_colour = colour of the bar under the time
Date_colour = colour of the date text
*/
#if defined(PBL_COLOR)
  #define Main_colour GColorLightGray
  #define Accent_colour GColorDarkGray
  #define Date_colour Accent_colour
  #define Underline_colour Accent_colour
#elif defined(PBL_BW)
  #define Main_colour GColorBlack
  #define Accent_colour GColorWhite
  #define Date_colour GColorBlack
  #define Underline_colour GColorBlack
#endif



static void display_batt_attr(BatteryChargeState batt_state){
  static char batt_status[20];
  char percent = '%';
  if(batt_state.is_charging){
    snprintf(batt_status, sizeof(batt_status), "charging..."); 
  }else{
    snprintf(batt_status, sizeof(batt_status), "%d%c", batt_state.charge_percent, percent);
  }
  
  text_layer_set_text(battery_layer, batt_status); 

}

static void update_display(int minute){
  
  GColor main_col;
  GColor acc_col;
  int cur_min = minute;
  // Cycles through Red -> Green -> Blue -> Indigo every 15 minutes
  if(cur_min >= 0 && cur_min < 15){
      main_col = GColorRed;
      acc_col = GColorDarkCandyAppleRed;
    } else if(cur_min >= 15 && cur_min < 30){
      main_col = GColorGreen;
      acc_col = GColorIslamicGreen;
    } else if(cur_min >= 30 && cur_min < 45){
      main_col = GColorVividCerulean;
      acc_col = GColorCobaltBlue;
    } else if(cur_min >= 45){
      main_col = GColorLavenderIndigo;
      acc_col = GColorIndigo;
  }
    
  text_layer_set_background_color(background, main_col);
  text_layer_set_background_color(underline, acc_col);
  text_layer_set_text_color(s_time_layer, main_col );
  text_layer_set_text_color(time_shadow_layer, acc_col );
  text_layer_set_text_color(battery_layer, acc_col );
  text_layer_set_text_color(date_layer, acc_col );
  
}


static void update_time(){
  // Get a tm struct
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  static char shadow_buffer[8];
  static char date_buffer[16];
  if(clock_is_24h_style()){
    strftime(shadow_buffer, sizeof(shadow_buffer), "%H:%M", tick_time);
    strftime(s_buffer, sizeof(s_buffer), "%H:%M", tick_time);
    strftime(date_buffer, sizeof(date_buffer), "%a, %d %b", tick_time);
  }else{
    strftime(shadow_buffer, sizeof(shadow_buffer), "%I:%M", tick_time);
    strftime(s_buffer, sizeof(s_buffer), "%I:%M", tick_time);
    strftime(date_buffer, sizeof(date_buffer), "%a, %d %b", tick_time);
  }
  
  if(colour_state){
    update_display(tick_time->tm_min);
  }
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(time_shadow_layer, shadow_buffer);
  text_layer_set_text(date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
    update_time();
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  
  // Background
  background = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  inner_rect = text_layer_create(GRect(12, 16, bounds.size.w - 24, bounds.size.h - 32));
  text_layer_set_background_color(background, Main_colour);
  text_layer_set_background_color(inner_rect, GColorWhite);
  
  // Underline bar
  underline = text_layer_create(GRect(20, bounds.size.h/2 + 13 - 5, bounds.size.w - 40, 6));
  text_layer_set_background_color(underline, Underline_colour);
  
  // Time Displays
  s_time_layer = text_layer_create(GRect(0, bounds.size.h/2 - 30 - 5, bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, Main_colour );
  text_layer_set_text(s_time_layer, "00:00");
  const char* font;
  PBL_IF_COLOR_ELSE(font = FONT_KEY_LECO_38_BOLD_NUMBERS, font = FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  text_layer_set_font(s_time_layer, fonts_get_system_font(font));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Time display shadow
  if(colour_state){
    time_shadow_layer = text_layer_create(GRect(0, bounds.size.h/2 - 27 - 5, bounds.size.w + 7, 45));
    text_layer_set_background_color(time_shadow_layer, GColorClear);
    text_layer_set_text_color(time_shadow_layer, Accent_colour );
    text_layer_set_text(time_shadow_layer, "00:00");
    text_layer_set_font(time_shadow_layer, fonts_get_system_font(font));
    text_layer_set_text_alignment(time_shadow_layer, GTextAlignmentCenter);
  }
  
  // Battery
  battery_layer = text_layer_create(GRect(35, 0, 80, 50));
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_text_color(battery_layer, Accent_colour );
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  text_layer_set_text(battery_layer, "000%");
  
  // Date
  date_layer = text_layer_create(GRect(20, bounds.size.h/2 + 13 - 5, bounds.size.w - 40, 48));
  text_layer_set_text_color(date_layer, Date_colour );
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);  
  
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(background));
  layer_add_child(window_layer, text_layer_get_layer(inner_rect));
  layer_add_child(window_layer, text_layer_get_layer(underline));
  layer_add_child(window_layer, text_layer_get_layer(time_shadow_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));

  // Handlers
  display_batt_attr(battery_state_service_peek());


  
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(time_shadow_layer);
  text_layer_destroy(battery_layer);
  text_layer_destroy(background);
  text_layer_destroy(date_layer);
  text_layer_destroy(underline);
  text_layer_destroy(inner_rect);

}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  //Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //Register with BatteryStateService
  battery_state_service_subscribe(display_batt_attr);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  PBL_IF_COLOR_ELSE(colour_state = 1, colour_state = 0);
  
  // Init event handlers
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
