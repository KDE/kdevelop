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
    void setIsProjectFile(bool b)
    { m_isProjectFile = b; }
    bool isProjectFile() const
    { return m_isProjectFile; }

    virtual void setOpen(bool o);
    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int alignment);
                           
    void hideOrShow();

private:
    FileTreeWidget* listView()
    { return static_cast<FileTreeWidget*>(QListViewItem::listView()); }

    void init();
    Type typ;
    bool m_isProjectFile;
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
    if (typ == File)
        setPixmap(0, SmallIcon("document"));
    else {
        setExpandable(true);
        setPixmap(0, SmallIcon("folder"));
        kdDebug(9017) << "Watch dir " << path() << endl;
        listView()->watchDir(path());
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
            
            // TODO: Update when files added or removed.
            QStringList projectFiles = listView()->m_part->project()->allFiles();
            for (; it.current(); ++it) {
                QFileInfo *fi = it.current();
                FileTreeItem* item;
                if (fi->fileName() == "." || fi->fileName() == ".." )
                    continue;
                if (fi->isDir())
                    item = new FileTreeItem(this, Dir, fi->fileName());
                else
                    item = new FileTreeItem(this, File, fi->fileName());
                item->setIsProjectFile( projectFiles.contains( item->path() ) );
                item->hideOrShow();
            }
        }
    }
    QListViewItem::setOpen(o);
}

void FileTreeItem::hideOrShow()
{
    bool projectFile = listView()->m_showNonProjectFiles || isProjectFile();
    bool matchesHidePattern = listView()->matchesHidePattern( text(0) );
    
    setVisible( (type() == Dir || projectFile) && !matchesHidePattern );
    
    FileTreeItem* item = static_cast<FileTreeItem*>(firstChild());
    while( item ) {
        item->hideOrShow();
        item = static_cast<FileTreeItem*>(item->nextSibling());
    }
}

void FileTreeItem::paintCell(QPainter *p, const QColorGroup &cg,
                             int column, int width, int alignment)
{
    if ( isProjectFile() && listView()->m_showNonProjectFiles ) {
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
    
 // Comment out until config UI is implemented
    QString patterns; // = DomUtil::readEntry(dom, "/kdevfileview/tree/hidepatterns");
//    if (patterns.isEmpty())
        patterns = "*.o,*.lo,CVS";
    m_hidePatterns = QStringList::split(",", patterns);
}


FileTreeWidget::~FileTreeWidget()
{
    QDomDocument &dom = *m_part->projectDom();
    DomUtil::writeBoolEntry( dom, "/kdevfileview/tree/hidenonprojectfiles", !m_showNonProjectFiles );
    DomUtil::writeEntry( dom, "/kdevfileview/tree/hidepatterns", m_hidePatterns.join(",") );
}


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
    FileTreeItem* item = static_cast<FileTreeItem*>(firstChild());
    if( !item )
      return;
    
    // Need to skip the root item.
    item = static_cast<FileTreeItem*>(item->firstChild());
    while( item ) {
        item->hideOrShow();
        item = static_cast<FileTreeItem*>(item->nextSibling());
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
            bool wasOpen = ftitem->isOpen();
            while (ftitem->firstChild())
                delete ftitem->firstChild();
            ftitem->setOpen( wasOpen );
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


void FileTreeWidget::slotContextMenu(KListView* listView, QListViewItem* item, const QPoint &p)
{
    KPopupMenu popup(i18n("File Tree"), this);
    
    int id = popup.insertItem( i18n("Show Non-Project Files"),
                               this, SLOT(slotToggleShowNonProjectFiles()) );
    popup.setItemChecked(id, m_showNonProjectFiles);
    
    if( item != 0 ) {
      FileTreeItem *ftitem = static_cast<FileTreeItem*>(item);
      FileContext context(ftitem->path(), ftitem->type() == FileTreeItem::Dir);
      m_part->core()->fillContextMenu(&popup, &context);
    }
    
    popup.exec(p);
}


void FileTreeWidget::slotToggleShowNonProjectFiles()
{
    m_showNonProjectFiles = !m_showNonProjectFiles;
    hideOrShow();
}

#include "filetreewidget.moc"
