#ifndef KDEVDEEPCOPY_H
#define KDEVDEEPCOPY_H

#include <qstring.h>

inline QString deepCopy( const QString& s )
{
	QCString str = s.utf8();
	return QString::fromUtf8( str, str.length() );
}

#endif 
// kate: indent-mode csands; tab-width 4;

