/***************************************************************************
 *   Copyright (C) 2002 by Sandy Meier                                     *
 *   smeier@rz.uni-potsdam.de                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
    //QString fileName = (p == Default) ?
    //   (part->project()->projectDirectory() + "/templates/" + name) : name;

    return QFile::exists( fullPathForName(part,name,p) );
}

QString FileTemplate::read(KDevPlugin *part, const QString &name, Policy p)
{
    
  //KDevProject *project = part->project();
    //QString fileName = (p == Default) ? (project->projectDirectory() +
//					 "/templates/" + name) : name;

    return readFile(part, fullPathForName(part, name, p) );
}


QString FileTemplate::readFile(KDevPlugin *part, const QString &fileName)
{
    QDomDocument &dom = *part->projectDom();

    QFile f(fileName);
    if (!f.open(IO_ReadOnly))
        return QString::null;
    QTextStream stream(&f);
    QString str = stream.read();

    QString author = DomUtil::readEntry(dom, "/general/author");
    QString email = DomUtil::readEntry(dom, "/general/email");
    QString version = DomUtil::readEntry(dom, "/general/version");
    QString date = QDate::currentDate().toString();
    QString year = QString::number(QDate::currentDate().year());

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
    if (!f.open(IO_WriteOnly))
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

    // first try project-specific
    QString fileName = (part->project()->projectDirectory() + "/templates/" + name);
    if (QFile::exists(fileName)) return fileName;

    // next try global
    QString globalName = ::locate("data", "kdevfilecreate/file-templates/" + name);
    return globalName.isNull() ? fileName : globalName;
}
