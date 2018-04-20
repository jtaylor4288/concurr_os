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
  int8_t dx, dy;
} EventMouseMove;

typedef struct {
  enum { ButtonLeft, ButtonRight, ButtonMiddle } button;
} EventMouseButtonPress;

typedef struct {
  //
} EventMouseButtonRelease;

typedef struct {
  EventType type;

  union {
             EventMouseMove mouse_move;
      EventMouseButtonPress mouse_button_press;
    EventMouseButtonRelease mouse_button_release;
  };

} Event;

bool open_event_loop();
bool poll_event( Event* );
void close_event_loop();

#endif
