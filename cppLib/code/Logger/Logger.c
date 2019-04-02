#include "Logger.h"

static char Logger_cbuffer[2 * 2048] = { 0 };
static std::string logger_buff;

static CPPLogCallback logCallBack = nullptr;
static CPPLogCallback logWarningCallBack = nullptr;
static CPPLogCallback logErrorCallBack = nullptr;

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

int LogContent(LoggerType eType, const char	* f, int line, const char* func, const char* format, ...)
{
	logger_buff.clear();
	memset(Logger_cbuffer, 0, sizeof(Logger_cbuffer));
	va_list valist;
	va_start(valist, format);
	vsprintf_s(Logger_cbuffer, format, valist);
	logger_buff.append(Logger_cbuffer);
	logger_buff.append("\n");
	va_end(valist);
#ifdef LOG_TRACE
	sprintf_s(Logger_cbuffer, "in file %s,function %s at line %d\n", f, func, line);
	logger_buff.append(Logger_cbuffer);
#endif // LOG_TRACE
	int logtype = -1;
	switch (eType)
	{
	case LoggerType::LOGGER_ERROR:
		if (nullptr != logErrorCallBack)
		{
			logErrorCallBack(logger_buff.c_str());
			logtype = 1;
		}
		else
			printf_s(logger_buff.c_str());
		break;
	case LoggerType::LOGGER_WARN:
		if (nullptr != logWarningCallBack)
		{
			logWarningCallBack(logger_buff.c_str());
			logtype = 1;
		}
		else
			printf_s(logger_buff.c_str());
		break;
	default:
		if (nullptr != logCallBack)
		{
			logCallBack(logger_buff.c_str());
			logtype = 1;
		}
		else
			printf_s(logger_buff.c_str());
		break;
	}
	return logCallBack == nullptr ? -5 : 3;
}