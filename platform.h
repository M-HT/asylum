#define DISPLAY_HWDOUBLEBUF 0

#ifndef DISABLE_OPENGL
    #define COLORKEY 0
#else
    #define COLORKEY 0x00000001
    #define NCOLORKEY 0x00000000
#endif