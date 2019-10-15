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

static int MOUSE_X = 0;
static int MOUSE_Y = 0;

static int CLICK_X = 0;
static int CLICK_Y = 0;

static void draw();

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

  XSelectInput(display, window, ExposureMask|KeyPressMask|PointerMotionMask|ButtonPressMask);
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

      int width = WINDOW_WIDTH = wa.width;
      int height = WINDOW_HEIGHT = wa.height;

      draw();

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
      MOUSE_X = xevent.xmotion.x;
      MOUSE_Y = xevent.xmotion.y;
      draw();
    }

    if (xevent.type == ButtonPress) {

      if (xevent.xbutton.button == Button1) {

        char buf[128] = {'\0'};

        int x = CLICK_X = xevent.xbutton.x;
        int y = CLICK_Y = xevent.xbutton.y;

        draw();

      }

    }

  }

  XDestroyWindow(display, window);
  XCloseDisplay(display);

  return 0;
}

static void draw()
{
  XClearWindow(display, window);

  char buf[128] = {'\0'};

  sprintf(buf, "Current window size: %dx%d", WINDOW_WIDTH, WINDOW_HEIGHT);
  XDrawString(display, window, DefaultGC(display, screen), 10, 20, buf, strlen(buf));

  sprintf(buf, "Mouse clicked at: %dx%d", CLICK_X, CLICK_Y);
  XDrawString(display, window, DefaultGC(display, screen), 10, 40, buf, strlen(buf));

  sprintf(buf, "Current pointer location: %dx%d", MOUSE_X, MOUSE_Y);
  XDrawString(display, window, DefaultGC(display, screen), 10, 60, buf, strlen(buf));
  XDrawArc(display, window, DefaultGC(display, screen), MOUSE_X-25, MOUSE_Y-25, 50, 50, 0, 360*64);

  /* circle */
  /*XDrawArc(display, window, DefaultGC(display, screen), WINDOW_WIDTH/2-50, WINDOW_HEIGHT/2-50, 100, 100, 0, 360*64);*/

  /* rectangle */
  /*XDrawRectangle(display, window, DefaultGC(display, screen), WINDOW_WIDTH/2-50, WINDOW_HEIGHT/2-50, 100, 100);*/

}
