#ifndef KDEVDEEPCOPY_H
#define KDEVDEEPCOPY_H

#include <qstring.h>
//Added by qt3to4:
#include <Q3CString>

inline QString deepCopy( const QString& s )
{
   Q3CString str = s.utf8();
   return QString::fromUtf8( str, str.length() );
}

#endif
