#if defined(GP2X) || defined(PANDORA)
    #define DISABLE_OPENGL
    #undef RESOURCEPATH
    #undef SCOREPATH
    #undef HAVE_GET_EXE_PATH
#endif

#if defined(GP2X)
    #define DISPLAY_HWDOUBLEBUF 1
#else
    #define DISPLAY_HWDOUBLEBUF 0
#endif

#ifndef DISABLE_OPENGL
    #define COLORKEY 0
#else
    #define COLORKEY 0x00000001
    #define NCOLORKEY 0x00000000
#endif
