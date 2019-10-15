#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>

static Display *display;
static Window window;
static int screen;
static Window root;
static XEvent xevent;
static int is_open = 1;

static int WINDOW_WIDTH = 800;
static int WINDOW_HEIGHT = 600;

int main(int argc, char *argv[])
{

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "Can not open display.\n");
    return -1;
  }

  screen = DefaultScreen(display);

  root = RootWindow(display, screen);

  window = XCreateSimpleWindow(display, root, 10, 10, WINDOW_WIDTH, WINDOW_HEIGHT, 1, BlackPixel(display, screen), WhitePixel(display, screen));

  XSelectInput(display, window, ExposureMask|KeyPressMask|PointerMotionMask);
  XMapWindow(display, window);

  Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);

  while (is_open)
  {

    XNextEvent(display, &xevent);

    if (xevent.type == Expose) {

      XWindowAttributes wa;
      char buf[128] = {'\0'};
      XGetWindowAttributes(display, window, &wa);

      int width = WINDOW_WIDTH= wa.width;
      int height = WINDOW_HEIGHT = wa.height;

      sprintf(buf, "Current window size: %dx%d", width, height);
      XDrawString(display, window, DefaultGC(display, screen), 10, 20, buf, strlen(buf));

      /* circle */
      /*XDrawArc(display, window, DefaultGC(display, screen), width/2-50, height/2-50, 100, 100, 0, 360*64);*/

      /* rectangle */
      /*XDrawRectangle(display, window, DefaultGC(display, screen), width/2-50, height/2-50, 100, 100);*/

    }

    if (xevent.type == KeyPress) {
      char buf[128] = {'\0'};
      KeySym keysym;
      int len = XLookupString(&xevent.xkey, buf, sizeof(buf), &keysym, NULL);
      if (keysym == XK_Escape) {
        is_open = 0;
      }
    }

    if (xevent.type == ClientMessage) {
      if (xevent.xclient.data.l[0] == WM_DELETE_WINDOW) {
        is_open = 0;
      }
    }

    if (xevent.type == MotionNotify) {
      char buf[128] = {'\0'};
      sprintf(buf, "Current pointer location: %dx%d", xevent.xmotion.x, xevent.xmotion.y);

      XClearWindow(display, window);
      XDrawString(display, window, DefaultGC(display, screen), 10, 20, buf, strlen(buf));
      XDrawArc(display, window, DefaultGC(display, screen), xevent.xmotion.x-25, xevent.xmotion.y-25, 50, 50, 0, 360*64);
    }

  }

  XDestroyWindow(display, window);
  XCloseDisplay(display);

  return 0;
}
