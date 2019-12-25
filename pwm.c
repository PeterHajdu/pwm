#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

struct Win {
  struct Win* next;
  Window id;
};

struct Pwm {
  struct Win* windows;
  struct Win* currentWindow;
};

void addWindow(struct Pwm* pwm, Window id) {
  struct Win* newWin = malloc(sizeof(struct Win));
  newWin->id = id;
  newWin->next = pwm->windows;
  pwm->windows = newWin;
  if (!pwm->currentWindow) {
    pwm->currentWindow = pwm->windows;
  }
}

void deleteWindow(struct Pwm* pwm, Window id) {
}

struct Win* nextWindow(struct Pwm* pwm) {
  if (!pwm->currentWindow) {
    pwm->currentWindow = pwm->windows;
  } else if (pwm->currentWindow->next) {
    pwm->currentWindow = pwm->currentWindow->next;
  } else {
    pwm->currentWindow = pwm->windows;
  }

  return pwm->currentWindow;
}

int main() {
  struct Pwm pwm = {NULL, NULL};

  FILE* logFd = fopen("/tmp/asdf", "w");
  fprintf(logFd, "starting\n");
  fflush(logFd);

  Display * display = XOpenDisplay(0x0);
  if (!display) return 1;
  Window root = DefaultRootWindow(display);
  fprintf(logFd, "root window: %lu\n", root);
  fflush(logFd);

  Window temp;
  int xtemp, ytemp;
  unsigned int width, height;
  unsigned int t1, t2;
  XGetGeometry(display, root, &temp, &xtemp, &ytemp, &width, &height, &t1, &t2);

  XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("q")), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("1")), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);

  XSelectInput(display, root, SubstructureNotifyMask);

  while(1) {
    XEvent event;
    XNextEvent(display, &event);
    fprintf(logFd, "log: %d\n", event.type);
    fflush(logFd);
    if (DestroyNotify == event.type) {
      fprintf(logFd, "destroyed window: %lu\n", event.xdestroywindow.window);
      fflush(logFd);
      deleteWindow(&pwm, event.xdestroywindow.window);
    } else if (CirculateNotify == event.type) {
      fprintf(logFd, "circulate notify: %lu\n", event.xcirculate.window);
      fflush(logFd);
    } else if (ConfigureNotify == event.type) {
      fprintf(logFd, "configure notify: %lu\n", event.xconfigure.window);
      fflush(logFd);
    } else if (GravityNotify== event.type) {
      fprintf(logFd, "gravity notify: %lu\n", event.xgravity.window);
      fflush(logFd);
    } else if (MapNotify == event.type) {
      fprintf(logFd, "map notify: %d, %lu\n", event.xmap.override_redirect, event.xmap.window);
      fflush(logFd);
      if (!event.xmap.override_redirect) {
        Window newId = event.xmap.window;
        addWindow(&pwm, newId);
        XMoveResizeWindow(display, newId, 0, 0, width, height);
      }
    } else if (ReparentNotify == event.type) {
      fprintf(logFd, "reparent notif: %lu\n", event.xreparent.window);
      fflush(logFd);
    } else if (UnmapNotify == event.type) {
      fprintf(logFd, "unmap notif: %lu\n", event.xcirculate.window);
      fflush(logFd);
      deleteWindow(&pwm, event.xunmap.window);
    } else if (CreateNotify == event.type) {
      fprintf(logFd, "new window: %d, %lu, %lu\n",event.xcreatewindow.override_redirect, event.xcreatewindow.window, event.xcreatewindow.parent);
      fflush(logFd);
    } else if (KeyPress == event.type ) {
      if (event.xkey.keycode == XKeysymToKeycode(display, XStringToKeysym("q"))) break;
      if (event.xkey.keycode == XKeysymToKeycode(display, XStringToKeysym("1"))) {
        struct Win* win = nextWindow(&pwm);
        if (win) XRaiseWindow(display, win->id);
      }
    }
    XSync(display, False);
  }

  fprintf(logFd, "closing\n");
  fclose(logFd);
  XCloseDisplay(display);
  return 0;
}
