#ifndef _SD_LOG_H
#define _SD_LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <iostream>
#include <string.h>
#include "define.h"
#define LOG_IN_THREAD 0
//#define LOG_TYPE_AS_TITLE
#define LOG_SWITCH 1
//#define LOG_TRACE
#ifdef WIN32
#include <Windows.h>
#include "DbgHelp.h"
#else
#include <execinfo.h>
#endif


enum LoggerType
{
	LOGGER_LOG = 0,
	LOGGER_WARN = 1,
	LOGGER_ERROR = 2
};
#if LOG_SWITCH
#define LogFormat(format, ...) LogContent(LoggerType::LOGGER_LOG,__FILE__, __LINE__,__FUNCTION__, format, __VA_ARGS__)
#define Log(format) LogContent(LoggerType::LOGGER_LOG,__FILE__, __LINE__,__FUNCTION__,  format)
#define LogWarningFormat(format, ...) LogContent(LoggerType::LOGGER_WARN,__FILE__, __LINE__,__FUNCTION__,  format, __VA_ARGS__)
#define LogWarning(format) LogContent(LoggerType::LOGGER_WARN,  format)
#define LogErrorFormat(format, ...) LogContent(LoggerType::LOGGER_ERROR,__FILE__, __LINE__,__FUNCTION__,  format, __VA_ARGS__)
#define LogError(format) LogContent(LoggerType::LOGGER_ERROR,__FILE__, __LINE__,__FUNCTION__,  format)
#else
#define LogFormat(logType,format, ...) 
#define Log(logType,format)
#define LogWarningFormat(logType,format, ...) 
#define LogWarning(logType,format) 
#define LogErrorFormat(logType,format, ...)
#define LogError(logType,format) 
#endif

typedef void(__stdcall *CPPLogCallback)(const char* log);
typedef void(__stdcall *CPPLogWarningCallback)(const char* log);
typedef void(__stdcall *CPPLogErrorCallback)(const char* log);
static CPPLogCallback logCallBack = nullptr;
static CPPLogWarningCallback logWarningCallBack = nullptr;
static CPPLogErrorCallback logErrorCallBack = nullptr;

#ifdef __cplusplus
extern "C"
{
#endif
	void LogContent(LoggerType eType,const char	* f, int line, const char* func, const char* format, ...);
	void printStackTrace(void);
	void ClearLogger(void);

#ifdef __cplusplus
}
#endif

#endif