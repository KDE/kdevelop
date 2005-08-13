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

#ifndef SOURCEFILE_H
#define SOURCEFILE_H

#include <qobject.h>
#include "timestamp.h"

struct DesignerSourceFile;
class SourceEditor;
class Project;

class SourceFile : public QObject
{
    Q_OBJECT

public:
    SourceFile( const QString &fn, bool temp, Project *p );
    ~SourceFile();

    void setText( const QString &s );
    void setModified( bool m );

    bool save( bool ignoreModified = FALSE );
    bool saveAs( bool ignoreModified = FALSE );
    bool load();
    bool close();
    bool closeEvent();
    Project *project() const;

    QString text() const;
    QString fileName() const { return filename; }
    bool isModified() const;

    void checkTimeStamp();

    DesignerSourceFile *iFace();

    void setEditor( SourceEditor *e );
    SourceEditor *editor() const { return ed; }

    static QString createUnnamedFileName( const QString &extension );

    bool isAccepted() const { return accepted; }

private:
    bool checkFileName( bool allowBreak );

private:
    QString filename;
    QString txt;
    DesignerSourceFile *iface;
    SourceEditor *ed;
    bool fileNameTemp;
    TimeStamp timeStamp;
    Project *pro;
    bool pkg;
    bool accepted;

};

#endif
