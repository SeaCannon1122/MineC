
#if defined(WINDOW_IMPLEMENTATION_WINDOWS)
#include "window_windows.c"

#elif defined(WINDOW_IMPLEMENTATION_X11)
#include "window_x11.c"

#elif defined(WINDOW_IMPLEMENTATION_WAYLAND)
#include "window_wayland.c"

#endif

