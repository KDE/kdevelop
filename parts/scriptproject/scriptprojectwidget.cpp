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

#include <qdir.h>
#include <qheader.h>
#include <qstack.h>
#include <kiconloader.h>

#include "scriptprojectwidget.h"


class ScriptProjectFileItem : public QListViewItem
{
public:
    ScriptProjectFileItem(QListViewItem *parent, const QString &nam);
};


ScriptProjectFileItem::ScriptProjectFileItem(QListViewItem *parent, const QString &name)
    : QListViewItem(parent, name)
{
    setPixmap(0, SmallIcon("document"));
}


class ScriptProjectDirItem : public QListViewItem
{
public:
    ScriptProjectDirItem(ScriptProjectWidget *parent, const QString &name);
    ScriptProjectDirItem(ScriptProjectDirItem *parent, const QString &name);
    QString path();

    void setOpen(bool o);
};


ScriptProjectDirItem::ScriptProjectDirItem(ScriptProjectWidget *parent, const QString &name)
    : QListViewItem(parent, name)
{
    setExpandable(true);
    setPixmap(0, SmallIcon("folder"));
}


ScriptProjectDirItem::ScriptProjectDirItem(ScriptProjectDirItem *parent, const QString &name)
    : QListViewItem(parent, name)
{
    setExpandable(true);
    setPixmap(0, SmallIcon("folder"));
}


QString ScriptProjectDirItem::path()
{
    QString name = text(0);
    if (parent()) {
        name.prepend("/");
        name.prepend(static_cast<ScriptProjectDirItem*>(parent())->path());
    }

    return name;
}


void ScriptProjectDirItem::setOpen(bool o)
{
    if (o && !childCount()) {
        QDir dir(path());

	const QFileInfoList *fileList = dir.entryInfoList();
	if (fileList) {
            QFileInfoListIterator it(*fileList);
            for (; it.current(); ++it) {
                QFileInfo *fi = it.current();
                if (fi->isDir() && fi->fileName() != "." && fi->fileName() != "..")
                    ( new ScriptProjectDirItem(this, fi->fileName()) )->setOpen(true);
                else
                    (void) new ScriptProjectFileItem(this, fi->fileName());
            }
        }

    }
    QListViewItem::setOpen(o);
}


ScriptProjectWidget::ScriptProjectWidget(QWidget *parent, const char *name)
    : KListView(parent, name)
{
    setFrameStyle(Panel | Sunken);
    setSorting(-1);
    setLineWidth(2); 
    header()->hide();
    addColumn("");
}

ScriptProjectWidget::~ScriptProjectWidget()
{}


void ScriptProjectWidget::openProject(const QString &dirName)
{
    m_projectDirectory = dirName;

    ( new ScriptProjectDirItem(this, dirName) )->setOpen(true);
}


void ScriptProjectWidget::closeProject()
{
    clear();
}


QStringList ScriptProjectWidget::allSourceFiles()
{
    QStringList fileList;

    QListViewItemIterator it(this);
    for (; it.current(); ++it) {
        QString name = it.current()->text(0);
        QListViewItem *item = it.current();
        while (item->parent()) {
            item = item->parent();
            name.prepend("/");
            name.prepend(item->text(0));
        }
        fileList.append(name);
    }

    return fileList;
}


QString ScriptProjectWidget::projectDirectory()
{
    return m_projectDirectory;
}
