/*
 * Copyright (C) 2004-2012 Jonathan Koren <jonathan@jonathankoren.com>
 * set_pixmap_property() copyright (C) 1998 Michael Jennings <mej@eterm.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\
 */

/* imlibsetroot
 * 
 * Sticks a pixmap(s) on the root window.
 * 
 * 
 * usage: imlibsetroot [globalopts] {[screenopts] [imageopts] <filename>}+
 * 
 * global options
 * 	--help		print usage information
 * 	--version	print version information
 * 	--composite	combine the images with the current root window
 * 	--display DDD	connect to X display DDD
 * 	--store		store the new root image to ~/.background.png
 * 	--store FILE	store the new root image to FILE
 * 	--storeslices	store the root image of each xinerama display
 * 			  as a seperate file.  (Implies --store .)
 * 
 * screen options
 * 	-x s		operate on the single virtual xinerama screen
 * 			  (default)
 * 	-x e		operate on all xinerama screens independently
 * 	-x N		operate on xinerama screen N
 * 
 * image options
 * 	-s		scale pixmap to the current screen's size
 * 			  preserving the pixmap's original aspect ratio
 * 	-s a		scale pixmap to the current screen's size
 * 			  preserving the pixmap's original aspect ratio
 * 	-s f		scale pixmap to the current screen's size
 * 			  without regard to the pixmap's original
 *	-s w		scale pixmap to the current screen's width
 * 			  preserving the pixmap's original aspect ratio
 *	-s h		scale pixmap to the current screen's height
 * 			  preserving the pixmap's original aspect ratio
 * 			  aspect ratio
 * 	-s N		scale pixmap by N times
 * 	-s X,Y		set the pixmap's width to X pixels and height
 * 			  to Y pixels
 * 	-m {xy}		mirror across the X/Y axis
 * 	-r {r|rr|l|ll}	rotate right/left once or twice
 * 	-f {xy}		flip the pixmap across the X/Y axis
 * 	-p c		center pixmap on both axes
 * 	-p cx		center pixmap on x
 * 	-p cy		center pixmap on y
 * 	-p X,Y		place pixmap at position X, Y on the current
 * 			  screen (current screen based coordinates)
 * 	-t		tile pixmap (integer tiling if combined with center)
 * 	-t W,H		tile pixmap W tiles wide, H tiles high
 * 	--bg COLOR	set background color (the area not covered by the
 *  		  image) to COLOR. COLOR may be a mnemonic, or a
 * 			  RRR,GGG,BBB triple, or "#RRGGBB" hex string.
 *  		  May be used without an image.
 * 	--bgcolor COLOR	Same as --bg
 */

/* About This Program
 *
 * `imlibsetroot` is pretty much `Esetroot`, but MUCH more feature rich.
 * This program was originally designed to set backgrounds on individual
 * monitors in a multihead setup.  Then composition was added so that it could
 * be used with `webcollage`, but `webcollage` has its own program that does
 * pretty much the same thing, so that was pretty much a waste of effort.
 * Alas, that's what I get for checking how `webcollage` actually  worked
 * after composition.
 */

/* Created:  2004 Jonathan Koren
 * Modified: Tue Feb  7 14:49:52 CST 2006		Jonathan Koren
 * 		Added negative coordinates
 *           Tue Mar 30 06:07:20 PDT 2010		Jonathan Koren
 * 		Incorporated bug fix from Kyle Rose <krose@krose.org> so that
 *                the aspect ratio preserving scaling to letterbox according 
 *                to the aspect ratio of the underlying display rather than 
 *                cropping pixmaps that are wider than they are tall, but not
 *                as oblong as the display.
 *              Changed exit codes for easier scripting.  0 now only when we
 *                set the background.
 *           Thu Apr  1 11:38:08 PDT 2010		Jonathan Koren
 *              Added --storeslices for multihead support on MacOSX.
 *              Added --bgcolor .
 *          Fri Oct 29 10:21:54 PDT 2010	 	Jonathan Koren
 * 		        Fixed bug when tiling on only one screen on multihead.
 *          Wed Nov 10 18:25:48 PST 2010		Jonathan Koren
 *  		    Added tiling dimensions.
 *          Fri Oct 19 16:54:46 PDT 2012		Jonathan Koren
 *  		    Enforce cropping to monitor size.
 *          Wed Dec 19 13:45:10 PST 2012		Jonathan Koren
 *  		    Enforce cropping to monitor size even when tiling.
 *  		    Changed -T to -t .
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Imlib2.h>
#include <X11/extensions/Xinerama.h>
#include "misc.h"


#define SHORT2BYTE(x) ((x) / 257)

#define X_AXIS 0x01
#define Y_AXIS 0x02

/* global options */
char *DISPLAY_STRING = NULL;
int COMPOSITE = 0;
int STORE_PIXMAP = 0;
char *STORE_PIXMAP_FILENAME = NULL;
int STORE_PIXMAP_SLICES = 0;


#define XINERAMA_SPAN -1
#define XINERAMA_EACH -2
typedef struct
{
    int x;
    int y;
    int height;
    int width;
    
    /* background color */
    int red;
    int blue;
    int green;
} screen_t;
screen_t  XINERAMA_VIRTUAL_SCREEN;
int NUM_XINERAMA_SCREENS;
screen_t *XINERAMA_SCREENS;


#define SCALE_NONE              0
#define SCALE_FIT_ASPECT        1
#define SCALE_FIT_SCREEN        2
#define SCALE_ABSOLUTE          3
#define SCALE_FACTOR            4
#define SCALE_FIT_HEIGHT_ASPECT 5
#define SCALE_FIT_WIDTH_ASPECT  6

typedef struct
{
    char *filename;
    Imlib_Image image;
    int screen;					/* which xinerama screen */

    
    int scaling_mode;
    int width;					/* used with SCALE_ABSOLUTE */
    int height;					/* used with SCALE_ABSOLUTE */
    float scaling_factor;			/* used with SCALE_FACTOR */

    int mirror;
    int orientation;				/* 0 = none
                                                 * 1 = clockwise 90
                                                 * 2 = clockwise 180
                                                 * 3 = clockwise 270
                                                 */
    int flip;
    int center;					/* which axes to center on */
    int x;
    int y;
    int tile;
    int tile_w;
    int tile_h;
} image_t;
int NUM_IMAGES;
image_t *IMAGES;




/* X Server Information */
Display   *XDISPLAY;
int        XSCREEN;
Screen    *XSCR;
Window     XROOT;
int        XDEPTH;
Colormap   XCOLORMAP;
Visual    *XVISUAL;





void set_pixmap_property(Pixmap p);
void process_args(int argc, char **argv);
void print_usage();
void init_image(image_t *i);
Pixmap create_background();
void fill_background();
void store_background();
void scale_image(int image_index, int xinerama_screen,
                 int *width, int *height);
void position_image(int image_index, int xinerama_screen,
                    int width, int height, int *x, int *y);
void tile_image(int image_index, int xinerama_screen, int width, int height);
void flip_and_rotate_image(int image_index);
void mirror_image(int image_index);
void crop_image_to_screen(int image_index, int xinerama_screen);
Pixmap get_background();
Window get_desktop_window();

/*****************************************************************************/

/* get_desktop_window() finds the window that draws the desktop.
 *
 * FIXME:  This isn't a complete search.  It only checks the immediate
 * children of the root window.
 */

Window get_desktop_window()
{
    Atom prop_desktop;
    Window desktop_window = None;
    
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;

    Window root;
    unsigned int nchildren = 0;
    Window *children;
    Window parent;

    prop_desktop = XInternAtom(XDISPLAY, "_NET_WM_DESKTOP", True);
    if (prop_desktop)
    {
        /* Don't bother checking the root window.  We set the property here
         * anyway.
         */

        if (XQueryTree(XDISPLAY, XROOT,
                       &root, &parent, &children, &nchildren) == False)
        {	
            fprintf(stderr, "XQueryTree() failed!\n");
            desktop_window = None;
        }
        else
        {
            int i;
            for (i = 0; i < nchildren; i++)
            {
                if (XGetWindowProperty(XDISPLAY, children[i], prop_desktop,
                                       0L, 1L, False, AnyPropertyType,
                                       &type, &format, &length, &after,
                                       &data) == Success)
                {	
                    /* found the property */
                    desktop_window = children[i];
                    if (nchildren)
                        XFree(children);
                    break;
                }
            }
            
            if (desktop_window == None)
            {
                /* no child of XROOT has the property */
                fprintf(stderr, "no child of XROOT has property\n");
                if (nchildren)
                    XFree(children);
                desktop_window = None;
            }
        }
    }
    else
        fprintf(stderr, "couldn't find desktop window property\n");

    return desktop_window;
}

/*****************************************************************************/

/*
 * set_pixmap_property() was swipped wholesale from Esetroot.c (part of 
 * Eterm, the terminal for the Enlightment project (enlightenment.org)).
 * Esetroot was written by Nat Friedman <ndf@mit.edu>.  The license of this
 * code is unclear, but Eterm itself is licensed under the GPL, so this 
 * is most likely covered as well.
 *
 * We're SUPPOSED to find the parent window that set _XROOTPMAP_ID, but
 * I'm lazy so I just check the root window.
 */

void set_pixmap_property(Pixmap p)
{
    Atom prop_root, prop_esetroot, type;
    int format;
    unsigned long length, after;
    unsigned char *data_root, *data_esetroot;

    /* First check to see if the properties already exist, and if they are
     * equal
     */
    prop_root = XInternAtom(XDISPLAY, "_XROOTPMAP_ID", True);
    prop_esetroot = XInternAtom(XDISPLAY, "ESETROOT_PMAP_ID", True);

    if ((prop_root != None) && (prop_esetroot != None))
    {
        XGetWindowProperty(XDISPLAY, XROOT, prop_root, 0L, 1L,
                           False, AnyPropertyType, &type, &format,
                           &length, &after, &data_root);
        if (type == XA_PIXMAP)
        {
            XGetWindowProperty(XDISPLAY, XROOT, prop_esetroot, 0L, 1L,
                               False, AnyPropertyType, &type, &format,
                               &length, &after, &data_esetroot);
            if (data_root && data_esetroot)
            {
                if ((type == XA_PIXMAP) &&
                    (*((Pixmap *) data_root) == *((Pixmap *) data_esetroot)))
                {
                    XKillClient(XDISPLAY, *((Pixmap *) data_root));
                }
            }
        }
    }

    /* This will locate the property, creating it if it doesn't exist */
    prop_root = XInternAtom(XDISPLAY, "_XROOTPMAP_ID", False);
    prop_esetroot = XInternAtom(XDISPLAY, "ESETROOT_PMAP_ID", False);

    /* The call above should have created it.  If that failed, we can't
     * continue.
     */
    if (prop_root == None || prop_esetroot == None)
    {
        PANIC(1, "create of pixmap property failed")
    }
    XChangeProperty(XDISPLAY, XROOT, prop_root, XA_PIXMAP, 32,
                    PropModeReplace, (unsigned char *) &p, 1);
    XChangeProperty(XDISPLAY, XROOT, prop_esetroot, XA_PIXMAP, 32,
                    PropModeReplace, (unsigned char *) &p, 1);

    XSetCloseDownMode(XDISPLAY, RetainPermanent);
    XFlush(XDISPLAY);

    return;
}


/*****************************************************************************/

/* print_usage() writes usage information to STDOUT.  Takes nothing.  Returns
 * nothing.
 */

void print_usage()
{
    printf(
"usage: imlibsetroot [globalopts] {[screenopts] [imageopts] <filename>}+\n"\
"\n"\
"global options\n"\
"	--help		print usage information\n"\
"	--version	print version information\n"\
"	--composite	combine the images with the current root window\n"\
"	--display DDD	connect to X display DDD\n"\
"	--store		store the new root image to ~/.background.png\n"\
"	--store FILE	store the new root image to FILE\n"\
"	--storeslices	store the root image of each xinerama display\n"\
"			  as a seperate file.  (Implies --store .)\n"\
"\n"\
"screen options\n"\
"	-x s		operate on the single virtual xinerama screen\n"\
"			  (default)\n"\
"	-x e		operate on all xinerama screens independently\n"\
"	-x N		operate on xinerama screen N\n"\
"\n"\
"image options\n"\
"	-s		scale pixmap to the current screen's size\n"\
"			  preserving the pixmap's original aspect ratio\n"\
"	-s a		scale pixmap to the current screen's size\n"\
" 			  preserving the pixmap's original aspect ratio\n"\
"	-s w		scale pixmap to the current screen's width\n"\
" 			  preserving the pixmap's original aspect ratio\n"\
"	-s h		scale pixmap to the current screen's height\n"\
" 			  preserving the pixmap's original aspect ratio\n"\
"	-s f		scale pixmap to the current screen's size\n"\
"			  without regard to the pixmap's original\n"\
"			  aspect ratio\n"\
"	-s N		scale pixmap by N times\n"\
"	-s X,Y		set the pixmap's width to X pixels and height\n"\
"			  to Y pixels\n"\
"	-m {xy}		mirror across the X/Y axis\n"\
"	-r {r|rr|l|ll}	rotate right/left once or twice\n"\
"	-f {xy}		flip the pixmap across the X/Y axis\n"\
"	-p c		center pixmap on both axes\n"\
"	-p cx		center pixmap on x\n"\
"	-p cy		center pixmap on y\n"\
"	-p X,Y		place pixmap at position X, Y on the current\n"\
"			  screen (current screen based coordinates)\n"\
"	-t		tiling.  (integer tiling if combined with center\n"\
"	-t W,H          tile an explicit W wide and H high\n"\
"	--bg COLOR	set background color (the area not covered by the\n"\
"            image) to COLOR. COLOR may be a mnemonic, or a\n"\
"            RRR,GGG,BBB triple, or \"#RRGGBB\" hex string.\n"\
"            May be used without an image.\n"\
"	--bgcolor COLOR	same as --bg\n");

    return;
}

/*****************************************************************************/

/* print_version() writes version information to STDOUT.  Takes nothing.
 * Returns nothing.
 */

void print_version()
{
    printf(
"imlibsetroot version 1.6\n"\
"Copyright (C) 2004-2012, Jonathan Koren <jonathan@jonathankoren.com>.\n"\
"Portions copyright (C) 1998 Michael Jennings <mej@eterm.org>\n");

    return;
}

/*****************************************************************************/

/* process_args() processes the command line arguments, and loading the globals
 * appropriately.  
 */

void process_args(int argc, char **argv)
{
    int cur_screen = XINERAMA_SPAN;
    int i;

    if (argc < 2)
    {
	    print_usage();
	    exit(3);
    }

    /* handle global options */
    for (i = 1; i < argc; i++)
    {
        if      (strcmp(argv[i], "--composite") == 0)
            COMPOSITE = 1;
        else if (strcmp(argv[i], "--display") == 0)
        {
            DISPLAY_STRING = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "--store") == 0)
        {
            STORE_PIXMAP = 1;
            if (argv[i + 1][0] != '-')
            {
                STRDUP(STORE_PIXMAP_FILENAME, argv[i + 1]);
                i++;
            }
            else
            {
                MALLOC(STORE_PIXMAP_FILENAME,
                       sizeof(char) * (strlen(getenv("HOME")) +
                                       strlen("/.background.png") + 1));
                STORE_PIXMAP_FILENAME[0] = '\0';
                strcat(STORE_PIXMAP_FILENAME, getenv("HOME"));
                strcat(STORE_PIXMAP_FILENAME, "/.background.png");
            }
        }
        else if (strcmp(argv[i], "--storeslices") == 0) 
        {
            STORE_PIXMAP_SLICES = 1;
            if (!STORE_PIXMAP) 
            {
                /* implies --store */
                STORE_PIXMAP = 1;
                MALLOC(STORE_PIXMAP_FILENAME,
                       sizeof(char) * (strlen(getenv("HOME")) +
                                       strlen("/.background.png") + 1));
                STORE_PIXMAP_FILENAME[0] = '\0';
                strcat(STORE_PIXMAP_FILENAME, getenv("HOME"));
                strcat(STORE_PIXMAP_FILENAME, "/.background.png");
            }
            
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            print_usage();
            exit(3);
        }
        else if (strcmp(argv[i], "--version") == 0)
        {
            print_version();
            exit(3);
        }
        else
            break;
    }

    
    /* image options and filename*/
    NUM_IMAGES = 1;
    MALLOC(IMAGES, sizeof(image_t));
    init_image(&(IMAGES[0]));

    for (; i < argc; i++)
    {
        if     (strcmp(argv[i], "-x") == 0)
        {
            if      (strcmp(argv[i + 1], "s") == 0)
            {
                cur_screen = IMAGES[NUM_IMAGES - 1].screen = XINERAMA_SPAN;
                i++;
            }
            else if (strcmp(argv[i + 1], "e") == 0)
            {
                cur_screen = IMAGES[NUM_IMAGES - 1].screen = XINERAMA_EACH;
                i++;
            }
            else if (isdigit(argv[i + 1][0]))
            {
                int xinerama_screen = atoi(argv[i + 1]);
                
                if (!((NUM_XINERAMA_SCREENS) &&
                      (xinerama_screen >= 0) &&
                      (xinerama_screen < NUM_XINERAMA_SCREENS)))
                {
                    fprintf(stderr, "No xinerama screen %d not found.  Defaulting to the virtual screen.\n", xinerama_screen);
                    xinerama_screen = XINERAMA_SPAN;
                }

                cur_screen = IMAGES[NUM_IMAGES - 1].screen = xinerama_screen;
                i++;
            }
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            if      (strcmp(argv[i + 1], "f") == 0)
            {
                /* scale image to screen size, regardless of aspect ratio */
                IMAGES[NUM_IMAGES - 1].scaling_mode = SCALE_FIT_SCREEN;
                i++;
            }
            else if (strcmp(argv[i + 1], "a") == 0)
            {
                /* scale to current screen's size, but preserve aspect ratio */
                IMAGES[NUM_IMAGES - 1].scaling_mode = SCALE_FIT_ASPECT;
                i++;
            }


            else if (strcmp(argv[i + 1], "h") == 0)
            {
                /* scale to current screen's height, but preserve aspect ratio */
                IMAGES[NUM_IMAGES - 1].scaling_mode = SCALE_FIT_HEIGHT_ASPECT;
                i++;
            }
            else if (strcmp(argv[i + 1], "w") == 0)
            {
                /* scale to current screen's width, but preserve aspect ratio */
                IMAGES[NUM_IMAGES - 1].scaling_mode = SCALE_FIT_WIDTH_ASPECT;
                i++;
            }


            else if ((isdigit(argv[i + 1][0])) || (argv[i + 1][0] == '.'))
            {
                /* set width and height to X,Y */

                char *loc = strchr(argv[i + 1], ',');
                if (loc)
                {
                    char *tuple = NULL;
                    char *tok;
                    int width = -1;
                    int height = -1;

                    STRDUP(tuple, argv[i + 1]);
                    if (tok = strtok(tuple, ","))
                        width = atoi(tok);
                    if (tok = strtok(NULL, ","))
                        height = atoi(tok);
                    FREE(tuple);

                    if ((height >= 0) && (width >= 0))
                    {
                        IMAGES[NUM_IMAGES - 1].scaling_mode = SCALE_ABSOLUTE;
                        IMAGES[NUM_IMAGES - 1].height = height;
                        IMAGES[NUM_IMAGES - 1].width  = width;
                    }
                    else
                    {
                        fprintf(stderr, "can't set image size to %d, %d\n",
                                height, width);
                        exit(1);
                    }
                }
                else
                {
                    /* scale image by N */
                    IMAGES[NUM_IMAGES - 1].scaling_mode = SCALE_FACTOR;
                    IMAGES[NUM_IMAGES - 1].scaling_factor = atof(argv[i + 1]);
                }
                i++;
            }
            else
            {
                /* scale to current screen's size, but preserve aspect ratio */
                IMAGES[NUM_IMAGES - 1].scaling_mode = SCALE_FIT_ASPECT;
            }
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            if     (isdigit(argv[i + 1][0]) || 
                    (argv[i + 1][0] == '-')  || (argv[i + 1][0] == '+'))
            {
                /* set position to X,Y */

                char *tuple = NULL;
                char *tok;
                int x = -1;
                int y = -1;
                
                STRDUP(tuple, argv[i + 1]);
                if (tok = strtok(tuple, ","))
                    x = atoi(tok);
                if (tok = strtok(NULL, ","))
                    y = atoi(tok);
                FREE(tuple);

                IMAGES[NUM_IMAGES - 1].center = 0;
                IMAGES[NUM_IMAGES - 1].x = x;
                IMAGES[NUM_IMAGES - 1].y = y;
            }
            else if (strcmp(argv[i + 1], "c") == 0)
            {
                IMAGES[NUM_IMAGES - 1].center = X_AXIS | Y_AXIS;
            }
            else if (strcmp(argv[i + 1], "cx") == 0)
            {
                IMAGES[NUM_IMAGES - 1].center |= X_AXIS;
            }           
            else if (strcmp(argv[i + 1], "cy") == 0)
            {
                IMAGES[NUM_IMAGES - 1].center |= Y_AXIS;
            }
            else
            {
                fprintf(stderr, "can't place image at location %s\n",
                        argv[i + 1]);
                exit(1);
            }
            i++;
        }
        else if (strcmp(argv[i], "-m") == 0)
        {
            if      (strcmp(argv[i + 1], "x") == 0)
            {
                IMAGES[NUM_IMAGES - 1].mirror |= X_AXIS;
            }
            else if (strcmp(argv[i + 1], "y") == 0)
            {
                IMAGES[NUM_IMAGES - 1].mirror |= Y_AXIS;
            }
            else
            {
                fprintf(stderr, "can't mirror across the %s axis!\n",
                        argv[i + 1]);
                exit(1);
            }
            i++;
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            if      ((strcmp(argv[i + 1], "r") == 0) ||
                     (strcmp(argv[i + 1], "lll") == 0))
                IMAGES[NUM_IMAGES - 1].orientation = 1;
            else if ((strcmp(argv[i + 1], "l") == 0) ||
                     (strcmp(argv[i + 1], "rrr") == 0))
                IMAGES[NUM_IMAGES - 1].orientation = 3;
            else if ((strcmp(argv[i + 1], "rr") == 0) ||
                     (strcmp(argv[i + 1], "ll") == 0))
                IMAGES[NUM_IMAGES - 1].orientation = 2;
            else if ((strcmp(argv[i + 1], "rrrr") == 0) ||
                     (strcmp(argv[i + 1], "llll") == 0))
                IMAGES[NUM_IMAGES - 1].orientation = 0;
            else
            {
                fprintf(stderr, "don't understand rotation %s\n",
                        argv[i + 1]);
                exit(1);
            }
            i++;
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            if      (strcmp(argv[i + 1], "x") == 0)
                IMAGES[NUM_IMAGES - 1].flip |= X_AXIS;
            else if (strcmp(argv[i + 1], "y") == 0)
                IMAGES[NUM_IMAGES - 1].flip |= Y_AXIS;
            else
            {
                fprintf(stderr, "can't flip across the %s axis!\n",
                        argv[i + 1]);
                exit(1);
            }
            i++;
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            IMAGES[NUM_IMAGES - 1].tile = 1;

            if (isdigit(argv[i + 1][0]))
            {
                /* set tiling W,H */

                char *loc = strchr(argv[i + 1], ',');
                if (loc)
                {
                    char *tuple = NULL;
                    char *tok;
                    int w = -1;
                    int h = -1;
                
                    STRDUP(tuple, argv[i + 1]);
                    if (tok = strtok(tuple, ","))
                        w = atoi(tok);
                    if (tok = strtok(NULL, ","))
                        h = atoi(tok);
                    FREE(tuple);

                   IMAGES[NUM_IMAGES - 1].tile_w = w;
                   IMAGES[NUM_IMAGES - 1].tile_h = h;   
               }
               i++;
           }
        }
        else if ((strcmp(argv[i], "--bg") == 0) ||
                 (strcmp(argv[i], "--bgcolor") == 0))
        {
            if (argv[i + 1][0] == '#') 
            {
                /* hex rgb #rrggbb */
                int red = -1;
                int green = -1;
                int blue = -1;
                char *tok = NULL;


                CALLOC(tok, 3, sizeof(char));
                strncpy(tok, &(argv[i + 1][1]), 2);
                red = strtol(tok, NULL, 16);
                FREE(tok);

                CALLOC(tok, 3, sizeof(char));
                strncpy(tok, &(argv[i + 1][3]), 2);
                blue = strtol(tok, NULL, 16);
                FREE(tok);

                CALLOC(tok, 3, sizeof(char));
                strncpy(tok, &(argv[i + 1][5]), 2);
                green = strtol(tok, NULL, 16);
                FREE(tok);
                
                if ((red >= 0)   && (green >= 0)   && (blue >= 0) &&
                    (red <= 255) && (green <= 255) && (blue <= 255))
                {   
                    /* set current screen's background */
                    if (cur_screen == XINERAMA_SPAN)
                    {
                        XINERAMA_VIRTUAL_SCREEN.red   = red;
                        XINERAMA_VIRTUAL_SCREEN.green = green;
                        XINERAMA_VIRTUAL_SCREEN.blue  = blue;
                    }   
                    else if (cur_screen == XINERAMA_EACH)
                    {
                        int j;
                        
                        for (j = 0; j < NUM_XINERAMA_SCREENS; j++)
                        {
                            XINERAMA_SCREENS[j].red   = red;
                            XINERAMA_SCREENS[j].green = green;
                            XINERAMA_SCREENS[j].blue  = blue;
                        }   
                    }   
                    else
                    {
                        XINERAMA_SCREENS[cur_screen].red   = red;
                        XINERAMA_SCREENS[cur_screen].green = green;
                        XINERAMA_SCREENS[cur_screen].blue  = blue;
                    }   
                }   
                else
                {
                    fprintf(stderr, "invalid background color = %s\n",
                            argv[i + 1]);
                    exit(1);
                }   

            }
            else if (isdigit(argv[i + 1][0]))
            {
                /* decimal RGB triple rrr,ggg,bbb */
                int red = -1;
                int green = -1;
                int blue = -1;
                char *tuple = NULL;
                char *tok;
                
                STRDUP(tuple, argv[i + 1]);
                if (tok = strtok(tuple, ","))
                    red = atoi(tok);
                if (tok = strtok(NULL, ","))
                    green = atoi(tok);
                if (tok = strtok(NULL, ","))
                    blue = atoi(tok);
                FREE(tuple);

                if ((red >= 0)   && (green >= 0)   && (blue >= 0) &&
                    (red <= 255) && (green <= 255) && (blue <= 255))
                {
                    /* set current screen's background */
                    if (cur_screen == XINERAMA_SPAN)
                    {
                        XINERAMA_VIRTUAL_SCREEN.red   = red;
                        XINERAMA_VIRTUAL_SCREEN.green = green;
                        XINERAMA_VIRTUAL_SCREEN.blue  = blue;
                    }
                    else if (cur_screen == XINERAMA_EACH)
                    {
                        int j;

                        for (j = 0; j < NUM_XINERAMA_SCREENS; j++)
                        {
                            XINERAMA_SCREENS[j].red   = red;
                            XINERAMA_SCREENS[j].green = green;
                            XINERAMA_SCREENS[j].blue  = blue;
                        }
                    }
                    else
                    {
                        XINERAMA_SCREENS[cur_screen].red   = red;
                        XINERAMA_SCREENS[cur_screen].green = green;
                        XINERAMA_SCREENS[cur_screen].blue  = blue;
                    }
                }
                else
                {
                    fprintf(stderr, "invalid background color = %s\n",
                            argv[i + 1]);
                    exit(1);
                }
            }
            else
            {
                /* xcolor mnemonic */
                XColor xcolor;

                /* check mnemonic */
                if (XParseColor(XDISPLAY, XCOLORMAP, argv[i + 1], &xcolor))
                {
                    if (cur_screen == XINERAMA_SPAN)
                    {
                        int j;

                        XINERAMA_VIRTUAL_SCREEN.red   =
                          SHORT2BYTE(xcolor.red);
                        XINERAMA_VIRTUAL_SCREEN.green =
                          SHORT2BYTE(xcolor.green);
                        XINERAMA_VIRTUAL_SCREEN.blue  =
                          SHORT2BYTE(xcolor.blue);
                    }
                    else if (cur_screen == XINERAMA_EACH)
                    {
                        int j;

                        for (j = 0; j < NUM_XINERAMA_SCREENS; j++)
                        {
                            XINERAMA_SCREENS[j].red   =
                              SHORT2BYTE(xcolor.red);
                            XINERAMA_SCREENS[j].green =
                              SHORT2BYTE(xcolor.green);
                            XINERAMA_SCREENS[j].blue  =
                              SHORT2BYTE(xcolor.blue);
                        }
                    }
                    else
                    {
                        XINERAMA_SCREENS[cur_screen].red   =
                          SHORT2BYTE(xcolor.red);
                        XINERAMA_SCREENS[cur_screen].green =
                          SHORT2BYTE(xcolor.green);
                        XINERAMA_SCREENS[cur_screen].blue  =
                          SHORT2BYTE(xcolor.blue);
                    }
                }
                else
                {
                    fprintf(stderr, "unknown color %s\n", argv[i + 1]);
                    exit(1);
                }
            }
            i++;
        }
        else
        {
            /* filename */
            if (!(IMAGES[NUM_IMAGES - 1].image = imlib_load_image(argv[i])))
            {
                fprintf(stderr, "can't load image %s\n", argv[i]);
                exit(1);
            } 
            IMAGES[NUM_IMAGES - 1].filename = argv[i];

            /* ready the next image */
            NUM_IMAGES++;
            REALLOC(IMAGES, NUM_IMAGES * sizeof(image_t));
            init_image(&(IMAGES[NUM_IMAGES - 1]));
        }
    }


    /* remove the last image_t, since it was preallocated and now unused */
    NUM_IMAGES--;
    if (NUM_IMAGES)
    {
        REALLOC(IMAGES, NUM_IMAGES * sizeof(image_t));
    }
    else
    {
        FREE(IMAGES);
    }
    
    return;
}

/*****************************************************************************/

/* init_image() initializes an image_t */

void init_image(image_t *i)
{
    i->filename = "";
    i->screen = XINERAMA_SPAN;
    i->scaling_mode = SCALE_NONE;
    i->mirror = 0;
    i->orientation = 0;
    i->flip = 0;
    i->center = 0;
    i->x = 0;
    i->y = 0;
    i->tile = 0;
    i->tile_w = 0;
    i->tile_h = 0;

    return;
}

/*****************************************************************************/

/* create_background() creates the actual pixmap to be displayed on the
 * root window.
 */

Pixmap create_background()
{
    Pixmap background_pixmap;

    int image_index;
    int xinerama_screen;


    /* initialize imlib */
    imlib_set_cache_size(2048 * 1024);
    imlib_set_color_usage(128);
    imlib_context_set_dither(1);

    imlib_context_set_display(XDISPLAY);
    imlib_context_set_visual(XVISUAL);
    imlib_context_set_colormap(XCOLORMAP);

    if (COMPOSITE)
        background_pixmap = get_background();
    else
        background_pixmap = XCreatePixmap(XDISPLAY, XROOT,
                                          XINERAMA_VIRTUAL_SCREEN.width,
                                          XINERAMA_VIRTUAL_SCREEN.height,
                                          XDEPTH);
    imlib_context_set_drawable(background_pixmap);

    if (!COMPOSITE)
        fill_background();
    else
        fprintf(stderr, "background filling disabled during compositing\n");
    

    /* draw images */
    for (image_index = 0; image_index < NUM_IMAGES; image_index++)
    {
        int x, y;
        int width, height;
        
        imlib_context_set_image(IMAGES[image_index].image);

        flip_and_rotate_image(image_index);
        mirror_image(image_index);

        if ((IMAGES[image_index].screen != XINERAMA_SPAN) &&
            (NUM_XINERAMA_SCREENS > 0))
        {
            for (xinerama_screen = 0;
                 xinerama_screen < NUM_XINERAMA_SCREENS;
                 xinerama_screen++)
            {
                if ((IMAGES[image_index].screen == xinerama_screen) ||
                    (IMAGES[image_index].screen == XINERAMA_EACH))
                {
                    scale_image(image_index, xinerama_screen, &width, &height);
                    position_image(image_index, xinerama_screen, width, height,
                                   &x, &y);

                    if  (IMAGES[image_index].tile)
                        tile_image(image_index, xinerama_screen,
                                   width, height);
                    else
                    {
                        crop_image_to_screen(image_index, xinerama_screen);
                        imlib_render_image_on_drawable_at_size(x, y,
                                                               width, height);
                    }
                }
            }
        }
        else /* XINERAMA_SPAN */
        {
            scale_image(image_index, -1, &width, &height);
            position_image(image_index, -1, width, height, &x, &y);

            if (IMAGES[image_index].tile)
                tile_image(image_index, -1, width, height);
            else
            {
                imlib_render_image_on_drawable_at_size(x, y, width, height);
            }
        }
    }

    
    if (STORE_PIXMAP)
        store_background();

    return background_pixmap;
}

/*****************************************************************************/
/* crop_image_to_screen() crops the specified image to the dimensions of the
 * screen that it will be displayed on this.  This prevents bleeding across
 * monitors.
 */

void crop_image_to_screen(int image_index, int xinerama_screen)
{
    int x = IMAGES[image_index].x;
    int y = IMAGES[image_index].y;
    int image_width  = IMAGES[image_index].width;
    int image_height = IMAGES[image_index].height;
    int screen_width  = XINERAMA_SCREENS[xinerama_screen].width;
    int screen_height = XINERAMA_SCREENS[xinerama_screen].height;

    if (((x + image_width)  > screen_width) ||
        ((y + image_height) > screen_height))
    {
fprintf(stderr, "cropping image %d to %d by %d\n", 
        image_index, 
        XINERAMA_SCREENS[xinerama_screen].width,
        XINERAMA_SCREENS[xinerama_screen].height);

        Imlib_Image cropped = 
          imlib_create_cropped_scaled_image(x,
                                            y,
                                            image_width,
                                            image_height,
                                            x + image_width  - screen_width,
                                            x + image_height - screen_height);
        IMAGES[image_index].image = cropped;
        imlib_context_set_image(cropped);
    }
}

/*****************************************************************************/

/* fill_background() fills the background with a solid color.  Takes nothing.
 * Returns nothing.  This function assumes the imlib drawable context is
 * set to the background pixmap.
 */

void fill_background()
{
    Imlib_Image background_color_image;

    background_color_image =
      imlib_create_image(XINERAMA_VIRTUAL_SCREEN.width,
                         XINERAMA_VIRTUAL_SCREEN.height);
    imlib_context_set_image(background_color_image);

    /* paint the complete screen */
    imlib_context_set_color(XINERAMA_VIRTUAL_SCREEN.red,
                            XINERAMA_VIRTUAL_SCREEN.green,
                            XINERAMA_VIRTUAL_SCREEN.blue,
                            255);
    imlib_image_fill_rectangle(XINERAMA_VIRTUAL_SCREEN.x, 
                               XINERAMA_VIRTUAL_SCREEN.y,
                               XINERAMA_VIRTUAL_SCREEN.width,
                               XINERAMA_VIRTUAL_SCREEN.height);
    
    /* paint the individual screens */
    if (NUM_XINERAMA_SCREENS)
    {
        int i;

        for (i = 0; i < NUM_XINERAMA_SCREENS; i++)
        {
            if (XINERAMA_SCREENS[i].red >= 0)
            {
                imlib_context_set_color(XINERAMA_SCREENS[i].red,
                                        XINERAMA_SCREENS[i].green,
                                        XINERAMA_SCREENS[i].blue,
                                        255);
                imlib_image_fill_rectangle(XINERAMA_SCREENS[i].x, 
                                           XINERAMA_SCREENS[i].y,
                                           XINERAMA_SCREENS[i].width,
                                           XINERAMA_SCREENS[i].height);
            }
        }
    }

    /* render the whole background */
    imlib_render_image_on_drawable_at_size(XINERAMA_VIRTUAL_SCREEN.x,
                                           XINERAMA_VIRTUAL_SCREEN.y,
                                           XINERAMA_VIRTUAL_SCREEN.width,
                                           XINERAMA_VIRTUAL_SCREEN.height);

    imlib_free_image();

    return;
}


/*****************************************************************************/

/* store_background() writes the background image to the disk.  Takes nothing.
 * Returns nothing.  This function assumes the imlib drawable context is
 * set to the background pixmap.
 */

void store_background()
{
    Imlib_Image complete_image;
    Imlib_Image slice;
    char *fileExtension;
    char *completeSliceFilename;
    char *baseFilename;
    int xinescreen;

    /* get filetype */
    fileExtension = strrchr(STORE_PIXMAP_FILENAME, '.');
    if (fileExtension)
    {
        fileExtension++;
        imlib_image_set_format(fileExtension);
    }
    else
        imlib_image_set_format("png");


    complete_image =
      imlib_create_image_from_drawable(0,
                                       0, 0,
                                       XINERAMA_VIRTUAL_SCREEN.width,
                                       XINERAMA_VIRTUAL_SCREEN.height,
                                       0);

    if (!STORE_PIXMAP_SLICES) 
    {
        /* store complete image */
        imlib_context_set_image(complete_image);
        imlib_save_image(STORE_PIXMAP_FILENAME);
        imlib_free_image();
    } 
    else
    {
        /* build names of slices */
        CALLOC(completeSliceFilename, 
               strlen(STORE_PIXMAP_FILENAME) * 2,
               sizeof(char));

        /* Avoid strndup() since it's GNU only */
        CALLOC(baseFilename, 
               fileExtension - STORE_PIXMAP_FILENAME,
               sizeof(char));
        strncpy(baseFilename,
                STORE_PIXMAP_FILENAME,
                fileExtension - STORE_PIXMAP_FILENAME - 1);
        completeSliceFilename[0] = '\0';

        for (xinescreen = 0; xinescreen < NUM_XINERAMA_SCREENS; xinescreen++)
        {
            imlib_context_set_image(complete_image);
            slice =
              imlib_create_cropped_image(XINERAMA_SCREENS[xinescreen].x,
                                         XINERAMA_SCREENS[xinescreen].y,
                                         XINERAMA_SCREENS[xinescreen].width,
                                         XINERAMA_SCREENS[xinescreen].height);
            if (slice) 
            {
                sprintf(completeSliceFilename,
                        "%s%d.%s",
                        baseFilename, 
                        xinescreen,
                        fileExtension);

                imlib_context_set_image(slice);
                imlib_save_image(completeSliceFilename);
                imlib_free_image();
            }
            else
            {
                fprintf(stderr, "WARNING: Couldn't store slice for xinerama screen %d located at %d, %d with dimensions %d, %d\n",
                        xinescreen,
                        XINERAMA_SCREENS[xinescreen].x,
                        XINERAMA_SCREENS[xinescreen].y,
                        XINERAMA_SCREENS[xinescreen].width,
                        XINERAMA_SCREENS[xinescreen].height);
            }
        }

        FREE(completeSliceFilename);
        FREE(baseFilename);
    }
    
    return;
}

/*****************************************************************************/

/* scale_image() scales the image for the proper screen.  The image's new
 * width and height are returned via width and height pointers.
 */

void scale_image(int image_index, int xinerama_screen, int *width, int *height)
{
    int image_width  = imlib_image_get_width();
    int image_height = imlib_image_get_height();

    if      (IMAGES[image_index].scaling_mode == SCALE_FIT_ASPECT)
    {
        float wh_ratio = (float)image_width / (float)image_height;

        if ((IMAGES[image_index].screen == XINERAMA_SPAN) ||
            (NUM_XINERAMA_SCREENS == 0))
        {
            float span_wh_ratio = 
              (float)XINERAMA_VIRTUAL_SCREEN.width /
              (float)XINERAMA_VIRTUAL_SCREEN.height;

            if (wh_ratio > span_wh_ratio)
            {
                *width  = XINERAMA_VIRTUAL_SCREEN.width;
                *height = *width / wh_ratio;
            }
            else
            {
                *height = XINERAMA_VIRTUAL_SCREEN.height;
                *width = *height * wh_ratio;
            }
        }
        else /* XINERAMA_EACH or individual screen */
        {
	    float screen_wh_ratio =
              (float)XINERAMA_SCREENS[xinerama_screen].width /
              (float)XINERAMA_SCREENS[xinerama_screen].height;

            if (wh_ratio > screen_wh_ratio)
            {
                /* expand along the x axis */
                *width  = XINERAMA_SCREENS[xinerama_screen].width;
                *height = *width / wh_ratio;
            }
            else
            {
                /* expand along the y axis */
                *height = XINERAMA_SCREENS[xinerama_screen].height;
                *width = *height * wh_ratio;
            }
        }
    }
    else if (IMAGES[image_index].scaling_mode == SCALE_FIT_HEIGHT_ASPECT)
    {
        float wh_ratio = (float)image_width / (float)image_height;

        if ((IMAGES[image_index].screen == XINERAMA_SPAN) ||
            (NUM_XINERAMA_SCREENS == 0))
        {
            *height = XINERAMA_VIRTUAL_SCREEN.height;
            *width  = *height * wh_ratio;
        }
        else /* XINERAMA_EACH or individual screen */
        {
            *height = XINERAMA_SCREENS[xinerama_screen].height;
            *width = *height * wh_ratio;
        }
    }
    else if (IMAGES[image_index].scaling_mode == SCALE_FIT_WIDTH_ASPECT)
    {
        float wh_ratio = (float)image_width / (float)image_height;

        if ((IMAGES[image_index].screen == XINERAMA_SPAN) ||
            (NUM_XINERAMA_SCREENS == 0))
        {
            *width = XINERAMA_VIRTUAL_SCREEN.width;
            *height = *width / wh_ratio;
        }
        else /* XINERAMA_EACH or individual screen */
        {
            *width = XINERAMA_SCREENS[xinerama_screen].width;
            *height = *width / wh_ratio;
        }
    }
    else if (IMAGES[image_index].scaling_mode == SCALE_FIT_SCREEN)
    {
        if ((IMAGES[image_index].screen == XINERAMA_SPAN) ||
            (NUM_XINERAMA_SCREENS == 0))
        {
            *width  = XINERAMA_VIRTUAL_SCREEN.width;
            *height = XINERAMA_VIRTUAL_SCREEN.height;
        }
        else  /* XINERAMA_EACH  or individual screen */
        {
            *width  = XINERAMA_SCREENS[xinerama_screen].width;
            *height = XINERAMA_SCREENS[xinerama_screen].height;
        }
    }
    else if (IMAGES[image_index].scaling_mode == SCALE_ABSOLUTE)
    {
        *width  = IMAGES[image_index].width;
        *height = IMAGES[image_index].height;
    }
    else if (IMAGES[image_index].scaling_mode == SCALE_FACTOR)
    {
        *width  = image_width  * IMAGES[image_index].scaling_factor;
        *height = image_height * IMAGES[image_index].scaling_factor;
    }
    else /* SCALE_NONE */
    {
        *width  = image_width;
        *height = image_height;
    }

    return;
}

/*****************************************************************************/

/* position_image() positions the image on the proper screen.  Position
 * is returned via the x and y pointers.
 */

void position_image(int image_index, int xinerama_screen,
                    int width, int height, int *x, int *y)
{
    *x = 0;
    *y = 0;
    
    /* find the X-axis position */
    if (IMAGES[image_index].center & X_AXIS)
    {
        if ((IMAGES[image_index].screen == XINERAMA_SPAN) ||
            (NUM_XINERAMA_SCREENS == 0))
            *x = (XINERAMA_VIRTUAL_SCREEN.width / 2) - (width / 2);
        else /* XINERAMA_EACH or individual screen */
            *x = XINERAMA_SCREENS[xinerama_screen].x +
                 (XINERAMA_SCREENS[xinerama_screen].width / 2) - (width / 2);
    }
    else
    {
        if ((IMAGES[image_index].screen == XINERAMA_SPAN) ||
            (NUM_XINERAMA_SCREENS == 0))
            *x = IMAGES[image_index].x;
        else /* XINERAMA_EACH or individual screen */
            *x = XINERAMA_SCREENS[xinerama_screen].x + IMAGES[image_index].x;
    }

    /* find the Y-axis position */
    if (IMAGES[image_index].center & Y_AXIS)
    {
        if ((IMAGES[image_index].screen == XINERAMA_SPAN) ||
            (NUM_XINERAMA_SCREENS == 0))
            *y = (XINERAMA_VIRTUAL_SCREEN.height / 2) - (height / 2);
        else /* XINERAMA_EACH or individual screen */
            *y = XINERAMA_SCREENS[xinerama_screen].y +
                 (XINERAMA_SCREENS[xinerama_screen].height / 2) - (height / 2);
    }
    else
    {
        if ((IMAGES[image_index].screen == XINERAMA_SPAN) ||
            (NUM_XINERAMA_SCREENS == 0))
            *y = IMAGES[image_index].y;
        else /* XINERAMA_EACH or individual screen */
            *y = XINERAMA_SCREENS[xinerama_screen].y + IMAGES[image_index].y;
    }

    return;
}

/*****************************************************************************/

void tile_image(int image_index, int xinerama_screen, int width, int height)
{
    int start_x, start_y;		/* where to start tiling */
    int num_x, num_y;			/* number of tiles on each axis */
    int i, j;

    if ((IMAGES[image_index].screen == XINERAMA_SPAN) ||
        (NUM_XINERAMA_SCREENS == 0))
    {
        if ((IMAGES[image_index].center & X_AXIS) || 
            (IMAGES[image_index].tile_w > 0))
        {
            /* integer tile along the x axis */
            if (IMAGES[image_index].tile_w > 0)
                num_x = IMAGES[image_index].tile_w;
            else
                num_x = XINERAMA_VIRTUAL_SCREEN.width / width;
            start_x = (XINERAMA_VIRTUAL_SCREEN.width / 2) -
                        ((width * num_x) / 2);
        }
        else
        {
            /* fractional tile along the x axis */
            num_x   = (XINERAMA_VIRTUAL_SCREEN.width / width) + 1;
            start_x = IMAGES[image_index].x;
        }

        if ((IMAGES[image_index].center & Y_AXIS) ||
            (IMAGES[image_index].tile_h > 0))
        {
            /* integer tile along the y axis */            
            if (IMAGES[image_index].tile_h > 0)
                num_y = IMAGES[image_index].tile_h;
            else
                num_y = XINERAMA_VIRTUAL_SCREEN.height / height;
            start_y = (XINERAMA_VIRTUAL_SCREEN.height / 2) -
                        ((height * num_y) / 2);
        }
        else
        {
            /* fractional tile along the y axis */
            num_y   = (XINERAMA_VIRTUAL_SCREEN.height / height) + 1;
            start_y = IMAGES[image_index].y;
        }
    }
    else /* XINERAMA_EACH or individual screen */
    {
        if ((IMAGES[image_index].center & X_AXIS) ||
            (IMAGES[image_index].tile_w > 0))
        {
            /* integer tile along the x axis */
            if (IMAGES[image_index].tile_w > 0)
                num_x = IMAGES[image_index].tile_w;
            else
                num_x = XINERAMA_SCREENS[xinerama_screen].width / width;
            start_x = (XINERAMA_SCREENS[xinerama_screen].width / 2) -
                        ((width * num_x) / 2) +
                        XINERAMA_SCREENS[xinerama_screen].x;
                       
        }
        else
        {
            /* fractional tile along the x axis */
            num_x = (XINERAMA_SCREENS[xinerama_screen].width / width);
           if ((XINERAMA_SCREENS[xinerama_screen].width % width) > 0)
               num_x++;
            start_x = IMAGES[image_index].x +
                        XINERAMA_SCREENS[xinerama_screen].x;
        }


        if ((IMAGES[image_index].center & Y_AXIS) ||
            (IMAGES[image_index].tile_h > 0))
        {
            /* integer tile along the y axis */            
            if (IMAGES[image_index].tile_h > 0)
                num_y = IMAGES[image_index].tile_h;
            else
                num_y = XINERAMA_SCREENS[xinerama_screen].height / height;
            start_y = (XINERAMA_SCREENS[xinerama_screen].height / 2) -
                        ((height * num_y) / 2) +
                        XINERAMA_SCREENS[xinerama_screen].y;
        }
        else
        {
            /* fractional tile along the y axis */
            num_y   = (XINERAMA_SCREENS[xinerama_screen].height / height);
           if ((XINERAMA_SCREENS[xinerama_screen].height % height) > 0)
               num_y++;
            start_y = IMAGES[image_index].y +
                        XINERAMA_SCREENS[xinerama_screen].y;
        }
    }

    /* tile the image */
    for (i = 0; i < num_x; i++) 
    {
        for (j = 0; j < num_y; j++)
        {
            int cropped = 0;
            int leftmost_pixel = start_x + (width * i);
            int topmost_pixel  = start_y + (height * j);
            int rightmost_pixel  = leftmost_pixel  + width;
            int bottommost_pixel = topmost_pixel + height;
            int cropped_width  = width;
            int cropped_height = height;
            if (xinerama_screen >= 0) 
            {
                if (rightmost_pixel >
                    (XINERAMA_SCREENS[xinerama_screen].x +
                     XINERAMA_SCREENS[xinerama_screen].width))
                {
                    cropped_width = 
                      width - (rightmost_pixel -
                               (XINERAMA_SCREENS[xinerama_screen].x +
                                XINERAMA_SCREENS[xinerama_screen].width));
                }
                if (bottommost_pixel >
                    (XINERAMA_SCREENS[xinerama_screen].y +
                     XINERAMA_SCREENS[xinerama_screen].height))
                {
                    cropped_height =
                      height - (bottommost_pixel -
                                (XINERAMA_SCREENS[xinerama_screen].y +
                                 XINERAMA_SCREENS[xinerama_screen].height));
                }


                if ((cropped_width != width) || (cropped_height != height))
                    cropped = 1;
                else
                    cropped = 0;

                if (cropped && (cropped_height > 0) && (cropped_width > 0))
                {
                    Imlib_Image croppedImage =
                        imlib_create_cropped_image(
                            0, 0,
                            cropped_width, cropped_height);
                    if (croppedImage != NULL)
                        imlib_context_set_image(croppedImage);
                    else
                        fprintf(stderr, "can't crop tile!\n");
                }
            }

            imlib_render_image_on_drawable_at_size(leftmost_pixel,
                                                   topmost_pixel,
                                                   cropped_width,
                                                   cropped_height);
            if (cropped)
               imlib_context_set_image(IMAGES[image_index].image);
        }
    }

    return;
}

/*****************************************************************************/

/* flip_and_rotate_image() rotates the image, then flips it.  "Flipping" is
 * different from "mirroring" in the sense that "mirroring" will create
 * multiple images one original, one flipped.  "Flipping" yields only
 * one image.
 */

void flip_and_rotate_image(int image_index)
{
    imlib_image_orientate(IMAGES[image_index].orientation);
    if (IMAGES[image_index].flip & Y_AXIS)
        imlib_image_flip_horizontal();
    if (IMAGES[image_index].flip & X_AXIS)
        imlib_image_flip_vertical();

    return;
}

/*****************************************************************************/

/* mirror_image() changes the image so that it contains a flipped copy
 * along each axis that is mirrored.
 */

void mirror_image(int image_index)
{    
    Imlib_Image new_image;
    Imlib_Image old_image;

    Pixmap new_pixmap;
    Pixmap old_pixmap;

    int image_width, image_height;
    int pixmap_width, pixmap_height;

    if (IMAGES[image_index].mirror)
    {
        image_width  = imlib_image_get_width();
        image_height = imlib_image_get_height();

        if (IMAGES[image_index].mirror & Y_AXIS)
            pixmap_width = image_width * 2;
        else
            pixmap_width = image_width;
        if (IMAGES[image_index].mirror & X_AXIS)
            pixmap_height = image_height * 2;
        else
            pixmap_height = image_height;
        
        
        
        new_pixmap = XCreatePixmap(XDISPLAY, XROOT,
                                   pixmap_width, pixmap_height, XDEPTH);

        
        /* push */
        old_pixmap = imlib_context_get_drawable();
        old_image  = imlib_context_get_image();
        
        
        /* set */
        imlib_context_set_drawable(new_pixmap);
     
        imlib_render_image_on_drawable(0, 0);

        if (IMAGES[image_index].mirror & Y_AXIS)
        {
            imlib_image_flip_horizontal();
            imlib_render_image_on_drawable(image_width, 0);
        }


        if (IMAGES[image_index].mirror & X_AXIS)
            if (IMAGES[image_index].mirror & Y_AXIS)
            {
                /* double mirror */

                imlib_image_flip_vertical();
                imlib_render_image_on_drawable(image_width, image_height);
                imlib_image_flip_horizontal();
                imlib_render_image_on_drawable(0, image_height);
            }
            else
            {
                /* single mirror */

                imlib_image_flip_vertical();
                imlib_render_image_on_drawable(0, image_height);
            }


      
        imlib_free_image();
        new_image = imlib_create_image_from_drawable(0,
                                                     0, 0,
                                                     pixmap_width,
                                                     pixmap_height,
                                                     0);
        imlib_context_set_image(new_image);

                
        /* pop */
        imlib_context_set_drawable(old_pixmap);
        //imlib_context_set_image(old_image);        
    }
    
    return;
}

/*****************************************************************************/

Pixmap get_background()
{
    Atom prop_root, prop_esetroot, type;
    int format;
    unsigned long length, after;
    unsigned char *data_root;

    GC gc;
    XGCValues gcvalue;

    Pixmap  background_pixmap = None;
    Pixmap p;


    prop_root = XInternAtom(XDISPLAY, "_XROOTPMAP_ID", True);

    if (prop_root != None)
    {
        XGetWindowProperty(XDISPLAY, XROOT, prop_root, 0L, 1L,
                           False, AnyPropertyType, &type, &format,
                           &length, &after, &data_root);
        if (type == XA_PIXMAP)
        {
            p = *((Pixmap *)data_root);

            background_pixmap = XCreatePixmap(XDISPLAY, XROOT,
                                              XINERAMA_VIRTUAL_SCREEN.width,
                                              XINERAMA_VIRTUAL_SCREEN.height,
                                              XDEPTH);
                    
            gcvalue.foreground = BlackPixelOfScreen(XSCR);
            gc = XCreateGC(XDISPLAY, p, GCForeground, &gcvalue);

            XCopyArea(XDISPLAY,  p, background_pixmap, gc,
                      0, 0,
                      XINERAMA_VIRTUAL_SCREEN.width,
                      XINERAMA_VIRTUAL_SCREEN.height,
                      0, 0);
            XFree(data_root);

        }
    }

    if (background_pixmap == None)
    {
        /* Can't composite */

        COMPOSITE = 0;
        background_pixmap = XCreatePixmap(XDISPLAY, XROOT,
                                          XINERAMA_VIRTUAL_SCREEN.width,
                                          XINERAMA_VIRTUAL_SCREEN.height,
                                          XDEPTH);
    }

    
    return background_pixmap;
}

/*****************************************************************************/

int main(int argc, char **argv, char **env)
{
    Pixmap pixmap;

    /* determine X environment */
    if (DISPLAY_STRING)
    {
        if (!(XDISPLAY = XOpenDisplay(DISPLAY_STRING)))
        {
            fprintf(stderr, "Couldn't open DISPLAY %s\n", DISPLAY_STRING);
            exit(2);
        }
    }
    else
        if (!((XDISPLAY = XOpenDisplay(getenv("DISPLAY"))) ||
              (XDISPLAY = XOpenDisplay(":0.0"))))
        {
            fprintf(stderr, "Couldn't open a DISPLAY\n");
            exit(2);
        }
    
    XSCREEN   = DefaultScreen(XDISPLAY);
    XROOT     = DefaultRootWindow(XDISPLAY);
    XSCR      = ScreenOfDisplay(XDISPLAY, XSCREEN);
    XCOLORMAP = DefaultColormap(XDISPLAY, XSCREEN);
    XVISUAL   = DefaultVisual(XDISPLAY, XSCREEN);
    XDEPTH    = DefaultDepth(XDISPLAY, XSCREEN);


    /* store screen dimensions */
    XINERAMA_VIRTUAL_SCREEN.x      = 0;
    XINERAMA_VIRTUAL_SCREEN.y      = 0;
    XINERAMA_VIRTUAL_SCREEN.height = XSCR->height;
    XINERAMA_VIRTUAL_SCREEN.width  = XSCR->width;
    XINERAMA_VIRTUAL_SCREEN.red   = 0;
    XINERAMA_VIRTUAL_SCREEN.green = 0;
    XINERAMA_VIRTUAL_SCREEN.blue  = 0;

    if (XineramaIsActive(XDISPLAY))
    {
        XineramaScreenInfo *xsi;
        int i;

        if (ScreenCount(XDISPLAY) > 1)
        {
            fprintf(stderr, "Ewww!!! Xinerama with multiple screens???  We'll try to use Xinerama anyway.\n");
        }

        xsi = XineramaQueryScreens(XDISPLAY, &NUM_XINERAMA_SCREENS);
        MALLOC(XINERAMA_SCREENS, sizeof(screen_t) * NUM_XINERAMA_SCREENS);
        for (i = 0; i < NUM_XINERAMA_SCREENS; i++)
        {
            XINERAMA_SCREENS[i].x      = xsi[i].x_org;
            XINERAMA_SCREENS[i].y      = xsi[i].y_org;
            XINERAMA_SCREENS[i].height = xsi[i].height;
            XINERAMA_SCREENS[i].width  = xsi[i].width;
            XINERAMA_SCREENS[i].red    = -1;
            XINERAMA_SCREENS[i].green  = -1;
            XINERAMA_SCREENS[i].blue   = -1;
        }
        XFree(xsi);
        XSync(XDISPLAY, False);
    }
    else
    {
        NUM_XINERAMA_SCREENS = 0;
        XINERAMA_SCREENS = NULL;
    }

    /* process arguments */
    process_args(argc, argv);

    if (pixmap = create_background())
    {
        set_pixmap_property(pixmap);
        XSetWindowBackgroundPixmap(XDISPLAY, XROOT, pixmap);
        XSetWindowBackgroundPixmap(XDISPLAY, get_desktop_window(), pixmap);
        XClearWindow(XDISPLAY, XROOT);
        XFlush(XDISPLAY); 
    }
    XCloseDisplay(XDISPLAY);
    
    return 0;
}

