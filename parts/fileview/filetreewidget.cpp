/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filetreewidget.h"

#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qvaluestack.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "kdevtoplevel.h"
#include "domutil.h"
#include "fileviewpart.h"


class FileTreeItem : public QListViewItem
{
public:
    enum Type { File, Dir };
    
    FileTreeItem(FileTreeWidget *parent, Type type, const QString &nam);
    FileTreeItem(FileTreeItem *parent, Type type, const QString &nam);
    ~FileTreeItem();
    
    QString path();
    Type type() const
    { return typ; }
    void setBold(bool b)
    { bld = b; }
    bool isBold() const
    { return bld; }

    virtual void setOpen(bool o);
    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int alignment);

private:
    void init();
    Type typ;
    bool bld;
};


FileTreeItem::FileTreeItem(FileTreeWidget *parent, Type type, const QString &name)
    : QListViewItem(parent, name), typ(type)
{
    init();
}


FileTreeItem::FileTreeItem(FileTreeItem *parent, Type type, const QString &name)
    : QListViewItem(parent, name), typ(type)
{
    init();
}


void FileTreeItem::init()
{
    bld = false;
    
    if (typ == File)
        setPixmap(0, SmallIcon("document"));
    else {
        setExpandable(true);
        setPixmap(0, SmallIcon("folder"));
        kdDebug(9017) << "Watch dir " << path() << endl;
        static_cast<FileTreeWidget*>(listView())->watchDir(path());
    }
}


FileTreeItem::~FileTreeItem()
{}


QString FileTreeItem::path()
{
    QString name = text(0);
    if (parent()) {
        name.prepend("/");
        name.prepend(static_cast<FileTreeItem*>(parent())->path());
    }

    return name;
}


void FileTreeItem::setOpen(bool o)
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
                    ( new FileTreeItem(this, Dir, fi->fileName()) )->setOpen(true);
                else
                    (void) new FileTreeItem(this, File, fi->fileName());
            }
        }
    }
    QListViewItem::setOpen(o);
}


void FileTreeItem::paintCell(QPainter *p, const QColorGroup &cg,
                             int column, int width, int alignment)
{
    if (isBold()) {
        QFont font(p->font());
        font.setBold(true);
        p->setFont(font);
    }
    QListViewItem::paintCell(p, cg, column, width, alignment);
}


FileTreeWidget::FileTreeWidget(FileViewPart *part, QWidget *parent, const char *name)
    : KListView(parent, name)
{
    setResizeMode(QListView::LastColumn);
    setSorting(0);
    header()->hide();
    addColumn(QString::null);

    m_part = part;
    
    m_dirWatch = new KDirWatch(this);
    connect( m_dirWatch, SIGNAL(dirty(const QString&)),
             this, SLOT(slotDirectoryDirty(const QString &)) );

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    QDomDocument &dom = *m_part->projectDom();
    m_showNonProjectFiles = !DomUtil::readBoolEntry(dom, "/kdevfileview/tree/hidenonprojectfiles");
    QString patterns = DomUtil::readEntry(dom, "/kdevfileview/tree/hidepatterns");
    if (patterns.isEmpty())
        patterns = "*o,*.lo";
    m_hidePatterns = QStringList::split(",", patterns);
}


FileTreeWidget::~FileTreeWidget()
{}


void FileTreeWidget::watchDir(const QString &dirName)
{
    m_dirWatch->addDir(dirName);
}


bool FileTreeWidget::matchesHidePattern(const QString &fileName)
{
    QStringList::ConstIterator it;
    for (it = m_hidePatterns.begin(); it != m_hidePatterns.end(); ++it) {
        QRegExp re(*it, true, true);
        if (re.search(fileName) == 0 && (uint)re.matchedLength() == fileName.length())
            return true;
    }

    return false;
}


void FileTreeWidget::hideOrShow()
{
    QStringList projectFiles = m_part->project()->allFiles();
    
    QListViewItemIterator it(this);
    for (; it.current(); ++it) {
        FileTreeItem *ftitem = static_cast<FileTreeItem*>(it.current());

        // Show all directory items
        if (ftitem->type() == FileTreeItem::Dir)
            continue;

        bool isProjectFile = projectFiles.contains(ftitem->path());
        bool b1 = m_showNonProjectFiles || isProjectFile;
        bool b2 = !matchesHidePattern(it.current()->text(0));
        ftitem->setVisible(b1 && b2);
        ftitem->setBold(m_showNonProjectFiles && isProjectFile);
    }
}


void FileTreeWidget::openDirectory(const QString &dirName)
{
    ( new FileTreeItem(this, FileTreeItem::Dir, dirName) )->setOpen(true);
    hideOrShow();
}


void FileTreeWidget::slotDirectoryDirty(const QString &dirName)
{
    kdDebug(9017) << "Directory dirty " << dirName << endl;

    QListViewItemIterator it(this);
    for (; it.current(); ++it) {
        FileTreeItem *ftitem = static_cast<FileTreeItem*>(it.current());
        if (ftitem->path() == dirName) {
            while (ftitem->firstChild())
                delete ftitem->firstChild();
            ftitem->setOpen(true);
        }
    }

    hideOrShow();
}


void FileTreeWidget::slotItemExecuted(QListViewItem *item)
{
    if (!item)
        return;
    FileTreeItem *ftitem = static_cast<FileTreeItem*>(item);

    // Is it a group item?
    if (ftitem->type() != FileTreeItem::File)
        return;

    m_part->partController()->editDocument(QString("file://") + ftitem->path());
    m_part->topLevel()->lowerView(this);
}


void FileTreeWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;

    // Root item
    if (item == firstChild()) {
        KPopupMenu popup(i18n("File tree"), this);
        int id = popup.insertItem( i18n("Show non-project files"),
                                   this, SLOT(slotToggleShowNonProjectFiles()) );
        popup.setItemChecked(id, m_showNonProjectFiles);
        popup.exec(p);
        return;
    }
        
    FileTreeItem *ftitem = static_cast<FileTreeItem*>(item);
    KPopupMenu popup(i18n("File Tree"), this);
    FileContext context(ftitem->path(), ftitem->type() == FileTreeItem::Dir);
    m_part->core()->fillContextMenu(&popup, &context);
    popup.exec(p);
}


void FileTreeWidget::slotToggleShowNonProjectFiles()
{
    m_showNonProjectFiles = !m_showNonProjectFiles;
    hideOrShow();
}

#include "filetreewidget.moc"
