/*  This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __chm_h__
#define __chm_h__

#include <qstring.h>
#include <qcstring.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>
#include "chmfile.h"

class QCString;

class ChmProtocol : public KIO::SlaveBase
{
public:
    ChmProtocol( const QCString&, const QCString& );
    ~ChmProtocol();

    void get( const KURL& );

private:
    bool checkNewFile( QString, QString& );
    ChmDirectoryMap m_dirMap;
    QByteArray      m_contents;
    QString         m_chmFile;
    Chm             m_chm;
};


#endif // __chm_h__

