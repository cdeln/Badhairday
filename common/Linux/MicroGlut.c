// MicroGlut is a stripped-down reimplementation of the classic GLUT/FreeGLUT library. By Ingemar Ragnemalm 2012-2015.
// This is the Linux version. There is also a Mac version (and drafts for a Windows version).

// Why do we use MicroGlut?
// 1) Transparency! Including it as a single source file makes it easy to see what it does.
// 2) Editability. Missing something? Want something to work a bit different? Just hack it in.
// 3) No obsolete stuff! The old GLUT has a lot of functions that are inherently obsolete. Avoid!

// Please note that this is still in an early stage. A lot of functionality is still missing.
// If you add/change something of interest, especially if it follows the GLUT API, please sumbit
// to me so I can consider adding that to the official version.

// 2012: Made a first draft by extracting context creation from other code.
// 130131: Fixed bugs in keyboard and update events. Seems to work pretty well! Things are missing, GL3 untested, but otherwise it runs stable with glutgears.
// 130206: Timers now tested and working. CPU load kept decent when not intentionally high.
// 130907: Bug fixes, test for non-existing gReshape, glutKeyboardUpFunc corrected.
// 130916: Fixed the event bug. Cleaned up most comments left from debugging.
// 130926: Cleaned up warnings, added missing #includes.
// 140130: A bit more cleanup for avoiding warnings (_BSD_SOURCE below).
// 140401: glutKeyIsDown and glutWarpPointer added and got an extra round of testing.
// 150205: glutRepeatingTimer new, better name for glutRepeatingTimerFunc
// Added a default glViewport call when the window is resized.
// Made a bug fix in the event processing so that mouse drag events won't stack up.
// Somewhere here I added a kind of full-screen support (but without removing window borders).
// 150216: Added proper OpenGL3 initalization. (Based on a patch by Sebastian Parborg.)
// 150223: Finally, decent handling on the GLUT configuration!
// 150227: Resize triggers an update!
// 150302: Window position, multisample, even better config
// 150618: Added glutMouseIsDown() (not in the old GLUT API but a nice extension!).
// Added #ifdefs to produce errors if compiled on the wrong platform!
// 150909: Added glutExit.
// 150924: Added support for special keys.
// 160302: Added glutShowCursor and glutHideCursor.
// 170405: Made some globals static.
// 170406: Added "const" to string arguments to make C++ happier. glutSpecialFunc and glutSpecialUpFunc are now officially supported - despite being deprecated. (I recommend that you use the same keyboard func for everything.) Added support for multiple mouse buttons (right and left).
// 170410: Modified glutWarpPointer to make it more robust. Commended out some unused variables to avoid warnings.

#define _BSD_SOURCE
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glext.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "MicroGlut.h"
#include <sys/time.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

// If this is compiled on the Mac or Windows, tell me!
#ifdef __APPLE__
	ERROR! This is NOT the Mac version of MicroGlut and will not work on the Mac!
#endif
#ifdef _WIN32
	ERROR! This is NOT the Windows version of MicroGlut and will not work on Windows!
#endif

static unsigned int winWidth = 300, winHeight = 300;
static unsigned int winPosX = 40, winPosY = 40;
//static int mode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH;
static int gContextVersionMajor = 0;
static int gContextVersionMinor = 0;

static Display *dpy;
static Window win;
static GLXContext ctx;
//static char *dpyName = NULL;
int gMode; // NOT YET USED
static char animate = 1; // Use for glutNeedsRedisplay?
struct timeval timeStart;
static Atom wmDeleteMessage; // To handle delete msg
static char gKeymap[256];
static char gRunning = 1;

void glutInit(int *argc, char *argv[])
{
	gettimeofday(&timeStart, NULL);
	memset(gKeymap, 0, sizeof(gKeymap));
}
void glutInitDisplayMode(unsigned int mode)
{
	gMode = mode; // NOT YET USED
}
void glutInitWindowSize(int w, int h)
{
	winWidth = w;
	winHeight = h;
}

void glutInitWindowPosition (int x, int y)
{
	winPosX = x;
	winPosY = y;
}

static void checktimers();

/*
 * Create an RGB, double-buffered window.
 * Return the window and context handles.
 */

static void
make_window( Display *dpy, const char *name,
             int x, int y, int width, int height,
             Window *winRet, GLXContext *ctxRet)
{
   int scrnum;
   XSetWindowAttributes attr;
   unsigned long mask;
   Window root;
   Window win;
   GLXContext ctx;
   XVisualInfo *visinfo;

   scrnum = DefaultScreen( dpy );
   root = RootWindow( dpy, scrnum );

// 3.2 support
//#ifdef glXCreateContextAttribsARB
	if (gContextVersionMajor > 2)
	{
// We asked for OpenGL3+, but can we do it?

		typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

		// Verify GL driver supports glXCreateContextAttribsARB()
		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;

		// Verify that GLX implementation supports the new context create call
		if ( strstr( glXQueryExtensionsString( dpy, scrnum ), 
			"GLX_ARB_create_context" ) != 0 )
		glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
			glXGetProcAddress( (const GLubyte *) "glXCreateContextAttribsARB" );

		if ( !glXCreateContextAttribsARB )
		{
			printf( "Can't create new-style GL context\n" );
		}

// We need this for OpenGL3
		int elemc;
		GLXFBConfig *fbcfg;

	   int attribs[] = { GLX_RENDER_TYPE, GLX_RGBA_BIT,
                     GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, // ?
                     GLX_RED_SIZE, 1, // 1 = prefer high precision
                     GLX_GREEN_SIZE, 1,
                     GLX_BLUE_SIZE, 1,
                     GLX_ALPHA_SIZE, 1,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None };

		int i = 12;
		if (gMode & GLUT_DOUBLE)
		{
			attribs[i++] = GLX_DOUBLEBUFFER;
			attribs[i++] = 1;
		}
		if (gMode & GLUT_DEPTH)
		{
			attribs[i++] = GLX_DEPTH_SIZE;
			attribs[i++] = 1;
		}
		if (gMode & GLUT_STENCIL)
		{
			attribs[i++] = GLX_STENCIL_SIZE;
			attribs[i++] = 8; // Smallest available, at least 8. Configurable setting needed!
		}
		if (gMode & GLUT_MULTISAMPLE)
		{
			attribs[i++] = GLX_SAMPLE_BUFFERS;
			attribs[i++] = 1;
			attribs[i++] = GLX_SAMPLES;
			attribs[i++] = 4;
		}

		fbcfg = glXChooseFBConfig(dpy, scrnum, attribs, &elemc);
		if (!fbcfg)
		{
			fbcfg = glXChooseFBConfig(dpy, scrnum, NULL, &elemc);
		}
		if (!fbcfg)
			printf("Couldn't get FB configs\n");

		int gl3attr[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, gContextVersionMajor,
			GLX_CONTEXT_MINOR_VERSION_ARB, gContextVersionMinor,
//			GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
			None
		};
		ctx = glXCreateContextAttribsARB(dpy, fbcfg[0], NULL, 1, gl3attr);
		if (ctx == NULL) printf("No ctx!\n");

        visinfo = glXGetVisualFromFBConfig(dpy, fbcfg[0]);
        if (!visinfo)
            printf("Error: couldn't create OpenGL window with this pixel format.\n");

	}
	else // old style
//#endif
	{
	   int attribs[] = { GLX_RGBA,
                     GLX_RED_SIZE, 1, // 1 = prefer high precision
                     GLX_GREEN_SIZE, 1,
                     GLX_BLUE_SIZE, 1,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None,
                     None };

		int i = 7;
		if (gMode & GLUT_DOUBLE)
			attribs[i++] = GLX_DOUBLEBUFFER;
		if (gMode & GLUT_DEPTH)
		{
			attribs[i++] = GLX_DEPTH_SIZE;
			attribs[i++] = 1;
		}
		if (gMode & GLUT_STENCIL)
		{
			attribs[i++] = GLX_STENCIL_SIZE;
			attribs[i++] = 8; // Smallest available, at least 8. Configurable setting needed!
		}
    	visinfo = glXChooseVisual( dpy, scrnum, attribs );
		if (!visinfo)
		{
			printf("Error: couldn't get a visual according to settings\n");
			exit(1);
		}

		ctx = glXCreateContext( dpy, visinfo, 0, True );
		if (ctx == NULL) printf("No ctx!\n");
	}

   /* window attributes */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap( dpy, root, visinfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPress | ButtonReleaseMask | Button1MotionMask | PointerMotionMask;
   attr.override_redirect = 0;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;

   win = XCreateWindow( dpy, root, x, y, width, height,
		        0, visinfo->depth, InputOutput,
		        visinfo->visual, mask, &attr );

// Register delete!
	wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDeleteMessage, 1); // Register

   /* set hints and properties */
      XSizeHints sizehints;
      sizehints.x = x;
      sizehints.y = y;
      sizehints.width  = width;
      sizehints.height = height;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(dpy, win, &sizehints);
      XSetStandardProperties(dpy, win, name, name,
                              None, (char **)NULL, 0, &sizehints);

   if (!ctx)
   {
      printf("Error: glXCreateContext failed\n");
      exit(1);
   }

   XFree(visinfo);

   *winRet = win;
   *ctxRet = ctx;
}

void glutCreateWindow(const char *windowTitle)
{
   dpy = XOpenDisplay(NULL);
   if (!dpy)
   {
      printf("Error: couldn't open display %s\n",
	     windowTitle ? windowTitle : getenv("DISPLAY"));
   }

   make_window(dpy, windowTitle, winPosX, winPosY, winWidth, winHeight, &win, &ctx);
   
   XMapWindow(dpy, win);
   glXMakeCurrent(dpy, win, ctx);
}

void (*gDisplay)(void);
void (*gReshape)(int width, int height);
void (*gIdle)(void);
void (*gKey)(unsigned char key, int x, int y);
void (*gKeyUp)(unsigned char key, int x, int y);
void (*gSpecialKey)(unsigned char key, int x, int y);
void (*gSpecialKeyUp)(unsigned char key, int x, int y);
void (*gMouseMoved)(int x, int y);
void (*gMouseDragged)(int x, int y);
void (*gMouseFunc)(int button, int state, int x, int y);
int gLastMousePositionX, gLastMousePositionY; // Avoids a problem with glutWarpPointer

// Maybe I should just drop these for simplicity
//void (*gSpecialKey)(unsigned char key, int x, int y) = NULL;
//void (*gSpecialKeyUp)(unsigned char key, int x, int y) = NULL;


void glutReshapeFunc(void (*func)(int width, int height))
{
	gReshape = func;
}
void glutDisplayFunc(void (*func)(void))
{
	gDisplay = func;
}
void glutIdleFunc(void (*func)(void))
{gIdle = func;}

void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y))
{
	gKey = func;
}
void glutKeyboardUpFunc(void (*func)(unsigned char key, int x, int y))
{
	gKeyUp = func;
}
void glutSpecialFunc(void (*func)(unsigned char key, int x, int y))
{
	gSpecialKey = func;
}

void glutSpecialUpFunc(void (*func)(unsigned char key, int x, int y))
{
	gSpecialKeyUp = func;
}

void glutMouseFunc(void (*func)(int button, int state, int x, int y))
{gMouseFunc = func;}
void glutMotionFunc(void (*func)(int x, int y))
{gMouseDragged = func;}
void glutPassiveMotionFunc(void (*func)(int x, int y))
{gMouseMoved = func;}

char gButtonPressed[10] = {0,0,0,0,0,0,0,0,0,0};

void doKeyboardEvent(XEvent event, void (*keyProc)(unsigned char key, int x, int y), void (*specialKeyProc)(unsigned char key, int x, int y), int keyMapValue)
{
	char buffer[10];
//	int r; // code;
	
	int code = ((XKeyEvent *)&event)->keycode;
	
//	r = 
	XLookupString(&event.xkey, buffer, sizeof(buffer), NULL, NULL);
	char raw = buffer[0]; // Before remapping
	switch(code)
	{
		case 111: buffer[0] = GLUT_KEY_UP; break;
		case 114: buffer[0] = GLUT_KEY_RIGHT; break;
		case 116: buffer[0] = GLUT_KEY_DOWN; break;
		case 113: buffer[0] = GLUT_KEY_LEFT; break;
		case 67: buffer[0] = GLUT_KEY_F1; break;
		case 68: buffer[0] = GLUT_KEY_F2; break;
		case 69: buffer[0] = GLUT_KEY_F3; break;
		case 70: buffer[0] = GLUT_KEY_F4; break;
		case 71: buffer[0] = GLUT_KEY_F5; break;
		case 72: buffer[0] = GLUT_KEY_F6; break;
		case 73: buffer[0] = GLUT_KEY_F7; break;
		case 112: buffer[0] = GLUT_KEY_PAGE_UP; break;
		case 117: buffer[0] = GLUT_KEY_PAGE_DOWN; break;
		case 110: buffer[0] = GLUT_KEY_HOME; break;
		case 115: buffer[0] = GLUT_KEY_END; break;
		case 118: buffer[0] = GLUT_KEY_INSERT; break;

		case 50: buffer[0] = GLUT_KEY_LEFT_SHIFT; break;
		case 62: buffer[0] = GLUT_KEY_RIGHT_SHIFT; break;
		case 37:case 105: buffer[0] = GLUT_KEY_CONTROL; break;
		case 64:case 108: buffer[0] = GLUT_KEY_ALT; break;
		case 133:case 134: buffer[0] = GLUT_KEY_COMMAND; break;

// Keypad
		case 90: buffer[0] = GLUT_KEY_INSERT; break;
		case 87: buffer[0] = GLUT_KEY_END; break;
		case 88: buffer[0] = GLUT_KEY_DOWN; break;
		case 89: buffer[0] = GLUT_KEY_PAGE_DOWN; break;
		case 83: buffer[0] = GLUT_KEY_LEFT; break;
//		case 84: buffer[0] = GLUT_KEY_KEYPAD_5; break;
		case 85: buffer[0] = GLUT_KEY_RIGHT; break;
		case 79: buffer[0] = GLUT_KEY_HOME; break;
		case 80: buffer[0] = GLUT_KEY_UP; break;
		case 81: buffer[0] = GLUT_KEY_PAGE_UP; break;
		case 82: buffer[0] = 127; break;
//		case 77: buffer[0] = GLUT_KEY_KEYPAD_NUMLOCK; break;
	}	
	
	// If we asked for a separate callback for special ketys, call it. Otherwise call the standard one.
	// I am considering removing the special callback for simplicity!
	if (raw == 0)
	{
		if (specialKeyProc)
			specialKeyProc(buffer[0], 0, 0);
		else
			if (keyProc)
				keyProc(buffer[0], 0, 0);
	}
	else
		if (keyProc)
			keyProc(buffer[0], 0, 0);
	gKeymap[(int)buffer[0]] = keyMapValue;
	
//	printf("%c %d %d %d\n", buffer[0], buffer[0], r, code);

//	      			if (event.type == KeyPress)
//		      		{	if (gKey) gKey(buffer[0], 0, 0); gKeymap[(int)buffer[0]] = 1;}
//		      		else
//		      		{	if (gKeyUp) gKeyUp(buffer[0], 0, 0); gKeymap[(int)buffer[0]] = 0;}
}

void glutMainLoop()
{
	char pressed = 0;
	int i;

	XAllowEvents(dpy, AsyncBoth, CurrentTime);

	while (gRunning)
	{
//      int op = 0;
      while (XPending(dpy) > 0)
      {
         XEvent event;
         XNextEvent(dpy, &event);

         switch (event.type)
         {
         	case ClientMessage:
         		if (event.xclient.data.l[0] == wmDeleteMessage) // quit!
         			gRunning = 0;
	         	break;
         	case Expose: 
//			op = 1; 
				break; // Update event! Should do draw here.
         	case ConfigureNotify:
				if (gReshape)
	      			gReshape(event.xconfigure.width, event.xconfigure.height);
				else
				{
					glViewport(0, 0, event.xconfigure.width, event.xconfigure.height);
				}
				animate = 1;
      			break;
      		case KeyPress:
				doKeyboardEvent(event, gKey, gSpecialKey, 1);break;
      		case KeyRelease:
				doKeyboardEvent(event, gKeyUp, gSpecialKeyUp, 0);break;
			case ButtonPress:
				gButtonPressed[event.xbutton.button] = 1;
				if (gMouseFunc != NULL)
				switch (event.xbutton.button)
				{
				case Button1:
					gMouseFunc(GLUT_LEFT_BUTTON, GLUT_DOWN, event.xbutton.x, event.xbutton.y);break;
				case Button2:
					gMouseFunc(GLUT_MIDDLE_BUTTON, GLUT_DOWN, event.xbutton.x, event.xbutton.y);break;
				case Button3:
					gMouseFunc(GLUT_RIGHT_BUTTON, GLUT_DOWN, event.xbutton.x, event.xbutton.y);break;
                case Button4:
                    gMouseFunc(GLUT_SCROLL, GLUT_UP, event.xbutton.x, event.xbutton.y);break;
                case Button5:
                    gMouseFunc(GLUT_SCROLL, GLUT_DOWN, event.xbutton.x, event.xbutton.y);break;
				}
				break;
			case ButtonRelease:
				gButtonPressed[event.xbutton.button] = 0;
				if (gMouseFunc != NULL)
				switch (event.xbutton.button)
				{
				case Button1:
					gMouseFunc(GLUT_LEFT_BUTTON, GLUT_UP, event.xbutton.x, event.xbutton.y);break;
				case Button2:
					gMouseFunc(GLUT_MIDDLE_BUTTON, GLUT_UP, event.xbutton.x, event.xbutton.y);break;
				case Button3:
					gMouseFunc(GLUT_RIGHT_BUTTON, GLUT_UP, event.xbutton.x, event.xbutton.y);break;
				}
				break;
		case MotionNotify:
				pressed = 0;
				for (i = 0; i < 5; i++)
					if (gButtonPressed[i]) pressed = 1;

				// Saving the last known position in order to avoid problems for glutWarpPointer
				// If we try warping to this position, don't!
				gLastMousePositionX = event.xbutton.x;
				gLastMousePositionY = event.xbutton.y;

				if (pressed && gMouseDragged)
					gMouseDragged(event.xbutton.x, event.xbutton.y);
				else
				if (gMouseMoved)
					gMouseMoved(event.xbutton.x, event.xbutton.y);
				break;

		default:
			break;
         }
      }
      
      if (animate)
      {
      	animate = 0;
		if (gDisplay)
		  	gDisplay();
		else
			printf("No display function!\n");
//      	op = 0;
      }
		else
		if (gIdle) gIdle();
      checktimers();
   }

	glXMakeCurrent(dpy, None, NULL);
   glXDestroyContext(dpy, ctx);
   XDestroyWindow(dpy, win);
   XCloseDisplay(dpy);
}

void glutSwapBuffers()
{
	glXSwapBuffers(dpy, win);
}

void glutPostRedisplay()
{
	animate = 1;
}

int glutGet(int type)
{
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	return (tv.tv_usec - timeStart.tv_usec) / 1000 + (tv.tv_sec - timeStart.tv_sec)*1000;
}

// NOTE: The timer is not designed with any multithreading in mind!
typedef struct TimerRec
{
	int arg;
	int time;
	int repeatTime;
	void (*func)(int arg);
	char repeating;
	struct TimerRec *next;
	struct TimerRec *prev;
} TimerRec;

TimerRec *gTimers = NULL;

void glutTimerFunc(int millis, void (*func)(int arg), int arg)
{
	TimerRec *t	= (TimerRec *)malloc(sizeof(TimerRec));
	t->arg = arg;
	t->time = millis + glutGet(GLUT_ELAPSED_TIME);
	t->repeatTime = 0;
	t->repeating = 0;
	t->func = func;
	t->next = gTimers;
	t->prev = NULL;
	if (gTimers != NULL)
		gTimers->prev = t;
	gTimers = t;
}

// Added by Ingemar
// void glutRepeatingTimerFunc(int millis)
void glutRepeatingTimer(int millis)
{
	TimerRec *t	= (TimerRec *)malloc(sizeof(TimerRec));
	t->arg = 0;
	t->time = millis + glutGet(GLUT_ELAPSED_TIME);
	t->repeatTime = millis;
	t->repeating = 1;
	t->func = NULL;
	t->next = gTimers;
	t->prev = NULL;
	if (gTimers != NULL)
		gTimers->prev = t;
	gTimers = t;
}

static void checktimers()
{
	if (gTimers != NULL)
	{
		TimerRec *t, *firethis = NULL;
		int now = glutGet(GLUT_ELAPSED_TIME);
		int nextTime = now + 1000; // Distant future, 1 second
		t = gTimers;
		for (t = gTimers; t != NULL; t = t->next)
		{
			if (t->time < nextTime) nextTime = t->time; // Time for the next one
			if (t->time < now) // See if this is due to fire
			{
				firethis = t;
			}
		}
		if (firethis != NULL)
		{
		// Fire the timer
			if (firethis->func != NULL)
				firethis->func(firethis->arg);
			else
				glutPostRedisplay();
		// Remove the timer if it was one-shot, otherwise update the time
			if (firethis->repeating)
			{
				firethis->time = now + firethis->repeatTime;
			}
			else
			{
				if (firethis->prev != NULL)
					firethis->prev->next = firethis->next;
				else
					gTimers = firethis->next;
                if (firethis->next != NULL)
					firethis->next->prev = firethis->prev;
				free(firethis);
			}
		}
		// Otherwise, sleep until any timer should fire
        if (!animate)
			if (nextTime > now)
            {
		usleep((nextTime - now)*1000);
            }
	}
    else
// If no timer and no update, sleep a little to keep CPU load low
        if (!animate)
            usleep(10);
}

void glutInitContextVersion(int major, int minor)
{
	gContextVersionMajor = major;
	gContextVersionMinor = minor;
}

// Based on FreeGlut glutWarpPointer, but with a significant improvement!
/*
 * Moves the mouse pointer to given window coordinates
 */
void glutWarpPointer( int x, int y )
{
    if (dpy == NULL)
    {
      fprintf(stderr, "glutWarpPointer failed: MicroGlut not initialized!\n");
    	return;
    }

	if (x == gLastMousePositionX && y == gLastMousePositionY)
		return; // Don't warp to where we already are - this causes event flooding!

    XWarpPointer(
        dpy, // fgDisplay.Display,
        None,
        win, // fgStructure.CurrentWindow->Window.Handle,
        0, 0, 0, 0,
        x, y
    );
    /* Make the warp visible immediately. */
    XFlush( dpy );
//    XFlush( fgDisplay.Display );
}

// Replaces glutSetMousePointer. This limits us to the two most common cases: None and arrow!
void glutShowCursor()
{
	XUndefineCursor(dpy, win);
}

void glutHideCursor()
{
	if (dpy == NULL) 
	{
	   printf("glutHideCursor failed: MicroGlut not initialized!\n");
   	return;
	}
	
	Cursor invisibleCursor;
	Pixmap bitmapNoData;
	static char noll[] = { 0,0,0};
	bitmapNoData = XCreateBitmapFromData(dpy, win, noll, 1, 1);
	invisibleCursor = XCreatePixmapCursor(dpy,bitmapNoData, bitmapNoData, 
	                                     (XColor *)noll, (XColor *)noll, 0, 0);
	XDefineCursor(dpy,win, invisibleCursor);
	XFreeCursor(dpy, invisibleCursor);
	XFreePixmap(dpy, bitmapNoData);
}



char glutKeyIsDown(unsigned char c)
{
	return gKeymap[(unsigned int)c];
}

// Added by the Risinger/R�berg/Wikstr�m project! But... gButtonPressed
// was already here! Did I miss something?
char glutMouseIsDown(unsigned char c)
{
	return gButtonPressed[(unsigned int)c];
}



// These were missing up to 150205

void glutReshapeWindow(int width, int height)
{
	XResizeWindow(dpy, win, width, height);
}
void glutPositionWindow(int x, int y)
{
	XMoveWindow(dpy, win, x, y);
}
void glutSetWindowTitle(char *title)
{
	XStoreName(dpy, win, title);
}

// Not complete full screen mode yet since the window frame and menu are not hidden yet

char gFullScreen = 0;
unsigned int savedHeight, savedWidth;
int savedX, savedY;

void glutFullScreen()
{
	gFullScreen = 1;


	Drawable d;
	unsigned int a, b;

	XGetGeometry(dpy, win, &d, &savedX, &savedY, &savedWidth, &savedHeight, &a, &b);

    int scrnum = DefaultScreen(dpy);
    int width = DisplayWidth( dpy, scrnum );
    int height = DisplayHeight( dpy, scrnum );

	XMoveResizeWindow(dpy, win, 0, 0, width, height);
}

void glutExitFullScreen()
{
	gFullScreen = 0;
	XMoveResizeWindow(dpy, win, savedX, savedY, savedWidth, savedHeight);
}

void glutToggleFullScreen()
{
	if (gFullScreen)
		glutExitFullScreen();
	else
		glutFullScreen();
}

void glutExit()
{
	gRunning = 0;
}

