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
#endif