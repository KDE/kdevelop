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
#include <qregexp.h>
#include <qtextstream.h>
#include "kdevpart.h"
#include "kdevproject.h"
#include "domutil.h"


QString FileTemplate::read(KDevPart *part, const QString &relName)
{
    KDevProject *project = part->project();
    QDomDocument &dom = *part->projectDom();
    
    QString fileName = project->projectDirectory() + "/" + relName;

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
