/*  sound.c */

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

#include <math.h>
#include "asylum.h"

extern asylum_options options;
char sound_available;

void bidforsoundforce(int r0, char r1, char r2, int r3, int r4, int r5, char r6, int r7, Mix_Chunk* chunk)
{
  if (options.soundtype && sound_available)
  {
    r2 = (r2 > options.soundvol) ? options.soundvol : r2;
    soundclaim(r0&7, r1, r2, r3, r4, r5, r6, r7, chunk);
  }
}

void bidforsound(int r0, char r1, char r2, int r3, int r4, int r5, char r6, int r7, Mix_Chunk* chunk)
{
  if (options.soundtype && sound_available)
  {
    r2 = (r2 > options.soundvol) ? options.soundvol : r2;
    if ((r0&7)==_Explochannel)
      soundclaimexplo(r0&7, r1, r2, r3, r4, r5, r6, r7, chunk);
    else
      soundclaimmaybe(r0&7, r1, r2, r3, r4, r5, r6, r7, chunk);
  }
}

void soundclaimmaybe(int r0, char r1, char r2, int r3, int r4, int r5, char r6, int r7, Mix_Chunk* chunk)
{
//soundtab=soundtabofs+(r0<<soundtabshift);
    if ((!Mix_Playing(r0)) || (Mix_GetChunk(r0)->volume < r2)) soundclaim(r0, r1, r2, r3, r4, r5, r6, r7, chunk);
}

void soundclaimexplo(int r0, char r1, char r2, int r3, int r4, int r5, char r6, int r7, Mix_Chunk* chunk)
{
//soundtab=soundtabofs+(r0<<soundtabshift);
   //bidforexplo:
    if ((!Mix_Playing(3)) || (Mix_GetChunk(3)->volume < r2)) soundclaim(3, r1, r2, r3, r4, r5, r6, r7, chunk);
    else if ((!Mix_Playing(4)) || (Mix_GetChunk(4)->volume < r2)) soundclaim(4, r1, r2, r3, r4, r5, r6, r7, chunk);
    else if ((!Mix_Playing(5)) || (Mix_GetChunk(5)->volume < r2)) soundclaim(5, r1, r2, r3, r4, r5, r6, r7, chunk);
    else if ((!Mix_Playing(6)) || (Mix_GetChunk(6)->volume < r2)) soundclaim(6, r1, r2, r3, r4, r5, r6, r7, chunk);
    else if ((!Mix_Playing(7)) || (Mix_GetChunk(7)->volume < r2)) soundclaim(7, r1, r2, r3, r4, r5, r6, r7, chunk);
}

typedef struct
{
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
Mix_Music* oggmusic[4];

void init_audio()
{
    if (Mix_Init(0) < 0)
    {
        fprintf(stderr, "Sound disabled: initializing audio failed: %s\n", Mix_GetError());
        sound_available = 0;
        return;
    }
    atexit(Mix_Quit);

#if defined(PANDORA) || defined(PYRA)
    #define CHUNKSIZE 2048
#else
    #define CHUNKSIZE 1024
#endif

#if SDL_VERSIONNUM(SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL) >= SDL_VERSIONNUM(2,0,2)
    const SDL_version *link_version = Mix_Linked_Version();
    if (SDL_VERSIONNUM(link_version->major, link_version->minor, link_version->patch) >= SDL_VERSIONNUM(2,0,2))
    {
        sound_available = !Mix_OpenAudioDevice(22050, MIX_DEFAULT_FORMAT, 2, CHUNKSIZE, NULL,
        #ifdef SDL_AUDIO_ALLOW_SAMPLES_CHANGE
            SDL_AUDIO_ALLOW_SAMPLES_CHANGE
        #else
            0
        #endif
        );
    }
    else
#endif
    {
        sound_available = !Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, CHUNKSIZE);
        if (sound_available)
        {
            int frequency, channels;
            Uint16 format;

            if (Mix_QuerySpec(&frequency, &format, &channels))
            {
                if ((frequency != 22050) ||
                    (channels != 2) ||
                    (format != MIX_DEFAULT_FORMAT)
                   )
                {
                    sound_available = 0;
                }
            }
            else sound_available = 0;

            if (!sound_available) Mix_CloseAudio();
        }
    }
    if (!sound_available) fprintf(stderr, "Sound disabled: opening audio device failed: %s\n", Mix_GetError());
    else atexit(Mix_CloseAudio);
}

void init_mulaw()
{
    Sint16 exp[8] = { 0, 132, 396, 924, 1980, 4092, 8316, 16764 };
    for (int i = 0; i < 256; i++)
        mulaw[i] = ((i&1) ? -1 : 1)*(exp[i>>5]+((i&0x1e)<<(2+(i>>5))));
}

Mix_Chunk* make_sound(char samp, int initpitch, int volslide, int pitchslide, char frames)
{
    int numsamples = (int)(frames*22050.0/50.0*2); // frames are 50Hz?
    Mix_Chunk* mc = (Mix_Chunk*)malloc(sizeof(Mix_Chunk));
    Uint16* s = (Uint16*)malloc(numsamples*2*sizeof(Uint16));

    mc->abuf = (Uint8*)s;
    mc->alen = numsamples*2*sizeof(Uint16);
    mc->allocated = 0; // don't automatically free buffer
    mc->volume = 0xff; // XXX volume

    // try reading file from soundcache
    {
        FILE *f;
        char name[256];
        uint16_t *pstereo, *pmono;

        sprintf(name, "soundcache/%02x-%x-%x-%x-%02x.mono", samp, initpitch, volslide, pitchslide, frames);
        f = fopen(name, "rb");
        if (f != NULL)
        {
            pstereo = (uint16_t *)(mc->abuf);
            pmono = pstereo + (mc->alen / 4);
            if (fread(pmono, 1, mc->alen / 2, f) == (mc->alen / 2))
            {
                fclose(f);

                for (unsigned int i = 0; i < mc->alen / 4; i++) {
                    pstereo[2*i] = pstereo[2*i + 1] = pmono[i];
                }

                return mc;
            }
            fclose(f);
        }
    }

    int time = 0;
    double ps = ((Sint16)(pitchslide&0xffff))*22/22050.0;
    double vs = ((Sint16)(volslide&0xffff))/22050.0;
    //Sint16 psmax = pitchslide>>16;
    double matching = 0;
    fprintf(stderr, "."); fflush(stderr);
    for (unsigned int i = 0; i < numsamples*2*sizeof(Uint16); i += 4, time++, s += 2)
    {
        Uint16 mono = 0;

        double pitch = initpitch+ps*time; // XXX pitchslide
        //if ((ps>0)&&(pitch>psmax)) pitch=psmax;
        //if ((ps<0)&&(pitch<psmax)) pitch=psmax;
        //int off = time;
        double old_sample_rate = 500.0*pow(2, pitch/4096.0);
        matching += old_sample_rate/22050;
        for (int j = (int)matching-32; j <= (int)matching+32; j++)
        {
            if ((j < 0)) continue;
            int jj = j;
            uint32_t len = read_littleendian(((uint32_t*)voice[(unsigned char)samp])+6);
            uint32_t gap = read_littleendian(((uint32_t*)voice[(unsigned char)samp])+7); // ???
            uint32_t rep = read_littleendian(((uint32_t*)voice[(unsigned char)samp])+8);
            if ((unsigned int)jj >= len) {
                if (rep == 0) continue;
                else jj = (len-rep)+((jj-len)%rep);
            }
            double w;
            if (jj >= (int32_t)(len-gap))
                w = ((jj-(len-gap))*(double)mulaw[(unsigned char)voice[(unsigned char)samp][jj+44-rep]]
                     +(len-jj)*(double)mulaw[(unsigned char)voice[(unsigned char)samp][jj+44]])/gap/4;
            else w = mulaw[(unsigned char)voice[(unsigned char)samp][jj+44]]/4;
            double vscale = 0x7f+vs*time;
            if (vscale > 0xff) vscale = 0xff;
            if (vscale > 0) w *= (vscale/0x7f);else w = 0;
            double x = M_PI*(j-matching);
            if (x == 0) mono += (Sint16)(w/1.3);
            else mono += (Sint16)(w*sinf(x)/(1.3*x));
        }
        s[1] = *s = mono; // XXX stereo
    }
    // write file to soundcache
    /*{
        FILE *f;
        char name[256];
        uint16_t *pstereo, *pmono;

        pstereo = (uint16_t *)(mc->abuf);
        pmono = (uint16_t *)malloc(mc->alen / 2);
        for (int i = 0; i < mc->alen / 4; i++) {
            pmono[i] = pstereo[2*i];
        }

        sprintf(name, "soundcache/%02x-%x-%x-%x-%02x.mono", samp, initpitch, volslide, pitchslide, frames);
        f = fopen(name, "wb");
        if (f != NULL)
        {
            fwrite(pmono, 1, mc->alen / 2, f);
            fclose(f);
        }
        free(pmono);
    }*/
    return mc;
}

void soundclaim(int c, char samp, char initvol, int initpitch, int volslide, int pitchslide, char frames, int stereo, Mix_Chunk* static_chunk)
{
    //int* argv = (int*)vargv;
    //int c = argv[0]; char samp = argv[1]; char initvol = argv[2]; int initpitch = argv[3];
    //int volslide = argv[4]; int pitchslide = argv[5]; char frames = argv[6]; int stereo = argv[7];
    //Mix_Chunk* old_chunk = Mix_GetChunk(c);
    Mix_Chunk* chunk = static_chunk ? static_chunk : make_sound(samp, initpitch, volslide, pitchslide, frames);

    Mix_HaltChannel(c);
    Mix_Volume(c, initvol);
    Mix_SetPanning(c, 254-stereo, stereo);
    Mix_PlayChannelTimed(c, chunk, 0, -1);
    //if (old_chunk) Mix_FreeChunk(old_chunk); XXX memory leak
}

FILE* musicdumpfile;

void sdl_music_hook(void* udata, Uint8* stream, int len)
{
    music_state* m = (music_state*)udata;
    char* tuneload = m->tune;
    Sint16* s = (Sint16*)stream;
    int window = (len < 0) ? 16 : 4;
    unsigned long ln = (len < 0) ? -ftell(musicdumpfile) : 0;

    for (int i = 0; (len < 0) || (i < len); i += 4, m->time++, (len < 0) || (s += 2))
    {
        if (0 == (m->time%m->tempo))
        {
            // new beat
            if (len == -2)
            {
                ln += ftell(musicdumpfile); fseek(musicdumpfile, 8, 0);
                fputc(ln>>24, musicdumpfile); fputc((ln&0xff0000)>>16, musicdumpfile);
                fputc((ln&0xff00)>>8, musicdumpfile); fputc(ln&0xff, musicdumpfile);
                break;
            }
            int inst = 0;
            int pitch = 8;
            char first;
            char second;
            do
            {
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
            }
            while ((second&0xc0) == 0);
            if (second&0x80)
            {
                m->section++;
                if (0x80&*m->section)
                {
                    m->section = tuneload+8; if (len < 0) len = -2;
                }
                m->pointer = read_littleendian(((uint32_t*)tuneload)+0x42+*m->section);
            }
        }
        *s = 0; s[1] = 0;
        for (int v = 0; v < 4; v++)
        {
            if (m->inst[v] == 0) continue;
            int pitch = m->pitch[v];
            int off = m->time-m->start_time[v];
            double old_sample_rate = 4000.0*pow(2, pitch/12.0);
            double matching = off*old_sample_rate/22050;
            for (int j = (int)matching-window; j <= (int)matching+window; j++)
            {
                if ((j < 0)) continue;
                int jj = j;
                uint32_t len = read_littleendian(((uint32_t*)voice[m->inst[v]])+6);
                uint32_t gap = read_littleendian(((uint32_t*)voice[m->inst[v]])+7); // ???
                uint32_t rep = read_littleendian(((uint32_t*)voice[m->inst[v]])+8);
                /*if (m->inst[v]==16)
                   while (jj>=6960+1000) jj-=6960;
                   else
                   if (jj>=8192) continue;*/
                if ((unsigned int)jj >= len) jj = rep ? ((len-rep)+((jj-len)%rep)) : 0;
                double w;
                if (jj >= (int32_t)(len-gap))
                    w = ((jj-(len-gap))*(double)mulaw[(unsigned char)voice[m->inst[v]][jj+44-rep]]
                         +(len-jj)*(double)mulaw[(unsigned char)voice[m->inst[v]][jj+44]])/gap/4;
                else w = mulaw[(unsigned char)voice[m->inst[v]][jj+44]]/4;
                double x = M_PI*(j-matching);
                if (x == 0) s[v&1] += (Sint16)(w/1.3);
                else s[v&1] += (Sint16)(w*sinf(x)/(1.3*x));
            }
        }
        if (len < 0)
        {
            /* .au is big-endian format */
            fputc(s[0]>>8, musicdumpfile);
            fputc(s[0]&0xff, musicdumpfile);
            fputc(s[1]>>8, musicdumpfile);
            fputc(s[1]&0xff, musicdumpfile);
        }
    }
}

void load_voices(int dumpmusic)
{
    if (!sound_available && !dumpmusic) return;
    load_voice(1, "./Voices/Jump");
    load_voice(2, "./Voices/Bonus");
    load_voice(3, "./Voices/Explo");
    load_voice(4, "./Voices/AtomExplo");
    load_voice(5, "./Voices/Cannon");
    load_voice(6, "./Voices/Rocket");
    load_voice(7, "./Voices/Hiss");
    load_voice(8, "./Voices/Stunned");
    load_voice(9, "./Voices/SmallZap");
    load_voice(10, "./Voices/BigZap");
    load_voice(16, "./Voices/Organ");
    load_voice(17, "./Voices/Plink");
    load_voice(18, "./Voices/PlinkHard");
    load_voice(19, "./Voices/Raver");
    load_voice(20, "./Voices/Dome");
    load_voice(21, "./Voices/NasalStr");
    load_voice(22, "./Voices/BassHollow");
    load_voice(23, "./Voices/NasalBass");
    load_voice(24, "./Voices/BassDrum");
    load_voice(25, "./Voices/Snare");
    load_voice(26, "./Voices/Cymbal");
}

void init_sounds()
{
    if (!sound_available) return;
    init_mulaw();
    fprintf(stderr, "Building sound effects ");
    init_chunk_bullet();
    init_chunk_player();
    init_chunk_alien();
    init_chunk_maze();
    fprintf(stderr, " done.\n");
}

void load_voice(int v, const char* filename)
{
    FILE* file = fopen(filename, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "Loading sound effect %s failed\n", filename);
        return;
    }
    fseek(file, 0, SEEK_END);
    int file_len = ftell(file) /*-44*/;
    fseek(file, 0 /*44*/, SEEK_SET);
    fread(voice[v], 1, file_len, file);
    fclose(file);
}


void initialize_music(int a)
{
    swi_sound_qtempo(0x1000);
    music.time = 0;
    music.tune = tuneload[a];
    music.section = tuneload[a]+8;
    music.pointer = read_littleendian(((uint32_t*)(tuneload[a]))+0x42+*music.section);
    for (int v = 0; v < 4; v++)
    {
        music.pitch[v] = 0; music.inst[v] = 0; music.start_time[v] = 0;
    }
}

void dumpmusic(int argc,char** argv)
{
    init_mulaw();
    Uint8 wav[4];
    char musicdumppath[1024] = "";
    char* musicinputpath = argv[2];
    strncat(musicdumppath, musicinputpath, 1000);
    strncat(musicdumppath, ".au", 20);
    fprintf(stderr, "Dumping music ");
    //swi_bodgemusic_load(1,musicinputpath);
    musicdumpfile = fopen(musicdumppath, "wb");
    FILE* tune = fopen(musicinputpath, "rb");
    fread(tuneload+1, 1, 30000, tune);
    fclose(tune);
    initialize_music(1);
    if (argc > 3) if (!strcmp(argv[3], "--slower")) swi_sound_qtempo(0x980);
    fputs(".snd", musicdumpfile);
    fputc(0, musicdumpfile); fputc(0, musicdumpfile); fputc(0, musicdumpfile); fputc(32, musicdumpfile);        // data start
    fputc(255, musicdumpfile); fputc(255, musicdumpfile); fputc(255, musicdumpfile); fputc(255, musicdumpfile); // length
    fputc(0, musicdumpfile); fputc(0, musicdumpfile); fputc(0, musicdumpfile); fputc(3, musicdumpfile);         // 16-bit linear
    fputc(0, musicdumpfile); fputc(0, musicdumpfile); fputc(22050>>8, musicdumpfile); fputc(22050&0xff, musicdumpfile);
    fputc(0, musicdumpfile); fputc(0, musicdumpfile); fputc(0, musicdumpfile); fputc(2, musicdumpfile);         // stereo
    fputs("blotwell", musicdumpfile);
    sdl_music_hook(&music, wav+2, -1 /* dump entire track into buffer */);
    fclose(musicdumpfile);
    exit(0);
}

void maketestsound(int r1)
{
    swi_stasis_link(1, 1);
    swi_sound_control(1, 0x100|r1, 0x20, 0xfe);
    //swi_bodgemusic_volume(musicvol);
}
void swi_bodgemusic_start(int a, int b)
{
    if (!sound_available) return;
    swi_bodgemusic_stop();
    if (oggmusic[a]) Mix_PlayMusic(oggmusic[a], -1);
    else
    {
        initialize_music(a);
        Mix_HookMusic(sdl_music_hook, &music);
    }
}
void swi_bodgemusic_stop()
{
    if (!sound_available) return;
    Mix_HaltMusic(); Mix_HookMusic(NULL, NULL);
}
void swi_bodgemusic_volume(int v)
{
    Mix_VolumeMusic(v);
}
void swi_bodgemusic_load(int a, char* b)
{
    char name[1024] = "";

    strncpy(name, b, 1000);
    strncat(name, ".ogg", 20);
    if (oggmusic[a]) Mix_FreeMusic(oggmusic[a]);
    oggmusic[a] = Mix_LoadMUS(name);
    if (oggmusic[a]) return;
    FILE* tune = fopen(b, "rb");
    //fseek(tune, 8+256+64, SEEK_SET);
    fread(tuneload+a, 1, 30000, tune);
    fclose(tune);
}
void swi_sound_qtempo(int t)
{
    music.tempo = (2756*0x1000)/t;
}
void swi_sound_control(int c, int a, int p, int d)
{
    ;
}
int swi_sound_speaker(int s)
{
    return 0;
}
void swi_stasis_link(int a, int b)
{
    ;
}
void swi_stasis_control(int a, int b)
{
    ;
}
void swi_stasis_volslide(int a, int b, int c)
{
    ;
}
