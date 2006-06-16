/* This file is part of the KDE project
   Copyright (C) 2002 Sandy Meier <smeier@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "filetemplate.h"

#include <QDateTime>
#include <QFile>
#include <qfileinfo.h>
#include <QRegExp>
#include <qtextstream.h>

#include <kmainwindow.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>

#include "kdevplugin.h"
#include "kdevproject.h"
#include "domutil.h"
#include "kdevmainwindow.h"


bool FileTemplate::exists(const QString &name, Policy p)
{
    return KIO::NetAccess::exists( fullPathForName(name,p), true, KDevApi::self()->mainWindow()->main() );
}

QString FileTemplate::read(const QString &name, Policy p)
{
    return readFile(fullPathForName(name, p) );
}

QString FileTemplate::readFile(const QString &fileName)
{
    QDomDocument* dom = KDevApi::self()->projectDom();
    Q_ASSERT(dom);

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    QTextStream stream(&f);
    QString str = stream.readAll();

    return makeSubstitutions( *dom, str );
}

QString FileTemplate::makeSubstitutions( QDomDocument & dom, const QString & text )
{
    QString author = DomUtil::readEntry(dom, "/general/author");
    QString email = DomUtil::readEntry(dom, "/general/email");
    QString version = DomUtil::readEntry(dom, "/general/version");
    QString date = QDate::currentDate().toString();
    QString year = QString::number(QDate::currentDate().year());

    QString str = text;
    str.replace(QRegExp("\\$EMAIL\\$"),email);
    str.replace(QRegExp("\\$AUTHOR\\$"),author);
    str.replace(QRegExp("\\$VERSION\\$"),version);
    str.replace(QRegExp("\\$DATE\\$"),date);
    str.replace(QRegExp("\\$YEAR\\$"),year);

    return str;
}


bool FileTemplate::copy(const QString &name,
                        const QString &dest, Policy p)
{
    QString text = read(name, p);

    QFile f(dest);
    if (!f.open(QIODevice::WriteOnly))
        return false;

    QFileInfo fi(f);
    QString module = fi.baseName();
    QString basefilename = fi.completeBaseName();
    text.replace(QRegExp("\\$MODULE\\$"),module);
    text.replace(QRegExp("\\$FILENAME\\$"),basefilename);

    QTextStream stream(&f);
    stream << text;
    f.close();

    return true;
}

KUrl FileTemplate::fullPathForName(const QString &name, Policy p)
{
    // if Policy is not default, full path is just the name
    if (p!=Default) return name;

    KUrl url;
    // first try project-specific
    if (KDevApi::self()->project())
    {
        url = (KDevApi::self()->project()->projectDirectory() + "/templates/" + name);
        if (KIO::NetAccess::exists(url, true, KDevApi::self()->mainWindow()->main()))
            return url;
    }

    // next try global
    KUrl globalName = ::locate("data", "kdevfilecreate/file-templates/" + name);
    return globalName.isEmpty() ? url : globalName;
}


QHash<QString,QString> FileTemplate::normalSubstMapToXML( const QHash<QString,QString>& src )
{
    QHash<QString,QString> result;
    QHash<QString,QString>::Iterator it( src.begin() );
    for( ; it != src.end(); ++it )
    {
        QString escaped( it.value() );
        escaped.replace( "&", "&amp;" );
        escaped.replace( "<", "&lt;" );
        escaped.replace( ">", "&gt;" );
        result.insert( it.key(), escaped );
    }
    return result;
}

void FileTemplate::readSubstitutionMap(const QDomDocument& xml)
{
    m_templExpandMap = DomUtil::readHashEntry(xml, "substmap");
    m_templExpandMapXML = normalSubstMapToXML(m_templExpandMap);
}

const QHash<QString, QString>& FileTemplate::substMap( SubstitutionMapTypes type )
{
    switch( type )
    {
    default:
    case NormalFile:
        return m_templExpandMap;
    case XMLFile:
        return m_templExpandMapXML;
    }
}
