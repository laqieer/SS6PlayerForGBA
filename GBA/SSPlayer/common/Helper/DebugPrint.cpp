#include "DebugPrint.h"
#include <stdio.h>  
#include <string>
#include <iostream>
#include <stdarg.h>

#include "tonc_mgba.h"

namespace ss
{


void DEBUG_PRINTF( const char* strFormat, ...   )
{
#ifdef _DEBUG
	char strBuffer[1024];

	va_list arglist;
	va_start( arglist, strFormat);
#if _WIN32
	vsprintf_s(strBuffer, 1024, strFormat, arglist);
	va_end(arglist);
	OutputDebugStringA(strBuffer);
#else
	vsnprintf(strBuffer, 1024, strFormat, arglist);
    mgba_log(LOG_INFO, strBuffer);
	va_end(arglist);
#endif // MASTER_RELEASE

	std::cerr << strBuffer << "\n";
#endif

}


void DEBUG_ERRORF( const char* strFormat, ...   )
{
#ifdef _DEBUG
    char strBuffer[1024];

    va_list arglist;
    va_start( arglist, strFormat);
#if _WIN32
    vsprintf_s(strBuffer, 1024, strFormat, arglist);
    va_end(arglist);
    OutputDebugStringA(strBuffer);
#else
    vsnprintf(strBuffer, 1024, strFormat, arglist);
    mgba_log(LOG_ERR, strBuffer);
    va_end(arglist);
#endif // MASTER_RELEASE

    std::cerr << strBuffer << "\n";
#endif

}

void	THROW_ERROR_MESSAGE_MAIN( std::string str , char* fname , size_t line )
{
#ifdef _DEBUG
	char	___str__buffer[1024];
	sprintf( ___str__buffer , "%s(%d) : %s \n" , fname , (int)line , str.c_str() );
	std::string ___err_message = ___str__buffer;

    mgba_log(LOG_ERR, ___str__buffer);
	//throw ThrowErrorMessage( 0 , ___err_message );
#endif
}
};
