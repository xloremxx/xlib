#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <sys/time.h>

static Display *display;
static Window window;
static int screen;
static Window root;
static XEvent xevent;
static int is_open = 1;

static int WINDOW_WIDTH = 800;
static int WINDOW_HEIGHT = 600;

static Window child_window;
static int CHILD_WINDOW_WIDTH = 400;
static int CHILD_WINDOW_HEIGHT = 400;
static XVisualInfo *vi;
static int att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
static int MOUSE_X = 0;
static int MOUSE_Y = 0;
static Colormap cmap;
static XSetWindowAttributes swa;
static GLXContext glc;
static XWindowAttributes gwa;

static int CLICK_X = 0;
static int CLICK_Y = 0;

static XColor red;

static void draw();

static Colormap colormap;

static Status rc;

static struct timeval stop, start;

static int child_window_expose();
static int loop();

static float time = 0.0f;
static float delta = 0.0f;
static float last_frame = 0.0f;

static float spin = 0.0f;

int main(int argc, char *argv[])
{

  display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "Can not open display.\n");
    return -1;
  }

  screen = DefaultScreen(display);

  root = RootWindow(display, screen);

  vi = glXChooseVisual(display, 0, att);

  if (vi == NULL) {
    fprintf(stderr, "No appropriate visual found!\n");
    return -1;
  }

  cmap = XCreateColormap(display, root, vi->visual, AllocNone);

  swa.colormap = cmap;
  swa.event_mask = ExposureMask|KeyPressMask;

  window = XCreateSimpleWindow(display, root, 10, 10, WINDOW_WIDTH, WINDOW_HEIGHT, 1, BlackPixel(display, screen), WhitePixel(display, screen));

  /*child_window = XCreateSimpleWindow(display, window, 10, 70, CHILD_WINDOW_WIDTH, CHILD_WINDOW_HEIGHT, 1, BlackPixel(display, screen), WhitePixel(display, screen));*/
  child_window = XCreateWindow(display, window, 10, 70, CHILD_WINDOW_WIDTH, CHILD_WINDOW_HEIGHT, 1, vi->depth, InputOutput, vi->visual, CWColormap|CWEventMask, &swa);

  colormap = DefaultColormap(display, screen);

  rc = XAllocNamedColor(display, colormap, "red", &red, &red);

  if (rc == 0) {
    fprintf(stderr, "Was unable to allocate red color.\n");
    return -1;
  }

  XSelectInput(display, window, ExposureMask|KeyPressMask|PointerMotionMask|ButtonPressMask);
  XSelectInput(display, child_window, ExposureMask|KeyPressMask|PointerMotionMask|ButtonPressMask);

  XMapWindow(display, window);
  XMapWindow(display, child_window);

  glc = glXCreateContext(display, vi, NULL, GL_TRUE);
  glXMakeCurrent(display, child_window, glc);

  glEnable(GL_DEPTH_TEST);

  Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);

  gettimeofday(&start, NULL);

  while (is_open)
  {

    while (XPending(display)) {
      XNextEvent(display, &xevent);

      if (xevent.xany.window == child_window) {

        if (xevent.type == Expose) {

          child_window_expose();

        }

      }

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

    loop();

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

  sprintf(buf, "Time: %.2f", time);
  XDrawString(display, window, DefaultGC(display, screen), 10 + CHILD_WINDOW_WIDTH + 10, 80, buf, strlen(buf));

  sprintf(buf, "Delta: %.2f", delta);
  XDrawString(display, window, DefaultGC(display, screen), 10 + CHILD_WINDOW_WIDTH + 10, 100, buf, strlen(buf));

  /* circle */
  /*XDrawArc(display, window, DefaultGC(display, screen), WINDOW_WIDTH/2-50, WINDOW_HEIGHT/2-50, 100, 100, 0, 360*64);*/

  /* rectangle */
  /*XDrawRectangle(display, window, DefaultGC(display, screen), WINDOW_WIDTH/2-50, WINDOW_HEIGHT/2-50, 100, 100);*/

  /*XSetForeground(display, DefaultGC(display, screen), red.pixel);                                              */
  /*XFillRectangle(display, window, DefaultGC(display, screen), WINDOW_WIDTH/2-50, WINDOW_HEIGHT/2-50, 100, 100);*/
  /*XSetForeground(display, DefaultGC(display, screen), BlackPixel(display, screen));                            */

}

static int child_window_expose()
{

  XGetWindowAttributes(display, child_window, &gwa);
  glViewport(0, 0, gwa.width, gwa.height);

  /*XFillRectangle(display, child_window, DefaultGC(display, screen), CHILD_WINDOW_WIDTH/2-50, CHILD_WINDOW_HEIGHT/2-50, 100, 100);*/

}

static int loop()
{

  /* time */
  gettimeofday(&stop, NULL);
  time = (float)(stop.tv_sec - start.tv_sec) * 1000 + (float) (stop.tv_usec - start.tv_usec) / 1000;

  /* delta */
  float current_frame = time;
  delta = current_frame - last_frame;
  last_frame = current_frame;

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glColor3f(1.0f, 1.0f, 1.0f);
  /*glOrtho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);*/
  glOrtho(-50.0f, 50.0f, -50.0f, 50.0f, -1.0f, 1.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glPushMatrix();
  glRotatef(spin, 0.0f, 0.0f, 1.0f);
  glBegin(GL_POLYGON);
  glVertex3f(-25.0f, -25.0f, 0.0f);
  glVertex3f(+25.0f, -25.0f, 0.0f);
  glVertex3f(+25.0f, +25.0f, 0.0f);
  glVertex3f(-25.0f, +25.0f, 0.0f);
  glEnd();
  glPopMatrix();

  glXSwapBuffers(display, child_window);

  draw();

  spin += delta * 0.1;

}
