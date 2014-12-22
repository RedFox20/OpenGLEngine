/**
 * Copyright (c) 2014 - Jorma Rebane
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	void console_initialize();

	void console_size(int bufferW, int bufferH, int windowW, int windowH);

	/**
	 * Very fast console IO. The console is initialized
	 * with first call to these console functions
	 */
	int console(const char* buffer, int len);
	int consolef(const char* fmt, ...);

	int process_cmdline(char** argv, int maxCount);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
template<int SIZE> inline int console(const char (&str)[SIZE])
{
	return console(str, SIZE - 1);
}
#endif