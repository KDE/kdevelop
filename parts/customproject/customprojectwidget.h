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

#ifndef _CUSTOMPROJECTWIDGET_H_
#define _CUSTOMPROJECTWIDGET_H_

#include <klistview.h>

class CustomProjectPart;


class CustomProjectWidget : public KListView
{
    Q_OBJECT

public:
    CustomProjectWidget( CustomProjectPart *part, QWidget *parent=0, const char *name=0 );
    ~CustomProjectWidget();

    void openProject(const QString &dirName);
    void closeProject();
    QStringList allSourceFiles();
    QString projectDirectory();

private:
    void populateProject();
    
    QString m_projectDirectory;
    CustomProjectPart *m_part;
    QStringList m_sourceFiles;
};


class CustomProjectItem : public QListViewItem
{
public:
    enum Type { File, Dir };
    
    CustomProjectItem(CustomProjectWidget *parent, Type type, const QString &nam);
    CustomProjectItem(CustomProjectItem *parent, Type type, const QString &nam);
    QString path();
    Type type()
    { return typ; }

    virtual void setOpen(bool o);

private:
    void init();
    Type typ;
};

#endif
