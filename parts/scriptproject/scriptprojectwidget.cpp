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
#include <qptrstack.h>
#include <kiconloader.h>

#include "scriptprojectwidget.h"


ScriptProjectItem::ScriptProjectItem(ScriptProjectWidget *parent, Type type, const QString &name)
    : QListViewItem(parent, name), typ(type)
{
    init();
}


ScriptProjectItem::ScriptProjectItem(ScriptProjectItem *parent, Type type, const QString &name)
    : QListViewItem(parent, name), typ(type)
{
    init();
}


void ScriptProjectItem::init()
{
    if (typ == File)
        setPixmap(0, SmallIcon("document"));
    else {
        setExpandable(true);
        setPixmap(0, SmallIcon("folder"));
    }
}


QString ScriptProjectItem::path()
{
    QString name = text(0);
    if (parent()) {
        name.prepend("/");
        name.prepend(static_cast<ScriptProjectItem*>(parent())->path());
    }

    return name;
}


void ScriptProjectItem::setOpen(bool o)
{
    if (o && !childCount()) {
        QDir dir(path());

	const QFileInfoList *fileList = dir.entryInfoList();
	if (fileList) {
            QFileInfoListIterator it(*fileList);
            for (; it.current(); ++it) {
                QFileInfo *fi = it.current();
                if (fi->fileName() == "." || fi->fileName() == "..")
                    continue;
                if (fi->isDir())
                    ( new ScriptProjectItem(this, Dir, fi->fileName()) )->setOpen(true);
                else
                    (void) new ScriptProjectItem(this, File, fi->fileName());
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

    ( new ScriptProjectItem(this, ScriptProjectItem::Dir, dirName) )->setOpen(true);
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

#include "scriptprojectwidget.moc"
