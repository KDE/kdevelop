#warning Consider switching to QStringList::split

#include "misc.h"
#include <string.h>

enum quoteEnum { NO_QUOTE=0, SINGLE_QUOTE, DOUBLE_QUOTE };

StringTokenizer::StringTokenizer()
{
    buffer = 0;
    pos    = 0;
    end    = 0;
    bufLen = 0;
}
 
void StringTokenizer::tokenize( const char *str, const char *_separators )
{
    if ( *str == '\0' )
    {
        pos = 0;
        return;
    }
 
    int strLength = strlen( str ) + 1;
 
    if ( bufLen < strLength )
    {
        delete [] buffer;
        buffer = new char[ strLength ];
        bufLen = strLength;
    }
 
    const char *src = str;
    end = buffer;
    int quoted = NO_QUOTE;

    for ( ; *src != '\0'; src++ )
    {
        char *x = strchr( _separators, *src );
        if (( *src == '\"' ) && !quoted)
           quoted = DOUBLE_QUOTE;
        else if (( *src == '\'') && !quoted)
           quoted = SINGLE_QUOTE;
        else if ( (( *src == '\"') && (quoted == DOUBLE_QUOTE)) ||
                 (( *src == '\'') && (quoted == SINGLE_QUOTE)))
           quoted = NO_QUOTE;
        else if ( x && !quoted )
            *end++ = 0;
        else
            *end++ = *src;
    }
 
    *end = 0;
 
    if ( end - buffer <= 1 )
        pos = 0;        // no tokens
    else
        pos = buffer;
}            

const char* StringTokenizer::nextToken()
{
    if ( pos == 0 )
        return 0;
 
    char *ret = pos;
    pos += strlen( ret ) + 1;
    if ( pos >= end )
        pos = 0;
 
    return ret;
}
 
StringTokenizer::~StringTokenizer()
{
    if ( buffer != 0 )
        delete [] buffer;
}
