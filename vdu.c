/*  vdu.c */

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

#include <SDL/SDL.h>

#include "asylum.h"

extern fastspr_sprite charsadr[48];
extern fastspr_sprite blockadr[256];
extern board *boardadr;
extern int boardwidth;

extern const char _bonuslow;
#define _strengthmax (108<<8)

struct vduvar
{
    int width; int height;
    int gamex; int gamey;
    int gameh; int gamew;
    int backh; int backw;
    int scorex; int scorey;
    int sprw; int sprh;
    int xblocks; int yblocks;
    int livesx; int livesy;
    int strengthx; int strengthy;
    int strengthw; int strengthh;
    int bonusx; int bonusy; int bonush;
} vduvar;

SDL_Surface* ArcScreen;
SDL_Surface* GameScreen;
SDL_Surface* ChatScreen;
SDL_Surface* backsprite;
SDL_Surface* wipescr;
SDL_Surface* redness;
SDL_Surface* greyness;
SDL_Rect clip;

#define _textno 32
#define _textlen (20+60)
#define _charwidth 16

extern int framectr;
extern char frameinc;

typedef struct textinfo
{
    int count; int x; int y; int dx; int dy;
    char text[60];
} textinfo;
textinfo texttabofs[_textno];

void switchbank()
{
    //osbyte_71();
    //swi_blitz_screenretrieve();
    SDL_Flip(ArcScreen);
}

void fspplot(fastspr_sprite* sprites, char n, int x, int y)
{
    fastspr_sprite sprite = sprites[(char)n];
    static SDL_Rect pos;

    pos.x = x-sprite.x; pos.y = y-sprite.y;
    SDL_BlitSurface(sprite.s, NULL, ArcScreen, &pos);
}

void cenplot(fastspr_sprite* sprites, char n, int x, int y)
{
    fspplot(sprites, n,
            ((x-1)*vduvar.sprw)/16+vduvar.gamex+(vduvar.gamew/2),
            (y*vduvar.sprh)/16+vduvar.gamey+(vduvar.gameh/2)-(vduvar.sprh/2));
}


void cenplotdying(fastspr_sprite* sprites, char n, int x, int y, int r5)
{
    int fsplx, fsply, fsphx, fsphy;
    int r8 = 8-(r5>>17);
    int r1 = x+vduvar.gamex+(vduvar.gamew/2);
    int r2 = r8+y+vduvar.gamey+(vduvar.gameh/2)-(vduvar.sprh/2);
    fsply = vduvar.gamey;
    fsplx = vduvar.gamex;
    fsphy = r2;
    if (fsphy <= vduvar.gamey+vduvar.gameh)
        if (fsply < fsphy)
        {
            fsphx = r1;
            swi_fastspr_setclipwindow(fsplx, fsply, fsphx, fsphy);
            if (fsphx <= vduvar.gamex+vduvar.gamew)
                if (fsplx < fsphx)
                    fspplot(sprites, n, fsphx+r8, fsphy+r8);
            fsphx = vduvar.gamex+vduvar.gamew;
            fsplx = r1;
            swi_fastspr_setclipwindow(fsplx, fsply, fsphx, fsphy);
            if (fsplx >= vduvar.gamex)
                if (fsphx > vduvar.gamex)
                    fspplot(sprites, n, fsplx-r8, fsphy+r8);
        }
   nodyingtop:
    fsphy = vduvar.gamey+vduvar.gameh;
    fsplx = vduvar.gamex;
    fsply = r2;
    if (fsply >= vduvar.gamey)
        if (fsply < fsphy)
        {
            fsphx = r1;
            swi_fastspr_setclipwindow(fsplx, fsply, fsphx, fsphy);
            if (fsphx <= vduvar.gamex+vduvar.gamew)
                if (fsplx < fsphx)
                    fspplot(sprites, n, fsphx+r8, fsply-r8);
            fsphx = vduvar.gamex+vduvar.gamew;
            fsplx = r1;
            swi_fastspr_setclipwindow(fsplx, fsply, fsphx, fsphy);
            if (fsplx >= vduvar.gamex)
                if (fsphx > fsplx)
                    fspplot(sprites, n, fsplx-r8, fsply-r8);
        }
   nodyingbot:
    writeclip();
}

void blokeplot_cen(fastspr_sprite* sprites, char n, int x, int y)
{
    fspplot(sprites, n, x+vduvar.gamex+(vduvar.gamew/2),
            y+vduvar.gamey+(vduvar.gameh/2)-(vduvar.sprh/2));
}

void message_scroll(const char* a)
{
    message(vduvar.gamex+vduvar.gamew+32, vduvar.gamey+vduvar.gameh-8, -3, 0, a);
}

void message(int x, int y, float xv, float yv, const char* a)
{
    int time = 400;
    char b[60];
    char* q = b;

    if (x > 1000)
    {
        x -= 1000; time = 50;
    }
    if (x > 1000)
    {
        x -= 1000; time = 0x10000;
    }
//int32_t z[5]={time,x<<8,y<<8,(int)(xv*256),(int)(yv*256)}; // might be non-integers
    for (const char* p = a; *p != 0; p++, q++)
    {
        switch (*p)
        {
        case '-': *q = ';'; break;
        case 164: *q = ':'; break;
        case '\'': *q = '~'; break;
        case '.': *q = '{'; break;
        case ',': *q = '|'; break;
        case '?': *q = '<'; break;
        case '!': *q = '}'; break;
        case '%': *q = '>'; break;
        case 200: *q = '?'; break;
        case 215: *q = '@'; break;
        case '#': *q = 127; break;
        default: *q = *p;
        }
        if (*q >= 96) *q -= 32;
        if (*q == ' ') *q = 0xff;
        else if ((*q&~1) != 16)
        {
            *q -= 47; if (*q > 55) exit(printf("Bad message character %c (%i)", *p, *p));
        }
    }
    *q = 0;
    {
        textinfo* r11 = texttabofs;
        int r9;
        for (r9 = _textno; r9 > 0; r9--)
        {
           loopa8:
            if (r11->count == 0) break;
            r11++;
        }
        if (r9 == 0) return;
       messageproc:
        r11->count = time; r11->x = x<<8; r11->y = y<<8;
        r11->dx = (int)(xv*256); r11->dy = (int)(yv*256);
        if (strlen(b) > 58) exit(printf("Bad string %s", b));
        strncpy(r11->text, b, 60);
    }
}

void wipetexttab()
{
    char* r10 = (char*)texttabofs;

    for (int r3 = _textno*_textlen; r3 > 0; r3 -= 1)
       loopa5:
        *(r10++) = 0;
}

void texthandler()
{
    textinfo* r11 = texttabofs;
    int r9 = _textno;

    for (; r9 > 0; r11++, r9--)
    {
       loopa6: if (r11->count == 0) continue;
       textproc:;
        int r4 = (frameinc > 4) ? 4 : frameinc;
        r11->count -= r4; if (r11->count < 0) r11->count = 0;

        for (; r4 > 0; r4--)
        {
           loopb1:
            r11->x += r11->dx;
            r11->y += r11->dy;
        }
        int XxX = r11->x>>8;
        int YyY = r11->y>>8;
        for (char* r10 = r11->text; *r10 != 0; r10++)
        {
            char r0;
           loopa7:
            r0 = *r10;
            if ((*r10-1) < 48)  // only plot known characters (charsadr is [48])
                fspplot(charsadr, *r10-1, XxX, YyY);
            XxX += 14;
            if (*r10 <= 10) XxX += 2;
            if (*r10 > 43) XxX -= 6;
        }
    }
   textdone:;
//textdelete: r11->count = 0;
}

void mazeplot(int xpos, int ypos)
{
    writeclip();
    backdrop(xpos, ypos);

    int r0 = (xpos-((vduvar.xblocks*8)<<8))>>8;
    int r1 = (ypos-((vduvar.yblocks*8)<<8))>>8;
    int r8 = ((15-(r0&15)-16)*vduvar.sprw)/16+vduvar.gamex;
    int r9 = ((15-(r1&15)-8)*vduvar.sprh)/16+vduvar.gamey;


    int r5 = 0, r7 = (r1>>4);
    if (r7 < 0)
    {
        r5 = -r7; r7 = 0;
    }

// Draw midground elements
    for (; /*ins2:*/ (r5 < vduvar.yblocks+2) && (r7 < boardadr->height); r5++, r7++)
    {
        int r4 = 0, r6 = ((xpos>>8)-(vduvar.xblocks*8))>>4;
        if (r6 < 0)
        {
            r4 = -r6; r6 = 0;
        }
        for (; /*ins1:*/ (r4 < vduvar.xblocks+3)&(r6 < boardadr->width); /*skip1:*/ r4++, r6++)
        {
            r0 = *(boardadr->contents+boardwidth*r7+r6);
	    draw_block(blockadr, r0, (r4*vduvar.sprw*15)/16+vduvar.xblocks/2+1+r8,
		       (r5*vduvar.sprh*15)/16+vduvar.yblocks/2+1+r9, 1);
        }
    }
// Now foreground ones
    r5 = 0, r7 = (r1>>4);
    if (r7 < 0)
    {
        r5 = -r7; r7 = 0;
    }

    for (; /*ins2:*/ (r5 < vduvar.yblocks+2) && (r7 < boardadr->height); r5++, r7++)
    {
       l2:;
        int r4 = 0, r6 = ((xpos>>8)-(vduvar.xblocks*8))>>4;
        if (r6 < 0)
        {
            r4 = -r6; r6 = 0;
        }

        for (; /*ins1:*/ (r4 < vduvar.xblocks+3)&(r6 < boardadr->width); /*skip1:*/ r4++, r6++)
        {
           l1:
            r0 = *(boardadr->contents+boardwidth*r7+r6);
            draw_block(blockadr, r0, r4*vduvar.sprw+r8, r5*vduvar.sprh+r9, 0);
        }
    }
   skip2:;
}

void backdrop(int xpos, int ypos)
{
    //  swi_fastspr_clearwindow();
    static SDL_Rect back_to_blit;
    static SDL_Rect loc_to_blit;

    int r3 = ((xpos>>8)-(xpos>>10)+3072-768); // parallax
    int r2 = (44+48-(r3%48))%48;

    int r4 = 0x1f&((ypos>>8)-(ypos>>10)); // parallax

   modpos:
    back_to_blit.x = ((48-r2)*vduvar.backw)/48; back_to_blit.y = (r4*vduvar.backh)/32;
    back_to_blit.w = vduvar.backw; back_to_blit.h = vduvar.backh;

    for (loc_to_blit.y = vduvar.gamey; loc_to_blit.y < vduvar.gamey+vduvar.gameh; loc_to_blit.y += vduvar.backh)
        for (loc_to_blit.x = vduvar.gamex; loc_to_blit.x < vduvar.gamex+vduvar.gamew; loc_to_blit.x += vduvar.backw)
            SDL_BlitSurface(backsprite, &back_to_blit, ArcScreen, &loc_to_blit);
}

void plotbonus(char bonusctr, int16_t bonusreplot)
{
    int fsplx, fsply, fsphx, fsphy;
    fsplx = vduvar.bonusx-vduvar.sprw/2; // align to write clip window to FastSpr
    fsply = vduvar.bonusy-vduvar.bonush;
    fsphx = vduvar.bonusx+vduvar.sprw/2;
    fsphy = vduvar.bonusy+vduvar.bonush;
    swi_fastspr_setclipwindow(fsplx, fsply, fsphx, fsphy);
    swi_fastspr_clearwindow();
    int r3 = (((bonusreplot > 7) ? bonusreplot-8 : 0)*vduvar.bonush)/20;
    fspplot(blockadr, (bonusctr+16 > _bonuslow+12) ? 0 : (bonusctr+16),
            vduvar.bonusx, vduvar.bonusy+r3);
    if (bonusctr != 0)
        fspplot(blockadr, (bonusctr+15 > _bonuslow+12) ? 0 : (bonusctr+15),
                vduvar.bonusx, vduvar.bonusy-vduvar.bonush+r3);
   nosecond:
    if (bonusctr != 12)
        fspplot(blockadr, (bonusctr+17 > _bonuslow+12) ? 0 : (bonusctr+17),
                vduvar.bonusx, vduvar.bonusy+vduvar.bonush+r3);
   nothird:
    writeclip(); // reset the clip window
}

void showstrength(int r3)
{
   nolager:;
    if (r3 > _strengthmax) r3 = _strengthmax;
    if (r3 < 0) r3 = 0;
    releaseclip();
    static SDL_Rect strengthloc, strengthpart;
    strengthloc.x = vduvar.strengthx;
    strengthloc.y = vduvar.strengthy;
    strengthpart.x = 0; strengthpart.y = framectr&0x1f;
    strengthpart.w = vduvar.strengthw;
    strengthpart.h = vduvar.strengthh;
    SDL_BlitSurface(greyness, &strengthpart, ArcScreen, &strengthloc);
    strengthpart.w = (vduvar.strengthw*r3)/_strengthmax;
    SDL_BlitSurface(redness, &strengthpart, ArcScreen, &strengthloc);
    writeclip();
    return;
}

void scorewipe()
{
    static SDL_Rect scorearea;

    scorearea.x = vduvar.scorex;
    scorearea.y = vduvar.scorey-16/2;
    scorearea.w = 128; scorearea.h = 16;
    releaseclip();
    SDL_BlitSurface(wipescr, NULL, ArcScreen, &scorearea);
    writeclip();
}

void scorewiperead()
{
    static SDL_Rect scorearea;

    scorearea.x = vduvar.scorex;
    scorearea.y = vduvar.scorey-16/2;
    scorearea.w = 128; scorearea.h = 16;
    SDL_BlitSurface(ArcScreen, &scorearea, wipescr, NULL);
}

void showscore(char plscore[8])
{
    releaseclip();
    int i = 0;
    int x = vduvar.scorex;
    int y = vduvar.scorey;

    if (plscore[0] == 0)
    {
        x += _charwidth/2;
        i = 1;
        if (plscore[1] == 0)
        {
            x += _charwidth/2;
            i = 2;
        }
    }
   score10mil:
    /*r7=0*/;

    for (; i < 8; i++)
    {
       loop36:
        if (plscore[i] <= 10)
        {
            fspplot(charsadr, plscore[i], x, y);
            /*r8=1*/;
        }
       scoreskip:
        x += _charwidth;
    }
}

void setfullclip()
{
    clip.x = vduvar.gamex; clip.y = vduvar.gamey;
    clip.w = vduvar.gamew;
    clip.h = vduvar.gameh;
    writeclip();
}

void writeclip()
{
    SDL_SetClipRect(ArcScreen, &clip);
}

void releaseclip()
{
    SDL_SetClipRect(ArcScreen, NULL);
// 0, 0, 319, 255 ; size of mode
}

void clearkeybuf()
{
    do clearkbloop:;
    while (osbyte_79(0) != -1);
    do clearkbloop2 :;
    while (osbyte_81(1) != -1);
}

void showlives(int lives)
{
    releaseclip();
    fspplot(charsadr, lives&7, vduvar.livesx, vduvar.livesy);
    switchbank();
    writeclip();
}

void showgamescreen()
{
    releaseclip();
    //SDL_SoftStretch(GameScreen,NULL,ArcScreen,NULL);
    SDL_BlitSurface(GameScreen, NULL, ArcScreen, NULL);
    writeclip();
    SDL_Flip(ArcScreen);
    scorewiperead();
    showlives();
}

void showchatscreen()
{
    releaseclip();
    //SDL_SoftStretch(ArcScreen,NULL,DecompScreen,NULL);
    SDL_BlitSurface(ChatScreen, NULL, ArcScreen, NULL);
    writeclip();
    SDL_Flip(ArcScreen);
}

void showchatscores()
{
    releaseclip();
    //SDL_SoftStretch(ArcScreen,NULL,DecompScreen,NULL);
    SDL_BlitSurface(ChatScreen, NULL, ArcScreen, NULL);
    writeclip();
}

void initialize_chatscreen(char* chatscreenadr)
{
    ChatScreen = decomp(chatscreenadr);
}

void initialize_gamescreen(char* gamescreenadr)
{
    GameScreen = decomp(gamescreenadr);
}

void init_strengthcol()
{
    int w = vduvar.strengthw, h = 32+vduvar.strengthh;

    redness = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32, 0xff, 0xff00, 0xff0000, 0);
    greyness = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32, 0xff, 0xff00, 0xff0000, 0);
    SDL_LockSurface(redness);
    SDL_LockSurface(greyness);
    for (int j = 0; j < 32; j++)
        for (int i = 0; i < w; i++)
        {
            ((Uint32*)redness->pixels)[j*(redness->pitch/4)+i] = 0xff000000+0x11*(11+(random()%5));  // varying shade of red
            ((Uint32*)greyness->pixels)[j*(greyness->pitch/4)+i] = 0xff000000+0x111111*(3&random()); // varying shade of dark grey
        }
    for (int j = 32; j < 32+vduvar.strengthh; j++)
        for (int i = 0; i < w; i++)
        {
            ((Uint32*)redness->pixels)[j*(redness->pitch/4)+i] = ((Uint32*)redness->pixels)[(j-32)*(redness->pitch/4)+i];
            ((Uint32*)greyness->pixels)[j*(greyness->pitch/4)+i] = ((Uint32*)greyness->pixels)[(j-32)*(greyness->pitch/4)+i];
        }
    SDL_UnlockSurface(greyness);
    SDL_UnlockSurface(redness);
}

int palette[256];

void init_palette()
{
    for (int i = 0; i < 256; i++)
        palette[i]
        = 0xff000000 // opaque
          +((i&0x80) ? 0x880000 : 0)+((i&0x40) ? 0x8800 : 0)
          +((i&0x20) ? 0x4400 : 0)+((i&0x10) ? 0x88 : 0)
          +((i&0x08) ? 0x440000 : 0)+((i&0x04) ? 0x44 : 0)
          +(i&0x03)*0x111111;
}

SDL_Surface* decomp(char* r11)
{
    SDL_Surface* DecompScreen = SDL_CreateRGBSurface(SDL_HWSURFACE, 320, 256, 32,
                                                     0xff, 0xff00, 0xff0000, 0);
    static SDL_Rect from;
    static SDL_Rect to;

    from.x = from.y = to.x = to.y = 0;
    to.w = ArcScreen->w; to.h = ArcScreen->h;
    from.w = DecompScreen->w; from.h = DecompScreen->h;
    SDL_LockSurface(DecompScreen);
    Uint32* r10 = (Uint32*)DecompScreen->pixels;
    Uint32* r9 = 0x14000+r10;

    r11 += 68;
    while (r9 > r10) // > or >=?
    {
       loopb4:;
        char r0 = *(r11++);
        if (r0&0x80)
        {
           sequence:; Uint32 s = palette[0xff&*(r11++)];
            for (int r3 = (r0&0x7f)+2; (r9 > r10) && (r3 != 0); r3--)
               loopb6: *(r10++) = s;
        }
        else pattern: for (int r3 = (r0&0x7f)+1; (r9 > r10) && (r3 != 0); r3--)
               loopb5: *(r10++) = palette[0xff&*(r11++)];
       decompdone:;
    }
    SDL_UnlockSurface(DecompScreen);
    return DecompScreen;
}

void vduread(char fullscreen)
{
    vduvar.width = 320; vduvar.height = 256;
    vduvar.gamex = 16; vduvar.gamey = 8;
    vduvar.gamew = 16*18; vduvar.gameh = 16*12;
    vduvar.backw = 48; vduvar.backh = 32;
    vduvar.scorex = (160-(16*7)/2); vduvar.scorey = 220;
    vduvar.xblocks = 18; vduvar.yblocks = 12;
    vduvar.livesx = 44; vduvar.livesy = 234;
    vduvar.strengthx = 108; vduvar.strengthy = 239;
    vduvar.strengthw = (_strengthmax>>8); vduvar.strengthh = 6;
    vduvar.bonusx = 290; vduvar.bonusy = 232; vduvar.bonush = 20;
    vduvar.sprw = 16; vduvar.sprh = 16;
    ArcScreen = SDL_SetVideoMode( vduvar.width, vduvar.height, 32 /*bpp*/,
                                  SDL_HWSURFACE|SDL_DOUBLEBUF|(fullscreen ? SDL_FULLSCREEN : 0));
    wipescr = SDL_CreateRGBSurface(SDL_HWSURFACE, 128, 16, 32, 0xff, 0xff00, 0xff0000, 0);
// backsprite contains four copies of the backdrop tile
    backsprite = SDL_CreateRGBSurface(SDL_HWSURFACE, 2*48, 2*32, 32, 0xff, 0xff00, 0xff0000, 0);
    /* initialise screenstart(149), modesize(7), hbytes(6) */
    init_strengthcol();
    SDL_ShowCursor(SDL_DISABLE);
}

void backprep(char* backadr)
{
    SDL_LockSurface(backsprite);
    Uint32* ba = (Uint32*)backsprite->pixels;
    for (int j = 63; j >= 0; j--)
        for (int i = 95; i >= 0; i--)
            ba[j*96+i] = palette[backadr[(j%32)*48+(i%48)]];
    SDL_UnlockSurface(backsprite);
}

void startmessage()
{
    message(1000+vduvar.gamex+vduvar.gamew/2-32, vduvar.gamey+(vduvar.gameh*2)/3, 0, 0, "Let's Go!");
}

void deathmessage()
{
    message(vduvar.gamex+vduvar.gamew/2-88, vduvar.gamey+vduvar.gameh+8, 0, -1, "¤ Snuffed It! ¤");
}

void endgamemessage()
{
    message(vduvar.gamex+vduvar.gamew/2-88, vduvar.gamey+vduvar.gameh+56, 0, -1, "-  GAME OVER  -");
}

void swi_blitz_wait(int d)
{
    SDL_Delay(d*10);
}

void swi_fastspr_clearwindow()
{
    SDL_FillRect(ArcScreen, NULL, 0);
}

void swi_fastspr_setclipwindow(int x1, int y1, int x2, int y2)
{
    static SDL_Rect clip;

    clip.x = x1; clip.y = y1;
    clip.w = x2-x1; clip.h = y2-y1;
    SDL_SetClipRect(ArcScreen, &clip);
}

void set_player_clip()
{
    int fsphy = vduvar.gamey+(vduvar.gameh/2)-(vduvar.sprh/2)+24;
    swi_fastspr_setclipwindow(vduvar.gamex, vduvar.gamey, vduvar.gamex+vduvar.gamew, fsphy);
}

int initialize_sprites(char* start, fastspr_sprite* sprites, int max_sprites, char* end)
{
    uint32_t* s = (uint32_t*)start;

    if (read_littleendian(s) != 0x31505346) return 0; // "FSP1"
    int num_sprites = read_littleendian(s+3);
    if (num_sprites > max_sprites) num_sprites = max_sprites;
    for (int i = 0; i < num_sprites; i++)
    {
        if (read_littleendian(s+4+i) == 0)
        {
            sprites[i].s = NULL; continue;
        }
        uint32_t* p = s+(read_littleendian(s+4+i)>>2);
        uint32_t* r = s;
        if (i == num_sprites-1) r = (uint32_t*)end;
        else for (int j = 0; (r == s) && (i+j < num_sprites-1); j++) r = s+(read_littleendian(s+5+i+j)>>2);
        if (r == s) r = (uint32_t*)end;
        uint8_t* pp = (uint8_t*)p;
        int wid = pp[0], hei = pp[1], xcen = pp[2], ycen = pp[3];
        sprites[i].x = xcen; sprites[i].y = ycen;
        sprites[i].s = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA, wid, hei, 32,
                                            0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_LockSurface(sprites[i].s);
        uint32_t* data = (uint32_t*)sprites[i].s->pixels;
        for (int z = 0; z < wid*hei; z++) data[z] = 0x0;
        for (uint32_t* q = p+2+hei; q < r; q++)
        {
            int x = ((0x0fffff00&read_littleendian(q))>>8)%320;
            int y = ((0x0fffff00&read_littleendian(q))>>8)/320;
            if ((y*wid+x < 0) || (y*wid+x >= wid*hei)) printf("%i: x=%i y=%i wid=%i hei=%i: bad idea\n", i, x, y, wid, hei);
            else data[y*wid+x] = palette[0xff&read_littleendian(q)];
        }
        SDL_UnlockSurface(sprites[i].s);
    }
    return num_sprites;
}
