/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "parser.h"
#include <qobject.h>
#include <qstringlist.h>

class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static QCString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};

QString Parser::cleanArgs( const QString &func )
{
    QString slot( func );
    int begin = slot.find( "(" ) + 1;
    QString args = slot.mid( begin );
    args = args.left( args.find( ")" ) );
    QStringList lst = QStringList::split( ',', args );
    QString res = slot.left( begin );
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	if ( it != lst.begin() )
	    res += ",";
	QString arg = *it;
	int pos = 0;
	if ( ( pos = arg.find( "&" ) ) != -1 ) {
	    arg = arg.left( pos + 1 );
	} else if ( ( pos = arg.find( "*" ) ) != -1 ) {
	    arg = arg.left( pos + 1 );
	} else {
	    arg = arg.simplifyWhiteSpace();
	    if ( ( pos = arg.find( ':' ) ) != -1 )
		arg = arg.left( pos ).simplifyWhiteSpace() + ":" + arg.mid( pos + 1 ).simplifyWhiteSpace();
	    QStringList l = QStringList::split( ' ', arg );
	    if ( l.count() == 2 ) {
		if ( l[ 0 ] != "const" && l[ 0 ] != "unsigned" && l[ 0 ] != "var" )
		    arg = l[ 0 ];
	    } else if ( l.count() == 3 ) {
		arg = l[ 0 ] + " " + l[ 1 ];
	    }
	}
	res += arg;
    }	
    res += ")";

    return QString::fromLatin1( NormalizeObject::normalizeSignalSlot( res.latin1() ) );
}
