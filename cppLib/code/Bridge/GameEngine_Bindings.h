#ifndef Game_Engine_bindings_h
#define Game_Engine_bindings_h
#include "define.h"

EXTERN_C_BEGIN
EXPORT_API void STD_CALL StartUpServer(const char* ip, int32_t port);
EXPORT_API void STD_CALL StopServer();
EXPORT_API void STD_CALL StartUpClientTest();

EXTERN_C_END
#endif
