/*  file.c */

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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "asylum.h"

#define STOREAREALEN (16*0x40000)

int installed;

char storage[STOREAREALEN];
#define storageend (storage+STOREAREALEN)

//const char idpermitpath[]="<PsychoResource$Path>Idpermit";

static char resource_path[240];
static char score_path[240];

char configname[] = "/.asylum"; //"<PsychoResource$Path>Config";

FILE* find_config(int op)
{
    char fullname[240] = "";

    char* home = getenv("HOME");
    if (home)
	strcat(fullname, home);
    else
	strcat(fullname, resource_path);
    strcat(fullname, configname);
    switch (op)
    {
    case 0x40: return fopen(fullname, "rb");
    case 0x80: return fopen(fullname, "wb");
    case 0xc0: return fopen(fullname, "ab");
    default: return NULL;
    }
}

void dropprivs()
{
#ifndef _WIN32
    setegid(getgid());
    seteuid(getuid());
#endif
}

void set_resource_path()
{
	resource_path[0] = '\0';
	if(chdir(RESOURCEPATH) == 0) {
		strcpy(resource_path, RESOURCEPATH);
#ifdef HAVE_GET_EXE_PATH
	} else {
		get_exe_path(resource_path, sizeof(resource_path));
		strcat(resource_path, "/data");
#endif
	}
}

void set_score_path()
{
	score_path[0] = '\0';
	if(chdir(SCOREPATH) == 0) {
		strcpy(score_path, SCOREPATH);
#ifdef HAVE_GET_EXE_PATH
	} else {
		get_exe_path(score_path, sizeof(score_path));
		strcat(score_path, "/hiscores");
#endif
	}
}



uint32_t read_littleendian(uint32_t* word)
{
    uint8_t* bytes = (uint8_t*)word;

    return (*bytes)|(bytes[1]<<8)|(bytes[2]<<16)|(bytes[3]<<24);
}

int loadhammered_game(char** spaceptr, char* r1, char* path)
{
    int reload;
    do
    {
        reload = loadhammered(spaceptr, r1, path);
        if (reload == -1)
        {
            badload(); return 0;
        }
    }
    while (reload == 1);
    return reload;
}

int loadhammered_level(char** spaceptr, char* r1, char* path)
{
    int reload;
    do
    {
        reload = loadhammered(spaceptr, r1, path);
        if (reload == -1) reload = badlevelload();
    }
    while (reload == 1);
    return reload;
}

int loadvitalfile(char** spaceptr, char* r1, char* path)
{
// if VS or if r0==1
    char fullname[240] = "";

    strcat(fullname, path);
    strcat(fullname, r1);
    int r4 = swi_osfile(15, fullname, 0, 0);
    if (r4 <= 0) fatalfile();
    *spaceptr = (char*)malloc(r4);
    if (swi_osfile(14, fullname, *spaceptr, 0)) fatalfile();
    return r4;
}

int loadhammered(char** spaceptr, char* r1, char* path)
{
    int r3 = swi_blitz_hammerop(0, r1, path, NULL);

    if (r3 == -1)
    {
        filenotthere(); return -1;
    }
    if (r3 == 0) return loadfile(spaceptr, r1, path);
    *spaceptr = (char*)malloc(r3);
    if (*spaceptr == NULL)
    {
        nomemory(); return 1;
    }
    int k;
    if ((k = swi_blitz_hammerop(1, r1, path, *spaceptr)) < 0)
    {
        printf("Error %i\n", -k); filesyserror(); return 1;
    }
    return k;
}

int loadfile(char** spaceptr, char* r1, char* path)
{
    int r4 = filelength(r1, path);
    char fullname[240] = "";
    // hack: +4 as feof doesn't trigger until we've passed the end
    *spaceptr = (char*)malloc(r4+4);

    strcat(fullname, path);
    strcat(fullname, r1);
    if (r4 == -1)
    {
        filenotthere(); return 1;
    }
    if (*spaceptr == NULL)
    {
        nomemory(); return 1;
    }
    if (swi_osfile(14, fullname, *spaceptr, 0))
    {
        filesyserror(); return 1;
    }
    return r4;
}

char egosave[] = "/EgoHighScores";
char psychesave[] = "/PsycheHighScores";
char idsave[] = "/IdHighScores";
char extendedsave[] = "/ExtendedHighScores";
char testsave[] = "/TestPermissions";

void find_resources()
{
    char r1[240], cwd[240];
    getcwd(cwd, 240);
    set_resource_path();
    set_score_path();
    chdir(cwd);
    strcpy(r1, score_path);
    strcat(r1, testsave);
    FILE* permission_test = fopen(r1, "w");
    installed = (NULL != permission_test);
    if (installed)
    {
        fclose(permission_test); unlink(r1);
        if (chdir(resource_path))
        {
            fprintf(stderr, "Couldn't find resources directory %s\n", RESOURCEPATH);
            exit(1);
        }
    }
    else
    {
        fprintf(stderr, "Running as uninstalled, looking for files in local directory.\n");
        if (chdir("./data"))
        {
            fprintf(stderr, "Couldn't find resources directory ./data\n");
            exit(1);
        }
    }
}

void savescores(char* highscorearea, int mentalzone)
{
    highscorearea[13*5] = swi_oscrc(0, highscorearea, highscorearea+13*5, 1);
    char r1[240];
    strcpy(r1, score_path);
    if (!installed) strcpy(r1, "../hiscores");
    switch (mentalzone)
    {
    case 2: strcat(r1, psychesave); break;
    case 3: strcat(r1, idsave); break;
    case 4: strcat(r1, extendedsave); break;
    default: strcat(r1, egosave);
    }
    swi_osfile(10, r1, highscorearea, highscorearea+13*5+1);
    if (installed) chmod(r1, 0660);
}

void loadscores(char* highscorearea, int mentalzone)
{
    char r1[240];
    strcpy(r1, score_path);
    if (!installed) strcpy(r1, "../hiscores");
    switch (mentalzone)
    {
    case 2: strcat(r1, psychesave); break;
    case 3: strcat(r1, idsave); break;
    case 4: strcat(r1, extendedsave); break;
    default: strcat(r1, egosave);
    }
    if (swi_osfile(0xff, r1, highscorearea, highscorearea+13*5+1))
        setdefaultscores();
    else if (swi_oscrc(0, highscorearea, highscorearea+13*5, 1)
             != highscorearea[13*5]) setdefaultscores();
}

int filelength(char* name, char* path)
{
    char fullname[240] = "";

    strcat(fullname, path);
    strcat(fullname, name);
    int r4 = swi_osfile(15, fullname, NULL, NULL);
    if (r4 == -1)
    {
        filesyserror(); return 0;
    }
//if (r0!=1) {fileerror: return -1;}
    return r4;
}

void swi_osgbpb(int op, FILE* f, char* start, char* end, int b)
{
    switch (op)
    {
    case 3: for (char* i = start; (i < end) && !feof(f); i++) *i = fgetc(f);
        break;
    case 1: for (char* i = start; (i < end) && !feof(f); i++) fputc(*i, f);
        break;
    }
}

int swi_osfile(int op, const char* name, char* start, char* end)
{
    FILE* f;
    int x;

    //printf("Looking for %s\n",name);
    switch (op)
    {
    case 10: // save file
        f = fopen(name, "wb");
        for (char* i = start; i < end; i++) fputc(*i, f);
        fclose(f);
        return 0;
    case 5: // test file existence
        f = fopen(name, "rb");
        if (f == NULL) return 0;
        fclose(f);
        return 1;
    case 15: // file length
        f = fopen(name, "rb");
        if (f == NULL) return -1;
        fseek(f, 0, SEEK_END);
        x = ftell(f);
        fclose(f);
        return x;
    case 0xff:
    case 14: // load file
        f = fopen(name, "rb");
        if (f == NULL) return -1;
        for (char* i = start; !feof(f); i++) *i = fgetc(f);
        fclose(f);
        return 0;
    }
}

int swi_blitz_hammerop(int op, char* name, char* path, char* space)
{
    char fullname[240] = "";

    strcat(fullname, path);
    strcat(fullname, name);
    FILE* f = fopen(fullname, "rb");
    if (f == NULL) return -1; // file does not exist
    if ((getc(f) != 'H') || (getc(f) != 'm') || (getc(f) != 'r'))
    {
        fclose(f); return op;
    }                            // file is not Hammered

    if (op == 0) return 0x24000; // hack: should return length
    char a[524288];
    int p = 0;
    char c;
    int fmt = getc(f);
    int len = 0;
    for (int i = 0; i < 4; i++) len = ((len>>8)&0xffffff)|((getc(f)<<24)&0xff000000);

    while (!feof(f))
    {
        c = getc(f);
        if (c == 0xff) break; // end flag
        else if (c < 0x10)
        {
            // type 1
            int n = (c == 15) ? 256 : c+2;
            char d = getc(f);
            for (int i = 0; i < n; i++) a[p++] = d;
        }
        else if (c < 0x20)
        {
            // type 2
            int n = (c&0xf)+1;
            for (int i = 0; i < n; i++) a[p++] = getc(f);
        }
        else if (c < 0x40)
        {
            // type 3
            char d = getc(f);
            int P = p-1-((c&3)<<8)-d;
            if (P < 0)
            {
                fclose(f); return -3;
            }
            int n = ((c&0x1c)>>2)+2;
            for (int i = 0; i < n; i++) a[p++] = a[P--];
        }
        else if (c < 0x80)
        {
            // type 4
            char d = getc(f);
            int D = 1<<(((c&4) ? ((c&6)+2) : (c&6))>>1);
            int P = p-1-((c&1)<<8)-d;
            if (P < 0)
            {
                fclose(f); return -4;
            }
            int n = ((c&0x38)>>3)+2;
            for (int i = 0; i < n; i++) a[p++] = a[P+(i*D)/4];
        }
        else
        {
            // type 5
            char d = getc(f);
            int P = p-1-((c&7)<<8)-d;
            if (P < 0)
            {
                fclose(f); return -5;
            }
            int n = ((c&0x78)>>3)+2;
            for (int i = 0; i < n; i++) a[p++] = a[P++];
        }
    }

    if (fmt != 0)
    {
        int wlen = (len+3)>>2;
        for (int i = 0; i < p; i++)
            space[i] = a[(i%4)*wlen+(i>>2)];
    }
    else
        for (int i = 0; i < p; i++)
            space[i] = a[i];

    fclose(f);
    return p;
}
