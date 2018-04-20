#ifndef __EVENT_H
#define __EVENT_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  event_type_mouse_move,
  event_type_mouse_button_press,
  event_type_mouse_button_release
} EventType;

typedef struct {
  // unimplemented
} EventMouseMove;

typedef enum {
  ButtonLeft   = 0b001,
  ButtonRight  = 0b010,
  ButtonMiddle = 0b100
} ButtonState;

typedef struct {
  ButtonState button;
} EventMouseButtonPress;

typedef struct {
  ButtonState button;
} EventMouseButtonRelease;

typedef struct {
  EventType type;

  union {
             EventMouseMove mouse_move;
      EventMouseButtonPress mouse_press;
    EventMouseButtonRelease mouse_release;
  };

} Event;

bool open_event_loop();
bool poll_event( Event* );
void close_event_loop();

#endif
