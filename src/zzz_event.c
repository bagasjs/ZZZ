#include "zzz.h"
#include "zzz_internal.h"

ZEvent* _zNewEvent(ZEventQueue* eq, int type)
{
    if(!eq)
        return NULL;

    ZEvent* ev = eq->events + eq->head;
    eq->head = (eq->head + 1) % ZZZ_EVENT_QUEUE_CAPACITY;
    if(eq->head == eq->tail)
        return NULL;

    zMemZero(ev, sizeof(ZEvent));
    ev->type = type;
    return ev;
}

void _zInputKey(ZEventQueue* eq, i32 key, i32 scancode, i32 action, i32 mods)
{
   ZEvent* ev = _zNewEvent(eq, action);
   if(!ev)
       return;
   ev->keyboard.key = key;
   ev->keyboard.scancode = scancode;
   ev->keyboard.mods = mods;
}

b32 zNextEvent(ZZZ *app, ZEvent *ev)
{
    if(!ev || !app)
        return FALSE;
    zMemZero(ev, sizeof(ZEvent));
    if(app->eq.head != app->eq.tail) {
        *ev = app->eq.events[app->eq.tail];
        app->eq.tail = (app->eq.tail + 1) % ZZZ_EVENT_QUEUE_CAPACITY;
    }
    return ev->type != ZEVENT_UNKNOWN;
}
