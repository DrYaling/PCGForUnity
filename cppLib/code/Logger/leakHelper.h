#ifndef _leak_helper_h
#define _leak_helper_h
//set DUMP_LEAK to 1 to enable leak check
#define  DUMP_LEAK 1
#if DUMP_LEAK
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h> 
#if _DEBUG && _WIN32

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#define PLACEMENT_DEBUG_NEW new(_UNKNOWN_BLOCK,_NORMAL_BLOCK, __FILE__, __LINE__)
#undef placement_new
#define placement_new PLACEMENT_DEBUG_NEW

#endif

#endif

#endif
