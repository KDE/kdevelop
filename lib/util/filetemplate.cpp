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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "filetemplate.h"

#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qtextstream.h>

#include <kstandarddirs.h>

#include "kdevplugin.h"
#include "kdevproject.h"
#include "domutil.h"


bool FileTemplate::exists(KDevPlugin *part, const QString &name, Policy p)
{
    return QFile::exists( fullPathForName(part,name,p) );
}

QString FileTemplate::read(KDevPlugin *part, const QString &name, Policy p)
{
    return readFile(part, fullPathForName(part, name, p) );
}

QString FileTemplate::readFile(KDevPlugin *part, const QString &fileName)
{
    QDomDocument &dom = *part->projectDom();

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    QTextStream stream(&f);
    QString str = stream.read();

    return makeSubstitutions( dom, str );
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


bool FileTemplate::copy(KDevPlugin *part, const QString &name,
                        const QString &dest, Policy p)
{
    QString text = read(part, name, p);

    QFile f(dest);
    if (!f.open(QIODevice::WriteOnly))
        return false;

    QFileInfo fi(f);
    QString module = fi.baseName();
    QString basefilename = fi.baseName(true);
    text.replace(QRegExp("\\$MODULE\\$"),module);
    text.replace(QRegExp("\\$FILENAME\\$"),basefilename);

    QTextStream stream(&f);
    stream << text;
    f.close();

    return true;
}

QString FileTemplate::fullPathForName(KDevPlugin *part, const QString &name,
                                      Policy p) {
    // if Policy is not default, full path is just the name
    if (p!=Default) return name;

    QString fileName;
    // first try project-specific
    if (part->project())
    {
        fileName = (part->project()->projectDirectory() + "/templates/" + name);
        if (QFile::exists(fileName)) return fileName;
    }

    // next try global
    QString globalName = ::locate("data", "kdevfilecreate/file-templates/" + name);
    return globalName.isNull() ? fileName : globalName;
}
