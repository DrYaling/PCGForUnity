#ifndef _DEFINE_H
#define _DEFINE_H

#if BUILDING_COREMODULE
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __declspec(dllimport)
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
//#include <memory>
#include <stdint.h>
typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#ifdef WIN32
//#pragma comment(lib, " dbghelp.lib")
#define sleep(x) Sleep(x)
#else
#include <algorithm>
#endif

#ifndef sd_max
#define sd_max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef sd_min
#define sd_min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define STD_CALL __stdcall
inline static const char* solutionDir() { return __FILE__; };
#define safe_delete(ptr) if(ptr) \
{\
delete ptr;\
ptr=nullptr;\
}
#define safe_delete_array(arr) if(arr)\
{\
delete[] arr;\
arr = nullptr;\
}
#define release_vector(__vector,__type) if(__vector.capacity()>0)\
{\
	__vector.clear();\
	std::vector<##__type>(__vector).swap(__vector);\
}
#define release_map(__map)	{\
	 for (auto itr = __map.begin(); itr != __map.end();)\
	{\
		itr = __map.erase(itr);\
	}\
	__map.clear();\
}
#endif