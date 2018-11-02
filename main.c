#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

static int xerror_handler(Display *dpy, XErrorEvent *e) {
  // Eh, who is going to look at this anyways. 
  return 0;
}

int main(void) {

  Display *dis;
  int screen;
  Window win;
  XEvent event;
  
  dis=XOpenDisplay((char *)0);
  screen=DefaultScreen(dis);
  win=DefaultRootWindow(dis);
  
  XSetErrorHandler(xerror_handler);
  XSelectInput(dis, win, SubstructureNotifyMask);
  
  while(1) { XNextEvent(dis, &event);

    if(event.type == MapNotify) { 
      
      XMapEvent *ev = (XMapEvent *) &event;

      // We only need look at windows who said they don't want the windows
      // manager to look at them. if(!ev->override_redirect) continue;

      // XFetchName seems to take some time to populate, and on my system I was
      // not seeing the property change event so this kludge will work for now.
      for( int x=0; x < 1000; x++) {

        char *name = NULL;
        XFetchName(dis, ev->window, &name);        

        if(name != NULL) {


          int fix = 0; 
          int map = 0;
          int manage = 0; 

          if(strcmp(name, "as_toolbar") == 0 ||
             strcmp(name, "zoom_linux_float_video_window") == 0 ||
             strcmp(name, "annotate_toolbar") == 0 ){
            fix = 1; 
            map = 1;
            manage = 1; 
          }

          if(strcmp(name, "AnnoLocalViewLinux") == 0 ||
             strcmp(name, "AnnoInputLinux") == 0){
           
            fix = 1; 
            map = 0; 
          }
          
          if(fix) { 

            printf("found name after %d tries: %ld:0x%08lx:%s:%d:%d:%d\n",
                   x, ev->window, ev->window, name, fix, map, manage);
            
            // Unmap the window. 
            XUnmapWindow(dis, ev->window);
            XFlush(dis);

            // Change set ovveride_redirect = 0; 
            if(manage){ 
              XSetWindowAttributes wattr;
              long mask = CWOverrideRedirect;
              wattr.override_redirect = 0;
              XChangeWindowAttributes(dis, ev->window, mask, &wattr);
              XFlush(dis);
            }

            // Map if needed. 
            if(map){
              XMapWindow(dis, ev->window);
              XFlush(dis);
            }
          }

          // Free our memory!
          XFree(name);
          name = NULL;
          
          break;
        }
                
        usleep(100); 
      }

    }
  }
}
