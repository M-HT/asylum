/*  keyboard.c */

/*  Copyright Hugh Robinson 2006-2008.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "asylum.h"

extern asylum_options options;

static char keyboard[512];
static int keybuf;
static int unibuf;
static int mouse;
static int exposed;

#define ESC_VALUE 27

#if defined(GP2X)

#if (SDL_MAJOR_VERSION > 1 || SDL_MAJOR_VERSION == 1 && (SDL_MINOR_VERSION > 2 || SDL_MINOR_VERSION == 2 && SDL_PATCHLEVEL >= 9 ) )
    #include <SDL/SDL_gp2x.h>
#endif

#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)

static int gp2x_keyboard[] = {
    SDLK_UP, 0, SDLK_LEFT, 0, SDLK_DOWN, 0, SDLK_RIGHT, 0,
    SDLK_SPACE, SDLK_ESCAPE, SDLK_l, SDLK_r, SDLK_a, SDLK_b, SDLK_x, SDLK_y,
    0, 0, 0
};

static char gp2x_keys[8];
static int keybuf2;
#endif

void init_keyboard()
{
    for (int i = 0; i < 512; i++) keyboard[i] = 0;

#if defined(GP2X)
    for (int i = 0; i < 8; i++) gp2x_keys[i] = 0;

    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickOpen(0);
#if defined(SDL_GP2X__H)
    if (SDL_GP2X_MouseType() == GP2X_MOUSE_TOUCHSCREEN)
    {
        SDL_GP2X_TouchpadMouseMotionEvents(0);
        SDL_GP2X_TouchpadMouseButtonEvents(0);
    }
#endif
#endif
}

void swi_removecursors()
{
    ;
}
int swi_readescapestate()
{
    return keyboard[ESC_VALUE];
}
int readmousestate()
{
    return mouse;
}

int osbyte_79(int c)
{
    update_keyboard();
    int key = keybuf;
#if defined(GP2X)
    keybuf = keybuf2;
    keybuf2 = -1;
#else
    keybuf = -1;
#endif
    return key;
    //for (int i=0;i<512;i++) if (keyboard[i]) {/*printf("Returning %i\n",i);*/ return i;}
    return -1;
}

int osbyte_79_unicode(int c)
{
    update_keyboard();
    int uni = unibuf;
    unibuf = -1;
    return uni;
}

int osbyte_7a()
{
    update_keyboard(); for (int i = 0; i < 512; i++) if (keyboard[i]) return i;return -1;
}
void osbyte_7c()
{
    keyboard[ESC_VALUE] = 0;
}

int osbyte_81(int c)
{
    if (c >= 0) return osbyte_79(c);
    update_keyboard();
    return keyboard[-c];
}

char swi_oscrc(int w, char* start, char* end, int bytes)
{
    return 0xff;
}


int swi_joystick_read(int a, int* x, int* y)
{
    ;
}

void update_keyboard()
{
    SDL_Event e;
    SDL_KeyboardEvent* ke;
    SDL_MouseButtonEvent* me;

    while (SDL_PollEvent(&e))
    {
        // why not SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_KEYDOWNMASK|SDL_KEYUPMASK)>0)?
        switch (e.type)
        {
        case SDL_KEYDOWN:
            ke = (SDL_KeyboardEvent*)&e;
            keyboard[ke->keysym.sym] = 0xff;
            keybuf = ke->keysym.sym;
            if (ke->keysym.unicode)
                unibuf = ke->keysym.unicode;
            else
            {
#if defined(PANDORA)
                if (keybuf > 32 && keybuf <= 127) unibuf = keybuf;
                else
#endif
                unibuf = -1;
            }
            break;
        case SDL_KEYUP:
            ke = (SDL_KeyboardEvent*)&e;
            keyboard[ke->keysym.sym] = 0;
            break;
        case SDL_MOUSEBUTTONDOWN:
            me = (SDL_MouseButtonEvent*)&e;
            switch (me->button)
            {
            case SDL_BUTTON_LEFT: mouse |= 4; break;
            case SDL_BUTTON_MIDDLE: mouse |= 2; break;
            case SDL_BUTTON_RIGHT: mouse |= 1; break;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            me = (SDL_MouseButtonEvent*)&e;
            switch (me->button)
            {
            case SDL_BUTTON_LEFT: mouse &= ~4; break;
            case SDL_BUTTON_MIDDLE: mouse &= ~2; break;
            case SDL_BUTTON_RIGHT: mouse &= ~1; break;
            }
            break;
        case SDL_VIDEOEXPOSE:
            exposed = 1;
            break;
        case SDL_QUIT:
            exithandler();
            break;
#if defined(GP2X)
        case SDL_JOYBUTTONDOWN:
            switch (e.jbutton.button)
            {
            case GP2X_BUTTON_A:
            case GP2X_BUTTON_B:
            case GP2X_BUTTON_X:
            case GP2X_BUTTON_Y:
            case GP2X_BUTTON_L:
            case GP2X_BUTTON_R:
                keybuf = unibuf = gp2x_keyboard[e.jbutton.button];
                keybuf2 = -1;
                keyboard[keybuf] = 0xff;
                break;
            case GP2X_BUTTON_START:
            case GP2X_BUTTON_SELECT:
                keybuf = gp2x_keyboard[e.jbutton.button];
                keybuf2 = unibuf = -1;
                keyboard[keybuf] = 0xff;
                break;

            case GP2X_BUTTON_UP:
                gp2x_keys[e.jbutton.button] = 0xff;
                keyboard[gp2x_keyboard[e.jbutton.button]] = 0xff;
                if (!gp2x_keys[GP2X_BUTTON_UPLEFT] && !gp2x_keys[GP2X_BUTTON_UPRIGHT])
                {
                    keybuf = gp2x_keyboard[e.jbutton.button];
                    keybuf2 = unibuf = -1;
                }
                break;
            case GP2X_BUTTON_DOWN:
                gp2x_keys[e.jbutton.button] = 0xff;
                keyboard[gp2x_keyboard[e.jbutton.button]] = 0xff;
                if (!gp2x_keys[GP2X_BUTTON_DOWNLEFT] && !gp2x_keys[GP2X_BUTTON_DOWNRIGHT])
                {
                    keybuf = gp2x_keyboard[e.jbutton.button];
                    keybuf2 = unibuf = -1;
                }
                break;
            case GP2X_BUTTON_LEFT:
                gp2x_keys[e.jbutton.button] = 0xff;
                keyboard[gp2x_keyboard[e.jbutton.button]] = 0xff;
                if (!gp2x_keys[GP2X_BUTTON_UPLEFT] && !gp2x_keys[GP2X_BUTTON_DOWNLEFT])
                {
                    keybuf = gp2x_keyboard[e.jbutton.button];
                    keybuf2 = unibuf = -1;
                }
                break;
            case GP2X_BUTTON_RIGHT:
                gp2x_keys[e.jbutton.button] = 0xff;
                keyboard[gp2x_keyboard[e.jbutton.button]] = 0xff;
                if (!gp2x_keys[GP2X_BUTTON_UPRIGHT] && !gp2x_keys[GP2X_BUTTON_DOWNRIGHT])
                {
                    keybuf = gp2x_keyboard[e.jbutton.button];
                    keybuf2 = unibuf = -1;
                }
                break;
            case GP2X_BUTTON_UPLEFT:
                gp2x_keys[e.jbutton.button] = 0xff;
                keyboard[gp2x_keyboard[GP2X_BUTTON_LEFT]] = 0xff;
                keyboard[gp2x_keyboard[GP2X_BUTTON_UP]] = 0xff;
                if (!gp2x_keys[GP2X_BUTTON_LEFT])
                {
                    keybuf = gp2x_keyboard[GP2X_BUTTON_LEFT];
                    unibuf = -1;
                    keybuf2 = (gp2x_keys[GP2X_BUTTON_UP]) ? -1 : gp2x_keyboard[GP2X_BUTTON_UP];
                }
                else if (!gp2x_keys[GP2X_BUTTON_UP])
                {
                    keybuf = gp2x_keyboard[GP2X_BUTTON_UP];
                    keybuf2 = unibuf = -1;
                }
                break;
            case GP2X_BUTTON_UPRIGHT:
                gp2x_keys[e.jbutton.button] = 0xff;
                keyboard[gp2x_keyboard[GP2X_BUTTON_RIGHT]] = 0xff;
                keyboard[gp2x_keyboard[GP2X_BUTTON_UP]] = 0xff;
                if (!gp2x_keys[GP2X_BUTTON_RIGHT])
                {
                    keybuf = gp2x_keyboard[GP2X_BUTTON_RIGHT];
                    unibuf = -1;
                    keybuf2 = (gp2x_keys[GP2X_BUTTON_UP]) ? -1 : gp2x_keyboard[GP2X_BUTTON_UP];
                }
                else if (!gp2x_keys[GP2X_BUTTON_UP])
                {
                    keybuf = gp2x_keyboard[GP2X_BUTTON_UP];
                    keybuf2 = unibuf = -1;
                }
                break;
            case GP2X_BUTTON_DOWNLEFT:
                gp2x_keys[e.jbutton.button] = 0xff;
                keyboard[gp2x_keyboard[GP2X_BUTTON_LEFT]] = 0xff;
                keyboard[gp2x_keyboard[GP2X_BUTTON_DOWN]] = 0xff;
                if (!gp2x_keys[GP2X_BUTTON_DOWN])
                {
                    keybuf = gp2x_keyboard[GP2X_BUTTON_DOWN];
                    unibuf = -1;
                    keybuf2 = (gp2x_keys[GP2X_BUTTON_LEFT]) ? -1 : gp2x_keyboard[GP2X_BUTTON_LEFT];
                }
                else if (!gp2x_keys[GP2X_BUTTON_LEFT])
                {
                    keybuf = gp2x_keyboard[GP2X_BUTTON_LEFT];
                    keybuf2 = unibuf = -1;
                }
                break;
            case GP2X_BUTTON_DOWNRIGHT:
                gp2x_keys[e.jbutton.button] = 0xff;
                keyboard[gp2x_keyboard[GP2X_BUTTON_RIGHT]] = 0xff;
                keyboard[gp2x_keyboard[GP2X_BUTTON_DOWN]] = 0xff;
                if (!gp2x_keys[GP2X_BUTTON_DOWN])
                {
                    keybuf = gp2x_keyboard[GP2X_BUTTON_DOWN];
                    unibuf = -1;
                    keybuf2 = (gp2x_keys[GP2X_BUTTON_RIGHT]) ? -1 : gp2x_keyboard[GP2X_BUTTON_RIGHT];
                }
                else if (!gp2x_keys[GP2X_BUTTON_RIGHT])
                {
                    keybuf = gp2x_keyboard[GP2X_BUTTON_RIGHT];
                    keybuf2 = unibuf = -1;
                }
                break;
            case GP2X_BUTTON_VOLUP:
                Change_HW_Audio_Volume(4);
                break;
            case GP2X_BUTTON_VOLDOWN:
                Change_HW_Audio_Volume(-4);
                break;
            }
            break;
        case SDL_JOYBUTTONUP:
            switch (e.jbutton.button)
            {
            case GP2X_BUTTON_A:
            case GP2X_BUTTON_B:
            case GP2X_BUTTON_X:
            case GP2X_BUTTON_Y:
            case GP2X_BUTTON_L:
            case GP2X_BUTTON_R:
            case GP2X_BUTTON_START:
            case GP2X_BUTTON_SELECT:
                keyboard[gp2x_keyboard[e.jbutton.button]] = 0;
                break;

            case GP2X_BUTTON_UP:
                gp2x_keys[e.jbutton.button] = 0;
                if (!gp2x_keys[GP2X_BUTTON_UPLEFT] && !gp2x_keys[GP2X_BUTTON_UPRIGHT])
                {
                    keyboard[gp2x_keyboard[e.jbutton.button]] = 0;
                }
                break;
            case GP2X_BUTTON_DOWN:
                gp2x_keys[e.jbutton.button] = 0;
                if (!gp2x_keys[GP2X_BUTTON_DOWNLEFT] && !gp2x_keys[GP2X_BUTTON_DOWNRIGHT])
                {
                    keyboard[gp2x_keyboard[e.jbutton.button]] = 0;
                }
                break;
            case GP2X_BUTTON_LEFT:
                gp2x_keys[e.jbutton.button] = 0;
                if (!gp2x_keys[GP2X_BUTTON_UPLEFT] && !gp2x_keys[GP2X_BUTTON_DOWNLEFT])
                {
                    keyboard[gp2x_keyboard[e.jbutton.button]] = 0;
                }
                break;
            case GP2X_BUTTON_RIGHT:
                gp2x_keys[e.jbutton.button] = 0;
                if (!gp2x_keys[GP2X_BUTTON_UPRIGHT] && !gp2x_keys[GP2X_BUTTON_DOWNRIGHT])
                {
                    keyboard[gp2x_keyboard[e.jbutton.button]] = 0;
                }
                break;
            case GP2X_BUTTON_UPLEFT:
                gp2x_keys[e.jbutton.button] = 0;
                if (!gp2x_keys[GP2X_BUTTON_LEFT])
                {
                    keyboard[gp2x_keyboard[GP2X_BUTTON_LEFT]] = 0;
                }
                if (!gp2x_keys[GP2X_BUTTON_UP])
                {
                    keyboard[gp2x_keyboard[GP2X_BUTTON_UP]] = 0;
                }
                break;
            case GP2X_BUTTON_UPRIGHT:
                gp2x_keys[e.jbutton.button] = 0;
                if (!gp2x_keys[GP2X_BUTTON_RIGHT])
                {
                    keyboard[gp2x_keyboard[GP2X_BUTTON_RIGHT]] = 0;
                }
                if (!gp2x_keys[GP2X_BUTTON_UP])
                {
                    keyboard[gp2x_keyboard[GP2X_BUTTON_UP]] = 0;
                }
                break;
            case GP2X_BUTTON_DOWNLEFT:
                gp2x_keys[e.jbutton.button] = 0;
                if (!gp2x_keys[GP2X_BUTTON_DOWN])
                {
                    keyboard[gp2x_keyboard[GP2X_BUTTON_DOWN]] = 0;
                }
                if (!gp2x_keys[GP2X_BUTTON_LEFT])
                {
                    keyboard[gp2x_keyboard[GP2X_BUTTON_LEFT]] = 0;
                }
                break;
            case GP2X_BUTTON_DOWNRIGHT:
                gp2x_keys[e.jbutton.button] = 0;
                if (!gp2x_keys[GP2X_BUTTON_DOWN])
                {
                    keyboard[gp2x_keyboard[GP2X_BUTTON_DOWN]] = 0;
                }
                if (!gp2x_keys[GP2X_BUTTON_RIGHT])
                {
                    keyboard[gp2x_keyboard[GP2X_BUTTON_RIGHT]] = 0;
                }
                break;
            }
            break;
#endif
        }
    }
}

void zonecheatread(int* zone)
{
    char r1 = osbyte_79_unicode(0); // was _81(0)

    if ((r1 < 48) || (r1 > 56)) return;
    *zone = r1-48;
}

void cheatread()
{
    if (osbyte_81(-282) == 0xff) getmpmg();
    if (osbyte_81(-283) == 0xff) getrocket();
    if (osbyte_81(-285) == 0xff) screensave();
    if (osbyte_81(-284) == 0xff) prepstrength();
}


void keyread(key_state* ks)
{
    int r4 = -1;

    if (options.joyno)
    {
        int r0, r1;
        int v = swi_joystick_read(options.joyno-1, &r0, &r1);
        if (v)
        {
           nostickerr:
            message(32, 32, 0, 1, "Can't see a joystick!");
            r4 = -1;
            options.joyno = 0;
        }
        else
            r4 = -1;
/*
   MOV R1,R0,ASL #24
   CMN R1,#32<<24
   BICLT R4,R4,#8; down
   CMP R1,#32<<24
   BICGT R4,R4,#4; up
   BIC R0,R0,#&FF
   MOV R1,R0,ASL #16
   CMN R1,#32<<24
   BICLT R4,R4,#1; left
   CMP R1,#32<<24
   BICGT R4,R4,#2; right

   TST R0,#1<<16
   BICNE R4,R4,#16; fire
   TST R0,#1<<17
   BICNE R4,R4,#4; up on fire button 2
 */
    }
   nojoystick:
    if ((osbyte_81(options.leftkey) == 0xff) || !(r4&1))
      { if (++ks->leftpress == 0) ks->leftpress = 0xff;}
    else ks->leftpress = 0;
    if ((osbyte_81(options.rightkey) == 0xff) || !(r4&2))
      { if (++ks->rightpress == 0) ks->rightpress = 0xff;}
    else ks->rightpress = 0;
    if ((osbyte_81(options.upkey) == 0xff) || !(r4&4))
      { if (++ks->uppress == 0) ks->uppress = 0xff; }
    else ks->uppress = 0;
    if ((osbyte_81(options.downkey) == 0xff) || !(r4&8))
      { if (++ks->downpress == 0) ks->downpress = 0xff; }
    else ks->downpress = 0;
    if ((osbyte_81(options.firekey) == 0xff) || !(r4&16))
      { if (++ks->fire == 0) ks->fire = 0xff; }
    else ks->fire = 0;
    if (ks->leftpress || ks->rightpress
	  || ks->uppress || ks->downpress || ks->fire)
        ks->keypressed = 1;
}

int need_redraw()
{
    int e = exposed;
    exposed = 0;
    return e;
}
