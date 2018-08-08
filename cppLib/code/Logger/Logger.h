#ifndef _SD_LOG_H
#define _SD_LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <iostream>
#include <string.h>
#include <mutex>
#include <map>
#include <thread>
#define LOG_IN_THREAD 0
//#define LOG_TYPE_AS_TITLE
#define LOG_SWITCH 1
#ifdef WIN32
#include "windows.h"
#define sleep(x) Sleep(x)
#endif
enum LoggerType
{
	LOGGER_LOG = 0,
	LOGGER_WARN = 1,
	LOGGER_ERROR = 2
};
static char cbuffer[2 * 2048] = { 0 };
static std::string buff;
#if LOG_SWITCH
#define LogFormat(format, ...) LogContent(LoggerType::LOGGER_LOG,  format, __VA_ARGS__)
#define Log(format) LogContent(LoggerType::LOGGER_LOG,  format)
#define LogWarningFormat(format, ...) LogContent(LoggerType::LOGGER_WARN,  format, __VA_ARGS__)
#define LogWarning(format) LogContent(LoggerType::LOGGER_WARN,  format)
#define LogErrorFormat(format, ...) LogContent(LoggerType::LOGGER_ERROR,  format, __VA_ARGS__)
#define LogError(format) LogContent(LoggerType::LOGGER_ERROR,  format)
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
void ClearLogger()
{
	logErrorCallBack = nullptr;
	logCallBack = nullptr;
	logWarningCallBack = nullptr;
}
void LogContent(LoggerType eType, const char* format, ...)
{
	buff.clear();
	memset(cbuffer, 0, sizeof(cbuffer));
	va_list valist;
	va_start(valist, format);
	vsprintf_s(cbuffer, format, valist);
	buff.append(cbuffer);
	buff.append("\n");
	va_end(valist);
	switch (eType)
	{
	case LoggerType::LOGGER_ERROR:
		if (nullptr != logErrorCallBack)
			logErrorCallBack(buff.c_str());
		break;
	case LoggerType::LOGGER_WARN:
		if (nullptr != logWarningCallBack)
			logWarningCallBack(buff.c_str());
		break;
	default:
		if (nullptr != logCallBack)
			logCallBack(buff.c_str());
		break;
	}
}
#endif