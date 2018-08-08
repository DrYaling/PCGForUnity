#ifndef _SD_LOG_H
#define _SD_LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <iostream>
#include <string.h>
#define LOG_IN_THREAD 0
//#define LOG_TYPE_AS_TITLE
#define LOG_SWITCH 1
#ifdef WIN32
#include "windows.h"
#include <Windows.h>
#include "DbgHelp.h"
//#pragma comment(lib, " dbghelp.lib")
#define sleep(x) Sleep(x)
#else
#include <execinfo.h>
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
#ifndef WIN32
void printStackTrace()
{
	int size = 16;
	void * array[16];
	int stack_num = backtrace(array, size);
	char ** stacktrace = backtrace_symbols(array, stack_num);
	for (int i = 0; i < stack_num; ++i)
	{
		printf("%s\n", stacktrace[i]);
	}
	free(stacktrace);
}
#elif false
void printStackTrace()
{
	unsigned int   i;
	void         * stack[100];
	unsigned short frames;
	SYMBOL_INFO  * symbol;
	HANDLE         process;

	process = GetCurrentProcess();

	SymInitializeW(process, NULL, TRUE);

	frames = CaptureStackBackTrace(0, 100, stack, NULL);
	symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	char buffer[256] = { 0 };
	for (i = 0; i < frames; i++)
	{
		SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
		memset(buffer, 256, 0);
		sprintf_s(buffer, "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
		//printf("%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
		buff.append(buffer);
		buff.append("\n");
	}

	free(symbol);
}

#else
void printStackTrace() {}
#endif
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
	printStackTrace();
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