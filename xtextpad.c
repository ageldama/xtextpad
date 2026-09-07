#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>


void send_shift_insert_to_window(GdkDisplay *gdk_display, Window target_wid) {
    if (!GDK_IS_X11_DISPLAY(gdk_display)) {
        g_printerr("Error: Window event injection requires an X11 display.\n");
        exit(EXIT_FAILURE);
    }

    Display *xdisplay = GDK_DISPLAY_XDISPLAY(gdk_display);

    // 1. Resolve keysyms to keycodes
    KeyCode shift_code = XKeysymToKeycode(xdisplay, XK_Shift_L);
    KeyCode insert_code = XKeysymToKeycode(xdisplay, XK_Insert);

    if (shift_code == 0 || insert_code == 0) {
        g_printerr("Failed to resolve keycodes.\n");
        exit(EXIT_FAILURE);
    }

    // 2. Prepare XEvent structure targeting $WID
    XEvent event;
    memset(&event, 0, sizeof(event));
    event.xkey.display     = xdisplay;
    event.xkey.window      = target_wid;
    event.xkey.root        = DefaultRootWindow(xdisplay);
    event.xkey.subwindow   = None;
    event.xkey.time        = CurrentTime;
    event.xkey.x           = 1;
    event.xkey.y           = 1;
    event.xkey.x_root      = 1;
    event.xkey.y_root      = 1;
    event.xkey.same_screen = True;

    // 3. Clear modifiers (--clearmodifiers equivalent)
    // Send Shift Press
    event.type = KeyPress;
    event.xkey.keycode = shift_code;
    event.xkey.state = 0; // No modifiers active initially
    XSendEvent(xdisplay, target_wid, True, KeyPressMask, &event);

    // Send Insert Press with Shift state active
    event.type = KeyPress;
    event.xkey.keycode = insert_code;
    event.xkey.state = ShiftMask; // Apply Shift modifier state
    XSendEvent(xdisplay, target_wid, True, KeyPressMask, &event);

    // Send Insert Release
    event.type = KeyRelease;
    event.xkey.keycode = insert_code;
    event.xkey.state = ShiftMask;
    XSendEvent(xdisplay, target_wid, True, KeyReleaseMask, &event);

    // Send Shift Release
    event.type = KeyRelease;
    event.xkey.keycode = shift_code;
    event.xkey.state = 0;
    XSendEvent(xdisplay, target_wid, True, KeyReleaseMask, &event);

    // Flush event queue to transmit immediately
    XFlush(xdisplay);
}

// GdkDisplay *display = gdk_display_get_default();
// send_shift_insert_to_window(display, target_wid);
