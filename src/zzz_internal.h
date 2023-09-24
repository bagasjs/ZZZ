#ifndef ZZZ_INTERNAL_H
#define ZZZ_INTERNAL_H

#include "zzz.h"

ZEvent* _zNewEvent(ZEventQueue* eq, int type);
void _zInputKey(ZEventQueue* eq, i32 key, i32 scancode, i32 action, i32 mods);

#endif // ZZZ_INTERNAL_H
