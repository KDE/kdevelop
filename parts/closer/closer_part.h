/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KDEVPART_CLOSER_H__
#define __KDEVPART_CLOSER_H__


#include <qguardedptr.h>
#include <kdevplugin.h>
#include <ktexteditor/editor.h>


class CloserPart : public KDevPlugin
{
    Q_OBJECT

public:
    CloserPart(QObject *parent, const char *name, const QStringList &);
    ~CloserPart();

public slots:
    void openDialog();

private:
    QStringList openFiles();
    QString relativeProjectPath( QString path );
    QString fullProjectPath( QString path );
    void closeFiles( QStringList const & fileList );
    KTextEditor::Editor * getEditorForFile( QString const & file );

};


#endif
