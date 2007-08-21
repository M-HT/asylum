/*  asylum.c */

/*  Copyright Hugh Robinson 2006-2007.

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

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <sys/types.h>
#include <sys/stat.h>

char keyboard[512];
int keybuf;
int mouse;
int installed;

#define ESC_VALUE 27

#ifndef RESOURCEPATH
#define RESOURCEPATH "./data"
#endif
#ifndef SCOREPATH
#define SCOREPATH "./hiscores"
#endif

#define random rand
const double PI=3.1415926535897932384626433832795028841971;

#define _firstzone 0;
#define STOREAREALEN (16*0x40000)

char storearea[STOREAREALEN];
SDL_Surface* ArcScreen;
SDL_Surface* DecompScreen;

const char _platblim=64;
const char _blim=112;
const char _bomblowlim=64, _bombhighlim=79;
const char _boobylowlim=76, _boobyhighlim=79;
const char _gaslowlim=54, _gashighlim=55;
const char _animlowlim=54, _animhighlim=63;
const char _eleclowlim=56, _elecvertlowlim=60, _elechighlim=63;
const char _translowlim=54, _transhighlim=63;
const char _atomlowlim=72, _atomhighlim=75;
const char _targetlowlim=80, _powertarget=84, _nuttertarget=88, _targethighlim=95;
const char _teleplowlim=192, _telephighlim=199;
const char _fuelairno=68;
const char _neuronlowlim=40, _neuronhighlim=47;
const char _bonuslow=16, _bonushigh=31, _megabonuslim=35;
const char _starsno=15;
const char _markerno=255;
const int _xofs=159, _yofs=96;
const char _platbase=2, _platno=176, _platlowlim=176, _plathighlim=190, _alplathighlim=177;
const char _weaplowlim=96, _weaphighlim=111;
const char _triggerlim=193;
const char _crumblelowlim=160, _crumblehighlim=167;
const char _crumblestandlowlim=168, _crumblestandhighlim=175;
const char _spcrumblelowlim=140, _spcrumblehighlim=143;
const char _extendno=152;
const char _solidlowlim=128, _solidhighlim=131;
const char _shutdownno=200;

const char _mpmgblamno=8, _rocketblamno=16;

const int _windowxsize=160<<8, _windowysize=112<<8;

//alentlen=32

#define _speedlim (15<<8)
#define _windspeed (1<<8)
#define xlowlim 0
#define ylowlim (15<<8)
#define _lowx 16
#define _lowy 8
#define _highx 304
#define _highy 200

#define _neuronstoget 8

#define _expwidth (24<<8)
#define _expheight (24<<8)
#define _alheight (32<<8)
#define _alwidth (16<<8)

#define backsize 48*60*8

int fsplx, fsply, fsphx, fsphy;

#define _boxwidth 20
#define _boxheight 16

#define targetscore 500

#define _sleeprange (512<<8)
#define _firerange (128<<8)
#define _charwidth 16
#define _alno 0x200
#define _projno 32
#define _bulno 64
//projentlen=24
//bulentlen=24
#define _rockettablen 192

#define _strengthmax (108<<8)
#define spstrengthmax (_strengthmax-(1<<8))
#define _strengthinit (108<<8)
#define _strengthxofs 108
#define _strengthyofs 239

#define _bonusxplace 34
#define _bonusyplace 232

#define _scorexofs (160-(16*7)/2)
#define _scoreyofs 220

#define bombloss (4<<8)
#define atomloss (32<<8)
#define _explocontloss (1<<7)
#define bulletloss (2<<8)

#define _fuelairduration 0x18

#define _plheight (32<<8)
#define _plwidth (16<<8)

// alien object types
#define _Explo (1)
#define _Ember (2)
#define _Platbase (3)
#define _Riseplat (_Platbase)
#define _Exploplat (_Platbase+1)
#define _Updownplat (_Platbase+2)
#define _Downplat (_Platbase+3)
#define _Fastplat (_Platbase+4)
#define _Fastplatstop (_Platbase+5)
#define _Fastplatexplo (_Platbase+6)
#define _Fastplatfire (_Platbase+7)
#define _Fallplat (_Platbase+8)
#define _Scoreobj (12)
#define _Dyingbonus (13)
#define _Flyingbonus (14)
#define _Booby (15)
#define _Decoration (16)
#define _Extender (17)
#define _Alien1 (18)

typedef struct fastspr_sprite { int x; int y;
  SDL_Surface* s;} fastspr_sprite;

#define _boardhdrlen 32
typedef struct board { int first_int; int width; int height; 
  int fourth; int fifth; int sixth; int seventh; int eighth;
  char contents[65536];
} board;


//alofs=&F00
//colchofs=&5000 // two tables
#define _colchlim 0x400
//bulcolchofs=&7000
#define _bulcolchlim 0x400

typedef struct alent
{
	int type; int x; int y; int dx;
	int dy; int r5; int r6; int foo;
} alent;
alent alofs[_alno];
typedef struct colchent
{
	alent* r0; int xmin; int ymin; int xmax; int ymax;
} colchent;
colchent colchofs[2*_colchlim];
typedef struct bulcolchent
{
	alent* r0; int xmin; int ymin; int xmax; int ymax;
} bulcolchent;
bulcolchent bulcolchofs[_bulcolchlim];

#define _fueltablim 0x3c0
char* fueltabofs[2][_fueltablim]; // &8000
// don't forget there's two of these

#define _projlim 0x1000
typedef struct projent
{
	int type; int x; int y;
	int dx; int dy; int flags;
} projent;
projent projofs[_projlim];

#define _bullim 0x1000
typedef struct bulent
{
	int type; int x; int y;
	int dx; int dy; int flags;
} bulent;
bulent bulofs[_bullim];
//bulofs=&9900

//soundtabofs=&A900:REM length 8*16
#define _savearealen (0x9c0/32)
alent saveareaalents[_savearealen+1]; // &C000
int saveareaints[8];
char highscorearea[13*5+1];//=&D000
char wipescrst[128*16*4];//=&D100


/*soundtabvol=0
soundtabsample=1
soundtabpitchmsb=2
soundtabpitchlsb=3
soundtabvolslide=4
soundtabpitchslide=8
soundtabctr=12
soundtabstereo=13
soundentlen=16
soundtabshift=4*/
#define _soundentlen 16
#define _textno 32
#define _textlen (20+60)
            // was 24 but I deref'd the string
typedef struct textinfo
{
  int count; int x; int y; int dx; int dy;
  char text[60];
} textinfo;
textinfo texttabofs[_textno]; //&AA00

#define _SampJump 1
#define _SampBonus 2
#define _SampExplo 3
#define _SampAtomExplo 4
#define _SampCannon 5
#define _SampRocket 6
#define _SampHiss 7
#define _SampStunned 8
#define _Sampsmallzap 9
#define _Sampbigzap 10
#define _Samporgan 16
#define _Samprave 19

#define _projsmallno 54
#define _bulspritebase 16
#define _funnyfacesprbase 66
#define _scoresprbase 16
#define _rocketspriteno 32
#define _rocketspritelim 41
#define _skelspriteno 46
#define _starssprbase 58
#define _deadsprbase 70

#define _rockbase 9
#define _mpmgbase 1

#define _Playerchannel 0
#define _Explochannel 3
#define _Firechannel 1
#define _Sparechannel 2

Mix_Chunk* CHUNK_BULLET_1;
Mix_Chunk* CHUNK_BULLET_2;
Mix_Chunk* CHUNK_BULLET_3;
Mix_Chunk* CHUNK_ELEC_1;
Mix_Chunk* CHUNK_ELEC_2;
Mix_Chunk* CHUNK_ELEC_3;
Mix_Chunk* CHUNK_EXPLO;
Mix_Chunk* CHUNK_ATOM;
Mix_Chunk* CHUNK_SPINFIRE;
Mix_Chunk* CHUNK_SPINPOWERFIRE;
Mix_Chunk* CHUNK_SHOOT;
Mix_Chunk* CHUNK_SHOOTNUTTER;
Mix_Chunk* CHUNK_JUMP;
Mix_Chunk* CHUNK_SHUTDOWN_1;
Mix_Chunk* CHUNK_SHUTDOWN_2;
Mix_Chunk* CHUNK_SHUTDOWN_3;
Mix_Chunk* CHUNK_TELEP_1;
Mix_Chunk* CHUNK_TELEP_2;
Mix_Chunk* CHUNK_TELEP_3;
Mix_Chunk* CHUNK_BLAM;
Mix_Chunk* CHUNK_FIRE;
Mix_Chunk* CHUNK_BLAMFIRE;
Mix_Chunk* CHUNK_ROCKET;
Mix_Chunk* CHUNK_WEAPON_1;
Mix_Chunk* CHUNK_WEAPON_2;
Mix_Chunk* CHUNK_WEAPON_3;
Mix_Chunk* CHUNK_WEAPON_4;
Mix_Chunk* CHUNK_FUELAIR;
Mix_Chunk* CHUNK_OBJGOT[12];
Mix_Chunk* CHUNK_BONUS_1;
Mix_Chunk* CHUNK_BONUS_2;
Mix_Chunk* CHUNK_STUNNED[17];


char bank, plframe, plface, fire;
char jumping, falling;
char plotal, masterplotal;
char uppress, downpress, leftpress, rightpress; // word-aligned
char lefthit, righthit, uphit, downhit; // word-aligned
char recheck, pluphit, atombombctr, frameinc, firerate;
char plfired, plweapontype, plweaponspeed;
char rocketflag, electrocuting, alonplat, platuphit;
char colchecktype, colchplace, alonobj, rate50, bonusctr;
uint16_t bonusreplot;
char keypressed, blamctr, rocketblamctr, plotterofs, hiddenplatctr, extending;
char charsok, arm3, gameon, savestart;
int leftkey, rightkey, upkey, downkey, firekey;
char soundtype, soundquality, explospeed, gearchange;
char fullscreen, sound_available;
char soundvol, musicvol, joyno, mentalzone, saveend, savedornot;
char bonusstore, plweaponstore;
char hstindex, idpermit, cheatpermit;
char initials[3];

// consecutive after o=64
board *boardadr;
fastspr_sprite blokeadr[77];
fastspr_sprite blockadr[256];
char* tmp_blockadr;
void **fspvars;
//void *fspplot;
char* backadr;
char* backuse;
int framectr;
int boardwidth;
int xtemp;
int ytemp;
board *brainadr;
board *neuronadr;
int fspareat;
colchent* colchtab;
colchent* colchptr;
int* platsandstr;
fastspr_sprite exploadr[32];
char** fueltabread;
char** fueltabwrite;
int fueltabctr;
int fuelexploctr;
//lifeseed1=o+88
//lifeseed2=o+92
Uint32* strengthcoltab;
int laststrengthframe;
int cliplx; int cliply; int cliphx; int cliphy;
int snuffctr;
fastspr_sprite charsadr[48];
int* addtabadr;
char* screennotuse;
//int rnseed;
projent* projadr;
int projctr;
int firelastframe;
int fullpitch=0x2155;
int rocketctr;
int* rockettabadr;
alent* aladr;
int alctr;
int fuelairlastframe;
int shutdownctr;
char* currentpath;
bulent* buladr;
int bulctr;
int xposmax; int yposmax;
fastspr_sprite alspradr[256];
int platypos;
colchent* oldcolchtab;
colchent* oldcolchptr;
colchent* colchadr;
colchent* colchtabuse;
colchent* colchptruse;
alent* dodgypointer;
bulcolchent* bulcolchtab;
bulcolchent* bulcolchptr;
int laststrength;
int currentzone;
int telepctr;
int telepxpos; int telepypos;
int bonusxpos; int bonusypos; int bonustimer;
int sprlx; int sprly; int sprhx; int sprhy;
int lagerctr;
char *boardlowlim, *boardhighlim;
char* storage;
char* storageend;
char* gamescreenadr;
char* chatscreenadr;
int windctr;
int xmode;

// consecutive words after vdu=&300
char *screenstart;
int modesize, hbytes;
char *screenuse, *screentop;

// consecutive words after pl=&340
int xpos, ypos, initplx, initply, hvec, vvec;
char *pladr1, *pladr2, *pladr3, *pladr4, *pladr5, *pladr6, *pladr7, *pladr8;
int pllx, plly, plhx, plhy;
char plscore[8]; // plscore=pl+72:REM Two words
int plscoreadd, plzone, pltophit, plstrength, lives, neuronctr;

#include "asylum.h"


void init()
{
storage=storearea;
storageend=storearea+STOREAREALEN;
xmode=15;
gethandlers();

// SWI "FastSpr_GetAddress";
 // set up fspplot, fspvars, fspareat=fspvars+24
 colchadr=colchofs;
 bulcolchtab=bulcolchofs;
vduread();
swi_removecursors();
bank=1;
switchbank()      ;//set up bank variables
switchfspbank();
switchbank()      ;//set up bank variables
switchfspbank();
checkifarm3();
 if (getfiles()) abort_game();
setdefaults();
loadconfig();
 vduread(); // set screen size from options

scorezero();
for (prelude()&&abort_game(); ; options(0)&&abort_game())
{
playloop:
osbyte_7c();
//if (options(0)) // not in game
//  {abort_game(); return;}
if (getlevelfiles())
  {notgotlevel: if (1) abort_game();
  // or, depending on what getlevelfiles() returned
  continue;}

 swi_bodgemusic_stop();
if (game()) continue;
if (neuronctr>=_neuronstoget)
{
zonedone:
if (idpermit!=1) permitid();
 swi_bodgemusic_start(1,0); // ?? (3,0) in original
showhighscore();
continue;
}
else // was "else if overflow clear"
{
if (soundtype==2) swi_bodgemusic_start(2,0);
swi_sound_qtempo(0x980);
swi_bodgemusic_volume(musicvol);
}
}
}

int abort_game()
{
swi_bodgemusic_stop();
losehandlers();
osbyte_7c();
SDL_Quit();
exit(0);
}

char endmes[] = "Game exited cleanly";

int game()
{
wipetexttab();
setup();
switchcolch(); //setup colch vars
startmessage();
findplayer();
getarms();
if (cheatpermit==1) zonecheatread();
do
{
zonerestart:
restartplayer();
do
{
mainrestart:
showgamescreen();
if (soundtype==2) swi_bodgemusic_start((plzone!=0),0);
swi_bodgemusic_volume(musicvol);
frameinc=1;
rate50=1;
swi_blitz_wait(1);
while (!swi_readescapestate())
{mainloop:
if (plzone != currentzone) {switchzone: loadzone(); goto zonerestart;}
//BL saveal
//BL restoreal
if ((char)rate50 != 1)
{
plmove();
bonuscheck();
fuelairproc();
switchcolch();
masterplotal=0;
moval();
project();
bullets();
alfire();
wakeupal();
}
rate50link:
plmove();
bonuscheck();
fuelairproc();
mazeplot();
switchcolch();
masterplotal=1;
if (snuffctr) playerplot();
moval();
project();
bullets();
alfire();
if (!snuffctr) playerplot();
bonusplot();
scoreadd();
showstrength();
texthandler();
seeifdead();
//makesounds();
wakeupal();
if (cheatpermit==1) cheatread();
scorewipe();
showscore();
if (xmode==49)
	swi_blitz_screenexpand(screenuse);
frameinc=((gearchange==0)?2:1);
swi_blitz_wait(frameinc);
if ((rate50!=1)&&(frameinc<2)) //rate 25 but one frame passed
{
swi_blitz_wait(1);
frameinc=2;
rate50=1;
}
else if (frameinc>1) rate50=0;
rateskip:

framectr+=frameinc;

switchbankins();
switchfspbank();
swi_blitz_smallretrieve();
if (snuffctr>=300)
{
 playerdead:
showgamescreen();
if (lives==0) return 0;
lives--;
if (lives>9) lives=9;
prepstrength();
goto zonerestart;
}
}
swi_bodgemusic_stop();
bonusreplot=4;
if (escapehandler()) return 1;
} while (1);
} while (1);
}


void switchcolch()
{
oldcolchtab=colchtab;
oldcolchptr=colchptr;
colchplace=!colchplace;
colchtab=colchptr=colchadr+(colchplace?0:_colchlim+16); //+16 for safety

bulcolchptr=bulcolchtab; //reset bullet checking table
}

void switchbank()
{
if (xmode==49) swi_blitz_screenexpand(screenuse);
switchbankins();
}

void switchbankins()
{
bank^=3;
osbyte_71(); // i.e. 0x71
swi_blitz_screenretrieve();
 SDL_Flip(ArcScreen);
screenuse=screenstart;
screennotuse=screenstart;
if (bank==1) screenuse=screenstart+modesize;
else screennotuse=screenstart+modesize;
}

void switchfspbank() {swi_fastspr_screenbank(bank);}

void bidforsoundforce(int r0,char r1,char r2,int r3,int r4,int r5,char r6,int r7,Mix_Chunk* chunk)
{
if (sound_available&&soundtype)
{
r0&=7;
soundclaim(r0,r1,r2,r3,r4,r5,r6,r7,chunk);
}
}

void bidforsound(int r0,char r1,char r2,int r3,int r4,int r5,char r6,int r7,Mix_Chunk* chunk)
{
if (sound_available&&soundtype)
{
r0&=7;
//soundtab=soundtabofs+(r0<<soundtabshift);
 if ((r0==_Explochannel)/*&&(soundtype==3)*/)
  goto bidforexplo;
if ((!Mix_Playing(r0))||(Mix_GetChunk(r0)->volume<r2)) soundclaim(r0,r1,r2,r3,r4,r5,r6,r7,chunk);
return;
bidforexplo:
if ((!Mix_Playing(3))||(Mix_GetChunk(3)->volume<r2)) soundclaim(3,r1,r2,r3,r4,r5,r6,r7,chunk);
else if ((!Mix_Playing(4))||(Mix_GetChunk(4)->volume<r2)) soundclaim(4,r1,r2,r3,r4,r5,r6,r7,chunk);
else if ((!Mix_Playing(5))||(Mix_GetChunk(5)->volume<r2)) soundclaim(5,r1,r2,r3,r4,r5,r6,r7,chunk);
else if ((!Mix_Playing(6))||(Mix_GetChunk(6)->volume<r2)) soundclaim(6,r1,r2,r3,r4,r5,r6,r7,chunk);
else if ((!Mix_Playing(7))||(Mix_GetChunk(7)->volume<r2)) soundclaim(7,r1,r2,r3,r4,r5,r6,r7,chunk);
return;
}
}

void showtext()
{
texthandler();
switchbank();
switchfspbank();
}

void texthandler()
{
textinfo* r11=texttabofs;
int r9=_textno;
for (;r9>0;r11++,r9--)
  {
  loopa6: if (r11->count==0) continue;
 textproc:;
int r4=(frameinc>4)?4:frameinc;
r11->count-=r4; if (r11->count<0) r11->count=0;

for (;r4>0;r4--)
{
loopb1:
r11->x += r11->dx;
r11->y += r11->dy;
}
int XxX = r11->x>>8;
int YyY = r11->y>>8;
for (char* r10=r11->text;*r10!=0;r10++)
{
  char r0;
loopa7:
r0=*r10;
fspplot(charsadr,*r10-1,XxX,YyY);
XxX+=14;
if (*r10<=10) XxX+=2;
if (*r10>43) XxX-=6;
}
}
textdone:;
//textdelete: r11->count = 0;
}

void startmessage() {message(1128,136,0,0,"Let's Go!");}

void deathmessage()
{
message(72,208,0,-1,"¤ Snuffed It! ¤");
if (lives==0) message(72,256,0,-1,"-  GAME OVER  -");
}

void alfire()
{
int x = xpos + (((random()&15)+(random()&7)-11)<<12);
int y = ypos + (((random()&15)-7)<<12);
if (x<(1<<12)) return;
if (y<(1<<12)) return;
if (x>xposmax-(1<<12)) return;
if (y>yposmax-(1<<12)) return;
x &= ~0xfff; y &= ~0xfff;
char* r0=translate(x,y);
if ((*r0>=_targetlowlim)&&(*r0<=_targethighlim))
	{foundtarget(x,y,*r0); return;}
nofoundtarget:
if ((r0[1]>=_targetlowlim)&&(r0[1]<=_targethighlim))
	{foundtarget(x+(1<<12),y,r0[1]); return;}
return;
}

void foundtarget(int x,int y,char target)
{
y+=16<<8;
int dx=(xpos-x)>>6;
int dy=((ypos+(12<<8))-y)>>6; // aim at body
char* r0=translate(x+(8<<8)+(dx>>4),y-(8<<8)+(dy>>4));
if (*r0>_targethighlim) return;
target&=~3;
int r4=random()&7;
if (r4==7) r4=6;
if (target>=_powertarget) r4-=6;
if (target==_powertarget) r4=8+(r4&2);
if (target==_nuttertarget) r4=12+(r4&3);
makebul(x,y,dx,dy,_bulspritebase+r4,1<<24);
}

int splittab[14];

void init_splittab() {
  for (int i=0;i<7;i++) {
    splittab[i*2] = (int)(0x200*sin((0.5+i)/3.5*PI));
    splittab[i*2+1] = (int)(0x200*cos((0.5+i)/3.5*PI));
  }}

void bullets()  // the bullet handler (aliens fire these)
{

bulent* r11 = buladr;
int i;
for (i=_bulno;i>0;i--,r11++)
{
	loop71:
	if (r11->type==0) continue;
foundbul:

r11->x += r11->dx;  r11->y += r11->dy;

if ((r11->x<=xlowlim) || (xposmax<=r11->x) || (r11->y<=ylowlim) || (yposmax<=r11->y))
{
	// goto projoffscr
	r11->type=0;
	continue;
}

if ((r11->dx<=(1<<12)) && (-r11->dx<=(1<<12)))
	if (r11->flags & (1<<14))
	{
		r11->dx += (r11->dx>>4);
		r11->dy += (r11->dy>>4);
	}
bulnoacc:


if ((r11->flags & (1<<13)) && (r11->flags <= (3<<22)))
{
	int r6 = 1<<6;
	if (!(r11->flags & (1<<12)))
	{
		r6 = 1<<4;
		r11->dx -= (r11->dx>>5);
		r11->dy -= (r11->dy>>5);
	}
	r11->dx += (r11->x<xpos)?r6:-r6;
	r11->dy += (r11->y<ypos)?r6:-r6;
}
bulnohome:

if (r11->dx>_speedlim)  r11->dx = _speedlim;
if (r11->dx<-_speedlim)  r11->dx = -_speedlim;
if (r11->dy>_speedlim)  r11->dy = _speedlim;
if (r11->dy<-_speedlim)  r11->dy = -_speedlim;

r11->flags -= (1<<16);  /* decrement the life counter */

if (r11->flags<0)
{
// out of time
// goto buldestroy
if ((r11->flags) & (1<<10)) goto bulsplit;
r11->type = 0;
continue;
}

if (!((r11->x<=pllx) || (plhx<=r11->x) || (r11->y<=plly) || (plhy<=r11->y)))
{
	// goto bulletkill;
	bulletkill:
	r11->type=0;
	// goto causeexplobullet;
int DX=r11->dx>>4, DY=r11->dy>>4;
explogonopyroquiet(r11->x-r11->dx,r11->y-(8<<8)-r11->dy,DX+DY,DY-DX,0,DX+DY,0);
explogonopyro(r11->x-r11->dx,r11->y-(8<<8)-r11->dy,DX-DY,DX+DY,0,DX-DY,0);
continue;
}
{
char r1=*fntranslate(r11->x,r11->y);
if ((r1 >= 16) // hit a block
    //     bulhit:
    && !((r1>=_translowlim) && (r1<=_transhighlim))
    && !((r1>=_targetlowlim) && (r1<=_targethighlim)))
     ;
     else
{
bulhitins:

if (masterplotal == 0) continue;

int r0=(r11->type);
if ((r0==_bulspritebase+8) || (r0==_bulspritebase+10))
	r0 ^= (framectr & 4) >> 2;
fspplot(exploadr,r0,((r11->x)>>8)-(xpos>>8)+_xofs,((r11->y)>>8)-(ypos>>8)+_yofs);
continue;
}
}
nobultarget:
r11->type=0;
bulnodestroy:
if ((r11->flags && (1<<15))!=0)
{
// causeexplobullet:
int DX=r11->dx>>4, DY=r11->dy>>4;
explogonopyroquiet(r11->x-r11->dx,r11->y-(8<<8)-r11->dy,DX+DY,DY-DX,DX+DY,0,0);
explogonopyro(r11->x-r11->dx,r11->y-(8<<8)-r11->dy,DX-DY,DX+DY,DX-DY,0,0);
continue;
}
buldestroy:
if ((r11->flags && (1<<10))==0)
{
	r11->type = 0;
	continue;
}
bulsplit:;
int newtype=(random()&7);
if (newtype==7)  newtype=0;
int r7=r11->type;
if (r7==_bulspritebase+13)  newtype=7;
if (r7==_bulspritebase+14)  newtype=10;
if (r7==_bulspritebase+15)  newtype=12+(newtype&3);
if (newtype==15)  newtype=0;
newtype+=_bulspritebase;

for (int i=0;i<14;i+=2)
{
	loop75:
	(void)makebul(r11->x,r11->y,(r11->dx>>2)+splittab[i],(r11->dy>>2)+splittab[i+1],newtype,(1<<22));
}
r11->type=0;
}
}



int makebul(int x,int y,int dx,int dy,int type,int flags)
{

int i;
bulent* r10 = buladr;
r10 += bulctr;
for (i=bulctr;i<_bulno;i++,r10++)
{
	loop72:
	if (r10->type==0) {bulctr=i; break;}
}
if (i==_bulno)
{
r10 = buladr;
for (i=0;i<bulctr;i++,r10++)
{
	loop69:
	if (r10->type==0) {bulctr=i; break;}
}
if (i==bulctr) return 1; // failed
}
foundmakebul:

if (flags<(1<<16))  flags |= (1<<22);
if (type>=_bulspritebase+8)  flags |= (9<<12);
if (type>=_bulspritebase+12)   flags = (1<<21) | (1<<10);
if ((type==_bulspritebase+7)||(type==_bulspritebase+8))  flags |= (1<<13);
if (type==_bulspritebase+10)  flags |= (1<<14);

r10->type = type;
r10->x = x;
r10->y = y;
r10->dx = dx;
r10->dy = dy;
r10->flags = flags;

bulctr = (bulctr+1) % _bulno;  // loop68:

// if (bulctr==0)  return; // XXX why???

int r7 = (r10->x - xpos) >> 9;
x=abs(r10->x - xpos);
y=abs(r10->y - ypos);
int vol = (0x7f - ((x+y)>>12));
if (vol>=80)
{
bidforsound(_Explochannel,(type>=_bulspritebase+8)?_Sampbigzap:_Sampsmallzap,
	(((vol&0x7f)>0x7c)?0x7c:(vol&0x7f))-((type==_bulspritebase+8)?0x10:0),
	(type==_bulspritebase+8)?(fullpitch+0x1800):(fullpitch+0x1000),
	0,(fullpitch<<16)|(0xfe<<8),2,r7,   // lifetime (frames)
	    (type>_bulspritebase+8)?CHUNK_BULLET_3:(type==_bulspritebase+8)?CHUNK_BULLET_2:CHUNK_BULLET_1);
}
nozapsound:;
}


void project()  // the projectile handler
{
int i = _projno;  // get table length
projent* r11 = projadr;
for (;i>0;i--,r11++)
{
	loop41:
	if (r11->type==0) continue;
foundproj:
r11->x+=r11->dx; r11->y+=r11->dy;
if ((r11->x<=xlowlim)||(xposmax<=r11->x)||(r11->y<=ylowlim)||(yposmax<=r11->y))
{
	// goto projoffscr
	r11->type=0;
	continue;
}
if (r11->flags&(1<<15))
	{r11->dx+=r11->dx>>4;  r11->dy+=r11->dy>>4;}

projnoacc:
if (r11->dx>_speedlim)  r11->dx=_speedlim;
if (r11->dx<-_speedlim)  r11->dx=-_speedlim;
if (r11->dy>_speedlim)  r11->dy=_speedlim;
if (r11->dy<-_speedlim)  r11->dy=-_speedlim;

r11->flags-=1<<16; // decrement the life counter
if (r11->flags<0)  // out of time
{
projdestroy:
  if (r11->flags&(1<<10))  {projsplit(r11); continue;}
projoffscr:
r11->type=0;
continue;
}
alent* rs=bulcolcheck(r11->x,r11->y);
if (rs!=NULL)
{
projhital:
if (r11->flags&(1<<15))
	rs->r6 -= (bulletloss<<2);
else rs->r6 -= bulletloss;
r11->type=0;
if (rs->r6>0) continue;
int r0=rs->type;
int r6=(random()&7)+((r0>=_Alien1)?(r0-_Alien1):0)+_bonuslow;
if (r6>_bonushigh) r6=_bonushigh-3;
if (r6<_bonuslow) r6=_bonuslow;
makeobj(_Flyingbonus,r11->x,r11->y,r11->dx>>2,-(1<<10),(0x200<<16),r6);
if (r11->flags & (1<<15)) causeexplo(r11);
else causeexplonopyro(r11);
continue;
}

char* r0=fntranslate(r11->x,r11->y);
char r1=*r0;
if ((r1<16)||
    //    projhit:  // hit a block
    ((r1>=_translowlim)&&(r1<=_transhighlim)&&!(r11->flags&(1<<9))))
{
projhitins:
if (masterplotal==0) continue;
fspplot(blokeadr,r11->type,_xofs+(r11->x>>8)-(xpos>>8),_yofs+(r11->y>>8)-(ypos>>8));
continue;
}
projhitcont:
if ((r1>=_spcrumblelowlim)&&(r1<=_spcrumblehighlim)&&(plweapontype==5))
{
	*r0=0;
	explogo(r11->x,r11->y,0,0,0,0,0);
}
nospcrumble:
r11->type=0;
destroy(r0);
if (r11->flags&(1<<9))  {atomrocket(r11,r0); continue;}
if (r11->flags&(1<<11))  {causeexplonopyro(r11); continue;}
if (r11->flags&(1<<15))  {causeexplo(r11); continue;}
}
}


void atomrocket(projent* r11,char* r0)
{
char r1=*r0;  /* switched r1 and r0 from ARM */
 if ((r1>=_solidlowlim)&&(r1<=_solidhighlim))  {causeexplo(r11); return;}
atomdest:
if (((r1>=_eleclowlim)&&(r1<=_elechighlim))
	||((r1>=_targetlowlim)&&(r1<=_targethighlim)))
{
	elecatom:
	elecdestroy(r0);
}
noelecatom2:
 if ((r1>=_neuronlowlim)&&(r1<=_neuronhighlim))  {causeexplo(r11); return;}
notreasatom:
*r0=0;
causeexplo(r11);
}

void causeexplo(alent* r11) {explogomaybe(r11->x-r11->dx,r11->y-r11->dy-(8<<8),0,0,1,-1,0);}
void causeexplo(projent* r11) {explogomaybe(r11->x-r11->dx,r11->y-r11->dy-(8<<8),0,0,1,-1,0);}

void causeexplonopyro(alent* r11) {explogonopyro(r11->x-r11->dx,r11->y-r11->dy-(8<<8),0,0,1,-1,0);}
void causeexplonopyro(projent* r11) {explogonopyro(r11->x-r11->dx,r11->y-r11->dy-(8<<8),0,0,1,-1,0);}

int projsplittab[10];

void init_projsplittab() {
  for (int i=0;i<5;i++) {
    projsplittab[i*2] = (int)(0x40*(0.5-sin(i/4.0*PI)));
    projsplittab[i*2+1] = 0x100*(i-2);
  }}

void projsplit(projent* r11)
{
int x=r11->x, y=r11->y, dx=(r11->dx)>>1, dy=(r11->dy)>>1;
int flags=r11->flags;
int* r10;
int r9;
if (flags&(1<<15)) {rocketsplit(r11); return;}

int r7=r11->type, r4, r6;

if (flags&(1<<14)) { r10=projsplittab; r9=5; r4=_projsmallno+1;}
else { r10=projsplittab+2; r9=3; r4=_projsmallno;}
if ((flags&(1<<13))==0)   dx>>=1;
if (flags&(1<<12))  { r4=_projsmallno+2; dx<<=2;}
if (flags&(1<<11))  { r4=_projsmallno+3; r6=1<<15;}
else r6=1<<22;

for (;r9>0;r9--)
{
loop76:
makeproj(x,y,dx+r10[0],dy+r10[1],r4,r6);
r10+=2;
}
r11->type=0;
}

int rocketbursttab[10];

void init_rocketbursttab() {
  for (int i=0;i<5;i++) {
    rocketbursttab[i*2] = (int)(0x200*sin((0.5+i)/2.5*PI));
    rocketbursttab[i*2+1] = (int)(0x200*cos((0.5+i)/2.5*PI));
  }}

void rocketsplit(projent* r11)
{
int x=r11->x, y=r11->y, dx=(r11->dx)>>3, dy=(r11->dy)>>3;
int flags=r11->flags;
if (flags&(1<<14)) {rocketpair(r11); return;}
 rocketburst:;

int r5=(1<<22)|(1<<15)|(1<<11);
int* r10=rocketbursttab;
for (int r9=5;r9>0;r9--)
{
loop77:
makeproj(x,y,dx+r10[0],dy+r10[1],_projsmallno+3,r5);
r10+=2;
}
r11->type=0;
causeexplonopyro(r11);
}

void rocketpair(projent* r11)
{
int x=r11->x, y=r11->y, dx=r11->dx, dy=r11->dy;
int flags=r11->flags;
int type=r11->type;
int newflags=1<<22;
if (flags&(1<<12))  newflags=(1<<20)|(5<<10);
if (flags&(1<<13))  // continue splitting
	{ newflags&=~(1<<22); newflags|=(1<<20)|(17<<10);}
newflags|=(1<<15);

int newtype=type-2;
if (newtype<_rocketspriteno) newtype+=2;

if (type&1) makeproj(x,y,dx-(dy>>2),dy+(dx>>2)-(dy>>4),newtype,newflags);
else makeproj(x,y,dx+(dy>>2),dy-(dx>>2)+(dy>>4),newtype,newflags);

newtype=type+2;
if (newtype>_rocketspritelim) newtype-=2;

if (type&1) makeproj(x,y,dx+(dy>>2),dy-(dx>>2)+(dy>>4),newtype,newflags);
else makeproj(x,y,dx-(dy>>2),dy+(dx>>2)-(dy>>4),newtype,newflags);

r11->type=0;
}

int makeproj(int x,int y,int dx,int dy,int type,int flags)
{
projent* r10=projadr+projctr;
int r9=_projno-projctr;
int r8=projctr;
for (;r9>0;r9--)
{
loop42:
if (r10->type==0)  return foundmakeproj(r10,r8,x,y,dx,dy,type,flags);
r10++;
r8++;
}
r10=projadr;
for (r9=projctr;r9>0;r9--)
{
loop43:
if (r10->type==0)  return foundmakeproj(r10,r8,x,y,dx,dy,type,flags);
r10++;
r8++;
}
return 1;
}

int foundmakeproj(projent* r10,int r8,int x,int y,int dx,int dy,int type,int flags)
{
if (flags<(1<<16)) flags|=(1<<22);
r10->type=type; r10->x=x; r10->y=y;
r10->dx=dx; r10->dy=dy; r10->flags=flags;

for (r8++;r8>=_projno;r8-=_projno)
{
loop45:;
}
projctr=r8;
return 0;
}

int softmakeobj(int r0,int r1,int r2,int r3,int r4,int r5,int r6)
{
alent* r10=aladr;
int r9=_alno;
if (alctr>=r9)  alctr=0;
r9-=alctr;
if (r9<10) /* check 10 object spaces */
{
	alctr=0;
	r10=aladr;
}
else r10+=alctr;
do {
softloop:
if (r10->type==0) return foundmakeal(r10,alctr,r0,r1,r2,r3,r4,r5,r6);
r10++;
alctr++;
} while (--r9>0);
return 1;
}

int makeobj(int r0,int r1,int r2,int r3,int r4,int r5,int r6)
{
int tmpalctr=alctr;
alent* r10=aladr;
int r9=_alno;
if (tmpalctr>=r9)  tmpalctr=0;
r9-=tmpalctr;
r10+=tmpalctr;
do {
loop52:
if (r10->type==0) return foundmakeal(r10,tmpalctr,r0,r1,r2,r3,r4,r5,r6);
r10++;
tmpalctr++;
} while (--r9>0);
r10=aladr;
r9=alctr;
do {
loop53:
if (r10->type==0) return foundmakeal(r10,tmpalctr,r0,r1,r2,r3,r4,r5,r6);
r10++;
tmpalctr++;
} while (--r9>0);
return 1;
}

int foundmakeal(alent* r10,int newalctr,int r0,int r1,int r2,int r3,int r4,int r5,int r6)
{
r10->type=r0;
r10->x=r1; r10->y=r2; r10->dx=r3; r10->dy=r4;
r10->r5=r5; r10->r6=r6;

alctr=newalctr+1;
if (alctr>=_alno)  alctr=0;
return 0;
}

void seestars()
{
if (plstrength>=laststrength)
{
	laststrength=plstrength;
	return;
}
laststrength-=1<<8;
int r2, r1=(framectr>>1)&7;
 int r0=_starssprbase+r1;
if ((r1&3)==0)  r2=90+plotterofs;
else if (r1>4)  r2=89+plotterofs;
else            r2=91+plotterofs;
fspplot(blokeadr,r0,160,r2);

int r4=laststrength-plstrength;
if (r4==0)  return;
if (electrocuting)
{
	elecsound:
	bidforsound(_Playerchannel,_Samprave,0x7f,0x2000,(0xff)<<8,0,10,0,CHUNK_ELEC_1);
	bidforsound(_Firechannel,_Samprave,0x7f,0x2400,(0xff)<<8,0,10,127,CHUNK_ELEC_2);
	bidforsound(_Sparechannel,_Samprave,0x7f,0x2800,(0xff)<<8,0,10,-127,CHUNK_ELEC_3);
}
else bidforsound(_Playerchannel,_SampStunned,0x70,0x4000-((r4>0x1000)?0x1000:r4),
		 0,0,50,0,CHUNK_STUNNED[(r4>=0x1000)?16:((r4<0)?0:(r4>>8))]);
}


void seeifdead()
{
if ((plstrength>0)||(snuffctr>0))  return;
snuffctr=1;

makeobj(_Decoration,1<<8,(2<<8)-(12<<8),0,-(2<<8),_deadsprbase-3+(250<<16),24);
makeobj(_Decoration,(1<<8)-(5<<8),2<<8,-(1<<8),-(3<<8),_deadsprbase+1+(250<<16),24);
makeobj(_Decoration,(1<<8)+(6<<8),2<<8,1<<8,-(3<<8),_deadsprbase+2+(250<<16),24);

deathmessage();
return;
}

void wipearea(int r0,int r1,int r2,int r3)
{
int ro=r1*hbytes;
int rz=r3*hbytes;
SDL_LockSurface(ArcScreen);
char* r11=wipescrst;
char* r10=screenuse+r0*4+ro-(rz>>1) /* R3 is a multiple of 2 */
                         -((r2*4)>>1);/* R2 is a multiple of 8 */
for (int r9=16;r9>0;r9--)
{
wipetest:
memcpy(r10,r11,128*4);
r11+=128*4;
r10+=hbytes;
}
SDL_UnlockSurface(ArcScreen);
}

void wipearearead(int r0,int r1,int r2,int r3)
{
int ro=r1*hbytes;
int rz=r3*hbytes;
SDL_LockSurface(ArcScreen);
char* r11=wipescrst;
char* r10=screenuse+r0*4+ro-(rz>>1) /* R3 is a multiple of 2 */
                         -((r2*4)>>1);/* R2 is a multiple of 8 */
for (int r9=16;r9>0;r9--)
{
wipetest2:
memcpy(r11,r10,128*4);
r11+=128*4;
r10+=hbytes;
}
SDL_UnlockSurface(ArcScreen);
}

void scorewipe() {wipearea(160,_scoreyofs,16*8,16);}

void scorewiperead() {wipearearead(160,_scoreyofs,16*8,16);}

void showscore()
{
  releaseclip();
  int i=0;
  int x=_scorexofs;
  int y=_scoreyofs;
  
  if (plscore[0]==0)
    {
      x+=_charwidth/2;
      i=1;
      if (plscore[1]==0)
	{
	  x+=_charwidth/2;
	  i=2;
	}
    }
 score10mil:
  /*r7=0*/;
  
  for (;i<8;i++)
    {
    loop36:
      if (plscore[i]<=10)
	{
	  fspplot(charsadr,plscore[i],x,y);
	  /*r8=1*/;
	}
    scoreskip:
      x+=_charwidth;
    }
}

void scoreadd()
{
if (!plscoreadd) return;
char* r10=plscore;
int* r11=addtabadr;
for (int r6=8;r6>0;r6--)
{
int placeval=*(r11++);
loop37:
if (plscoreadd>=placeval)
{
char digit=0;
if (plscoreadd>=(placeval<<3)) {digit+=8; plscoreadd-=(placeval<<3);}
if (plscoreadd>=(placeval<<2)) {digit+=4; plscoreadd-=(placeval<<2);}
if (plscoreadd>=(placeval<<1)) {digit+=2; plscoreadd-=(placeval<<1);}
if (plscoreadd>=(placeval<<0)) {digit+=1; plscoreadd-=(placeval<<0);}
*r10+=digit;
}
scoreaddskip:
r10++;
}
char carry=0;
for (int r6=8;r6>0;r6--) // now handle carries
{
loop38:
r10--;
*r10+=carry;
if (*r10>=10) {*r10-=10; carry=1;}
else carry=0;
}
}


void showstrength()
{
SDL_LockSurface(ArcScreen);
if (lagerctr!=0)
{
if ((lagerctr-=frameinc)<0)  lagerctr=0;
plstrength+=frameinc<<6;
if (plstrength>_strengthinit)  plstrength=_strengthinit;
laststrength+=frameinc<<6;
if (laststrength>_strengthinit)  laststrength=_strengthinit;
}
 nolager:;
Uint32* r10=((Uint32*)screenuse)+_strengthxofs+(hbytes/4)*_strengthyofs;
int r3=plstrength;
if (r3>_strengthmax)  r3=_strengthmax;
if (r3<0)  r3=0;
r3>>=8;
int r7=0;
loop31:
for (;r3>0;r3-=1)
{
Uint32* r4=r10;
loop32:
for (int r6=6;r6>0;r6--)
{
  *r4=*(strengthcoltab+(random()&0x1f));
r4+=hbytes/4;
}
r10++;
r7++;
}

writeblue:
r10=((Uint32*)screenuse)+_strengthxofs+(hbytes/4)*_strengthyofs;
r10+=(spstrengthmax>>8);
if (framectr-laststrengthframe>2)  laststrengthframe=framectr;

loop35:
for (;r7<=spstrengthmax>>8;)
{
Uint32* r4=r10;
loop34:
for (int r6=6;r6>0;r6--)
{
  *r4=*(strengthcoltab+0x24+(random()&0x1f));
r4+=hbytes/4;
}
r10--;
r7++;
}

int mask;
writemaskedlife:
mask=0;
if (r3>=-3)  mask+=0xff;
if (r3>=-2)  mask+=0xff00;
if (r3>=-1)  mask+=0xff0000;
if (r3<0) 
{
Uint32* r4=r10;
for (int r6=6;r6>0;r6--)
{
loop33:
*r4=(*(strengthcoltab+0x24+(random()&0x1f))&~mask)|((*r4)&mask);
r4+=hbytes/4;
}
}
SDL_UnlockSurface(ArcScreen);
return;
}


void fuelairproc()
{
if (framectr<fuelairlastframe+((_fuelairduration-fueltabctr)>>1)) return;
fuelairlastframe=framectr;

char **ftr, **ftw;
ftr=fueltabread; fueltabread=ftw=fueltabwrite; fueltabwrite=ftr;

if (--fueltabctr<0)  {*ftw=0;  return;}

for (char** r7=ftw+_fueltablim;r7>ftw;)
{
 loop22:;
char* r8=*(ftr++);
if (r8==NULL)  {*ftw=0;  return;}
char r0=(*r8)&~1;
if (r0!=_gaslowlim) skipfuelair: continue;
char* r1=r8-1;
if (*r1==0)  {*r1=_gaslowlim; *(ftw++)=r1;}
r1=r8+boardwidth;
if (*r1==0)  {*r1=_gaslowlim; *(ftw++)=r1;}
r1=r8+1;
if (*r1==0)  {*r1=_gaslowlim; *(ftw++)=r1;}
r1=r8-boardwidth;
if (*r1==0)  {*r1=_gaslowlim; *(ftw++)=r1;}
}
fuelprocdone:
*ftw=0;
}


void explogonopyro(int r1,int r2,int r3,int r4,int r5,int r6,alent* r10) {explocreate(r1,r2,r3,r4,r5,r6,r10);}

void explogonopyroquiet(int r1,int r2,int r3,int r4,int r5,int r6,alent* r10) {explocreatequiet(r1,r2,r3,r4,r5,r6,r10);}

void explogoquiet(int r1,int r2,int r3,int r4,int r5,int r6,alent* r10)
{
explocreatequiet(r1,r2,r3,r4,r5,r6,r10);
embercreate(r1,r2,r6);
}

void explogomaybe(int r1,int r2,int r3,int r4,int r5,int r6,alent* r10)
{
explocreate(r1,r2,r3,r4,r5,r6,r10);
if ((random()&3)==0) embercreate(r1,r2,r6);
}

void explogo(int r1,int r2,int r3,int r4,int r5,int r6,alent* r10)
{
explocreate(r1,r2,r3,r4,r5,r6,r10);
embercreate(r1,r2,r6);
}

void explocreate(int r1,int r2,int r3,int r4,int r5,int r6,alent* r10)
{
if (r10!=NULL)  {r1=r10->x;  r2=r10->y;}
int vol=0x7f-((abs(r1-xpos)+abs(r2-ypos))>>12);
if (vol>=80)
{
bidforsound(_Explochannel,_SampExplo,((vol&0x7f)>0x7c)?0x7c:(vol&0x7f),
	//   channel,        , vol,
	0x3800,0,0,10,((r1-xpos)>>9),CHUNK_EXPLO);
	//pitch, , ,lifetime (frames)
}
noexplosound:
explocreatequiet(r1,r2,r3,r4,r5,r6,r10);
}


void explocreatequiet(int r1,int r2,int r3,int r4,int r5,int r6,alent* r10)
{
if (r10==NULL) //(r10<8)
{
noobject:
softmakeobj(_Explo,r1,r2+(8<<8),r3,r4,(r5!=0)?(1<<15):0,r6);
}
else
{
r10->type=_Explo;
r10->dx=r3; r10->dy=r4;
r10->r5=0;
}
noobjectins:;
}

void embercreate(int r1,int r2,int r6)
{
r2+=(8<<8); // The original code achieves this by modifying r2
            // inside explocreatequiet.  Arrgh.
 softmakeobj(_Ember,r1,r2,1<<8,-(1<<8),0,r6);
 softmakeobj(_Ember,r1,r2,-(1<<8),-(1<<8),0,r6);
}

void atomexplogo(int r1,int r2,int r3,int r4,int r5,int r6,alent* r10)
{
explogoquiet(r1,r2,r3,r4,r5,r6,r10);
explogoquiet(r1,r2-(16<<8),r3,-(1<<6),r5,r6,r10);
explogoquiet(r1,r2-(32<<8),r3,-(1<<7),r5,r6,r10);
explogoquiet(r1+(12<<8),r2-(24<<8),1<<6,-(3<<5),r5,r6,r10);
explogoquiet(r1-(12<<8),r2-(24<<8),-(1<<6),-(3<<5),r5,r6,r10);
int r7=(r1-xpos)>>9;
r1=0x8f-((abs(r1-xpos)+abs(r2-ypos))>>12);
if (r1>0x7f) r1=0x7f;
if (r1>=60)
{
/* AND R2,R1,#&7F:MOV R2,#&7F ??? */
bidforsound(_Explochannel,_SampAtomExplo,0x7f,0x2800,0,0,50,r7,CHUNK_ATOM);
}
noatomsound:
return;
}


void screenwakeup()
{
char* r7=translate(xpos,ypos)-_boxwidth-(boardwidth<<4);
char* r8=boardadr->contents; //set up outer limits: start
char* r9=r8+boardadr->width*boardadr->height;   // end
for (int r5=_boxheight*2;r5>0;r7+=boardwidth,r5--)
{
 loopa4:
linecheck(r7,r8,r9);
}
}

void linecheck(char* r7,char* r8,char* r9)
{
  for (int r4=_boxwidth*2;r4>0;r4--)
{
 loopa3:
  if ((r7>r8)&&(r9>r7)&&(*r7>_triggerlim))
    dowakeup(r7);
  r7++;
}
}

void wakeupal()
{
char* r7=translate(xpos,ypos)-(_boxwidth/2)-(boardwidth<<3);
char* r8=boardadr->contents; //set up outer limits: start
char* r9=r8+boardadr->width*boardadr->height;   // end
boxcheck(_boxwidth,1,&r7,r8,r9);
boxcheck(_boxheight,boardwidth,&r7,r8,r9);
boxcheck(_boxwidth,-1,&r7,r8,r9);
boxcheck(_boxheight,-boardwidth,&r7,r8,r9);
}

void boxcheck(int r4,int r5,char** r7,char* r8,char* r9)
{
for (;r4>0;r4--)
{
 loop80:
  if ((*r7>r8)&&(r9>*r7)&&(**r7>_triggerlim))
    dowakeup(*r7);
  *r7+=r5;
// badalmarker: *(*r7-r5)=0;
 wakeinsert:;
}
}


void dowakeup(char* r7)
{
char r0=*r7;
int r1,r2;
if ((r0<240)||(r0>253)) return;
// r7-=r5;  not for me
*r7=0;
backtranslate(r7,&r1,&r2);
r0=r0-(240-_Alien1);
makeobj(r0,r1+(8<<8),r2+(7<<8),0,0,0,(r0<<9)-((_Alien1-1)<<9));
}

const int _savevalid = 0x4b4f6349;

void saveal()
{
bonusstore=bonusctr;
plweaponstore=plweapontype;

int* r10=saveareaints;
*(r10++)=_savevalid;
*(r10++)=xpos; *(r10++)=ypos;
*(r10++)=initplx; *(r10++)=initply;
*(r10++)=hvec; *(r10++)=vvec;

alent* r10b=saveareaalents;
alent* r7=r10b+_savearealen;
alent* r11=aladr;
for (int r9=_alno; r9>0; r9--)
  {
  procsaveal:
  if (r10b>=r7)  break;
  switch(r11->type)
    {
    case _Explo:  //don't save these
    case _Scoreobj:
    case _Flyingbonus:
    case _Dyingbonus:
    case 0:
      r11++; break;
    default:
      *r10b=*r11;
      r10b++;
      r11++;
    }
  }

r10b->type=-1; // end marker
}

void restoreal()
{
bonusctr=bonusstore;
plweapontype=plweaponstore;
wipealtab();
int* r11=saveareaints;
if (_savevalid!=*r11)  return;
*(r11++)=0;
xpos=*(r11++); ypos=*(r11++);
initplx=*(r11++); initply=*(r11++);
hvec=*(r11++); vvec=*(r11++);
alent* r10=aladr;
alent* r11b=saveareaalents;
for (;r11b->type!=-1;)
{
sl1:
*(r10++)=*(r11b++);
}
restored:;
}

void moval()
{
//BL writeclip - active for release version
if (extending) extending--;
fuelexploctr=extending;  /* ?!?! */
sprlx=xpos-_windowxsize;
sprhx=xpos+_windowxsize;
sprly=ypos-_windowysize;
sprhy=ypos+_windowysize;

alent* r11=aladr;
for (int r9=_alno; r9>0; r9-=8)
{
l8:
if (r11->type) procal(r11); r11++;
if (r11->type) procal(r11); r11++;
if (r11->type) procal(r11); r11++;
if (r11->type) procal(r11); r11++;
if (r11->type) procal(r11); r11++;
if (r11->type) procal(r11); r11++;
if (r11->type) procal(r11); r11++;
if (r11->type) procal(r11); r11++;
}
return;
}

void procal(alent* r11)
{
if ((r11->x<=xlowlim)||(xposmax<=r11->x)||(r11->y<=ylowlim)||(yposmax<=r11->y))
{
aloffscr:
if (r11->type!=_Decoration)  /* ??? */
{
r11->type=0;
return;
}
}
noaloffscr:
if ((r11->x>=sprlx)&&(sprhx>=r11->x)&&(r11->y>=sprly)&&(sprhy>=r11->y))
	plotal=masterplotal;
else plotal=0;
switch ((r11->type)&0x1f)
{
aljumptab:
t:
case 1: explo(r11); return;
case 2: ember(r11); return;
case 3: plat1(r11); return;
case 4: plat2(r11); return;
case 5: 
case 6: plat3(r11); return;
case 7: 
case 8: 
case 9: 
case 10: plat4(r11); return;
case 11: plat5(r11); return;
case 12: scoreobj(r11); return;
case 13: dyingbonus(r11); return;
case 14: flyingbonus(r11); return;
case 15: booby(r11); return;
case 16: decoration(r11); return;
case 17: extender(r11); return;
case 18: alien1(r11); return;
case 19: alien2(r11); return;
case 20: alien3(r11); return;
case 21: alien4(r11); return;
case 22: alien5(r11); return;
case 23: alien6(r11); return;
case 24: alien7(r11); return;
case 25: alien8(r11); return;
case 26: alien9(r11); return;
case 27: alien10(r11); return;
case 28: alien11(r11); return;
case 29: alien12(r11); return;
case 30: alien13(r11); return;
case 31: alien14(r11); return;
case 0:;
}
aljerr:
return;
}

void alienwander(alent* r11,char* r5)
{
if ((righthit==1)||(lefthit==1)) almightjump(r11);
else if (*r5==0) alpossjump(r11);
else if (*(r5+1)==0) alpossjump(r11);
else if ((random()&0x3f)==0) almightjump(r11);
else almightjumpins(r11);
}

void almightjumpins(alent* r11)
{
if (((r11->dx)>(1<<5))||((r11->dx)<-(1<<5))
	||((r11->dy)>(1<<7))||((r11->dy)<-(1<<7))) return;
alienstopped(r11);
}

void jumpyalwander(alent* r11)
{
if ((righthit==1)||(lefthit==1)) almightjump(r11);
else if (((r11->dx)>(1<<5))||((r11->dx)<-(1<<5))
	||((r11->dy)>(1<<7))||((r11->dy)<-(1<<7))) return;
else alienstopped(r11);
}

void alienwanderfly(alent* r11)
{
if ((random()&0xff)==0) alienstoppedfly(r11);
if (((r11->dx)>(1<<5))||((r11->dx)<-(1<<5))
	||((r11->dy)>(1<<5))||((r11->dy)<-(1<<5))) return;
alienstoppedfly(r11);
}

void alienwandernojump(alent* r11)
{
if (((r11->dx)>(1<<5))||((r11->dx)<-(1<<5))
	||((r11->dy)>(1<<8))||((r11->dy)<-(1<<8))) return;
alienstopped(r11);
}

void alienstopped(alent* r11)
{
if ((downhit==1)||(alonobj==1))
	r11->dx=(random()&(1<<9))-(1<<8);
}

void alienstoppedfly(alent* r11)
{
int r2=random();
if ((r2&6)==0)
{
	flyup:
	r11->dx=0;
	r11->dy=-(1<<7);
}
else if ((r2&1)==0)
{
	flyupdown:
	r11->dx=0;
	r11->dy=(r2&(1<<9))-(1<<8);
}
else {
	r11->dx=(r2&(1<<10))-(1<<9);
	r11->dy=0;
}
return;
}

void almightjump(alent* r11)
{
if (downhit!=1) r11->dx=0;
else if ((random()&3)==0)  r11->dy=-(8<<8);
almightjumpins(r11);
}

void alpossjump(alent* r11)
{
if (downhit!=1) r11->dx=0;
else if ((random()&0x1f)==0)  r11->dy=-(8<<8);
almightjumpins(r11);
}

void almightwelljump(alent* r11)
{
if (downhit!=1) r11->dx=0;
else if ((random()&3)!=0)  r11->dy=-(8<<8);
almightjumpins(r11);
}

void alientestplat(alent* r11,char* r5)
{
if (!alonplat) return;
plattoobj(r5);
}

void decoration(alent* r11)
{
if (--r11->r6<0)
{
r11->x+=r11->dx; r11->y+=r11->dy; r11->dy+=(1<<4);
}
if ((r11->r5-=(1<<16))<0)
	r11->type=0;
if (masterplotal==0) return;
fspplot(blokeadr,r11->r5&0xff,_xofs+(r11->x>>8),_yofs+8+(r11->y>>8));
}

void extender(alent* r11)
{
  int tmpx=r11->x+r11->dx, tmpy=r11->y+r11->dy;
if (!plcolcheck(r11->x+r11->dx,r11->y+r11->dy,(32<<8),(8<<8)));  // horiz.size, vert. size
	r11->x+=r11->dx, r11->y+=r11->dy;

char* r0=fntranslate(tmpx,tmpy);
char f=r11->r5&0xff;
if (r11->r5&(1<<8))
{
contracting:
if ((*r0==f)||(*r0==0))  f=*r0=0;
 else if ((*r0!=f+1)&&(*r0!=f+2)&&(*r0!=_gaslowlim)&&(*r0!=_gashighlim))
{
r11->type=0;
return;
}
}
else
{
if (*r0==0) *r0=f;
if ((*r0!=f)&&(*r0!=f+1)&&(*r0!=f+2)&&(*r0!=_gaslowlim)&&(*r0!=_gashighlim))
{
r11->dx=-r11->dx;
r11->r5|=(1<<8);
}
}

nostopextend:
if (plotal==0) return;
r0=fntranslate(r11->x+r11->dx*15,r11->y);
f=r11->r5&0xff;
if ((*r0!=0)&&(*r0!=f)&&(*r0!=f+1)&&(*r0!=f+2)) return;
fspplot(blockadr,r11->r5&0xff,
(r11->x>>8)-((xpos-(r11->dx*7))>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

int alspintab[40];

void init_alspintab() {
  for (int i=0;i<20;i++) {
    alspintab[i*2] = (int)((10<<8)*sin(i*2*PI/16));
    alspintab[i*2+1] = (int)((10<<8)*cos(i*2*PI/16));
  }}

void alien1(alent* r11)
{
colchecktype=2;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwandernojump(r11);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight/2))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(1,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(1,r11);

if (plotal==0) return;

fspplot(alspradr,8+(r11->dx>0)*2+((framectr&8)>>3),
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs+8);
}

void alien2(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwander(r11,r5);
alientestplat(r11,r5);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(2,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(2,r11);

if (plotal==0) return;

fspplot(alspradr,0,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}


void alien3(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwander(r11,r5);
alientestplat(r11,r5);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(3,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(3,r11);
if ((random()&0x3f)==0) alshoot(r11);

if (plotal==0) return;

fspplot(alspradr,1,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

void alien4(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
jumpyalwander(r11);
alientestplat(r11,r5);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(4,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(4,r11);
r11->r5+=(r11->r5<<16);
if (r11->r5&(1<<29)) r11->r5=alspinfire(r11);
if (r11->r5&(1<<31)) r11->r5-=8; else r11->r5+=4;
if ((r11->r5&0xffff)==0) r11->r5=0;

if (plotal==0) return;

 int x,y;
fspplot(alspradr,2,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
fspplot(alspradr,4+((r11->r5>>24)&3),
	x=(r11->x>>8)-(xpos>>8)+_xofs,y=(r11->y>>8)-(ypos>>8)+_yofs-7);
if (r11->r5&(1<<31)) return;
int f=(r11->r5>>26)&3;
int r0=(r11->r5>>24)&3;
int* r3=alspintab+r0*2;
for (int r4=4;r4>0;r4--)
{
loop81:
fspplot(exploadr,_bulspritebase+((r4+f)&3),x+(r3[0]>>8),y+(r3[1]>>8));
r3+=8; // yes, that's 8 ints (32 bytes)
}
}


void alien5(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwander(r11,r5);
alientestplat(r11,r5);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(5,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(5,r11);
r11->r5+=(r11->r5<<16);
if (r11->r5&(1<<29)) r11->r5=alspinpowerfire(r11,0);
if (r11->r5&(1<<31)) r11->r5-=2; else r11->r5+=2;
if ((r11->r5&0xffff)==0) r11->r5=0;

if (plotal==0) return;

 int x,y;
fspplot(alspradr,2,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
fspplot(alspradr,4+((r11->r5>>24)&3),
	x=(r11->x>>8)-(xpos>>8)+_xofs,y=(r11->y>>8)-(ypos>>8)+_yofs-7);
if (r11->r5&(1<<31)) return;
int f=(r11->r5>>26)&3;
int r0=(r11->r5>>24)&3;
int* r3=alspintab+r0*2;
for (int r4=4;r4>0;r4--)
{
loop83:
fspplot(exploadr,_bulspritebase+8+((r4+f)&1),x+(r3[0]>>8),y+(r3[1]>>8));
r3+=8; // yes, that's 8 ints (32 bytes)
}
}

void alien6(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwander(r11,r5);
alientestplat(r11,r5);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(6,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(6,r11);
r11->r5+=(r11->r5<<16);
if (r11->r5&(1<<29)) r11->r5=alspinpowerfire(r11,1);
if (r11->r5&(1<<31)) r11->r5-=2; else r11->r5+=8;
if ((r11->r5&0xffff)==0) r11->r5=0;

if (plotal==0) return;

 int x,y;
fspplot(alspradr,3,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
fspplot(alspradr,4+((r11->r5>>24)&3),
	x=(r11->x>>8)-(xpos>>8)+_xofs,y=(r11->y>>8)-(ypos>>8)+_yofs-7);
if (r11->r5&(1<<31)) return;
int f=(r11->r5>>26)&3;
int r0=(r11->r5>>24)&3;
int* r3=alspintab+r0*2;
for (int r4=4;r4>0;r4--)
{
loop85:
fspplot(exploadr,_bulspritebase+10+((r4+f)&1),x+(r3[0]>>8),y+(r3[1]>>8));
r3+=8; // yes, that's 8 ints (32 bytes)
}
}

int alspinfire(alent* r11)
{
int r0=(random()&3);
int* r6=alspintab+(r0<<1);
for (int r4=4;r4>0;r4--)
{
loop82:
makebul(r6[0]+r11->x,r6[1]+r11->y-(7<<8),r6[8]>>2,r6[9]>>2,
	_bulspritebase+((r4+r0)&3),1<<24);
r6+=8;
}
r6-=8;
int r2=0x7f-abs(r6[0]-xpos)-abs(r6[1]-(7<<8)-ypos); // XXX scaling?
if (r2>0x40)
	bidforsound(_Explochannel,_Sampsmallzap,r2,(fullpitch+0x1000) /*pitch*/,
		0,0,2 /* lifetime (frames) */,(r6[1]-(7<<8))>>9,CHUNK_SPINFIRE);
nospinzapsound:
return 0x80000100;
}

int alspinpowerfire(alent* r11,int r7)
{
int r0=(random()&3);
int* r6=alspintab+(r0<<1);
for (int r4=4;r4>0;r4--)
{
loop84:
makebul(r6[0]+r11->x,r6[1]+r11->y-(7<<8),r6[8]>>2,r6[9]>>2,
	_bulspritebase+8+(r7<<1),1<<24);
r6+=8;
}
r6-=8;
int r2=0x7f-abs(r6[0]-xpos)-abs(r6[1]-(7<<8)-ypos); // XXX scaling?
if (r2>0x40)
	bidforsound(_Explochannel,_Sampbigzap,r2,fullpitch /*pitch*/,
		0,0,2 /* lifetime (frames) */,(r6[1]-(7<<8))>>9,CHUNK_SPINPOWERFIRE);
nopowerzapsound:
return 0x80000040;
}

void alien7(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwander(r11,r5);
alientestplat(r11,r5);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(7,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(7,r11);
int rng=random();
if ((rng&0x3f)==0) alshootnutter(r11);
else if ((rng&0xf)==0) alshootfast(r11);

if (plotal==0) return;

fspplot(alspradr,16,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

void alien8(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwander(r11,r5);
alientestplat(r11,r5);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(8,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(8,r11);
int rng=random();
if ((rng&0x3f)==0) alshootnutter(r11);
else if ((rng&0xf)==0) alshootmental(r11);

if (plotal==0) return;

fspplot(alspradr,16,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

void alien9(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwander(r11,r5);
alientestplat(r11,r5);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(9,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(9,r11);
int rng=random();
if ((rng&0x1f)==0) alshootnutterplus(r11);

if (plotal==0) return;

fspplot(alspradr,17,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

void alien10(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy+=1<<7; // gravity
if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwander(r11,r5);
alientestplat(r11,r5);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(10,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(10,r11);
int rng=random();
if ((rng&0x1f)==0) alshootnuttermental(r11);

if (plotal==0) return;

fspplot(alspradr,17,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

void alien11(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy; // no gravity

alienwanderfly(r11);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>2*_sleeprange) alsleep(11,r11);
if (abs(r11->y-ypos)>2*_sleeprange) alsleep(11,r11);
int rng=random();
if ((rng&0x1f)==0) alshootfast(r11);

if (plotal==0) return;

fspplot(alspradr,12+(random()&3),
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

void alien12(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy; // no gravity
r11->dx+=r11->dx>>6; r11->dy+=r11->dy>>6;
if (r11->dx>_speedlim) r11->dx=_speedlim;
if (r11->dx<-_speedlim) r11->dx=-_speedlim;
if (r11->dy>_speedlim) r11->dy=_speedlim;
if (r11->dy<-_speedlim) r11->dy=-_speedlim;

alienwanderfly(r11);

bulcolchadd(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>4*_sleeprange) alsleep(12,r11);
if (abs(r11->y-ypos)>4*_sleeprange) alsleep(12,r11);
int rng=random();
if ((rng&0x1f)==0) alshootnuttermental(r11);

if (plotal==0) return;

fspplot(alspradr,12+(random()&3),
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

void alien13(alent* r11)
{
colchecktype=2;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

if (r11->dy>(1<<11)) r11->dy=1<<11;
alienwanderfly(r11);

bulcolchaddshort(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight/2))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(13,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(13,r11);

if (plotal==0) return;

fspplot(alspradr,8+2*(r11->dx>0)+((framectr&8)==8),
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

void alien14(alent* r11)
{
colchecktype=1;
platypos=r11->y-(8<<8);
colcheck(r11);
r11->y=platypos+(8<<8);
char* r5=albcheck(r11);
r11->x+=r11->dx; r11->y+=r11->dy;

r11->dy++;
if (r11->dy>(1<<9)) r11->dy=1<<8;

bulcolchadd(r11);
if (downhit==1) hamsterspecial(r11);
if (plcolcheck(r11->x,r11->y,_alwidth,_alheight))
	{plstrength-=_explocontloss; r11->r6-=_explocontloss;}
	// [R11,#20] ; alien strength
if (r11->r6<=0)  {alkill(r11); return;}
if (abs(r11->x-xpos)>_sleeprange) alsleep(14,r11);
if (abs(r11->y-ypos)>_sleeprange) alsleep(14,r11);

if (plotal==0) return;

fspplot(alspradr,18,
	(r11->x>>8)-(xpos>>8)+_xofs,(r11->y>>8)-(ypos>>8)+_yofs);
}

void hamsterspecial(alent* r11)
{
r11->type=_Alien1+11;
r11->r6=12<<8;
r11->dx=r11->dy=0;
}

void alkill(alent* r11)
{
plscoreadd+=((r11->type-(_Alien1-1))&0x1f)*100;
 explogo(r11->x,r11->y,0,0,0,/*r6*/0,r11/*-4*/);
exploins(r11);
}

void alshoot(alent* r11)
{
int r2=xpos-r11->x;
int r3=(ypos+(8<<8))-(r11->y-(6<<8)); //  aim at body
if ((r2>_firerange)||(r3>_firerange)) return;
if ((r2<-_firerange)||(r3<-_firerange)) return;
int r4=random()&7; if (r4==7) r4=6;
makebul(r11->x,r11->y,(r2>>6),(r3>>6),_bulspritebase+r4,1<<24);

bidforsound(_Explochannel,_Sampsmallzap,0x78,fullpitch+0x1000, // pitch
	0,(fullpitch<<16)|0xfe00,2 /* lifetime (frames) */,0,CHUNK_SHOOT);
noshootsound:;
}

void alshootfast(alent* r11)
{
int r2=xpos-r11->x;
int r3=(ypos+(8<<8))-(r11->y-(6<<8)); //  aim at body
if ((r2>_firerange)||(r3>_firerange)) return;
if ((r2<-_firerange)||(r3<-_firerange)) return;
int r4=random()&7; if (r4==7) r4=6;
makebul(r11->x,r11->y,(r2>>5),(r3>>5),_bulspritebase+r4,1<<24);

bidforsound(_Explochannel,_Sampsmallzap,0x6f,fullpitch+0x1000, // pitch
	0,(fullpitch<<16)|0xfe00,2 /* lifetime (frames) */,0,CHUNK_SHOOT);
}

void alshootnutter(alent* r11)
{
int r2=xpos-r11->x;
int r3=(ypos+(12<<8))-(r11->y-(8<<8)); //fire from sensible place;  aim at body
if ((r2>_firerange)||(r3>_firerange)) return;
if ((r2<-_firerange)||(r3<-_firerange)) return;
makebul(r11->x,r11->y,0,-(1<<10),_bulspritebase+12,1<<22);

bidforsound(_Explochannel,_Sampbigzap,0x6f,fullpitch, // pitch
	0,(fullpitch<<16)|0xfe00,2 /* lifetime (frames) */,0,CHUNK_SHOOTNUTTER);
}

void alshootnutterplus(alent* r11)
{
int r2=xpos-r11->x;
int r3=(ypos+(12<<8))-(r11->y-(8<<8)); //fire from sensible place;  aim at body
if ((r2>_firerange)||(r3>_firerange)) return;
if ((r2<-_firerange)||(r3<-_firerange)) return;
makebul(r11->x,r11->y,0,-(1<<10),_bulspritebase+13,1<<21);

bidforsound(_Explochannel,_Sampbigzap,0x6f,fullpitch, // pitch
	0,(fullpitch<<16)|0xfe00,2 /* lifetime (frames) */,0,CHUNK_SHOOTNUTTER);
}

void alshootnuttermental(alent* r11)
{
int r2=xpos-r11->x;
int r3=(ypos+(12<<8))-(r11->y-(8<<8)); //fire from sensible place;  aim at body
if ((r2>_firerange)||(r3>_firerange)) return;
if ((r2<-_firerange)||(r3<-_firerange)) return;
int r=random();
if (r&(1<<11)) {r2=0; r3=-(1<<10);}
else {r2=(r&(1<<10))?(1<<10):-(1<<10); r3=0;}
makebul(r11->x,r11->y,r2,r3,_bulspritebase+14,1<<21);

bidforsound(_Explochannel,_Sampbigzap,0x6f,fullpitch, // pitch
	0,(fullpitch<<16)|0xfe00,2 /* lifetime (frames) */,0,CHUNK_SHOOTNUTTER);
}

void alshootmental(alent* r11)
{
int r2=xpos-r11->x;
int r3=(ypos+(8<<8))-(r11->y-(6<<8)); //  aim at body
if ((r2>_firerange)||(r3>_firerange)) return;
if ((r2<-_firerange)||(r3<-_firerange)) return;
int r4=8+(random()&2);
makebul(r11->x,r11->y,(r2>>6),(r3>>6),_bulspritebase+r4,1<<24);

bidforsound(_Explochannel,_Sampsmallzap,0x6f,fullpitch+0x1000, // pitch
	0,(fullpitch<<16)|0xfe00,2 /* lifetime (frames) */,0,CHUNK_SHOOT);
}

void alsleep(int s,alent* r11)
{
char* r0=translate(r11->x-(4<<8),r11->y-(8<<8));
if (*r0!=0) return;
*r0=240+(s-1);
r11->type=0;
}

void explo(alent* r11)
{
r11->x+=r11->dx;
r11->y+=r11->dy;
int r4=r11->r5;
int r5=explospeed<<2;
if (r4<=r5)
  if (plcolcheck(r11->x,r11->y,_expwidth,_expheight)) // object size
		plstrength-=_explocontloss;
exploins(r11);
}

void exploins(alent* r11)
{
r11->r5=r11->r5+explospeed;
if (r11->r5<0) r11->r5=50<<8; //trap negatives
char r0=(r11->r5)&0xff;
if (r0>=(8<<2)) //explo frame length
  { r11->type=0; return;}
if (plotal==0) return;
fspplot(exploadr,(r11->r5>>2)&0x3f,
	(r11->x>>8)-(xpos>>8)+_xofs,
	(r11->y>>8)-(ypos>>8)+_yofs);
}

void booby(alent* r11)
{
char* r0=fntranslate(r11->x+r11->dx,r11->y);
if ((*r0>=_bomblowlim)&&(*r0<_targethighlim))
	{emberbooby(r0); return;}
noboobybomb:
if (*r0>=_blim) r11->dx=-(r11->dx>>1);
r0=fntranslate(r11->x+r11->dx,r11->y+r11->dy);
if ((*r0>=_boobylowlim)&&(*r0<=_boobyhighlim))
	{emberbooby(r0); return;}
noboobybooby:
if (*r0>=_blim) r11->dy=-(r11->dy>>1);
r11->x+=r11->dx;
r11->y+=r11->dy;
r11->dy+=(1<<4); //gravity
r11->r5++;
if (r11->r5<0) r11->r5=1<<8; //trap negatives
if (r11->r5>=140) // booby lifetime
  { r11->type=0; return;}
if (plotal==0) return;
fspplot(exploadr,12+((r11->r5>>2)&3),
	(r11->x>>8)-(xpos>>8)+_xofs,
	(r11->y>>8)-(ypos>>8)+_yofs);
}

void ember(alent* r11)
{
char* r0=fntranslate(r11->x+r11->dx,r11->y);
if ((*r0>=_bomblowlim)&&(*r0<_bombhighlim))
	{emberbomb(r0,r11); return;}
noemberbomb:
if (*r0>=_blim) r11->dx=-(r11->dx>>1);
r0=fntranslate(r11->x+r11->dx,r11->y+r11->dy);
if ((*r0>=_gaslowlim)&&(*r0<_gashighlim))
	{emberbomb(r0,r11); return;}
noembergas:
if (*r0>=_blim) r11->dy=-(r11->dy>>1);
r11->x+=r11->dx;
r11->y+=r11->dy;
r11->dy+=(1<<4); //gravity

r11->r5++;
if (r11->r5<0) r11->r5=1<<8; //trap negatives
if (r11->r5>=70) // ember lifetime
  { r11->type=0; return;}
if (plotal==0) return;
fspplot(exploadr,8+(((r11->r5)>>2)&3),
	(r11->x>>8)-(xpos>>8)+_xofs,
	(r11->y>>8)-(ypos>>8)+_yofs);
}

void flyingbonus(alent* r11)
{
int r4=0; // hit wall marker
char* r0=fntranslate(r11->x+r11->dx-(8<<8),r11->y-(8<<8));
if ((*r0>=_bonuslow)||(r0[boardwidth]>=_bonuslow))
	if (r11->dx<=0) {r11->dx=-r11->dx; r4=1;}
if ((r0[1]>=_bonuslow)||(r0[boardwidth+1]>=_bonuslow))
	if (r11->dx>=0) {r11->dx=-r11->dx; r4=1;}
flyingskip:
r0=fntranslate(r11->x+r11->dx-(8<<8),r11->y+r11->dy-(8<<8));
if ((*r0>=_bonuslow)||(r0[1]>=_bonuslow))
	if (r11->dy<=0) {r11->dy=-r11->dy; r4=1;}
if ((r0[boardwidth]>=_bonuslow)||(r0[boardwidth+1]>=_bonuslow))
	if (r11->dy>=0) {r11->dy=-r11->dy; r4=1;}
flyingvertskip:
r0=fntranslate(r11->x+r11->dx-(8<<8),r11->y+r11->dy-(8<<8));
if ((*r0>=_bonuslow)||(r0[boardwidth]>=_bonuslow))
	if (r11->dx<=0) {r11->dx=-r11->dx; r4=1;}
if ((r0[1]>=_bonuslow)||(r0[boardwidth+1]>=_bonuslow))
	if (r11->dx>=0) {r11->dx=-r11->dx; r4=1;}
r0=fntranslate(r11->x+r11->dx-(8<<8),r11->y+r11->dy-(8<<8));
if ((*r0>=_bonuslow)||(r0[1]>=_bonuslow))
	if (r11->dy<=0) {r11->dy=-r11->dy; r4=1;}
if ((r0[boardwidth]>=_bonuslow)||(r0[boardwidth+1]>=_bonuslow))
	if (r11->dy>=0) {r11->dy=-r11->dy; r4=1;}
r11->x+=r11->dx;
r11->y+=r11->dy;
r11->dy+=(1<<5); //gravity
if (r4!=0)
{
r11->dx-=(r11->dx>>2);
r11->dy-=(r11->dy>>2);
}
flyingnoslow:

r11->r5-=(1<<16);
if (r11->r5<0)
{
bonusdowngrade:
r11->type=_Dyingbonus;
r11->r5=r11->r6 | (1<<20);
}

if ((r11->x>(pllx-(8<<8)))&&((plhx+(8<<8))>r11->x)
	&&(r11->y>plly)&&(plhy>r11->y))
	bonusobjgot(r11);


if (plotal==0) return;
// r8=8-(r8>>17); ???
fspplot(blockadr,r11->r6&0xff,
	(r11->x>>8)-(xpos>>8)+_xofs,
	(r11->y>>8)-(ypos>>8)+_yofs);
}

void dyingbonus(alent* r11)
{
r11->r5-=(1<<16);
if (r11->r5<0) {deleteobj(r11); return;}
if (plotal==0) return;
int r8=8-(r11->r5>>17);
int r0=(r11->r5)&0xffff;
int r1=	(r11->x>>8)-(xpos>>8)+_xofs;
int r2=r8+	(r11->y>>8)-(ypos>>8)+_yofs;
fsphy=r2;
if (fsphy<=cliphy)
if (fsply<fsphy)
{
fsphx=r1;
swi_fastspr_setclipwindow(fsplx,fsply,fsphx,fsphy);
if (fsphx<=cliphx)
if (fsplx<fsphx)
	fspplot(blockadr,r0,fsphx+r8,fsphy+r8);
fsphx=cliphx;
fsplx=r1;
swi_fastspr_setclipwindow(fsplx,fsply,fsphx,fsphy);
if (fsplx>=cliplx)
if (fsphx>cliplx)
	fspplot(blockadr,r0,fsplx-r8,fsphy+r8);
}
nodyingtop:
fsphy=cliphy;
fsplx=cliplx;
fsply=r2;
if (fsply>=cliply)
if (fsply<fsphy)
{
fsphx=r1;
swi_fastspr_setclipwindow(fsplx,fsply,fsphx,fsphy);
if (fsphx<=cliphx)
if (fsplx<fsphx)
	fspplot(blockadr,r0,fsphx+r8,fsply-r8);
fsphx=cliphx;
fsplx=r1;
swi_fastspr_setclipwindow(fsplx,fsply,fsphx,fsphy);
if (fsplx>=cliplx)
if (fsphx>fsplx)
	fspplot(blockadr,r0,fsplx-r8,fsply-r8);
}
nodyingbot:
writeclip();
}

void scoreobj(alent* r11)
{
r11->x+=r11->dx;
r11->y+=r11->dy;
r11->dy+=(1<<5);
r11->r5-=1;
if ((r11->r5&0xff)==0)
	{r11->type=0; return;}
if (plotal==0) return;
fspplot(blokeadr,(r11->r5>>8),
	(r11->x>>8)-(xpos>>8)+_xofs,
	(r11->y>>8)-(ypos>>8)+_yofs);
}


void plat1(alent* r11)       // alien routines
{
r11->dx=0;
dvcheck(r11);
r11->y+=r11->dy;
r11->dy+=1<<4;
if (r11->dy>(1<<11)) r11->dy=1<<11;
if (downhit!=0) platland(r11,_platno);
colchadd(r11);
platins(r11,_platno);
}


void plat2(alent* r11)
{
r11->dx=0;
dvcheck(r11);
r11->y+=r11->dy;
r11->dy+=1<<4;
if (r11->dy>(1<<11)) r11->dy=1<<11;
 if ((downhit!=0)||(uphit!=0)) plattoexplo(r11/*,_platno+2*/);
colchadd(r11);
platins(r11,_platno+2);
}

void plat3(alent* r11)
{
r11->dx=0;
dvcheck(r11);
r11->y+=r11->dy;
r11->dy+=1<<4;
if (r11->dy>(1<<11)) r11->dy=1<<11;
if (downhit!=0) platland(r11,_platno+4);
colchadd(r11);
platins(r11,_platno+4);
}

void plat4(alent* r11)
{
r11->dx=0;
dvcheck(r11);
r11->y+=r11->dy;
r11->dy+=1<<4;
if (r11->dy>(1<<11)) r11->dy=1<<11;
if (downhit!=0) platland(r11,_platno-8+(r11->type<<1));
colchadd(r11);
platins(r11,_platno-8+(r11->type<<1));
}

void plat5(alent* r11)
{
r11->dx=0;
dvcheck(r11);
r11->y+=r11->dy;
r11->dy+=1<<2;
if (r11->dy>(1<<9)) r11->dy=1<<9;
if (downhit!=0) platland(r11,_platno+14);
  // .platlandins (duplicate label ignored)
colchadd(r11);
platins(r11,_platno+14);
}

//.platlandins
//colchadd(r11);
//goto platins;

void platins(alent* r11, char r9)
{
if (plotal==0) return;
int r1=(r11->x>>8)-(xpos>>8)+_xofs;
int r2=(r11->y>>8)-(ypos>>8)+_yofs;
fspplot(blockadr,r9,r1-8,r2);
fspplot(blockadr,r9+1,r1+8,r2);
}

//aldone:
//LDMFD R13!,{R9,R11,PC}

void colchadd(alent* r11)
{
if (colchptr-colchtab>=_colchlim) return;
colchptr->r0=r11;
int xoff=(r11->r5)>>16;
int yoff=(r11->r5)&0xffff;
colchptr->xmin=r11->x-(xoff<<7); colchptr->ymin=r11->y-(yoff<<7);
colchptr->xmax=r11->x+(xoff<<7); colchptr->ymax=r11->y+(yoff<<7);
colchptr++;
}

void bulcolchadd(alent* r11)
{
if (bulcolchptr-bulcolchtab>=_bulcolchlim) return;
bulcolchptr->r0=r11;
bulcolchptr->xmin=r11->x-(32<<7); bulcolchptr->ymin=r11->y-(32<<7);
bulcolchptr->xmax=r11->x+(32<<7); bulcolchptr->ymax=r11->y+(32<<7);
bulcolchptr++;
}

void bulcolchaddshort(alent* r11)
{
if (bulcolchptr-bulcolchtab>=_bulcolchlim) return;
bulcolchptr->r0=r11;
bulcolchptr->xmin=r11->x-(32<<7); bulcolchptr->ymin=r11->y+(8<<8)-(16<<7);
bulcolchptr->xmax=r11->x+(32<<7); bulcolchptr->ymax=r11->y+(8<<8)+(16<<7);
bulcolchptr++;
}

void platland(alent* r11, char r9)
{
char* r0=translate(r11->x,r11->y);
char r1=*r0;
if (*r0>=_platblim) r0-=boardwidth;
if ((*(r0+boardwidth)!=_extendno)
    &&(*(r0+boardwidth+1)!=_extendno))
{
*r0=r9; r0[1]=r9+1;
r11->type=0;
}
}

void plattoexplo(alent* r11) {blowup(r11);}

void deleteobj(alent* r11) {r11->type=0;}

void blowup(alent* r11) {explogo(r11->x,r11->y,0,1<<6,0,/*r6*/0,r11/*-4*/);}

void emberbooby(char* r0)
{
if ((*r0<_atomlowlim)||(*r0>_atomhighlim)) normbombsurvive(r0);
else atombomb(r0);
}

void emberbomb(char* r0,alent* r11)
{
if ((*r0<_atomlowlim)||(*r0>_atomhighlim)) normbomb(r0,r11);
else atombomb(r0);
}

void normbomb(char* r0,alent* r11)
{
r11->type=0;
normbombsurvive(r0);
}

void normbombsurvive(char* r0)
{
if ((*r0==_fuelairno)||(*r0==_fuelairno+1)) {fuelbomb(r0); return;}
normbombins:
destroy(r0);
}

void fuelbomb(char* r0)
{
if (*r0!=_fuelairno) r0--;
*r0=0; r0[1]=0;
int x,y;
backtranslate(r0,&x,&y);
explogo(x+(8<<8),y,0,-(1<<7),0,0,0);
}

int plcolcheck(int x,int y,int dx,int dy) // returns 0 for EQ
{
return ((x+(dx>>1)>pllx) && (plhx>x-(dx>>1))
	&& (y+(dy>>1)>plly) && (plhy>y-(dy>>1)));
}

void settestal()
{
makeobj(_Alien1+13,xpos-(65<<8),ypos-(24<<8),0,0,0x100020,1<<10);
}

void dvcheck(alent* r11)
{
xtemp=r11->x; ytemp=r11->y;
uphit=downhit=0;
char* r6=translate(r11->x,r11->y+r11->dy);
// up
if ((r6[-boardwidth]>=_platblim)||(r6[1-boardwidth]>=_platblim)) noup(r11);
//down
if ((r6[0]>=_platblim)||(r6[1]>=_platblim)) nodown(r11);
}

void playerplot()
{
fsphy=120;
swi_fastspr_setclipwindow(20,20,320-20,120);
if (snuffctr)
{
snuffhandler:
snuffctr+=frameinc;
plotterofs=((snuffctr-96<0)?0:(snuffctr-96))>>1;
fspplot(blokeadr,70,160,104+plotterofs);
// goto playerplotins;
seestars();
writeclip();
return;
}

if (telepctr)
{
int telepinc=frameinc;
if (telepinc>2) telepinc=2;
if (telepctr==31) telepinc=1;
telepctr+=telepinc;
if (telepctr>64) telepctr=0;
if (telepctr==32)
{
dotelep:
xpos=telepxpos; ypos=telepypos;
screenwakeup();
}
}

telepskip:
plotterofs=(telepctr>32)?(64-telepctr):telepctr;
plframe=(xpos>>10)&3;
if (plframe==2) plframe=0;
else if (plframe==3) plframe=2;
if (plface==1) plframe+=3;

if (electrocuting)
{
plotskel:
fspplot(blokeadr,_skelspriteno+(plface==1),160,plotterofs+104);
plotskelins:
seestars();
writeclip();
return;
}

fspplot(blokeadr,(plface==1)?1:0,160,plotterofs+104);
fspplot(blokeadr,plframe+6,160,plotterofs+115);

int r0;
if (plstrength<laststrength)
	r0=_funnyfacesprbase+((framectr>>2)&3);
else
	nofunnyface: if (plface==1) r0=15; else r0=12;
plotface:
fspplot(blokeadr,r0,160,plotterofs+94); //head

if (plweapontype==0)  plotarms();
else if (plweapontype==_mpmgblamno)  plotmpmgblam();
else if (plweapontype==_rocketblamno)  plotrocketblam();
else if (plweapontype<_rockbase)  plotmpmg();
else plotrocket();

playerplotins:
seestars();
writeclip();
}

void plotarms() {fspplot(blokeadr,(plframe%3)+2,160+(plframe>2),plotterofs+105);}

void plotmpmg()
{
fspplot(blokeadr,42+(plface==1),160-6+(plface!=1)*12+(plfired?0:2*plface-1),plotterofs+106);
fspplot(blokeadr,44+(plface==1),160,plotterofs+104);
plfired=0;
}

void plotmpmgblam()
{
int r2;
if (blamctr&8)  r2=(blamctr&15)-15;
else r2=blamctr&15;
fspplot(blockadr,_weaplowlim+6+(plface==1),
	160-6+(plface!=1)*12+(plfired?plface*2-1:0),plotterofs+107+(r2>>1));
fspplot(blokeadr,44+(plface==1),160,plotterofs+104+(r2>>1));
plfired=0;
}

void plotrocket()
{
int r0=rockettabadr[rocketctr];
fspplot(blokeadr,50+(r0>170)+(r0>60)-(r0<-60)-(r0<-170),
	160+(plfired?(plface*2-1):0),plotterofs+97);

fspplot(blokeadr,53,160+plface,plotterofs+102);
plfired=0;
if (rocketflag==0)  rocketctr=0;
}

void plotrocketblam()
{
int r2;
if (blamctr&8)  r2=(blamctr&15)-15;
else r2=blamctr&15;
fspplot(blockadr,_weaplowlim+15,160+(plfired?0:2*plface-1),plotterofs+(r2>>1)+96);
fspplot(blokeadr,53,160,plotterofs+(r2>>2)+102);
plfired=0;
}

void bonusplot()
{
int r2=(frameinc>2)?2:frameinc;
if (bonustimer!=0) bonustimer-=r2;
if ((bonustimer==1)||(bonustimer==2)) bonusreset();

if ((bonusctr>=13)&&(bonusreplot==0)) {bonusbonus(); return;}
if (bonusreplot==0) return;
bonusreplot-=r2; //from above
if (bonusreplot<0) bonusreplot=0;

fsplx=_bonusxplace-8+0x100; // align to write clip window to FastSpr
fsply=_bonusyplace-20;
fsphx=_bonusxplace+8+0x100;
fsphy=_bonusyplace+20;
swi_fastspr_setclipwindow(fsplx,fsply,fsphx,fsphy);
swi_fastspr_clearwindow();
int r3=((bonusreplot>7)?bonusreplot-8:0);
fspplot(blockadr,(bonusctr+16>_bonuslow+12)?0:(bonusctr+16),
	_bonusxplace+0x100,_bonusyplace+r3);
if (bonusctr!=0)
	fspplot(blockadr,(bonusctr+15>_bonuslow+12)?0:(bonusctr+15),
		_bonusxplace+0x100,_bonusyplace-20+r3);
nosecond:
if (bonusctr!=12)
	fspplot(blockadr,(bonusctr+17>_bonuslow+12)?0:(bonusctr+17),
		_bonusxplace+0x100,_bonusyplace+20+r3);
nothird:
writeclip(); // reset the clip window
}

void bonusbonus()    //not to be called by BL
{
bonusctr=8;
bonusreplot=28;
keypressed=0;
int bdx=boardadr->width*boardadr->height;
char* r11=boardadr->contents;
for (;bdx>0;r11++,bdx--)
{
loop90:
if (*r11==_starsno)
{
foundmarker:
*r11=0;
backtranslate(r11,&telepxpos,&telepypos);
telepypos-=(1<<8);
telepctr=1;
bonusxpos=xpos;
bonusypos=ypos;
bonustimer=0x180;
return;
}
}
bonus1();
}

void bonusreset()
{
if (keypressed)  {normreset(); return;}
bonusctr=0;
bonusreplot=28;
keypressed=1;

char* r11=boardadr->contents;
int r3;
for (r3=boardadr->width*boardadr->height;r3>0;r3--)
{
loop91:
if (*(r11++)==_starsno) break;
}
if (r3==0) {bonus1(); normreset(); return;}
foundresetmarker:
*(--r11)=0;
backtranslate(r11,&telepxpos,&telepypos);
telepypos-=(1<<8);
telepctr=1;
bonustimer=0x180;
}

void normreset()
{
telepctr=1;
telepxpos=bonusxpos;
telepypos=bonusypos;
bonustimer=0;
}

void zonecheatread()
{
  char r1=osbyte_7a(); // was _81(0)
if ((r1<48)||(r1>56)) return;
plzone=r1-48;
}

void cheatread()
{
if (osbyte_81(-282)==0xff) getmpmg();
if (osbyte_81(-283)==0xff) getrocket();
if (osbyte_81(-285)==0xff) screensave();
if (osbyte_81(-284)==0xff) prepstrength();
}


void keyread()
{
int r4=-1;
if (joyno)
{
  int r0, r1;
int v=swi_joystick_read(joyno-1,&r0,&r1);
if (v)
{
nostickerr:
message(32,32,0,1,"Can't see a joystick!");
r4=-1;
joyno=0;
}
else
{
r4=-1;
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
}
nojoystick:
if ((osbyte_81(leftkey)==0xff)||!(r4&1))
	{if (++leftpress==0) leftpress=0xff;}
else leftpress=0;
if ((osbyte_81(rightkey)==0xff)||!(r4&2))
	{if (++rightpress==0) rightpress=0xff;}
else rightpress=0;
if ((osbyte_81(upkey)==0xff)||!(r4&4))
	{if (++uppress==0) uppress=0xff;}
else uppress=0;
if ((osbyte_81(downkey)==0xff)||!(r4&8))
	{if (++downpress==0) downpress=0xff;}
else downpress=0;
if ((osbyte_81(firekey)==0xff)||!(r4&16))
	{if (++fire==0) fire=0xff;}
else fire=0;
// (don't) use word-aligned property
if (leftpress||rightpress||uppress||downpress||fire)
	keypressed=1;
}

alent fraudalent;

void plmove()
{
if (shutdownctr!=0) shutdownctr++;
if (shutdownctr==150) plzone=0;
else if (shutdownctr!=0) deletepoint();
keyread();
//if (fire) goto cheatmove;

if (snuffctr!=0)  uppress=downpress=leftpress=rightpress=fire=0;
vvec-=vvec>>5;
hvec-=hvec>>5;
if (leftpress!=rightpress) plface=(leftpress>rightpress)?1:0;

int r2;
if ((leftpress<4)&&(rightpress<4)) r2=1<<8; else r2=2<<8;
if (leftpress>rightpress) r2=-r2;
if (leftpress==rightpress) r2=0;

if (falling<=4) hvec=r2;
else hvec+=r2>>5;

cheatins:
if (downpress>=32) windctr=(1<<10);
if (windctr)  windcheck();
if (hvec>_speedlim) hvec=_speedlim;
if (hvec<-_speedlim) hvec=-_speedlim;
if (vvec>_speedlim) vvec=_speedlim;
if (vvec<-_speedlim) vvec=-_speedlim;
platypos=ypos;
colchecktype=0;
 fraudalent.x=xpos; fraudalent.y=ypos; fraudalent.dx=hvec; fraudalent.dy=vvec;
colcheck(&fraudalent);
 xpos=fraudalent.x; ypos=fraudalent.y; hvec=fraudalent.dx; vvec=fraudalent.dy;
pluphit=platuphit;
//if ((!)fire)

bcheck();
pluphit=uphit;
int dy=vvec;
if (dy>(1<<12)) dy=1<<12;
if (dy<-(1<<12)) dy=-(1<<12);
xpos+=hvec; ypos+=dy;
if ((xpos<=xlowlim)||(xposmax<=xpos)||(ypos<=ylowlim)||(yposmax<=ypos))
	restartplayer();
int x=xpos, y=ypos;

if (!snuffctr)
{
y+=(6<<8);
vvec=dy;
pllx=x-(_plwidth/2); plhx=x+(_plwidth/2);
plly=y-(_plheight/2)+(plotterofs<<8);
plhy=y+(_plheight/2);
y=ypos-(6<<8);  // - adjustment
}
else
{
fakecolch:
plhx=plhy=0;
pllx=plly=1;
}
fakeins:
pladr1=translate(x,y);
pladr2=pladr1+1;
pladr3=pladr1+boardwidth;
pladr4=pladr3+1;
pladr5=pladr1+2*boardwidth;
pladr6=pladr5+1;

if (((falling==0)||(falling==0xff))&&(uppress!=0))
{
if (downpress>uppress)
	vvec=-((0x100-(downpress-uppress))<<3);
else vvec=-(19<<7);
falling=1;
bidforsound(_Playerchannel,_SampJump,0x5f,0x4600,0,0,4,0,CHUNK_JUMP);
}

char* r11;
jumpins:
r11=translate(xpos,ypos+(1<<8));
pladr7=r11+boardwidth;
pladr8=pladr7+1;
crumblecheck(pladr7);
crumblecheck(pladr8);
if ((*pladr7<_blim)&&(*pladr8<_blim))
{
if (++falling==0xff) falling=0xfe;
if (falling)  // [else] standing on a platform
{
if (falling>1) if (vvec<(12<<8)) vvec+=(8<<4);
}
}
else
{nofall:
falling=0;
}
nofallins:;
fallins:
if ((*pladr8>=_platlowlim+1)&&(*pladr8<=_plathighlim+1)&&(1&*pladr8))
	pladr8[-1]=*pladr8-1;
noplatrebuild:
if ((*pladr7>=_platlowlim)&&(*pladr7<=_plathighlim))
{
if (*pladr7!=_plathighlim)
{
hiddenplatctr=0;
nohidplat:
if (!(1&*pladr7)) {plplattoobj(pladr7); return;}
}
else
{
hiddenplatctr+=frameinc;
if ((hiddenplatctr>=50)&&!(1&*pladr7))
  {/* I added */hiddenplatctr=0; plplattoobj(pladr7); return;}
}
}

fallinscont:
if ((!(1&*pladr7))&&(*pladr7>=_teleplowlim)&&(*pladr7<=_telephighlim))
	telep();

notelep:
if ((*(pladr1-boardwidth)>=_platlowlim)
	&&(*(pladr1-boardwidth)<=(_plathighlim-1))) //not hidden platforms
	plplattoobj(pladr1-boardwidth);
else notelepcont1:
     if ((*(pladr1-boardwidth+1)>=_platlowlim)
	&&(*(pladr1-boardwidth+1)<=(_plathighlim-1))) //not hidden platforms
	plplattoobj(pladr1-boardwidth+1);
else notelepcont2:
     if (*pladr7==_extendno+1)
{
doextendfromleft:
if (!extending)
if (*(pladr7+1)==0)
{
int x,y;
backtranslate(pladr7,&x,&y);
 makeobj(_Extender,x-(8<<8),y+(15<<8),1<<8,0,_extendno,0);
extending=20;
}
}
else if (*pladr8==_extendno+2)
{
doextendfromright:
if (!extending)
if (*(pladr8-1)==0)
{
int x,y;
backtranslate(pladr8,&x,&y);
makeobj(_Extender,x+(8<<8),y+(15<<8),-(1<<8),0,_extendno,0);
extending=20;
}
}
else if (*pladr7==_shutdownno)
{
*pladr7=14;
shutdownctr=1;
neuronctr++;
bidforsoundforce(3,_Samprave,0x1f,(3<<8)|0x3000,(soundvol<<25)|0x1000,
	0x100ff00,100,(3<<6)-96,CHUNK_SHUTDOWN_1);
bidforsoundforce(2,_Samprave,0x1f,(2<<8)|0x3000,(soundvol<<25)|0x1000,
	0x100ff00,100,(2<<6)-96,CHUNK_SHUTDOWN_2);
bidforsoundforce(1,_Samprave,0x1f,(1<<8)|0x3000,(soundvol<<25)|0x1000,
	0x100ff00,100,(1<<6)-96,CHUNK_SHUTDOWN_3);
}
//plattoins:
playerfire();
}

void windcheck()
{
if (*pladr1==0) return;
if (!seeifwind(pladr4,seeifwind(pladr3,seeifwind(pladr2,seeifwind(pladr1,0)))))
	windctr-=64;
if (--windctr<0) windctr=0;
}

int seeifwind(char* r1, int retval)
{
if (*r1==2) {hvec-=_windspeed*4; retval=1;}
if (*r1==3) {hvec+=_windspeed*4; retval=1;}
if (*r1==12) {vvec-=_windspeed; retval=1;}
if (*r1==13) {vvec+=_windspeed; retval=1;}
return retval;
}

void plattoobj(char* r0) {plattoobjins(r0,0);}

void plattoobjins(char* r0,int r4)
{
char r1=*r0;
if (1&r1) r0--;
int x,y;
backtranslate(r0,&x,&y);

char r=(*r0-_platno+2*_Platbase)>>1;
if (r==_Exploplat) r4=-1-(1>>5);  /* >> ???; stop instant explosion*/
if (r>=_Downplat) r++;
int v=makeobj(r,x+(8<<8),y+(15<<8),0,r4,(32<<16)+16,*r0);

if (!v) {*r0=0; r0[1]=0;}
}

void plplattoobj(char* r0)
{
  plattoobjins(r0,vvec>>3);
vvec=0;
 plattoins:
playerfire();
}

void crumblecheck(char* r1)
{
if (*r1<_crumblestandlowlim) return;
if (*r1>_crumblestandhighlim) return;
if (falling<8) return;
if (vvec<(2<<8)) return;
(*r1)++;
if (((*r1)&3)==0) *r1=0;
}

void telep()
{
initplx=xpos; initply=ypos;
if (downpress<16) return;
if (telepctr) return;

char* r2=boardadr->contents;
char* r3=r2+boardadr->width*boardadr->height;
int r4=*pladr7;
int r5=(r4&2)?1:-1;
if ((leftpress>=64)&&(uppress==0)&&(fire!=0)&&(fire<=downpress)&&(rightpress>=64))
{
r5=-r5;
bidforsound(_Explochannel,_SampJump,0x7f,0x1000,0,0,50,0,CHUNK_TELEP_1);
}
normtelep:
downpress=0;
r4^=2;
char* r1;
for (r1=pladr7;;r1+=r5)
{
loop62:
if (r1<=r2) {telepoffleft: return;}
if (r1>=r3) {telepoffright: return;}
if (*r1==r4) break;
}
telepfound:
backtranslate(r1,&telepxpos,&telepypos);
telepxpos+=(8<<8);
telepypos-=(17<<8);
telepctr=1;
bidforsound(_Playerchannel,_Samporgan,0x7f,0x1000,0xff00,0x50000a00,25,127,CHUNK_TELEP_2);
bidforsound(_Sparechannel,_Samporgan,0x7f,0x4000,0xff00,0x1000f400,25,-127,CHUNK_TELEP_3);
}


void playerfire()
{
if (snuffctr) return;
if (blamctr) blamctr+=2;
if (blamctr>=224) {goblam(); return;}
if (plweapontype==0) return;
if (fire) dofire();
 else // I added this, can't find it in the source
if (rocketflag) launchrocket();
}

void goblam()
{
if (plweapontype==_rocketblamno) {rocketblam(); return;}
for (int r9=32;r9>0;r9--)
{
  loopa0:;
int r3=r9-16;
int r4=(r3<0)?-r3:r3;
int r2, r0;
if (plface==1) {r0=xpos-(20<<8); r2=(r4<<5)-(4<<8);}
else {r0=xpos+(20<<8); r2=(4<<8)-(r4<<5);}

makeproj(r0,ypos+(12<<8),r2,r3<<6,57,1<<15);
}
 int r1,r3;
if (plface==1) {r1=xpos-(24<<8); r3=-(1<<8);}
else {r1=xpos+(24<<8); r3=(1<<8);}
 explogonopyroquiet(r1,ypos+(4<<8),r3,0,0,0/*r6?*/,0);
plfired=1;
blamctr=0;
bidforsound(_Firechannel,_SampAtomExplo,0x7f,fullpitch,0,0,8,0,CHUNK_BLAM);
}

void rocketblam()
{
  int r0,r1,r2,r3,r4;
for (int r9=32;r9>0;r9--)
{
loopa1:
r1=ypos+(2<<8);
r3=*(rockettabadr+r9*6);
if (plface==1) {r0=xpos-(12<<8); r4=1; r2=-(4<<8); r3=-r3;}
else {r0=xpos+(12<<8); r4=0; r2=(4<<8);}
r3*=2;

if (r3>170) r4+=2;
if (r3>60) r4+=2;
if (r3<-60) r4-=2;
if (r3<-170) r4-=2;
// ??? CMP R3,#1
r4+=36;
makeproj(r0+r2*2,r1+r3*2,r2,r3,r4,0x12<<8);
}

if (plface==1) {r1=xpos+(24<<8); r3=(1<<9);}
else {r1=xpos-(24<<8); r3=-(1<<9);}
 explogonopyroquiet(r1,ypos-(8<<8),r3,0,1,/*r6*/0,0);

plfired=1;
blamctr=0;

bidforsound(_Firechannel,_SampAtomExplo,0x7f,fullpitch,0,0,8,0,CHUNK_BLAM);
rocketblamctr-=1;
if ((rocketblamctr&(1<<7))==0) return;
plweapontype=_rockbase;
 explogoquiet(xpos,ypos+(4<<8),0,-(1<<7),0,/*r6*/0,0);
}

void dofire()
{
if (plweapontype>=_rockbase) {firerocket(); return;} // no return
firempmg:
if (plweapontype==_mpmgblamno) {blamfire(); return;}

if (framectr-firelastframe<firerate) return;
firelastframe=framectr;
int r2=(plface==1)?-(hvec>>1)-(plweaponspeed<<8):(hvec>>1)+(plweaponspeed<<8);
int r5;
switch (plweapontype&7)
{
case 2: r5=(1<<18)|(1<<10); break;
case 3: r5=(1<<18)|(1<<10)|(1<<14); break;
case 4: r5=(1<<18)|(1<<10)|(3<<13); break;
case 5: r5=(1<<18)|(1<<10)|(1<<20)|(3<<12); r2>>=2; break;
case 6: r5=(1<<18)|(1<<10)|(1<<11); break;
case 7: r5=(1<<18)|(1<<10)|(9<<11); break;
default: r5=1<<22;
}
makeproj(xpos+((plface==1)?-(14<<8):(14<<8)),ypos+(12<<8),
  r2,(random()&0xff)-0x7f,(plface==1)?31:30,r5);
plfired=1;
bidforsound(_Firechannel,_SampCannon,0x7e,fullpitch,0,0,2,0,CHUNK_FIRE);
}

void blamfire()
{
if (blamctr!=0) return;
if (plweapontype==8) blamctr=128;
else blamctr=1;
bidforsound(_Firechannel,_Samprave,0x7e,fullpitch,0,(fullpitch<<17)+0x200,40,0,CHUNK_BLAMFIRE);
}

void firerocket()
{
if (plweapontype==_rocketblamno) {blamfire(); return;}
rocketflag=1;
int r2=2;
if (plweapontype==_rockbase+6) r2=8;
else if (plweapontype>=_rockbase+1) r2=4;
if (plface==1) rocketctr+=r2;
else rocketctr-=r2;
if (rocketctr>=_rockettablen) rocketctr=0;
if (rocketctr<0) rocketctr=_rockettablen-1;
}

void launchrocket()
{
if (framectr-firelastframe<(int)firerate) return;
firelastframe=framectr;
 int r0,r1,r2,r3,r4;
if (plface==1) {r0=xpos-(12<<8); r2=-(plweaponspeed<<8); r3=-*(rockettabadr+rocketctr); r4=1;}
else {r0=xpos+(12<<8); r2=plweaponspeed<<8; r3=*(rockettabadr+rocketctr); r4=0;}
r1=ypos+(2<<8);
if (r3>170) r4+=2;
if (r3>60) r4+=2;
if (r3<-60) r4-=2;
if (r3<-170) r4-=2;
// ??? CMP R3,#1
r4+=36;
int r6=plweapontype-_rockbase;
if (r6>=6) r6=random()&7;
 int r5;
switch (r6)
{
case 1: r5=(1<<19)|(1<<10)|(1<<14); // split
	break;
case 2: r5=(1<<19)|(1<<10)|(3<<13); //split twice
	break;
case 3: r5=        (1<<10)|(9<<12)  // burst
		  |(5<<18)        ; //   a safe distance away
	break;
case 4: r5=(7<<17)|(1<<10)|(0xd<<12); //dual burst
	break;
case 5: r5=(1<<19)|(1<<10)|(0xf<<12); //dual burst
         /*(1<<18)*/
	break;
default: r5=1<<22;
}
r5|=(1<<15);
makeproj(r0+r2+r2,r1+r3+r3,r2,r3,r4,r5);
plfired=1;
bidforsound(_Firechannel,_SampRocket,0x7e,fullpitch,0,0,2,0,CHUNK_ROCKET);
rocketflag=0;
}

void getarms()
{
blamctr=0;
plweapontype=0;
}

void getrocket()
{
plweapontype=_rockbase+(random()&7);
plweaponspeed=2;
firerate=12;
blamctr=0;
}

void getmpmg()
{
plweapontype=_mpmgbase+(random()&7);
plweaponspeed=8;
firerate=4;
blamctr=0;
}

alent* bulcolcheck(int x,int y)
{
bulcolchins:
for (bulcolchent* r11=bulcolchtab;r11<bulcolchptr;r11++)
{
bl10:
if ((r11->xmax>=x)&&(r11->ymax>=y)&&(x>=r11->xmin)&&(y>=r11->ymin))
	bullethit: return r11->r0;
}
bulcolched:
return NULL;
}


void colcheck(alent* al)
{
  int x, y;
dodgypointer=al;
 if (colchecktype!=0) {x=al->x; y=al->y;}
 else {x=xpos; y=ypos;}
alonobj=colchecktype;
if (colchecktype==0) //player check
{
platsandstr=NULL;
colchtabuse=colchtab;
colchptruse=colchptr;
}
else
{
colchtabuse=oldcolchtab;
colchptruse=oldcolchptr;
}

//ADD R0,R0,R2
//ADD R1,R1,R3

xtemp=x; ytemp=y;
int r4=24<<8, r5=32<<8;
int xlo=x-(r4>>1);
int ylo=y-(r5>>1);
ylo+=8<<8;
int xhi=xlo+r4;
int yhi=ylo+r5;
yhi+=2<<8; //so always in contact with platforms

colchent* r11=colchtabuse-1;
while (1)
{
colchcon:
colchins:
do {
l10:
  r11++;
if (r11>=colchptruse)
{
colched:
return;
}
} while (!((r11->xmax>=xlo)&&(r11->ymax>=ylo)&&(xhi>=r11->xmin)&&(yhi>=r11->ymin)));
if ((r11->ymax-ylo>=(1<<8)) //lim for plat on head -> can move horiz.
	&&(yhi-r11->ymin>=(3<<8))) //same for standing on plat
	{
		limr2:
		if (r11->xmax>xhi) if (al->dx>0) al->dx=0;
		if (r11->xmin<xlo) if (al->dx<0) al->dx=0;
	}
nolimr2:
if (((r11->xmax-xlo)>=(6<<8)) //margin for right on edge
	&&((xhi-r11->xmin)>=(6<<8))) //ditto
{
	if ((yhi-r11->ymin)<=(16<<8)) //safety margin for platform under pl
		rise(r11->r0,al);
	if ((r11->ymax-ylo)<=(16<<8)) //ditto for plat on head
		platonhead(r11->r0,al);
}
}
}

void rise(alent* r6,alent* al)
{
if (colchecktype==2)  //no rise
	{dontrise: al->dy=0; alonobj=1; return;}
int r0=r6->type;
if (r0==_Fastplatfire) platfire(r6);
if ((r0==_Downplat)||(r0==_Fallplat))  //plat type 5 falls
{
platfall:
if (r0==_Fallplat) r0=r6->dy; else r0=(1<<4)+r6->dy;
if (r0<-(1<<5))  r0+=(1<<5); //plat is moving up - more speed
if (r0>(1<<11))  r0=1<<11;
r6->dy=r0;
if (al->dy>r0)  al->dy=r0;
if (colchecktype==0)   {falling=0xff; platsandstr=&r6->dy;}
al->dy+=r6->y-(32<<8)-platypos;
return;
}
if (r0>_Fastplat) //plat 6+ moves quickly
     r6->dy-=(1<<7);
if (colchecktype==0) r0=pluphit;
else r0=alheadcheck();

if (r0==0) r0=r6->dy /*hit*/ -(1<<5);
else r0=headonroof(r6);

if (r0>(1<<5)) r0-=1<<5; //plat is moving down - more speed

if (r0<-(3<<10))
{
r0=-(3<<10);
platmaxspeed:
  if (r6->type==_Fastplatstop)
    {
    platstop:
      platsurefire(r6);
      r0=-(1<<9);
    }
  else if ((r6->type==_Fastplatexplo)||(r6->type==_Fastplatfire))
    {
 platexplo:
      platdestroy(r6);
      return;
  }
}
platmaxspeedins:;

r6->dy=r0;
if (al->dy>=r0) al->dy=r0;

if (colchecktype==0) {falling=0xff; platsandstr=&r6->dy;}

al->dy+=(r6->y-(32<<8)-platypos);
if (colchecktype!=0) {alonobj=1; al->dx>>=2;}

} //B colchcon


void platonhead(alent* r6, alent* al)
{
if (r6->type==_Exploplat) {platdestroy(r6); return;}
if (r6->dy>0) r6->dy=-r6->dy;
if (r6->dy>al->dy) r6->dy+=(al->dy>>1);
platuphit=1;
if (al->dy<0) al->dy=0;
if (falling==0xff)
{
platsandwich:
if ((colchecktype==0)&&(platsandstr!=NULL)) *platsandstr=(1<<8);
}
} //B colchcon

void platdestroy(alent* r6) {explogo(r6->x,r6->y,0,-(1<<6),0,0,r6);}

void platfire(alent* r6)
{
// R3: best negative (up)
int r4=random();
if ((r4&(3<<24))==0) // firing rate
{
makebul(r6->x,r6->y+(16<<8),(r4&(0xfe<<2))-(0xfe<<1),-((r4&(0xfe<<12))>>11),
	_bulspritebase+8,1<<24);
}
platfireskip:
return;
}

void platsurefire(alent* r6)
{
// R3: best negative (up)
int r4=random();
makebul(r6->x,r6->y+(16<<8),(r4&(0xfe<<2))-(0xfe<<1),-((r4&(0xfe<<12))>>11),
	_bulspritebase+14,1<<24);
}

//.colchcon
//LDMFD R13!,{R0,R1,R4,R5,R6,R11}
//B colchins

int headonroof(alent* r6)
{
  int r0, r1;
if (colchecktype!=0)
{
if ((random()&7)==0)
	dodgypointer->y=(random()&(1<<9))-(1<<8);
skipalrandload:
r0=1; r1=0;
}
else
{
r0=1<<8; r1=downpress;
}
skipalrand:
r6->y=platypos+(32<<8); //force plat to player pos
if (r1>64)  r0=1<<11;
if (r6->type==_Updownplat)  r6->type=_Downplat;
return r0;
}

int alheadcheck()
{
//ADD R1,R1,R3
char* r0=translate(xtemp,ytemp);
 return ((*(r0-boardwidth)>=_blim)||(*(r0-boardwidth+1)>=_blim));
}

char* albcheck(alent* r11)
{
lefthit=righthit=uphit=downhit=0; // wipe all four
alonplat=0;
xtemp=r11->x-(1<<8); ytemp=r11->y-(8<<8);
int r6=ytemp&(15<<8);
char* z=translate(xtemp,ytemp+r11->dy);
// up
if ((*(z-boardwidth)>=_blim)||(*(z-boardwidth+1)>=_blim))
	noup(r11);
//down
if ((*(z+boardwidth)>=_blim)||(*(z+boardwidth+1)>=_blim))
	nodown(r11);
if (r6!=(15<<8))
{
z=translate(xtemp+r11->dx,ytemp);
//left
if ((*(z-boardwidth)>=_blim)||(*z>=_blim)||(*(z+boardwidth)>=_blim))
	noleft(r11);
//right
if ((*(z-boardwidth+1)>=_blim)||(*(z+1)>=_blim)||(*(z+boardwidth+1)>=_blim))
	noright(r11);
}
else {
alshort:
z=translate(xtemp+r11->dx,ytemp);
if ((*z>=_blim)||(*(z+boardwidth)>=_blim))
	noleft(r11);
//right
if ((*(z+1)>=_blim)||(*(z+boardwidth+1)>=_blim))
	noright(r11);
}

albcheckins:
z=translate(xtemp+r11->dx,ytemp+r11->dy);
// up
if ((*(z-boardwidth)>=_blim)||(*(z-boardwidth+1)>=_blim))
	noup(r11);
//down
if ((*(z+boardwidth)>=_blim)||(*(z+boardwidth+1)>=_blim))
	nodown(r11);
char* r5=(z+(boardwidth<<1)); //signal position to caller
if ((*r5<=_alplathighlim)&&(*r5>=_platlowlim)) alonplat=1;
return r5;
}

void bcheck()
{
uphit=downhit=0;
//AND R6,R1,#15<<8
 xtemp=fraudalent.x=xpos; ytemp=fraudalent.y=ypos;
 fraudalent.dx=hvec; fraudalent.dy=vvec;
char* r11=translate(fraudalent.x,fraudalent.y+fraudalent.dy);
// up
if ((*(r11-boardwidth)>=_blim)||(*(r11-boardwidth+1)>=_blim))
	noup(&fraudalent);
//down

if ((*(r11+boardwidth)>=_blim)||(*(r11+boardwidth+1)>=_blim))
	nodown(&fraudalent);
if ((*(r11+boardwidth*2)>=_blim)||(*(r11+boardwidth*2+1)>=_blim))
	nodownifplat(&fraudalent);
//fallinggap(&fraudalent);
// CMP R6,#15<<8
// not sure how the original game manages without fallinggap's return value.
int fg=fallinggap(&fraudalent);
if ((!fg)&&((ytemp&(15<<8))!=(15<<8)))
{
  r11=translate(fraudalent.x+fraudalent.dx,fraudalent.y);
 char* r10=translate(fraudalent.x+fraudalent.dx,fraudalent.y/*+(4<<8)*/); // 4<<8 was commented out
//left

if ((*(r10-boardwidth)>=_blim)||(*(r11)>=_blim)||(*(r11+boardwidth)>=_blim))
	noleft(&fraudalent);
//right

if ((*(r10-boardwidth+1)>=_blim)||(*(r11+1)>=_blim)||(*(r11+boardwidth+1)>=_blim))
	noright(&fraudalent);
}
 else if (fg==-1)
{
r11=translate(fraudalent.x+fraudalent.dx,fraudalent.y);
if ((*(r11-boardwidth)>=_blim)||(*r11>=_blim))
	noleft(&fraudalent);
//right
if ((*(r11-boardwidth+1)>=_blim)||(*(r11+1)>=_blim))
	noright(&fraudalent);
}
else
{
shorT:
r11=translate(fraudalent.x+fraudalent.dx,fraudalent.y);
if ((*(r11)>=_blim)||(*(r11+boardwidth)>=_blim))
	noleft(&fraudalent);
//right
if ((*(r11+1)>=_blim)||(*(r11+1+boardwidth)>=_blim))
	noright(&fraudalent);
}

bcheckins:
r11=translate(fraudalent.x+fraudalent.dx,fraudalent.y+fraudalent.dy);
// up
if ((*(r11-boardwidth)>=_blim)||(*(r11-boardwidth+1)>=_blim))
	noup(&fraudalent);
//down
if ((*(r11+boardwidth)>=_blim)||(*(r11+boardwidth+1)>=_blim))
	nodown(&fraudalent);
 xpos=fraudalent.x; ypos=fraudalent.y;
 hvec=fraudalent.dx; vvec=fraudalent.dy;
}


void noleft(alent* r11)
{
if (r11->dx<0)
{
  r11->dx=0; // -(xtemp&0xfff); // 0;
lefthit=1;
}
}

void noright(alent* r11)
{
if (r11->dx>0)
{
  r11->dx=0; // ((0xfff-xtemp)&0xfff); // 0;
righthit=1;
}
}

int fallinggap(alent* re)
{
  char* r11=translate(xtemp+re->dx,ytemp/*+dy*/-((re->dy<0)?(15<<8):0));
int r1=(re->dx<0)?0:1;

if ((*(r11+r1)<_blim)&&(*(r11+boardwidth+r1)<_blim))
{
stopfall:
if (re->dy>0)
{
stopfalldown:
if (*(r11-boardwidth+r1)<_blim) return 0;
if (*(r11+boardwidth*2+r1)<_blim) return 0;
 if (*(r11+boardwidth*2+(1-r1))<_blim) {nodown(re); return 1;}
}
else if (re->dy<0)
{
stopfallup:
if ((ytemp&(15<<8))>(8<<8)) return 0;
if (*(r11-boardwidth+r1)<_blim) return 0;
if (*(r11+boardwidth*2+r1)<_blim) return 0;
 if (*(r11+boardwidth*2+(1-r1))<_blim) {noup(re); return -1;}
}
}
}

void nodown(alent* r11)
{
if (r11->dy>0)
{
  r11->dy=((15<<8)&((16<<8)-ytemp))-1;
if (r11->dy<0) r11->dy=0;
}
nodownrel:
downhit=1;
}

void nodownifplat(alent* r11)
{
if (falling!=0xff) return;
if (r11->dy>0)
{
  r11->dy=((15<<8)&((16<<8)-ytemp))-1;
if (r11->dy<0) r11->dy=0;
}
//nodownrel:
downhit=1;
}

void noup(alent* r11)
{
if (r11->dy>=0) return;
r11->dy=-((ytemp+(1<<8))&(15<<8));
uphit=1;
}

char* translate(int r0,int r1)
  {return (char*)boardadr->contents+(r0>>12)+(r1>>12)*boardadr->width;}

char* fntranslate(int r0,int r1)
  {return (char*)boardadr->contents+((r0+(8<<8))>>12)+((r1-(8<<8))>>12)*boardadr->width;}

void backtranslate(char* r, int* x, int* y)
{
int r0=r-boardadr->contents;
int r1=boardwidth;
*x=(r0%r1)<<12;
*y=(r0/r1)<<12;
}
/*LDR R1,[R12,#boardadr]
ADD R1,R1,#boardhdrlen
SUB R0,R0,R1
LDR R1,[R12,#boardwidth]
MOV R2,#0
MOV R3,#0
MOV R4,#1<<31
.l9
MOVS R0,R0,ASL #1
ADC R3,R3,R3
CMP R3,R1
SUBGE R3,R3,R1
ORRGE R2,R2,R4
MOVS R4,R4,LSR #1
BNE l9

MOV R0,R3,LSL #12
MOV R1,R2,LSL #12
MOV PC,R14*/

void bonuscheck()
{
if (snuffctr)
{
deadbonuscheck:
atombombctr=0;
electrocuting=0;
deadbonuslim(pladr1);
deadbonuslim(pladr2);
deadbonuslim(pladr3);
deadbonuslim(pladr4);
return;
}

atombombctr=0;
electrocuting=0;
plbombcheck(pladr1);
bonuslim(pladr1);
weaponcheck(pladr1);
plbombcheck(pladr2);
bonuslim(pladr2);
weaponcheck(pladr2);
plbombcheck(pladr3);
bonuslim(pladr3);
weaponcheck(pladr3);
plbombcheck(pladr4);
bonuslim(pladr4);
weaponcheck(pladr4);
}

void weaponcheck(char* r5)
{
if ((*r5<_weaplowlim)||(*r5>_weaphighlim)) return;
plweapontype=*r5-_weaplowlim+1;
*r5=0;
if (plweapontype>=_rockbase)
	{plweaponspeed=2; firerate=12;}
else	{plweaponspeed=8; firerate=4;}
rocketblamctr=5;

int r2=plweapontype-1;
 int r7, r0;
if ((r2&7)==7) {r7=_scoresprbase+12; r0=4000;}
else if (r2&4) {r7=_scoresprbase+10; r0=2000;}
else {r7=_scoresprbase+9; r0=1000;}
plscoreadd+=r0;

makeobj(_Scoreobj,xpos,ypos,(random()&(0xfe<<1))-(0xfe)+(hvec>>2),
	-(random()&(0xfe<<2)),60+(r7<<8),666);
bidforsound(_Playerchannel,(r7==_scoresprbase+12)?_Samprave:_Samporgan,
	    0x7f,0x3000,0xff00,(r7==_scoresprbase+12)?0x60000800:0x60000200,10,127,
	    (r7==_scoresprbase+12)?CHUNK_WEAPON_3:CHUNK_WEAPON_1);
bidforsound(_Sparechannel,(r7==_scoresprbase+12)?_Samprave:_Samporgan,
	    0x7f,0x3800,0xff00,(r7==_scoresprbase+12)?0x60000800:0x60000200,10,-127,
	    (r7==_scoresprbase+12)?CHUNK_WEAPON_4:CHUNK_WEAPON_2);
/* XXX Original code also kills message so the same one is never redisplayed */
switch ((plweapontype-1)&15)
{
case 0:  message(320,192,-3,0,"Standard Mini-Gun"); break;
case 1:  message(320,192,-3,0,"Mini-Gun with spray"); break;
case 2:  message(320,192,-3,0,"Five Stream Gun"); break;
case 3:  message(320,192,-3,0,"Fast 5 way - zap those nasties!"); break;
case 4:  message(320,192,-3,0,"A weird one!"); break;
case 5:  message(320,192,-3,0,"Three Way Blitzer"); break;
case 6:  message(320,192,-3,0,"Blitzing Five Way - Lets Party"); break;
case 7:  message(320,192,-3,0,"MegaBlam 5000 Mini-Gun!!!!"); break;
case 8:  message(320,192,-3,0,"Standard Rocket Launcher"); break;
case 9:  message(320,192,-3,0,"Twin Rocket Launcher"); break;
case 10: message(320,192,-3,0,"Quad Launcher - Open Fire!"); break;
case 11: message(320,192,-3,0,"Launcher with starburst"); break;
case 12: message(320,192,-3,0,"Twin rockets with starburst - let's see some fireworks!"); break;
case 13: message(320,192,-3,0,"Quad Launcher with starburst! Arrgghhhh!!!!"); break;
case 14: message(320,192,-3,0,"A Pot Pourri Rocket Launcher - mix and match!"); break;
case 15: message(320,192,-3,0,"MegaBlam Rocket Launcher!!!!  Six Shots Only!"); break;
}

}

void plbombcheck(char* r5)
{
int r1=bombcheck(r5);
if ((*r5>=_targetlowlim)&&(*r5<=_targethighlim))
	r1=normalbomb(r5);
nopltarg:
plstrength-=r1;
}

int bombcheck(char* r5)
{
char r0=*r5;
if ((r0<_bomblowlim)||(r0>_bombhighlim)) return 0;
if ((r0==_fuelairno)||(r0==_fuelairno+1)) return fuelairbomb(r5);
if (r0>=_boobylowlim) return boobybomb(r5);
if ((r0<_atomlowlim)||(r0>_atomhighlim)) return normalbomb(r5);
return atombomb(r5);
}

int atombomb(char* r5) // Caution - recursive routine
{
if (atombombctr>=0x1f)  {atomabandon: return atomloss;}
atombombctr++;
char r0=(*r5)-_atomlowlim;
if (r0&1) r5-=1;
if (r0&2) r5-=boardwidth;

*r5=0;
*(r5+1)=0;
*(r5+boardwidth)=0;
*(r5+boardwidth+1)=0; // delete this a-bomb
procatom(r5-boardwidth);
procatom(r5-boardwidth+1);
procatom(r5-1);
procatom(r5+2);
procatom(r5+boardwidth-1);
procatom(r5+boardwidth+2);
procatom(r5+2*boardwidth);
procatom(r5+2*boardwidth+1);
int x,y;
backtranslate(r5,&x,&y);
atomexplogo(x+(8<<8),y+(24<<8),0,0,0,0,0 /*signal no existing object*/);
return atomloss;
}

void procatom(char* r5)
{
char r0=*r5;
if ((r0>=_targetlowlim)&&(r0<=_targethighlim))
	shoottarget(r5);
noatomtarget:
*r5=0;

if ((r0<_atomlowlim)||(r0>_atomhighlim))  return;

*r5=r0; //replace bomb segment
atombomb(r5);
}

int fuelairbomb(char* r5)
{
deletetwin(r5);
char* r0=r5-boardwidth;
*r0=_gaslowlim;
*(r0+1)=_gaslowlim;
if ((*fueltabread==0)||(*(fueltabread+1)==0))
	*(fueltabread+2) = 0; // replace end marker if overwritten
*fueltabread=r0;
*(fueltabread+1)=r0+1;
fueltabctr=_fuelairduration; // fuel-air duration
bidforsound(_Explochannel,_SampHiss,0x6f,0x6000,0xff80,0x2000ff00,50,0,CHUNK_FUELAIR);
return 0;
}

int normalbomb(char* r5)
{
elecdestroy(r5);
*r5=0;
int x,y;
backtranslate(r5,&x,&y);
explogo(x,y+(8<<8),0,0,0,0,0 /*signal no existing object*/);
return bombloss;
}

int boobyvectab[] =
	{-0x400, -0x80, 0x400, -0x80, 0, -0x400, 0, 0x400};
	
int boobybomb(char* r5)
{
int* r6=boobyvectab+2*((*r5-_boobylowlim)&3);
*r5=0;
int x,y;
backtranslate(r5,&x,&y);
explogonopyro(x,y+(8<<8),-(r6[0]>>2),-(r6[1]>>2),
	0,0,0/*;signal no existing object*/);
makeobj(_Booby,x,y+(8<<8),r6[0],r6[1],0,0);
return bombloss;
}


void bonusobjgot(alent* r11)
{
r11->dx=(random()&(0xfe<<1))-0xfe;
r11->dy=-(random()&(0xfe<<2))-(1<<9);

int r0=r11->r6;
sortbonus(r0);
r0=(r0-_bonuslow+2)>>1;
if (r0>10) r0=10;
plscoreadd+=100*r0;
r11->r5=0xf60+(r0<<8);
r11->type=_Scoreobj;
bidforsound(_Playerchannel,_SampBonus,0x6e,0x3000+(_Scoreobj<<6),0,0,3,0,CHUNK_OBJGOT[11]);
}
 

void bonuslim(char* r5)
{
if ((*r5>=_neuronlowlim)&&(*r5<=_neuronhighlim))
{
plzone=*r5-(_neuronlowlim-1);
*r5=0;
initplx=xpos;
initply=ypos;
return;
}
noneuron:
if ((*r5>=_eleclowlim)&&(*r5<=_elechighlim))
	{electrocute(r5); return;}
bonuslimcont:
if (*r5<_bonuslow) return;
if (*r5>_megabonuslim) return;
if (*r5>_bonushigh) megabonus(r5);
else bonusgot(r5); // was bonusgot
}

void deadbonuslim(char* r5)
{
char r0=*r5;
if ((r0<_bonushigh-2)||(r0>_bonushigh)) return;
bonusgot(r5);
}

void bonusgot(char* r5)
{
// MOV R1,R0 ???
char r0=*r5;
sortbonus(r0);
*r5=0;
r0=(((int)r0)-_bonuslow+2)>>1;
if (r0>10) r0=10;
addbonus(r5,r0);
bidforsound(_Playerchannel,_SampBonus,0x6e,(r0<<6)+0x3000,0,0,3,0,CHUNK_OBJGOT[r0-1]);
}

void addbonus(char* r5, int r0)
{
plscoreadd+=100*r0;
int x,y;
backtranslate(r5,&x,&y);
makeobj(_Scoreobj,x,y,(random()&(0xfe<<1))-(0xfe)+(hvec>>2),
	-(random()&(0xfe<<2)),0xf60+(r0<<8),*r5);
}

void sortbonus(char r0)
{
r0-=_bonuslow;
if ((r0==13)||(r0==14))
{
	bonusstrength:
	lagerctr+=(r0==14)?150:50;
	laststrength=plstrength;
}

if (r0==bonusctr)
{
	advancebonus:
	bonusctr++;
	//sortreplot:
	bonusreplot=28;
	return;
}

if (r0==15)
{
	bonusctr=0;
	if (snuffctr)
	if (plstrength>0)
	{
	prepstrength();
	message(48,24,0,0.5,"A potion and Skull!");
	message(48,176,0,-0.5,"Rise from the dead!");
	}
	sortreplot:
	bonusreplot=28;
}
return;
}

void bonusnumb(int r9)
{
int r6=-1;
for (r9&=0xff;r9>0;r9-=(r6+1))
{
loopc3:
r6=random()&3;
makeobj(_Scoreobj,xpos,ypos,(hvec>>2)+((random()&(0xfe<<1))-0xfe),
	-(random()&(0xfe<<2)),0xf60+(10<<8)+(r6<<8),r6);
}
}

void bonus1() {bonusnumb(10); message(96,224,0,-2,"Bonus 10000"); plscoreadd+=10000;}

void megabonus(char* r5)
{
char r0=*r5;
*r5=0;
bidforsound(_Playerchannel,_SampBonus,0x7e,0x2200,0,0,5,127,CHUNK_BONUS_1);
bidforsound(_Sparechannel,_SampBonus,0x7e,0x2000,0,0,5,-127,CHUNK_BONUS_2);
if (r0==_bonushigh+1)
{
	bonus1:
	bonusnumb(10);
	message(96,224,0,-2,"Bonus 10000");
	plscoreadd+=10000;
}
else if (r0==_bonushigh+2)
{
	bonus2:
	bonusnumb(20);
	message(96,224,0,-2,"Bonus 20000");
	plscoreadd+=20000;
}
else if (r0==_bonushigh+3)
{
	bonus3:
	bonusnumb(30);
	message(96,224,0,-2,"Bonus 30000");
	plscoreadd+=30000;
}
else if (r0==_bonushigh+4)
{
	bonus5:
	bonusnumb(50);
	message(96,224,0,-2,"Bonus 50000");
	plscoreadd+=50000;
}
else
{
	bonus10:
	bonusnumb(100);
	message(96,224,0,-2,"Bonus 100000");
	plscoreadd+=100000;
}
return;
}

void electrocute(char* r5)
{
electrocuting=1;
int x,y;
backtranslate(r5,&x,&y);
explogonopyro(x,y+(16<<8),(random()&(0xfe<<2))-(0xfe<<1),
	(random()&(0xfe<<2))-(0xfe<<1),0,0,0);
}

void destroy(char* r5)
{
/*LDR R1,[R12,#boardlowlim]
CMP R0,R1 ???
LDR R1,[R12,#boardhighlim]
CMP R0,R1*/

bombcheck(r5);

char r0=*r5;

if ((r0==_gaslowlim)||(r0==_gashighlim))  {normalbomb(r5); return;}
if ((r0>=_bonuslow)&&(r0<=_bonushigh))
{
        int x,y;
	backtranslate(r5,&x,&y);
	y+=(8<<8);
	makeobj(_Dyingbonus,x,y+(7<<8),0,0,(*r5)|(1<<20),666);
	*r5=0;
}
noshootbonus:
if ((r0>=_targetlowlim)&&(r0<=_targethighlim))
	{shoottarget(r5); return;}
noshoottarget:
if ((r0>=_crumblelowlim)&&(r0<=_crumblehighlim))
	if (0==(3&(++(*r5))))  *r5=0;
nocrumble:
return;
}

void shoottarget(char* r5)
{
char r3=*r5;
*r5=0;
plscoreadd+=targetscore;
if (r3>=_powertarget)  elecdestroy(r5);

int x,y;
backtranslate(r5,&x,&y);

explogo(x,y+(8<<8),0,0,0,0,0 /* signal no existing object */);
makeobj(_Scoreobj,x,y,(hvec>>2)+((random()&(0xfe<<1))-0xfe),
	-(random()&(0xfe<<2)),0xf60+((targetscore/100)<<8),666);
}

void elecdestroy(char* r5)
{
eleccheck(r5-boardwidth);
eleccheck(r5-1);
eleccheck(r5+1);
eleccheck(r5+boardwidth);
}

void eleccheck(char* r10)
{
char r0=*r10;
if ((r0<_eleclowlim)||(r0>_elechighlim))  return;
int r4=(r0>=_elecvertlowlim)?boardwidth:1;
elecdelete(r4,r10);
elecdelete(-r4,r10);
}

void elecdelete(int r4,char* r10)
{
char r2=_eleclowlim;
for (;(r2>=_eleclowlim)&&(r2<=_elechighlim);r2=*(r10+=r4))
	loop78:
	*r10=0;
}

void deletetwin(char* r5)
{
*r5=0;
if ((r5-boardadr->contents)&1) *(r5-1)=0;
else *(r5+1)=0;
}

void deletepoint()
{
for (int r3=16;r3>0;r3--)
  {
	delloop:;
  dodeletepoint:;
int x,y;
x=xpos+(((random()&15)+(random()&7)-11)<<12);
if (x<(1<<12)) return;
y=ypos+(((random()&15)-7)<<12);
if (y<(1<<12)) return;
if (x>xposmax-(1<<12)) return;
if (y>yposmax-(1<<12)) return;
x&=~0xfff;
y&=~0xfff;
char* z=translate(x,y);
if (*z==0) return;
*z=0;
makeobj(_Dyingbonus,x,y+(15<<8),0,0,r3|(1<<20),666);
  }
}

void mazeplot()
{
writeclip();
backdrop();

int r0=(xpos-(144<<8))>>8;
int r1=(ypos-(96<<8))>>8;
int r8=15-(r0&15);
int r9=15-(r1&15);


int r5=0, r7=(r1>>4);
if (r7<0) {r5=-r7; r7=0;}

 for (;/*ins2:*/ (r5<14)&&(r7<boardadr->height);r5++,r7++)
{
l2:;
int r4=0,r6=((xpos>>8)-144)>>4;
if (r6<0) {r4=-r6; r6=0;}

for (;/*ins1:*/ (r4<21)&(r6<boardadr->width);/*skip1:*/ r4++,r6++)
{
l1:
r0=*(boardadr->contents+boardwidth*r7+r6);
if (r0==0) continue;
if ((r0&~7)==_neuronlowlim) r0=_neuronlowlim+((framectr&(7<<2))>>2);
if (r0<=_crumblestandhighlim)
{
if (r0>=_crumblelowlim) r0&=~8; //alter for crumbles
if ((r0>=_animlowlim)&&(r0<=_animhighlim))
{
int r1=random()&7;
if ((r0==_gaslowlim)||(r0==_gashighlim)) r0^=r1>>2;
else r0^=r1>>1;
}}
noanimate:
if ((r0>=_weaplowlim+1)&&(r0<=_weaplowlim+6))
     r0=_weaplowlim+1;
fspplot(blockadr,r0,(r4<<4)+r8,(r5<<4)+r9);
}
}
skip2:;
}

#define FORTYEIGHT (48*4)
#define TTOFFSET (hbytes*32)

char backdrop_to_blit[FORTYEIGHT];

void backdrop()
{
  //  swi_fastspr_clearwindow();
SDL_LockSurface(ArcScreen);
screentop=screenuse+hbytes*8+4*16;

int r3=((xpos>>8)-(xpos>>10)+3072-768); // parallax
backuse=backadr+(r3&3)*1536*sizeof(Uint32);
int r2=11-((r3>>2)%12);

// self-modifying code went here on ARM:
// modpos[0]=backtab[0]; modpos[1]=backtab[1];

int r4=0x1f&((ypos>>8)-(ypos>>10)); // parallax

for (int r3=0;r3<32;r3++)
{
l4:
char* r14=screentop+r3*hbytes;
char* r12=backuse+r4*FORTYEIGHT;
modpos:
memcpy(backdrop_to_blit+r2*4*4,r12,(12-r2)*4*4);
memcpy(backdrop_to_blit,r12+(12-r2)*4*4,r2*4*4);

memcpy(r14     ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT  ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*2,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*3,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*4,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*5,backdrop_to_blit,FORTYEIGHT);
r14+=TTOFFSET;
memcpy(r14     ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT  ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*2,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*3,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*4,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*5,backdrop_to_blit,FORTYEIGHT);
r14+=TTOFFSET;
memcpy(r14     ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT  ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*2,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*3,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*4,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*5,backdrop_to_blit,FORTYEIGHT);
r14+=TTOFFSET;
memcpy(r14     ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT  ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*2,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*3,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*4,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*5,backdrop_to_blit,FORTYEIGHT);
r14+=TTOFFSET;
memcpy(r14     ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT  ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*2,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*3,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*4,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*5,backdrop_to_blit,FORTYEIGHT);
r14+=TTOFFSET;
memcpy(r14     ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT  ,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*2,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*3,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*4,backdrop_to_blit,FORTYEIGHT);
memcpy(r14+FORTYEIGHT*5,backdrop_to_blit,FORTYEIGHT);
if (++r4>31) r4=0;
}
SDL_UnlockSurface(ArcScreen);
}

int escapehandler()
{
frameinc=1;
showchatscreen();
swi_fastspr_setclipwindow(20,20,319-20,255-20);
swi_fastspr_clearwindow();
wipetexttab();
message(36,40-256,0,4,"¤ Game Interrupted ¤");
message(32-256,72,4,0,"Please select action");
message(46+256,104,-4,0,"ESC - Abandon game");
message(40-256,136,4,0,"Q    - lose this life");
message(40,168+256,0,-4,"R    - Return to Game");
message(40,200+256,0,-4,"O    - Alter Options");
showtext();
osbyte_7c(); //clear escape
for (int r9=64;swi_readescapestate()==0;)
{
loopb8:
swi_blitz_wait(0);
if (r9!=0)
{
r9--;
switchbank();
switchfspbank();
swi_fastspr_clearwindow();
texthandler();
}
esctextstop:
 if (osbyte_81(-113)) {loselife(); return 0;}
 if (osbyte_81(-111)) {adjustopt(); return 0;}
 if (osbyte_81(-114)) {rejoin(); return 0;}
}
osbyte_7c();
return 1;
}

void loselife()
{
lagerctr=0;
plstrength=0; //no return
rejoin();
}

void rejoin()
{
wipetexttab();
showgamescreen();
}

void adjustopt()
{
options(1); // mark as in game
rejoin();
}

void copyscreen()
{
//  char* r0=screenstart;
//  char* r2=screenstart+modesize;
//for (int r1=0x14000;r1>0;r1-=1)
//     loop30: *(r2++)=*(r0++);
swi_blitz_screenflush();
}

void setfullclip()
{
cliplx=_lowx; cliply=_lowy;
cliphx=_highx; cliphy=_highy;
writeclip();
return;
}

void writeclip()
{
  static SDL_Rect clip;
  clip.x=cliplx; clip.y=cliply;
  clip.w=cliphx-cliplx;
  clip.h=cliphy-cliply;
  SDL_SetClipRect(ArcScreen,&clip);
}

void releaseclip()
{
SDL_SetClipRect(ArcScreen, NULL);
// 0, 0, 319, 255 ; size of mode
}

void restartplayer()
{
xpos=initplx;
ypos=initply;
screenwakeup();
telepctr=33;
//bonusctr=0; (commented out in original)
bonusreplot=28;
pladr1=0; windctr=0; bonustimer=0;
rocketblamctr=0; shutdownctr=0;
if (neuronctr>=_neuronstoget)
{
completedzone:
message(114,64,0,0,"WELL DONE!");
message(44,96,0,0,"You have completed");
if (mentalzone==3) message(114,128,0,0,"The game!!!");
else message(114,128,0,0,"This zone!");
lives=0;
snuffctr=1;
}
return;
}

void findplayer()
{
bonusctr=0;
char* r5=NULL;
for (int r3=boardadr->width*boardadr->height;r3>0;r3--)
{
loop60:
if (*(boardadr->contents-1+r3)==_markerno)
	{*(boardadr->contents-1+r3)=0; r5=boardadr->contents-1+r3; }
}
int x,y;
if (r5) backtranslate(r5,&x,&y);
else x=y=64<<8;
initplx=x; initply=y-(1<<8);
}

void showgamescreen()
{
decomp(gamescreenadr);
copyscreen();
scorewiperead();
showlives();
}

void showlives()
{
releaseclip();
fspplot(charsadr,lives&7,44,234);
switchbank();
switchfspbank();
fspplot(charsadr,lives&7,44,234);
writeclip();
}

void showchatscreen()
{
decomp(chatscreenadr);
copyscreen();
}

void showchatscores()
{
decomptonot(chatscreenadr);
}

int palette[256], alt_palette[256];

void init_palette()
{
  for (int i=0;i<256;i++) 
    palette[i]
      = 0xff000000 // opaque
      + ((i&0x80)?0x880000:0) + ((i&0x40)?0x8800:0)
      + ((i&0x20)?0x4400:0) + ((i&0x10)?0x88:0)
      + ((i&0x08)?0x440000:0) + ((i&0x04)?0x44:0)
      + (i&0x03)*0x111111;
  for (int i=0;i<256;i++)
    alt_palette[i]
      = 0xff000000 // opaque
      + ((i&0x80)?0x88:0) + ((i&0x40)?0x8800:0)
      + ((i&0x20)?0x4400:0) + ((i&0x10)?0x880000:0)
      + ((i&0x08)?0x44:0) + ((i&0x04)?0x440000:0)
      + (i&0x03)*0x111111;
}

void decomptonot(char* r11) {decompins(screenuse,r11);}

void decomp(char* r11) {decompins(screenstart,r11);}

void decompins(char* rr10,char* r11) 
{
 static SDL_Rect from;
 static SDL_Rect to;
 from.x = from.y = to.x = to.y = 0;
 to.w = ArcScreen->w; to.h = ArcScreen->h;
 from.w = DecompScreen->w; from.h = DecompScreen->h;
  SDL_LockSurface(DecompScreen);
  Uint32* r10=(Uint32*)DecompScreen->pixels; //(Uint32*)rr10;
  Uint32* r9=0x14000+r10;

  r11+=68;
  while (r9>r10) // > or >=?
    {
    loopb4:;
   char r0=*(r11++);
  if (r0&0x80)
    {
    sequence:; Uint32 s=palette[0xff&*(r11++)];
   for (int r3=(r0&0x7f)+2;(r9>r10)&&(r3!=0);r3--)
     loopb6: *(r10++)=s;
    }
  else pattern: for (int r3=(r0&0x7f)+1;(r9>r10)&&(r3!=0);r3--)
    loopb5: *(r10++)=palette[0xff&*(r11++)];
decompdone:;
    }
  SDL_UnlockSurface(DecompScreen);
  releaseclip();
  //SDL_SoftStretch(ArcScreen,NULL,DecompScreen,NULL);
  SDL_BlitSurface(DecompScreen, NULL, ArcScreen, NULL);
  writeclip();
}

void clearkeybuf()
{
do clearkbloop:;
 while (osbyte_79(0)!=-1);
do clearkbloop2:;
 while (osbyte_81(1)!=-1);
}

const int keydefs[] =
  { -SDLK_z, -SDLK_x, -SDLK_SEMICOLON, -SDLK_PERIOD, -SDLK_RETURN};
//{ -52, -53, -47, -60, -36};
    //{ 97^0xff, 66^0xff, 79^0xff, 104^0xff, 73^0xff };

void setdefaults()
{
checkifarm3();
soundtype=2;
soundquality=(arm3==0)?0:1;
soundvol=0x7f;
musicvol=0x7f;
leftkey=keydefs[0];
rightkey=keydefs[1];
upkey=keydefs[2];
downkey=keydefs[3];
firekey=keydefs[4];
gearchange=(arm3==0)?0:1;
explospeed=(arm3==0)?2:1;
fullscreen=0;
joyno=0;
mentalzone=1;
initials[0] = 'P';
initials[1] = 'S';
initials[2] = 'Y';
}

void soundupdate() { return;}
/*
.soundupdate
STMFD R13!,{R14}
LDRB R5,[R12,#soundtype]
LDRB R6,[R12,#soundquality]
CMP R5,#0
BEQ soundkill
MOV R0,#2
SWI "XSound_Enable"
MOV R0,#129
MOV R1,#0
MOV R2,#&FF
SWI "OS_Byte"
MOV R8,R1

MOV R0,#8; channels
CMP R5,#1
MOVEQ R0,#4
MOV R1,#208
MOV R2,#96
TST R6,#1
MOVNE R2,#48
CMP R8,#&A3
MOVLT R2,#0; RISC OS 2 bug

MOV R3,#0
MOV R4,#0
SWI "XSound_Configure"
ADR R0,sptunenorm
TST R6,#1
ADRNE R0,sptunehq
CMP R8,#&A3
ADRLT R0,sptunehq
SWI "XOS_CLI"
LDRB R0,[R12,#soundquality]
TST R0,#1
MOV R0,#6
MOVNE R0,#12
SWI "XSound_QBeat"
MOV R0,#&1000
SWI "XSound_QTempo"
LDRB R0,[R12,#musicvol]
SWI "XBodgeMusic_Volume"
LDRB R0,[R12,#soundquality]
MOV R0,R0,LSR #1
MOV R1,#1
SWI "XStasis_Control"
LDMFD R13!,{PC}
.sptunenorm
EQUS "StasisTune &8000"
EQUB 0
.sptunehq
EQUS "StasisTune &4000"
EQUB 0
ALIGN
.soundkill
MOV R0,#1
SWI "XSound_Enable"
LDMFD R13!,{PC}
*/

int options(int go)
{
gameon=go;
savedornot=0;
return optionins();
}

int optionins()
{
while (1)
{
clearkeybuf();
wipetexttab();
showchatscreen();
 message(128,48,0,0,"Options");
 message(32,96,0,0,"1. Define Controls");
 message(32,128,0,0,"2. Tune Game Options");
 message(88,224,0,0,"Fire - Play");
 if (gameon==0) message(32,160,0,0,"3. Choose Mental Zone");
 if ((gameon==0)&&(savedornot==1)) message(32,192,0,0,"4. Save Settings");
showtext();
switch (readopt(gameon?2:4))
{
case -1: optionexit: return 1;
case  1: choosecontrol(); break;
case  2: tunegame(); break;
case  3: getzone(); break;
case  4: dosaveconf(); break;
default: soundupdate(); return 0;
}
savedornot=1;
}
}

void dosaveconf()
{
saveconfig();
savedornot=0;
}

void getzone()
{
  int r0;
wipetexttab();
showchatscreen();
message(88,48,0,0,"Choose which");
message(96,68,0,0,"mental zone");
message(64,96,0,0,"1. Ego");
message(64,128,0,0,"2. Psyche");
message(64,160,0,0,"3. Id");
if (checkifextend())
{
message(64,192,0,0,"4. Extended Level");
r0=4;
}
else r0=3;
showtext();
r0=readopt(r0);
if (r0==-1) return;
 if ((r0==3)&&(/*testid:*/ idpermit!=1))
{
wipetexttab();
showchatscreen();
message(80,100,0,0,"To play the id");
message(32,128,0,0,"you need to complete");
message(80,156,0,0,"the ego first.");
showtext();
readopt(0);
}
else mentalzone=r0;
}

void choosecontrol()
{
wipetexttab();
showchatscreen();
 message(96,48,0,0,"Controls");
 message(64,96,0,0,"1. Keyboard");
 message(64,128,0,0,"2. Joystick");
if (checkifextend()) message(64,160,0,0,"3. Fnord");
showtext();
switch (readopt(2))
{
case -1: return;
case  1: choosekeys(); return;
case  2: choosestick(); return;
}
}

void choosekeys()
{
wipetexttab();
showchatscreen();
leftkey=selectkey(48,128,0,0,"Press Key For Left");
rightkey=selectkey(48,128,0,0,"Press Key For Right");
upkey=selectkey(48,128,0,0,"Press Key For Up");
downkey=selectkey(48,128,0,0,"Press Key For Down");
firekey=selectkey(48,128,0,0,"Press Key to Fire");
}

void choosestick()
{
wipetexttab();
showchatscreen();
 message(88,48,0,0,"Joystick");
 message(64,96,0,0,"Joystick Number");
 message(136,128,0,0,"0-3");
showtext();
joyno=1+readopt(3);
if (joyno==0) return;
wipetexttab();
showchatscreen();
 message(96,48,0,0,"Reminder");
 message(48,96,0,0,"To jump, you can");
 message(32,128,0,0,"1. Use Joystick Up");
 message(32,160,0,0,"2. Use Fire Button 2");
 message(64,180,0,0,"If you have one.");
 message(32,212,0,0,"3. Use The Up Key");
showtext();
(void)readopt(9);
}


void tunegame()
{
wipetexttab();
showchatscreen();
 message(96,48,0,0,"Tune Game");
 if (sound_available) {
 message(64,96,0,0,"1. Sound System");
 message(64,128,0,0,"2. Sound Volume");
 }
 else {
 message(48,96,0,0,"\x11");
 message(48,128,0,0,"\x11");
 message(64,96,0,0,"1.");
 message(64,128,0,0,"2.");
 message(115,101,0,0,"Sound Not");
 message(112,123,0,0,"Available");
 }
 message(64,160,0,0,"3. Video System");
showtext();
while (1)
switch (readopt(3))
{
case -1: return;
case  1: if (sound_available) {tunesound(); return;} else break;
case  2: if (sound_available) {tunevolume(); return;} else break;
case  3: tunespeed(); return;
}
}

char sound1[] = "-1. No Sound";
char sound2[] = "-2. 4 Channels";
char sound3[] = "-3. 4 Channels";
char sound4[] = "-4. 8 Channels";
char sound5[] = "-5. Normal Quality";
char sound6[] = "-6. High Quality";
char sound7[] = "-7. Overdrive";

void tunesound()
{
showchatscreen();
swi_fastspr_setclipwindow(20,20,320-20-1,255-20);
 for (;;/*tunesoundloop:*/soundupdate(),swi_stasis_link(1,1),swi_sound_control(1,-15,0x20,0xfe))
{
tunesoundins:
wipetexttab();
soundfillin();
 message(96,32,0,0,"Tune Sound");
 message(32,60,0,0,sound1);
 message(32,80,0,0,sound2);
 message(32,100,0,0,sound3);
 message(80,120,0,0,"and music");
 message(32,140,0,0,sound4);
 message(32,160,0,0,sound5);
 message(32,180,0,0,sound6);
 message(32,200,0,0,sound7);
 message(96,220,0,0,"ESC - Exit");
swi_blitz_wait(0);
swi_fastspr_clearwindow();
showtext();

switch(readopt(7))
{
case 1: soundtype=0; break;
case 2: soundtype=1; break;
case 3: soundtype=2; break;
case 4: soundtype=3; break;
case 5: soundquality&=~1; break;
case 6: soundquality|=1; break;
case 7: soundquality^=2; break;
default: return;
}
}
}

void soundfillin()
{
sound1[0]=(soundtype==0)?16:17;
sound2[0]=(soundtype==1)?16:17;
sound3[0]=(soundtype==2)?16:17;
sound4[0]=(soundtype==3)?16:17;
sound5[0]=(soundquality&1)?17:16;
sound6[0]=(soundquality&1)?16:17;
sound7[0]=(soundquality&2)?16:17;
}

char tunevol1[]="-5. Speaker on";

void tunevolume()
{
showchatscreen();
wipetexttab();
if (soundtype==2) swi_bodgemusic_start(1,0);
swi_bodgemusic_volume(musicvol);
swi_fastspr_setclipwindow(20,20,320-20-1,255-20);
message(80,32,0,0,"Change volume");
message(48,96,0,0,"1. Louder effects");
message(48,116,0,0,"2. Quieter effects");
message(48,136,0,0,"3. Louder music");
message(48,156,0,0,"4. Quieter music");
message(48-14,176,0,0,tunevol1);
message(96,220,0,0,"ESC - Exit");
do
{
     tunevolumeloop:
if (swi_sound_speaker(0)) *tunevol1=17;
else *tunevol1=16;
swi_blitz_wait(0);
swi_fastspr_clearwindow();
showtext();
int r0=readopt(5);
if (r0==-1) return;

if (r0==1) {if (soundvol<0x40) soundvol=soundvol*2+1; maketestsound(soundvol); continue;}
if (r0==2) {if (soundvol>0) soundvol=(soundvol-1)/2; maketestsound(soundvol); continue;}
if (r0==3) {if (musicvol<0x40) musicvol=musicvol*2+1; maketestsound(musicvol); continue;}
if (r0==4) {if (musicvol>0) musicvol=(musicvol-1)/2; maketestsound(musicvol); continue;}

if (r0!=5) return;
swi_sound_speaker(3-swi_sound_speaker(0));
} while (1);
}

void maketestsound(int r1)
{
  swi_stasis_link(1,1);
  swi_sound_control(1,0x100|r1,0x20,0xfe);
  swi_bodgemusic_volume(musicvol);
}

char speed1[] = "-1. Auto frame rate";
char speed2[] = "-2. Full Explosions";
char speed3[] = "-3. Full Screen";

void tunespeed()
{
  do {
showchatscreen();
swi_fastspr_setclipwindow(20,20,319-20,255-20);
do
 {
 tunespeedloop:
wipetexttab();
if (gearchange==1) speed1[0]=16;
else speed1[0]=17;
if (explospeed==2) speed2[0]=17;
else speed2[0]=16;
if (fullscreen==1) speed3[0]=16;
else speed3[0]=17;
 message(96,48,0,0,"Tune Video");
message(32,96,0,0,speed1);
 message(80,116,0,0,"gearchanging.");
message(32,140,0,0,speed2);
message(32,164,0,0,speed3);
//message(40,172,0,0,"Tick Options if you");
//message(40,192,0,0,"Have a fast machine");
 message(96,220,0,0,"ESC - Exit");

swi_blitz_wait(0);
swi_fastspr_clearwindow();
showtext();
int r0=readopt(3);
if (r0==-1) return;
 else if (r0==1) gearchange^=1;
 else if (r0==2) explospeed^=3;
 else if (r0==3) {fullscreen^=1; vduread(); break;}
 } while (1);
  } while (1); 
}

int selectkey(int x,int y,int xv,int yv,char* a)
{
int r1;
wipetexttab();
showchatscreen();
clearkeybuf();
message(x,y,xv,yv,a);
showtext();
do {
  //do choosekeyloop:; // read key
  //while (osbyte_79(0)!=0xff); //no key pressed
  //if ((r4=osbyte_81(1))==ESC_VALUE) // read key
  //{ chooseescape: osbyte_7c();
  //return 0;} //early exit
}
while ((r1=osbyte_79(0))==-1); // scan keyboard
if (r1==ESC_VALUE) return 0;
return -r1; // and r4 (?)
}

int readopt(int maxopt)
{
  int r1;
do
{
keyloop:
if (joyno!=0)
{
swi_joystick_read(joyno-1, NULL, NULL);
// MOVVS R0,#0
//if (r0&(1<<16)) {/*optfire:*/ return 0;}
}
nooptstick:
r1=osbyte_81(1); // read key in time limit
// if (r1!=0xff) printf("%i\n",r1);
if (r1==ESC_VALUE)
{
optescape:
osbyte_7c(); // clear escape
return -1;
}
if (r1==0) continue;
if (osbyte_81(firekey)==0xff) {optfire: return 0;}
r1-=48; // '1' key returns value 49
} while (!((r1>=0)&&(r1<=maxopt)));
optexit:
return r1;
}

const int _x=250;
const int _v=-1;

int prelude()
{
cheatpermit=0;
frameinc=1;
showchatscreen();
 swi_fastspr_setclipwindow(20,20,320-20-1,255-20);
 swi_fastspr_clearwindow();
wipetexttab();
 message(2048,_x,0,_v,"Digital Psychosis");
 message(2108,_x+24,0,_v,"Presents");
 message(2152,_x+(24*5)/2,0,_v,"#");
 message(2040,_x+4*24,0,_v,"Young Sigmund has a");
 message(2056,_x+5*24,0,_v,"Few problems. Can");
 message(2056,_x+6*24,0,_v,"you help him find");
 message(2028,_x+7*24,0,_v,"the rogue brain cells");
 message(2036,_x+8*24,0,_v,"in his mind and shut");
 message(2100,_x+9*24,0,_v,"Them down?");
 message(2096,_x+10*24,0,_v,"PRESS SPACE");
 message(2088,_x+12*24,0,_v,"Cheat Mode!!!");
 message(2028,_x+14*24,0,_v,"F1, F2 Get Weapons");
 message(2028,_x+15*24,0,_v,"F3 - Restore Strength");
 message(2096,_x+17*24,0,_v,"HAVE FUN !!!");


showtext();
for (int scroll=256+8;swi_readescapestate()==0;)
{
loope0:
swi_blitz_wait(2);
if (scroll!=0)
{
scroll--;
switchbank();
switchfspbank();
swi_fastspr_clearwindow();
texthandler();
}
 preludetextstop:;
int r1=osbyte_7a();
if ((r1!=-1)&&(r1!=307)&&(r1!=308)) // escape
{
endprelude:
return 0;
}
if (mouse&2)
{
gocheat:
if (osbyte_81(-307)!=0xff) return 0;
if (osbyte_81(-308)!=0xff) return 0;
cheatpermit=1;
scroll=1024;
}
}
osbyte_7c();
return 1;
}

// buf:

void checkifarm3()
{
// The ARM3 is 25-33MHz with a 4Kb cache.  If you have
// less than that you should probably clear this flag.
arm3=1;
}

int checkifextend()
{
  //swi_XOS_ReadVarVal("PsychoExtend$Path",&buf,1<<31,0);
  return 0;
  //return (r2!=0); // z flag
}

//const char idpermitpath[]="<PsychoResource$Path>Idpermit";

char idpermitstring[]="You are now permitted to play the ID!!!\n";

char configname[] = "/.asylum"; //"<PsychoResource$Path>Config";

void permitid()
{
  //swi_osfile(10,idpermitpath,idpermitstring,idpermitstring+40);

  char fullname[240] = "";
  strcat(fullname,getenv("HOME"));
  strcat(fullname,configname);
FILE* r0=swi_osfind(0xc0, fullname);
if (r0!=NULL)
  { fprintf(r0,"%s",idpermitstring);
  fclose(r0); }
idpermit=1;
}
char config_keywords[13][12] =
  {"LeftKeysym", "RightKeysym", "UpKeysym", "DownKeysym", "FireKeysym",
   "SoundType", "SoundQ", "FullScreen",
   "SoundVolume", "MusicVolume", "MentalZone", "Initials", "You"};

void loadconfig()
{
  char fullname[240] = "";
  char keyword[12];
  strcat(fullname,getenv("HOME"));
  strcat(fullname,configname);
FILE* r0=swi_osfind(0x40, /* read access with path variable R2 */
	fullname);
if (r0!=NULL)
{
  //swi_osgbpb(3, /* read bytes from pointer R4 */
  //r0,&savestart,&saveend,0);
  while (fscanf(r0," %12s",&keyword)!=EOF)
    {
      int i;
    for (i=0;i<13;i++)
      if (!strncmp(keyword,config_keywords[i],12)) break;
    if (i==11) {fscanf(r0," %3c",initials); continue;}
    if (i==12) {idpermit=1; break;} // end of file
    if (i==13) break; // parsing failed
    int temp;
    fscanf(r0," %i",&temp);
    switch(i) {
    case 0: leftkey=-temp; break;
    case 1: rightkey=-temp; break;
    case 2: upkey=-temp; break;
    case 3: downkey=-temp; break;
    case 4: firekey=-temp; break;
    case 5: soundtype=temp; break;
      //case 6: soundquality=temp; break;
    case 7: fullscreen=temp; break;
    case 8: soundvol=temp; break;
    case 9: soundquality=temp; break;
    case 10: mentalzone=temp; break;
    }
    }
 fclose(r0);
}
 findoutid:;
 if (!idpermit) if (mentalzone>2) mentalzone=2; 
  //int idp=swi_osfile(5,idpermitpath,NULL,NULL);
  //idpermit=(idp==1)?1:0;
}

void saveconfig()
{
  char fullname[240] = "";
  strcat(fullname,getenv("HOME"));
  strcat(fullname,configname);
  //printf("%s\n", fullname);
FILE* r0=swi_osfind(0x80 /* create file with read/write access with path variable R2 */,
	fullname);
if (r0==NULL) return;
//swi_osgbpb(1, /* write bytes to pointer R4 */
//r0,&savestart,&saveend,0);
 fprintf(r0, "%s %i\n%s %i\n%s %i\n%s %i\n%s %i\n%s %i\n%s %i\n%s %i\n%s %i\n%s %i\n%s %c%c%c\n%s",
	 config_keywords[0], -leftkey,
	 config_keywords[1], -rightkey,
	 config_keywords[2], -upkey,
	 config_keywords[3], -downkey,
	 config_keywords[4], -firekey,
	 config_keywords[5], soundtype,
	 //config_keywords[6], soundquality,
	 config_keywords[7], fullscreen,
	 config_keywords[8], soundvol,
	 config_keywords[9], musicvol,
	 config_keywords[10], mentalzone,
	 config_keywords[11], initials[0], initials[1], initials[2],
	 ((idpermit==1)?idpermitstring:""));
 fclose(r0); 
}


char gamescreenpath[] = "GameScreen";
char chatscreenpath[] = "ChatScreen";
char blokepath[] = "FSPBloke";
char boardpath[] = "Brain";
char blockpath[] = "FSPBlocks";
char backpath[] = "Backfile";
char neuronbackpath[] = "Neurons/Backfile";
char neuronpath[] = "Neurons/Cell1";
char* neuronnumber = neuronpath+12;
char explopath[] = "FSPExplo";
char charpath[] = "FSPChars";
char alienpath[] = "FSPAliens";
char resourcepath[] = "./Resources/";
char extendpath[] = "./Extend/";
char idpath[] = "./Id/";
char psychepath[] = "./Psyche/";
char egopath[] = "./Ego/";
char egomusic1path[] = "./Ego/Music1";
char egomusic2path[] = "./Ego/Music2";
char psychemusic1path[] = "./Psyche/Music1";
char psychemusic2path[] = "./Psyche/Music2";
char idmusic1path[] = "./Id/Music1";
char idmusic2path[] = "./Id/Music2";
char mainmusicpath[] = "./Resources/Music1";
char deathmusicpath[] = "./Resources/Music2";

int getfiles()
{
getvitalfiles();
showloading();
getmusicfiles();
getgamefiles();
return 0;
}

void getvitalfiles()
{
charsok=0;
chatscreenadr=storage;
char* charsadr_load=chatscreenadr;
loadvitalfile(chatscreenpath,resourcepath,&charsadr_load);
gamescreenadr=charsadr_load;
loadvitalfile(charpath,resourcepath,&gamescreenadr);
initialize_sprites(charsadr_load,charsadr,48,gamescreenadr);
charsok=1;
}

void getmusicfiles()
{
  swi_bodgemusic_load(1,mainmusicpath);
  swi_bodgemusic_load(2,deathmusicpath);
  swi_bodgemusic_start(1,0);
}

void getgamefiles()
{
char* r10=gamescreenadr;
 char* o10=r10;
int reload=0;
load1:
do
{
reload=loadhammered(gamescreenpath,resourcepath,&r10);
if (reload==-1) {badload(); return;}
} while (reload==1);
o10=r10; //blokeadr=r10;
load2:
do
{
reload=loadhammered(blokepath,resourcepath,&r10);
if (reload==-1) {badload(); return;}
} while (reload==1);
initialize_sprites(o10,blokeadr,77,r10);
o10=r10; //exploadr=r10;
load3:
do
{
reload=loadhammered(explopath,resourcepath,&r10);
if (reload==-1) {badload(); return;}
} while (reload==1);
initialize_sprites(o10,exploadr,32,r10);
tmp_blockadr=r10;
}

int getlevelfiles()
{
int reload=0;
showgamescreen();
currentzone=0;
switch(mentalzone)
{
 case 2: currentpath=psychepath; break;
 case 3: currentpath=idpath; break;
 case 4: currentpath=extendpath; break;
default: currentpath=egopath;
}
char* r10=tmp_blockadr;
 char* o10=r10;
load4:
do
{
reload=loadhammered(blockpath,currentpath,&r10);
if (reload==-1) {reload=badlevelload();}
} while (reload==1);
initialize_sprites(o10,blockadr,256,r10);

//alspradr=r10;
o10=r10;
load5:
do
{
reload=loadhammered(alienpath,currentpath,&r10);
if (reload==-1) {reload=badlevelload();}
} while (reload==1);
initialize_sprites(o10,alspradr,256,r10);

brainadr=(board*)r10;
boardadr=(board*)r10;
load6:
do
{
reload=loadhammered(boardpath,currentpath,&r10);
if (reload==-1) {reload=badlevelload();}
} while (reload==1);

backadr=r10;
load7:
do
{
  reload=loadhammered(backpath,currentpath,&r10);
  if (reload==-1) {reload=badlevelload();}
  // backfiles don't work???
} while (reload==1);
char* r1;
switch (mentalzone)
{
case 2: r1 = psychemusic1path; break;
case 3: r1 = idmusic1path; break;
default: r1 = egomusic1path;
}

swi_bodgemusic_load(0,r1);

switch (mentalzone)
{
case 2: r1 = psychemusic2path; break;
case 3: r1 = idmusic2path; break;
default: r1 = egomusic2path;
}
swi_bodgemusic_load(1,r1);
return 0;
}

int retrievebackdrop()
{
char* r9=currentpath;
char* r10=backadr;
int reload;
load10:
do
{
reload=loadhammered(backpath,currentpath,&r10);
if (reload==-1) {reload=badlevelload();}
} while (reload==1);
return 0;
}

int getneuronfiles()
{
char* r10=backadr;
int reload=0;
//STR R10,[R12,#backadr]
load8:
do
{
reload=loadhammered(neuronbackpath,currentpath,&r10);
if (reload==-1) {reload=badlevelload();}
} while (reload==1);
r10+=backsize*3;
neuronadr=boardadr=(board*)r10;
while (1) {
neuronloadloop:
*neuronnumber='0'+plzone;
if (filelength(neuronpath,currentpath)) break;
currentzone=--plzone;
if (currentzone==0) {noneuronshere: return 1;}
}
load9:
do
{
reload=loadhammered(neuronpath,currentpath,&r10);
if (reload==-1) {reload=badlevelload();}
} while (reload==1);
return 0;
}


void loadvitalfile(char* r1,char* path,char** space)
{
// if VS or if r0==1
  char fullname[240]="";
  strcat(fullname,path);
  strcat(fullname,r1);
int r4=swi_osfile(15,fullname,0,0);
if (r4==-1) fatalfile();
if (*space+r4>=storageend) fatalfile();
if (swi_osfile(14,fullname,*space,0)) fatalfile();
*space+=r4;
}

int loadhammered(char* r1,char* path,char** spaceptr)
{
int r3=swi_blitz_hammerop(0,r1,path,NULL);
if (r3==-1) {filenotthere(); return -1;}
if (r3==0) {return loadfile(r1,path,spaceptr);}

if (*spaceptr+r3>=storageend) {nomemory(); return 1;}
 int k;
if ((k=swi_blitz_hammerop(1,r1,path,*spaceptr))<0) {printf("Error %i\n",-k); filesyserror(); return 1;}
*spaceptr+=k;
return 0;
}

int loadfile(char* r1,char* path,char** spaceptr)
{
int r4=filelength(r1,path);
  char fullname[240]="";
  strcat(fullname,path);
  strcat(fullname,r1);
if (r4==-1) {filenotthere(); return 1;}
if (*spaceptr+r4>=storageend) {nomemory(); return 1;}
if (swi_osfile(14,fullname,*spaceptr,0)) {filesyserror(); return 1;}
*spaceptr+=r4;
return 0;
}

char egosave[] = "/EgoHighScores";
char psychesave[] = "/PsycheHighScores";
char idsave[] = "/IdHighScores";
char extendedsave[] = "/ExtendedHighScores";
char testsave[] = "/TestPermissions";

void savescores()
{
highscorearea[13*5] = swi_oscrc(0,highscorearea,highscorearea+13*5,1);
 char r1[240]=SCOREPATH;
  if (!installed) strcpy(r1,"./hiscores");
switch (mentalzone)
{
case 2: strcat(r1,psychesave); break;
case 3: strcat(r1,idsave); break;
case 4: strcat(r1,extendedsave); break;
default: strcat(r1,egosave);
}
swi_osfile(10,r1,highscorearea,highscorearea+13*5+1);
if (installed) chmod(r1,0060);
}

void loadscores()
{
  char r1[240]=SCOREPATH;
  if (!installed) strcpy(r1,"./hiscores");
switch (mentalzone)
{
case 2: strcat(r1,psychesave); break;
case 3: strcat(r1,idsave); break;
case 4: strcat(r1,extendedsave); break;
default: strcat(r1,egosave);
}
if (swi_osfile(0xff,r1,highscorearea,highscorearea+13*5+1))
  setdefaultscores();
else if (swi_oscrc(0,highscorearea,highscorearea+13*5,1)
            != highscorearea[13*5]) setdefaultscores();
}

const char defscore[] = "00000000 PSY\n";

void setdefaultscores()
{
char* r10=highscorearea;
for (int r3=5;r3>0;r3--)
{
 loopd4:;
const char* r11=defscore;
for (int r2=13;r2>0;r2--)
{
loopd5: *(r10++)=*(r11++);
}
}
}

void fatalfile()
{
exit(printf("A file vital to the game cannot be loaded.  Please reset your machine and try   again"));
}

void showloading()
{
wipetexttab();
showchatscreen();
 message(88,64,0,0,"Please Wait");
 message(48,128,0,0,"Loading Game files");
showtext();
}


void filenotthere()
{
showerror();
 message(56,64,0,0,"A file is missing.");
 message(48,96,0,0,"It cannot be loaded");
showtext();
errorwait();
}


void filesyserror()
{
showerrorok();
 message(40,64,0,0,"Unable to Load File");
 message(56,96,0,0,"Please Check Disc");
showtext();
errorwait();
}

void badload()
{
showerrorok();
 message(32,32,0,0,"A game file could not");
 message(48,64,0,0,"be loaded. Program");
 message(80,96,0,0,"Must exit now.");
showtext();
errorwait();
}

int badlevelload()
{
showerrorok();
 message(40,32,0, 0,"The level cannot be");
 message(32,64,0 ,0,"Loaded. Please check");
 message(48,96,0,0,"the disc, or press");
 message(48,128,0,0,"escape to end game.");
showtext();
return errorwait(); // 'clear carry if carry clear (???)'
}

void nomemory()
{
showerrorok();
 message(48,32,0,0,"There is not enough");
 message(48,64,0,0,"memory available to");
 message(48,96,0,0,"load the game files.");
showtext();
errorwait();
// XXX smash one more entry from stack (than just nomemory(); return;)
}

int filelength(char* name,char* path)
{
  char fullname[240]="";
  strcat(fullname,path);
  strcat(fullname,name);
int r4=swi_osfile(15,fullname,NULL,NULL);
if (r4==-1)  {filesyserror(); return 0;}
//if (r0!=1) {fileerror: return -1;}
return r4;
}

void showerror()
{
frameinc=1;
showchatscreen();
swi_fastspr_setclipwindow(20,20,320-20-1,255-20);
swi_fastspr_clearwindow();
wipetexttab();
 message(72,200,0,0,"RET - Try Again");
 message(72,224,0,0,"ESC - Abandon");
}

void showerrorok()
{
frameinc=1;
showchatscreen();
swi_fastspr_setclipwindow(20,20,320-20-1,255-20);
swi_fastspr_clearwindow();
wipetexttab();
 message(72,200,0,0,"RET - OK");
 message(72,224,0,0,"ESC - Abandon");
}

int errorwait()
{
if (osbyte_81(-74)!=0xff)
	loopb9:
 while (osbyte_81(-61)!=0xff)
	if (swi_readescapestate()) {wipetexttab(); return 0;}
waitover:
wipetexttab();
return 1;
}

char buffer[256];

//.hta
void errorhandler(int r0)
{
losehandlers();
exit(printf("Error from Asylum:\n%s",buffer+4));
}

void exithandler(int r0)
{
losehandlers();
exit(255);
}

void upcallhandler(int sig)
{
if (sig==SIGTERM) {losehandlers(); exit(0);}
if (sig==SIGINT) {losehandlers(); exit(0);}
//noexitupcall:
//if (r0==1) {insertdiscagain(r2); return;}
//if (r0==2) {insertdisc(r2); return;}
}

void gethandlers() {signal(SIGTERM,upcallhandler); signal(SIGINT,upcallhandler); return;}
void losehandlers() {SDL_Quit(); return;}

void loadzone()
{
int r1=currentzone;
if ((currentzone=plzone)==0) exitneuron(r1);
else enterneuron(r1);
}

void enterneuron(int r1)
{
if (r1==0) saveal();
if (getneuronfiles())  {exitneuron(r1); return;}
wipealtab();
getarms();
initweapon();
initprojtab();
initbultab();
backprep();
boardreg();
prepfueltab();
findplayer();
}

void exitneuron(int r1)
{
boardadr=brainadr;
restoreal();
initweapon();
initprojtab();
initbultab();
retrievebackdrop();
backprep();
boardreg();
prepfueltab();
wipesoundtab();
}

int showhighscore()
{
loadscores();
updatehst();
showhst();
wipetexttab();
message(96,224,0,0,"press fire");
releaseclip();
showtext();
readopt(0);
return swi_readescapestate();
}

void updatehst()
{
hstindex=5;
char* r10=highscorearea;
r10+=4*13; //4*entry length
while (comparescore(r10))
{
loopd1:
r10-=13; //entry length
if (--hstindex==0) break;
}
lessthan:
r10+=13; //entry length
if (hstindex!=5)
{
if (4-hstindex>0)
{
char* r9;
int r3;
for (r9=highscorearea+4*13-1,r3=13*(4-hstindex);r3>0;r9--,r3--)
{
loopd7:
r9[13]=*r9;
}
}
noshiftscore:;

char* r11=plscore;
for (int r3=8;r3>0;r3--)
{
loopda:
*(r10++)=*(r11++)+'0';
}

r10++;
//int r9=1024;
swi_blitz_wait(20); //
for (int i=0;i<3;i++) r10[i] = initials[i];
for (int r8=3;r8>0;r8--,r10++)
{
while (osbyte_81(0)!=-firekey)
{
scorekeyloop:
//if (--r9<0) goto scoreexit;
keyread();
if (leftpress==0) {(*r10)++; savedornot=0;}
if (rightpress==0) {(*r10)--; savedornot=0;}
if (*r10<'0') *r10='0';
if (*r10>'Z'+4) *r10='Z'+4;
showhst();
 swi_blitz_wait(4);
}
initials[3-r8] = *r10;
swi_stasis_link(1,18);
swi_stasis_volslide(1,0,0);
swi_sound_control(1,0x17c,140,0);
 /*if (r8>1)*/ swi_blitz_wait(20);
}
scoreexit:
savescores();
}
notontable:;
}


int comparescore(char* r10)
{
char* r11=plscore;
for (int r3=8;r3>0;r3--)
{
loopd0:
if (*r10-'0'<*r11) scoregreater: return 1;
else if (*r10-'0'>*r11) scoreless: return 0;
r10++; r11++;
compnext:;
}
return 1;
}

void showhst()
{
swi_blitz_wait(0);
switchbank();
switchfspbank();
showchatscores();
wipetexttab();
 message(64,32,0,0,"Zone High Scores");
texthandler();
char* r10=highscorearea;
int x=32, y=64;
for (int r3=5;r3>0;r3--)
{
for (;*r10>0xa;r10++)
{
showhstloop:
fspplot(charsadr,*r10-'0',x,y);
x+=16;
}
showhstnewline:
x=32; y+=32; r10++;
}
if (hstindex==5) return;
fspplot(charsadr,13,280,(hstindex+2)<<5);
}

void setup()
{
framectr=0;
plzone=_firstzone;
wipesoundtab();
initweapon();
initprojtab();
initbultab();
setfullclip();
initrockettab();
addtabinit();
getvars();
prepstrength();
scorezero();
backprep();
boardreg();
wipealtab();
prepfueltab();
}

void wipesoundtab()
{
//r10=&soundtabofs; temporarily undefined
for (int r3=_soundentlen*8; r3>0; r3-=sizeof(int))
{
 loop51:;
  //*(r10++)=0;
}
for (int r0=7;r0>=0;r0--)
{
soundkillloop:
swi_stasis_volslide(r0,0xfc00,0);
}
}

void wipetexttab()
{
char* r10=(char*)texttabofs;
for (int r3=_textno*_textlen;r3-=1;r3>0)
{
loopa5:
*(r10++)=0;
}
}

void initprojtab()
{
projadr=projofs;
projent* r10=projadr;
projctr=0;
for (int r9=_projno;r9>0;r9--)
{
loop44:
*(int*)r10=0;
r10++;
}
}

void initbultab()
{
buladr=bulofs;
bulent* r10=buladr;
bulctr=0;
for (int r9=_bulno;r9>0;r9--)
{
loop74:
*(int*)r10=0;
r10++;
}
}

void initweapon()
{
firelastframe=0;
if (plweapontype>=_rockbase)
	{plweaponspeed=2; firerate=12;}
else	{plweaponspeed=8; firerate=4;}
rocketblamctr=5;
}

void prepfueltab()
{
fueltabread=fueltabofs[0];
fueltabwrite=fueltabofs[1];
*fueltabread=0;
*fueltabwrite=0;
fueltabctr=0;
fuelairlastframe=0;
}

void scorezero()
{
plscoreadd=0;
*(uint64_t*)plscore=0;
lives=2;
neuronctr=0;
}

void wipealtab()
{
aladr=alofs;
alent* r10=aladr;
alctr=0;
for (int r9=_alno;r9>=0;r9--)
{
l9:
r10->type=0;
r10++;
}
boardreg(); // fallthrough?
}

void boardreg()
{
boardwidth=boardadr->width;
xposmax=boardwidth<<12;
yposmax=boardadr->height<<12;
boardlowlim=boardadr->contents;
boardhighlim=boardlowlim+boardadr->width*boardadr->height;
}

void backprep()
{
  // This used to be multiply callable.  Now it's not.
Uint32* ba = (Uint32*)backadr;
 for (int i=32*48-1;i>=0;i--) ba[i] = alt_palette[backadr[i]];
backexec(ba+32*48,ba);
backexec(ba+64*48,ba+32*48);
backexec(ba+96*48,ba+64*48);
}

void backexec(Uint32* dst,Uint32* src)
{
for (int r4=32;r4>0;r4--)
{
 l6:;
Uint32 r2=*(src++);
l5: memcpy(dst,src,47*sizeof(Uint32));
dst+=47; src+=47;
*(dst++)=r2;
}
}

void prepstrength()
{
laststrength=plstrength=_strengthinit;
snuffctr=lagerctr=0;
}

void screensave()
{
showscore();
//oscli("Mount");
//oscli("Screensave Screenfile");
}

void getvars()
{
getstrengthtab();
xpos=64<<8; ypos=48<<8; initplx=0; initply=0;
hvec=0; vvec=0; pladr1=0; pladr2=0;
plface=0;
}

typedef struct {
  int time;
  int tempo;
  char* tune;
  int pitch[4];
  int inst[4];
  int start_time[4];
  char* section;
  int pointer;
} music_state;
music_state music;
 char voice[32][30000];
 char tuneload[4][30000];
 Sint16 mulaw[256];



Mix_Chunk* make_sound(char samp, int initpitch, int volslide, int pitchslide, char frames)
{
  int numsamples = frames*22050.0/50.0; // frames are 50Hz?
  Mix_Chunk* mc = (Mix_Chunk*)malloc(sizeof(Mix_Chunk));
  Uint16* s = (Uint16*)malloc(numsamples*2*sizeof(Uint16));
  mc->abuf = (Uint8*)s;
  mc->alen = numsamples*2*sizeof(Uint16);
  mc->allocated = 0; // don't automatically free buffer
  mc->volume = 0xff; // XXX volume
  int time = 0;
  double ps = ((Sint16)(pitchslide&0xffff))*22/22050.0;
  double vs = ((Sint16)(volslide&0xffff))/22050.0;
  Sint16 psmax = pitchslide>>16;
  double matching = 0;
  fprintf(stderr,"."); fflush(stderr);
  for (int i=0; i<numsamples*2*sizeof(Uint16); i+=4, time++, s+=2)
  {
    Uint16 mono = 0;

    double pitch = initpitch+ps*time; // XXX pitchslide
    //if ((ps>0)&&(pitch>psmax)) pitch=psmax;
    //if ((ps<0)&&(pitch<psmax)) pitch=psmax;
    int off = time;
    double old_sample_rate=1000.0*pow(2,pitch/4096.0);
   matching += old_sample_rate/22050;
   for (int j = (int)matching-16; j <= (int)matching+16; j++)
   {
     if ((j<0)) continue;
     int jj=j;
     Uint32 len=((Uint32*)voice[samp])[6];
     Uint32 gap=((Uint32*)voice[samp])[7]; // ???
     Uint32 rep=((Uint32*)voice[samp])[8];
     if (jj>=len) {if (rep==0) continue; else jj = (len-rep) + ((jj-len)%rep);}
     double w;
     if (jj>=len-gap)
       w = ((jj-(len-gap))*(double)mulaw[voice[samp][jj+44-rep]]
	    + (len-jj)*(double)mulaw[voice[samp][jj+44]])/gap/4;
     else w = mulaw[voice[samp][jj+44]]/4;
     double vscale = 0x7f + vs*time;
     if (vscale>0xff) vscale=0xff;
     if (vscale>0) w *= (vscale/0x7f); else w = 0;
     double x = PI*(j - matching);
     if (x==0) mono += w;
     else mono += w * sinf(x) / (1.3*x);
   }
   s[1]=*s=mono; // XXX stereo
  }
  return(mc);
}

void soundclaim(int c, char samp, char initvol, int initpitch, int volslide, int pitchslide, char frames, int stereo, Mix_Chunk* static_chunk)
{
  //int* argv = (int*)vargv;
  //int c = argv[0]; char samp = argv[1]; char initvol = argv[2]; int initpitch = argv[3];
  //int volslide = argv[4]; int pitchslide = argv[5]; char frames = argv[6]; int stereo = argv[7];
  Mix_Chunk* old_chunk = Mix_GetChunk(c);
  Mix_Chunk* chunk = static_chunk?static_chunk:make_sound(samp,initpitch,volslide,pitchslide,frames);
  Mix_HaltChannel(c);
  Mix_Volume(c, ((initvol>soundvol)?soundvol:initvol));
  Mix_SetPanning(c, 254-stereo, stereo);
  Mix_PlayChannel(c, chunk, 0);
  //if (old_chunk) Mix_FreeChunk(old_chunk); XXX memory leak
}

void sdl_music_hook(void* udata, Uint8* stream, int len)
{
  music_state* m = (music_state*)udata;
  char* tuneload = m->tune;
  Uint16* s = (Uint16*)stream;
  for (int i=0; i<len; i+=4, m->time++, s+=2)
  {
    if (0==(m->time%m->tempo))
    {
      // new beat
      int inst = 0;
      int pitch = 8;
      char first;
      char second;
      do {
	first = tuneload[m->pointer++];
	second = tuneload[m->pointer++];
	if (first)
        { // pitch word
	  inst = first&0x1f;
	  pitch = second&0x3f;
        }
	else
        {
	  m->pitch[second&3] = pitch;
	  m->inst[second&3] = inst;
	  m->start_time[second&3] = m->time;
	  inst = 0; pitch = 8;
        }
      } while ((second&0xc0)==0); 
      if (second&0x80)
      {
	m->section++;
	if (0x80&*m->section) m->section=tuneload+8;
	m->pointer=((Uint32*)tuneload)[0x42+*m->section];
      }
    }
    *s=0; s[1]=0;
    for (int v=0; v<4; v++)
    {
      if (m->inst[v] == 0) continue;
    int pitch = m->pitch[v];
    int off = m->time - m->start_time[v];
    double old_sample_rate=4000.0*pow(2,pitch/12.0);
   double matching = off*old_sample_rate/22050;
   for (int j = (int)matching-4; j <= (int)matching+4; j++)
   {
     if ((j<0)) continue;
     int jj=j;
     Uint32 len=((Uint32*)voice[m->inst[v]])[6];
     Uint32 gap=((Uint32*)voice[m->inst[v]])[7]; // ???
     Uint32 rep=((Uint32*)voice[m->inst[v]])[8];
     /*if (m->inst[v]==16)
       while (jj>=6960+1000) jj-=6960;
     else
       if (jj>=8192) continue;*/
     if (jj>=len) jj = rep?((len-rep) + ((jj-len)%rep)):0;
     double w;
     if (jj>=len-gap)
       w = ((jj-(len-gap))*(double)mulaw[voice[m->inst[v]][jj+44-rep]]
	    + (len-jj)*(double)mulaw[voice[m->inst[v]][jj+44]])/gap/4;
     else w = mulaw[voice[m->inst[v]][jj+44]]/4;
     double x = PI*(j - matching);
     if (x==0) s[v&1] += w;
     else s[v&1] += w * (sinf(x) * musicvol) / (1.3 * x * 0x7f);
   }
    }
 }    
}

void load_voice(int v, char* filename)
{
 SDL_RWops* file = SDL_RWFromFile(filename, "r");
 if (file==NULL) {sound_available=0; return;}
 SDL_RWseek(file, 0, SEEK_END);
 int file_len = SDL_RWtell(file)/*-44*/;
 SDL_RWseek(file, 0/*44*/, SEEK_SET);
 SDL_RWread(file,voice[v],1,file_len);
}

void load_voices()
{
 load_voice(1,"./Voices/Jump");
 load_voice(2,"./Voices/Bonus");
 load_voice(3,"./Voices/Explo");
 load_voice(4,"./Voices/AtomExplo");
 load_voice(5,"./Voices/Cannon");
 load_voice(6,"./Voices/Rocket");
 load_voice(7,"./Voices/Hiss");
 load_voice(8,"./Voices/Stunned");
 load_voice(9,"./Voices/SmallZap");
 load_voice(10,"./Voices/BigZap");
 load_voice(16,"./Voices/Organ");
 load_voice(17,"./Voices/Plink");
 load_voice(18,"./Voices/PlinkHard");
 load_voice(19,"./Voices/Raver");
 load_voice(20,"./Voices/Dome");
 load_voice(21,"./Voices/NasalStr");
 load_voice(22,"./Voices/BassHollow");
 load_voice(23,"./Voices/NasalBass");
 load_voice(24,"./Voices/BassDrum");
 load_voice(25,"./Voices/Snare");
 load_voice(26,"./Voices/Cymbal");
}

void vduread()
{
osbyte_f1(1); //get non-shadow screen bank


ArcScreen = SDL_SetVideoMode( 320, 256, 32 /*bpp*/, SDL_HWSURFACE | (fullscreen?SDL_FULLSCREEN:0));
DecompScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 256, 32,
						  0xff,0xff00,0xff0000,0);
 modesize = 0; // hack: don't double buffer 320*256*4;
 hbytes = ArcScreen->pitch;
 SDL_LockSurface(ArcScreen);
 screenstart = (char*)(ArcScreen->pixels);
 SDL_UnlockSurface(ArcScreen);
  /* initialise screenstart(149), modesize(7), hbytes(6) */
screenuse=screenstart;
 SDL_ShowCursor(SDL_DISABLE);
}

Uint32 strengthcol[72];

void getstrengthtab() {strengthcoltab = strengthcol;}

void init_strengthcol() {
  for (int i=0;i<36;i++) {
    strengthcol[i] = 0xff000000+0x110000*(11+(i%5)); // varying shade of red
    strengthcol[i+36] = 0xff000000+0x111111*(3&i); // varying shade of dark grey
  }}

int addtab[] = {10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};

void addtabinit() {addtabadr=addtab;}

int rockettab[_rockettablen];

void initrockettab() {rocketctr=0; rocketflag=0; rockettabadr=rockettab;}

void init_rockettab() {
  for (int i=0;i<184;i++) {
    rockettab[4+i] = (int)(256*sin(i*2*PI/(192-8)));
  }
  for (int i=0;i<4;i++) {
    rockettab[i] = rockettab[188+i] = 0;
  }}

void init_sounds() {
 Sint16 exp[8] = {0,132,396,924,1980,4092,8316,16764};
 for (int i=0;i<256;i++)
   mulaw[i] = ((i&1)?-1:1) * (exp[i>>5] + ((i&0x1e)<<(2+(i>>5))));
 fprintf(stderr,"Building sound effects ");
  CHUNK_BULLET_1 = make_sound(_Sampsmallzap,fullpitch+0x1000,0,(fullpitch<<16)|0xfe00,2);
  CHUNK_BULLET_2 = make_sound(_Sampbigzap,fullpitch+0x1800,0,(fullpitch<<16)|0xfe00,2);
  CHUNK_BULLET_3 = make_sound(_Sampbigzap,fullpitch+0x1000,0,(fullpitch<<16)|0xfe00,2);
  CHUNK_ELEC_1 = make_sound(_Samprave,0x2000,0xff00,0,10);
  CHUNK_ELEC_2 = make_sound(_Samprave,0x2400,0xff00,0,10);
  CHUNK_ELEC_3 = make_sound(_Samprave,0x2800,0xff00,0,10);
  CHUNK_EXPLO = make_sound(_SampExplo,0x3800,0,0,10);
  CHUNK_ATOM = make_sound(_SampAtomExplo,0x2800,0,0,50);
  CHUNK_SPINFIRE = make_sound(_Sampsmallzap,fullpitch+0x1000,0,0,2);
  CHUNK_SPINPOWERFIRE = make_sound(_Sampbigzap,fullpitch,0,0,2);
  CHUNK_SHOOT = make_sound(_Sampsmallzap,fullpitch+0x1000,0,(fullpitch<<16)|0xfe00,2);
  CHUNK_SHOOTNUTTER = make_sound(_Sampbigzap,fullpitch,0,(fullpitch<<16)|0xfe00,2);
  CHUNK_JUMP = make_sound(_SampJump,0x4600,0,0,4);
  CHUNK_SHUTDOWN_1 = make_sound(_Samprave,0x3300,(soundvol<<25)|0x1000,0x100ff00,100);
  CHUNK_SHUTDOWN_2 = make_sound(_Samprave,0x3200,(soundvol<<25)|0x1000,0x100ff00,100);
  CHUNK_SHUTDOWN_3 = make_sound(_Samprave,0x3100,(soundvol<<25)|0x1000,0x100ff00,100);
  CHUNK_TELEP_1 = make_sound(_SampJump,0x1000,0,0,50);
  CHUNK_TELEP_2 = make_sound(_Samporgan,0x1000,0xff00,0x50000a00,25);
  CHUNK_TELEP_3 = make_sound(_Samporgan,0x4000,0xff00,0x1000f400,25);
  CHUNK_BLAM = make_sound(_SampAtomExplo,fullpitch,0,0,8);
  CHUNK_FIRE = make_sound(_SampCannon,fullpitch,0,0,2);
  CHUNK_BLAMFIRE = make_sound(_Samprave,fullpitch,0,(fullpitch<<17)+0x200,40);
  CHUNK_ROCKET = make_sound(_SampRocket,fullpitch,0,0,2);
  CHUNK_WEAPON_1 = make_sound(_Samporgan,0x3000,0xff00,0x60000200,10);
  CHUNK_WEAPON_2 = make_sound(_Samporgan,0x3800,0xff00,0x60000200,10);
  CHUNK_WEAPON_3 = make_sound(_Samprave,0x3000,0xff00,0x60000800,10);
  CHUNK_WEAPON_4 = make_sound(_Samprave,0x3800,0xff00,0x60000800,10);
  CHUNK_FUELAIR = make_sound(_SampHiss,0x6000,0xff80,0x2000ff00,50);
  for (int i=0;i<12;i++) CHUNK_OBJGOT[i] = make_sound(_SampBonus,((i+1)<<6)+0x3000,0,0,3);
  CHUNK_BONUS_1 = make_sound(_SampBonus,0x2200,0,0,5);
  CHUNK_BONUS_2 = make_sound(_SampBonus,0x2000,0,0,5);
  for (int i=0;i<17;i++) CHUNK_STUNNED[i] = make_sound(_SampStunned,0x4000-i*0x100,0,0,50);

 fprintf(stderr," done.\n");
}

void c_array_initializers() {
  init_projsplittab(); init_rocketbursttab(); init_alspintab(); init_strengthcol(); init_rockettab();
  init_palette(); init_splittab();
  if (sound_available) {
    load_voices();
    if (sound_available) init_sounds();
    else fprintf(stderr,"Sound disabled: loading sound effects failed\n");
  }
  for (int i=0;i<512;i++) keyboard[i]=0;
}

int main()
{
 char r1[240] = SCOREPATH;
 strcat(r1,testsave);
 FILE* permission_test = fopen(r1,"w");
 installed = (NULL != permission_test);
 if (installed) { fclose(permission_test); unlink(r1);}
 if (chdir(RESOURCEPATH)&&chdir("./data"))
 {
   fprintf(stderr,"Couldn't find resources directory (tried %s, ./data)",RESOURCEPATH);
   exit(1);
 }
 SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
 sound_available=!Mix_OpenAudio(22050,AUDIO_U16LSB,2,1024);
 if (!sound_available) fprintf(stderr,"Sound disabled: opening audio device failed: %s\n",Mix_GetError());
 c_array_initializers();
/*MODE 15
MODE 13
VOICES 8
STEREO 5,-32
STEREO 6,32
STEREO 7,-32
STEREO 8,32*/
swi_stasis_control(8,8);
do
{init();}
 while (0);// (snuffctr>=300);
 SDL_Quit();
exit(0);
}


void message(int x,int y,float xv,float yv,char* a)
{
int time=400;
char b[60];
char* q=b;
if (x>1000) {x-=1000; time=50;}
if (x>1000) {x-=1000; time=0x10000;}
//int32_t z[5]={time,x<<8,y<<8,(int)(xv*256),(int)(yv*256)}; // might be non-integers
for (char* p=a;*p!=0;p++,q++)
{
switch (*p)
{
 case '-': *q=';'; break;
 case '¤': *q=':'; break;
 case '\'':*q='~'; break;
 case '.': *q='{'; break;
 case ',': *q='|'; break;
 case '?': *q='<'; break;
 case '!': *q='}'; break;
 case '%': *q='>'; break;
 case '€':*q='?'; break;
 case '×': *q='@'; break;
 case '#': *q=127; break;
default: *q=*p;
}
if (*q>=96) *q-=32;
if (*q==' ') *q=0xff;
else if ((*q&~1)!=16) {*q-=47; if (*q>55) exit(printf("Bad message character %c (%i)",*p,*p));}
}
*q=0;
{
textinfo* r11=texttabofs;
 int r9;
for (r9=_textno;r9>0;r9--)
{
loopa8:
if (r11->count==0) break;
r11++;
}
if (r9==0) return;
messageproc:
 r11->count=time; r11->x=x<<8; r11->y=y<<8;
 r11->dx=(int)(xv*256); r11->dy=(int)(yv*256);
if (strlen(b)>58) exit(printf("Bad string %s",b));
strncpy(r11->text,b,60);
}
}


void swi_bodgemusic_start(int a,int b) {
 if (!sound_available) return;
 swi_bodgemusic_stop();
 swi_sound_qtempo(0x1000);
 music.time=0;
 music.tune=tuneload[a];
 music.section=tuneload[a]+8;
 music.pointer=((Uint32*)(tuneload[a]))[0x42+*music.section];
 for (int v=0;v<4;v++) {music.pitch[v]=0; music.inst[v]=0; music.start_time[v]=0;}
 Mix_HookMusic(sdl_music_hook, &music);
}
void swi_bodgemusic_stop() { if (sound_available) {Mix_HaltMusic(); Mix_HookMusic(NULL,NULL);} }
void swi_bodgemusic_volume(int v) {;}
void swi_bodgemusic_load(int a,char* b) {
 SDL_RWops* tune = SDL_RWFromFile(b, "r");
 //SDL_RWseek(tune, 8+256+64, SEEK_SET);
 SDL_RWread(tune,tuneload+a,1,30000);
}
void swi_sound_qtempo(int t) { music.tempo=(2756*0x1000)/t; }
void swi_sound_control(int c,int a,int p,int d) {;}
int swi_sound_speaker(int s) {;}
void swi_stasis_link(int a,int b) {;}
void swi_stasis_control(int a,int b) {;}
void swi_stasis_volslide(int a,int b,int c) {;}


void swi_removecursors() {;}
int swi_readescapestate() { return keyboard[ESC_VALUE];}

void osbyte_71() {;}
int osbyte_79(int c) {
  update_keyboard();
  int key = keybuf;
  keybuf = -1;
  return key;
  //for (int i=0;i<512;i++) if (keyboard[i]) {/*printf("Returning %i\n",i);*/ return i;}
  return -1;
}

int osbyte_7a() { update_keyboard(); for (int i=0;i<512;i++) if (keyboard[i]) return i; return -1;}
void osbyte_7c() { keyboard[ESC_VALUE] = 0;}

int osbyte_81(int c) {
  if (c>=0) return osbyte_79(c);
  update_keyboard();
  return keyboard[-c];}

void osbyte_f1(char c) {;}
char swi_oscrc(int w,char* start,char* end,int bytes) { return 0xff;}
FILE* swi_osfind(int op, const char* name) {
  switch (op)
    {
    case 0x40: return fopen(name,"r");
    case 0x80: return fopen(name,"w");
    case 0xc0: return fopen(name,"a");
    default: return NULL;
    }
}
    
void swi_osgbpb(int op, FILE* f, char* start, char* end, int b) {
  switch (op)
    {
    case 3: for (char* i=start;(i<end)&&!feof(f);i++) *i=fgetc(f);
      break;
    case 1: for (char* i=start;(i<end)&&!feof(f);i++) fputc(*i,f);
      break;
    }
}

int swi_osfile(int op, const char* name, char* start, char* end) {
  FILE* f;
  int x;
  //printf("Looking for %s\n",name);
  switch (op)
    {
    case 10: // save file
      f = fopen(name,"w");
      for (char* i=start;i<end;i++) fputc(*i,f);
      fclose(f);
      return 0;
    case 5: // test file existence
      f = fopen(name,"r");
      if (f == NULL) return 0;
      fclose(f);
      return 1;
    case 15: // file length
      f = fopen(name,"r");
      if (f == NULL) return -1;
      fseek(f,0,SEEK_END);
      x = ftell(f);
      fclose(f);
      return x;
    case 0xff:
    case 14: // load file
      f = fopen(name,"r");
      if (f == NULL) return -1;
      for (char* i=start;!feof(f);i++) *i=fgetc(f);
      fclose(f);
      return 0;
    }
}

int swi_joystick_read(int a,int* x,int* y) {;}

void swi_blitz_wait(int d) { SDL_Delay(d*10);}
void swi_blitz_screenexpand(char* screen) {;}
void swi_blitz_smallretrieve() {;}
void swi_blitz_screenretrieve() {;}
void swi_blitz_screenflush() { SDL_Flip(ArcScreen);}
int swi_blitz_hammerop(int op, char* name, char* path, char* space) {
  char fullname[240]="";
  strcat(fullname,path);
  strcat(fullname,name);
  FILE* f = fopen(fullname,"r");
  if (f==NULL) return -1; // file does not exist
  if ((getc(f)!='H')||(getc(f)!='m')||(getc(f)!='r'))
    {fclose(f); return op;} // file is not Hammered
  
  if (op==0) return 0x24000; // hack: should return length
  char a[524288];
  int p=0;
  char c;
  int fmt = getc(f);
  int len = 0;
  for (int i=0;i<4;i++) len=((len>>8)&0xffffff)|((getc(f)<<24)&0xff000000);
  
  while(!feof(f))
    {
      c = getc(f);
      if (c==0xff) break; // end flag
      else if (c<0x10) {
	// type 1
	int n = (c==15)?256:c+2;
	char d = getc(f);
	for (int i=0;i<n;i++) {a[p++] = d;}
      }
      else if (c<0x20) {
	// type 2
	int n = (c&0xf) + 1;
	for (int i=0;i<n;i++) {a[p++] = getc(f);}
      }
      else if (c<0x40) {
	// type 3
	char d = getc(f);
	int P = p - 1 - ((c&3)<<8) - d;
	if (P<0) {fclose(f); return -3;}
	int n = ((c&0x1c) >> 2) + 2;
	for (int i=0;i<n;i++) {a[p++] = a[P--];}
      }
      else if (c<0x80) {
	// type 4
	char d = getc(f);
	int D = 1 << (((c&4)?((c&6)+2):(c&6))>>1);
	int P = p - 1 -((c&1)<<8) - d;
	if (P<0) {fclose(f); return -4;}
	int n = ((c&0x38)>>3) + 2;
	for (int i=0;i<n;i++) {a[p++] = a[P+(i*D)/4];}
      }
      else {
	// type 5
	char d = getc(f);
	int P = p - 1 - ((c&7)<<8) - d;
	if (P<0) {fclose(f); return -5;}
	int n = ((c&0x78)>>3) + 2;
	for (int i=0;i<n;i++) {a[p++] = a[P++];}
      }
    }

  if (fmt!=0) {
    int wlen = (len+3)>>2;
    for (int i=0;i<p;i++)
      space[i]=a[(i%4)*wlen + (i>>2)];
  }
  else
    for (int i=0;i<p;i++)
      space[i]=a[i];

  fclose(f);
  return p;
}

void swi_fastspr_screenbank(int b) {;}
void swi_fastspr_clearwindow() { SDL_FillRect(ArcScreen,NULL,0);}

void swi_fastspr_setclipwindow(int x1, int y1, int x2, int y2) {
  static SDL_Rect clip;
  clip.x=x1; clip.y=y1;
  clip.w=x2-x1; clip.h=y2-y1;
  SDL_SetClipRect(ArcScreen,&clip);
}

int initialize_sprites(char* start, fastspr_sprite* sprites, int max_sprites, char* end)
{
  uint32_t* s = (uint32_t*)start;
  if (*s != 0x31505346) return 0; // "FSP1"
  int num_sprites = *(s+3);
  if (num_sprites>max_sprites) num_sprites = max_sprites;
  for (int i=0;i<num_sprites;i++) {
    if (*(s+4+i)==0) {sprites[i].s=NULL; continue;}
    uint32_t* p = s + (*(s+4+i)>>2);
    uint32_t* r=s;
    if (i==num_sprites-1) r = (uint32_t*) end;
    else for (int j=0;(r==s)&&(i+j<num_sprites-1);j++) r = s + (*(s+5+i+j)>>2);
    if (r==s) r=(uint32_t*)end;
    uint8_t* pp = (uint8_t*) p;
    int wid = pp[0], hei = pp[1], xcen = pp[2], ycen = pp[3];
    sprites[i].x = xcen; sprites[i].y = ycen;
    sprites[i].s = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,wid,hei,32,
						  0xff,0xff00,0xff0000,0xff000000);
    SDL_LockSurface(sprites[i].s);
    uint32_t* data = (uint32_t*) sprites[i].s->pixels;
    for (int z=0; z<wid*hei; z++) data[z] = 0x0;
    for (uint32_t* q = p + 2 + hei; q<r; q++) {
      int x = ((0x0fffff00&*q)>>8) % 320;
      int y = ((0x0fffff00&*q)>>8) / 320;
      if ((y*wid+x<0)||(y*wid+x>=wid*hei)) printf("%i: x=%i y=%i wid=%i hei=%i: bad idea\n",i,x,y,wid,hei);
	else data[y*wid+x] = palette[0xff&*q];
    }
    SDL_UnlockSurface(sprites[i].s);
  }
  return num_sprites;
}

void update_keyboard()
{
  SDL_Event e;
  SDL_KeyboardEvent* ke;
  SDL_MouseButtonEvent* me;
  while (SDL_PollEvent(&e)) {
    // why not SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_KEYDOWNMASK|SDL_KEYUPMASK)>0)?
    switch (e.type) {
    case SDL_KEYDOWN:
      ke=(SDL_KeyboardEvent*)&e;
      keyboard[ke->keysym.sym] = 0xff;
      keybuf = ke->keysym.sym;
      break;
    case SDL_KEYUP:
      ke=(SDL_KeyboardEvent*)&e;
      keyboard[ke->keysym.sym] = 0;
      break;
    case SDL_MOUSEBUTTONDOWN:
      me=(SDL_MouseButtonEvent*)&e;
      switch (me->button) {
      case SDL_BUTTON_LEFT: mouse|=4; break;
      case SDL_BUTTON_MIDDLE: mouse|=2; break;
      case SDL_BUTTON_RIGHT: mouse|=1; break;
      }
      break;
    case SDL_MOUSEBUTTONUP:
      me=(SDL_MouseButtonEvent*)&e;
      switch (me->button) {
      case SDL_BUTTON_LEFT: mouse&=~4; break;
      case SDL_BUTTON_MIDDLE: mouse&=~2; break;
      case SDL_BUTTON_RIGHT: mouse&=~1; break;
      }
      break;
    }
  }
}

void fspplot(fastspr_sprite* sprites, char n, int x, int y)
{
  fastspr_sprite sprite = sprites[(char)n];
  static SDL_Rect pos;
  pos.x = x - sprite.x; pos.y = y - sprite.y;
  SDL_BlitSurface(sprite.s, NULL, ArcScreen, &pos);
}
