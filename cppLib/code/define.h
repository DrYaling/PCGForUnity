#ifndef _DEFINE_H
#define _DEFINE_H
#define EXPORT_API __declspec(dllexport)

#ifndef BUILDING_COREMODULE
#define BUILDING_COREMODULE 1
#endif

#if BUILDING_COREMODULE
#define EXPORT_COREMODULE __declspec(dllexport)
#else
#define EXPORT_COREMODULE __declspec(dllimport)
#endif
#define UNITY_CPP_PLUGIN_VERSION  1.0.0
#ifdef __cplusplus
#define  EXTERN_C_BEGIN  extern "C"\
{\

#define EXTERN_C_END }

#define EXTERN_C extern "C"
#else
#define  EXTERN_C_BEGIN  
#define EXTERN_C_END 
#define EXTERN_C 
#endif

#include <stdint.h>
typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#endif