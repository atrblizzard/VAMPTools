#ifndef LOGGING_H
#define LOGGING_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>

extern bool g_bVerboseMode;

#define VLOG( str, ... ) \
if( g_bVerboseMode ) \
{\
	char Buffer[256];\
	sprintf( Buffer, str, __VA_ARGS__ );\
	printf( Buffer );\
	OutputDebugString( Buffer );\
}
#define VLOGWARN( str, ... )\
if( g_bVerboseMode ) \
{\
	HANDLE H; CONSOLE_SCREEN_BUFFER_INFO Info;\
	H = GetStdHandle(STD_OUTPUT_HANDLE);\
	if( GetConsoleScreenBufferInfo(H,&Info) )\
	{\
		SetConsoleTextAttribute( H, FOREGROUND_RED );\
		printf( str, __VA_ARGS__ );\
		SetConsoleTextAttribute( H, Info.wAttributes );\
	}\
}

#endif // LOGGING_H