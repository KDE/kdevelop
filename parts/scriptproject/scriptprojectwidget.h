/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SCRIPTPROJECTWIDGET_H_
#define _SCRIPTPROJECTWIDGET_H_

#include <klistview.h>


class ScriptProjectWidget : public KListView
{
    Q_OBJECT

public:
    ScriptProjectWidget( QWidget *parent=0, const char *name=0 );
    ~ScriptProjectWidget();

    void openProject(const QString &dirName);
    void closeProject();
    QStringList allSourceFiles();
    QString projectDirectory();

private:
    QString m_projectDirectory;
};

#endif
