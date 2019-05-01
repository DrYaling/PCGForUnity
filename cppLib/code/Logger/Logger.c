#include "Logger.h"
#include <mutex>
namespace logger
{
#define log_buffer_size 2048
	static char Logger_cbuffer[log_buffer_size] = { 0 };
	static std::string logger_buff;
	std::mutex _logMtx;
	static CPPLogCallback logCallBack = nullptr;
	static CPPLogCallback logWarningCallBack = nullptr;
	static CPPLogCallback logErrorCallBack = nullptr;
#ifdef WIN32
#define strsplit strtok_s
#else
#define strsplit strtok_r
#endif
#ifndef WIN32
	static void printStackTrace()
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
	static void printStackTrace()
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
	void printStackTrace()
	{

	}
#endif
	static size_t exclude_dir_len = strlen(solutionDir()) - sizeof("define.h");

	void ClearLogger()
	{
		logErrorCallBack = nullptr;
		logCallBack = nullptr;
		logWarningCallBack = nullptr;
	}
	void SetLogCallBack(int type, CPPLogCallback cb)
	{
		switch (type)
		{
			case 1:
				logWarningCallBack = cb;
				break;
			case 2:
				logErrorCallBack = cb;
				break;
			default:
				logCallBack = cb;
				break;
		}
	}
#if WIN32
	DWORD start, stop;
	const char* profilerContent;
#endif
	void ProfilerStart(const char * content)
	{
#if WIN32
		start = GetTickCount();
		profilerContent = content;
#endif
	}
	void ProfilerEnd()
	{
#if WIN32
		stop = GetTickCount();
		if (!profilerContent)
		{
			profilerContent = "";
		}
		LogFormat("%s cost time %d ms", profilerContent, stop - start);
		profilerContent = nullptr;
#endif
	}
	int LogContent(LoggerType eType, const char	* fp, int line, const char* func, const char* format, ...)
	{
		std::lock_guard<std::mutex> lck(_logMtx);
		logger_buff.clear();
		memset(Logger_cbuffer, 0, sizeof(Logger_cbuffer));
		va_list valist;
		va_start(valist, format);
		_vsnprintf(Logger_cbuffer, log_buffer_size, format, valist);
		logger_buff.append(Logger_cbuffer);
		logger_buff.append("\n");
		va_end(valist);
#ifdef LOG_TRACE
		if (exclude_dir_len < 0 || exclude_dir_len > 255)
		{
			exclude_dir_len = 0;
		}
		memset(Logger_cbuffer, 0, sizeof(Logger_cbuffer));
		sprintf_s(Logger_cbuffer, "in file %s,function %s at line %d\n", fp + exclude_dir_len, func, line);
		logger_buff.append(Logger_cbuffer);
#endif // LOG_TRACE
		//int logtype = -1;
		switch (eType)
		{
			case LoggerType::LOGGER_ERROR:
				if (nullptr != logErrorCallBack)
				{
					logErrorCallBack(logger_buff.c_str());
					//logtype = 1;
				}
				else
					printf_s(logger_buff.c_str());
				break;
			case LoggerType::LOGGER_WARN:
				if (nullptr != logWarningCallBack)
				{
					logWarningCallBack(logger_buff.c_str());
					//logtype = 1;
				}
				else
					printf_s(logger_buff.c_str());
				break;
			default:
				if (nullptr != logCallBack)
				{
					logCallBack(logger_buff.c_str());
					//logtype = 1;
				}
				else
					printf_s(logger_buff.c_str());
				break;
		}
		return 0;
	}
}