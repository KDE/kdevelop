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

#include <qapplication.h>
#include <qdir.h>
#include <qdom.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qvaluestack.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "customprojectpart.h"
#include "customprojectwidget.h"


CustomProjectItem::CustomProjectItem(CustomProjectWidget *parent, Type type, const QString &name)
    : QListViewItem(parent, name), typ(type)
{
    init();
}


CustomProjectItem::CustomProjectItem(CustomProjectItem *parent, Type type, const QString &name)
    : QListViewItem(parent, name), typ(type)
{
    init();
}


void CustomProjectItem::init()
{
    if (typ == File)
        setPixmap(0, SmallIcon("document"));
    else {
        setExpandable(true);
        setPixmap(0, SmallIcon("folder"));
    }
}


QString CustomProjectItem::path()
{
    QString name = text(0);
    if (parent()) {
        name.prepend("/");
        name.prepend(static_cast<CustomProjectItem*>(parent())->path());
    }

    return name;
}


void CustomProjectItem::setOpen(bool o)
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
                    ( new CustomProjectItem(this, Dir, fi->fileName()) )->setOpen(true);
                else
                    (void) new CustomProjectItem(this, File, fi->fileName());
            }
        }

    }
    QListViewItem::setOpen(o);
}


CustomProjectWidget::CustomProjectWidget(CustomProjectPart *part, QWidget *parent, const char *name)
    : KListView(parent, name)
{
    setFrameStyle(Panel | Sunken);
    setSorting(-1);
    setLineWidth(2); 
    header()->hide();
    addColumn("");

    m_part = part;
}


CustomProjectWidget::~CustomProjectWidget()
{}


void CustomProjectWidget::openProject(const QString &dirName)
{
    m_projectDirectory = dirName;

    QDomElement docEl = m_part->projectDom()->documentElement();
    QDomElement customprojectEl = docEl.namedItem("kdevcustomproject").toElement();
    QDomElement filesEl = customprojectEl.namedItem("files").toElement();

    if (filesEl.isNull()) {
        int r = KMessageBox::questionYesNo(this, i18n("This project doesn not contain any files yet.\n"
                                                      "Populate it with all C/C++/Java files below\n"
                                                      "the project directory?"));
        if (r == KMessageBox::Yes)
            populateProject();
        // Try again now
        filesEl = customprojectEl.namedItem("files").toElement();
        if (filesEl.isNull())
            return;
    }
    
    QDomElement childEl = filesEl.firstChild().toElement();
    while (!childEl.isNull()) {
        if (childEl.tagName() == "file")
            m_sourceFiles << childEl.firstChild().toText().data();
        childEl = childEl.nextSibling().toElement();
    }
    
    ( new CustomProjectItem(this, CustomProjectItem::Dir, dirName) )->setOpen(true);
}


void CustomProjectWidget::populateProject()
{
    QApplication::setOverrideCursor(Qt::waitCursor);
    
    QDomDocument &dom = *m_part->projectDom();
    
    QDomElement docEl = dom.documentElement();
    QDomElement customprojectEl = docEl.namedItem("kdevcustomproject").toElement();
    QDomElement filesEl = dom.createElement("files");
    customprojectEl.appendChild(filesEl);

    QStringList fileList;
    QValueStack<QString> s;
    s.push(m_projectDirectory);
    
    QDir dir;
    do {
        dir.setPath(s.pop());
        kdDebug(9025) << "Examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
        QListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) {
                kdDebug(9025) << "Pushing: " << path << endl;
                s.push(path);
            }
            else {
                kdDebug(9025) << "Adding: " << path << endl;
                fileList.append(path);
            }
        }
    } while (!s.isEmpty());

    QStringList::ConstIterator it;
    for (it = fileList.begin(); it != fileList.end(); ++it) {
        kdDebug(9025) << "file: " << (*it) << endl;
        QDomElement fileEl = dom.createElement("file");
        fileEl.appendChild(dom.createTextNode(*it));
        filesEl.appendChild(fileEl);
    }

    QApplication::restoreOverrideCursor();
}


void CustomProjectWidget::closeProject()
{
    clear();
}


QStringList CustomProjectWidget::allSourceFiles()
{
    return m_sourceFiles;
}


QString CustomProjectWidget::projectDirectory()
{
    return m_projectDirectory;
}

#include "customprojectwidget.moc"
