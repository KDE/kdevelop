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

#include "filegroupswidget.h"

#include <qfileinfo.h>
#include <qheader.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qregexp.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "domutil.h"

#include "filegroupspart.h"
#include "filegroupsconfigwidget.h"


// Translations for strings in the project file
static const char *translations[] = {
    I18N_NOOP("Sources"),
    I18N_NOOP("Translations"),
    I18N_NOOP("User Interface"),
    I18N_NOOP("Others")
};


class FileViewFolderItem : public QListViewItem
{
public:
    FileViewFolderItem(QListView *parent, const QString &name, const QString &pattern);
    bool matches(const QString &fileName);

private:
    QStringList patterns;
};


FileViewFolderItem::FileViewFolderItem(QListView *parent, const QString &name, const QString &pattern)
    : QListViewItem(parent, name)
{
    setPixmap(0, SmallIcon("folder"));
    patterns = QStringList::split(';', pattern);
}


bool FileViewFolderItem::matches(const QString &fileName)
{
    QString fName = QFileInfo(fileName).fileName();
    QStringList::ConstIterator it;
    for (it = patterns.begin(); it != patterns.end(); ++it) {
        // The regexp objects could be created already
        // in the constructor
        QRegExp re(*it, true, true);
        if (re.exactMatch(fName))
            return true;
    }

    return false;
}


class FileGroupsFileItem : public QListViewItem
{
public:
    FileGroupsFileItem(QListViewItem *parent, const QString &fileName);
    QString fileName() const
    { return fullname; }
    
private:
    static QString extractName(const QString &fileName);
    QString fullname;
};


FileGroupsFileItem::FileGroupsFileItem(QListViewItem *parent, const QString &fileName)
    : QListViewItem(parent, extractName(fileName)), fullname(fileName)
{
    setPixmap(0, SmallIcon("document"));
    setText(1, fileName);
}


QString FileGroupsFileItem::extractName(const QString &fileName)
{
    QFileInfo fi(fileName);
    return fi.fileName();
}


FileGroupsWidget::FileGroupsWidget(FileGroupsPart *part)
    : KListView(0, "file view widget")
{
    setFocusPolicy(ClickFocus);
    setRootIsDecorated(true);
    setResizeMode(QListView::LastColumn);
    setSorting(-1);
    addColumn(i18n("Name"));
    addColumn(i18n("Location"));

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    m_part = part;
    (void) translations; // supress compiler warning
}


FileGroupsWidget::~FileGroupsWidget()
{
}


void FileGroupsWidget::slotItemExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // Is it a group item?
    if (!item->parent())
        return;

    FileGroupsFileItem *fgfitem = static_cast<FileGroupsFileItem*>(item);
    m_part->partController()->editDocument(QString("file://") + m_part->project()->projectDirectory() + "/" + fgfitem->fileName());
    m_part->mainWindow()->lowerView(this);
}


void FileGroupsWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;
    KPopupMenu popup(i18n("File Groups"), this);
    // TODO: Add, remove groups
    int customizeId = popup.insertItem(i18n("Customize..."));
    popup.insertSeparator();
    if (item->parent()) {
        // Not for group items
        FileGroupsFileItem *fvfitem = static_cast<FileGroupsFileItem*>(item);
        FileContext context(fvfitem->fileName(), false);
        m_part->core()->fillContextMenu(&popup, &context);
    }
    
    int res = popup.exec(p);
    if (res == customizeId) {
        KDialogBase dlg(KDialogBase::TreeList, i18n("Customize File Groups"),
                        KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this,
                        "customization dialog");
        QVBox *vbox = dlg.addVBoxPage(i18n("File Groups"));
        FileGroupsConfigWidget *w = new FileGroupsConfigWidget(m_part, vbox, "file groups config widget");
        connect(&dlg, SIGNAL(okClicked()), w, SLOT(accept()));
        dlg.exec();
    }
}


void FileGroupsWidget::refresh()
{
    while (firstChild())
        delete firstChild();

    QDomDocument &dom = *m_part->projectDom();
    DomUtil::PairList list =
        DomUtil::readPairListEntry(dom, "/kdevfileview/groups", "group", "name", "pattern");

    FileViewFolderItem *lastGroup = 0;
    
    DomUtil::PairList::ConstIterator git;
    for (git = list.begin(); git != list.end(); ++git) {
        FileViewFolderItem *newItem = new FileViewFolderItem(this, (*git).first, (*git).second);
        if (lastGroup)
            newItem->moveItem(lastGroup);
        lastGroup = newItem;
    }

    QStringList allFiles = m_part->project()->allFiles();
    QStringList::ConstIterator fit;
    for (fit = allFiles.begin(); fit != allFiles.end(); ++fit) {
        QListViewItem *item = firstChild();
        while (item) {
            FileViewFolderItem *fvgitem = static_cast<FileViewFolderItem*>(item);
            if (fvgitem->matches(*fit)) {
                (void) new FileGroupsFileItem(fvgitem, *fit);
                break;
            }
            item = item->nextSibling();
        }
    }

    QListViewItem *item = firstChild();
    while (item) {
        item->sortChildItems(0, true);
        item = item->nextSibling();
    }
}


void FileGroupsWidget::addFile(const QString &fileName)
{
    kdDebug(9017) << "FileView add " << fileName << endl;
    
    QListViewItem *item = firstChild();
    while (item) {
        FileViewFolderItem *fvgitem = static_cast<FileViewFolderItem*>(item);
        if (fvgitem->matches(fileName)) {
            (void) new FileGroupsFileItem(fvgitem, fileName);
            fvgitem->sortChildItems(0, true);
            break;
        }
        item = item->nextSibling();
    }
}

void FileGroupsWidget::addFiles ( const QStringList& fileList )
{
	QStringList::ConstIterator it;
	
	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		this->addFile ( *it );
	}
}

void FileGroupsWidget::removeFile(const QString &fileName)
{
	kdDebug(9017) << "FileView remove " << fileName << endl;
	
	QListViewItem *item = firstChild();
	while (item)
	{
		FileViewFolderItem *fvgitem = static_cast<FileViewFolderItem*>(item);
		QListViewItem *childItem = fvgitem->firstChild();
		while (childItem) 
		{
			FileGroupsFileItem *fgfitem = static_cast<FileGroupsFileItem*>(childItem);
			kdDebug ( 9017 ) << "fvfitem->fileName() is " << fgfitem->fileName() << endl;
			if (fgfitem->fileName() == fileName ) 
			{
				kdDebug ( 9017 ) << "Deleting: " << fgfitem->fileName() << endl;
				
				delete fgfitem;
				return;
			}
			childItem = childItem->nextSibling();
		}
		item = item->nextSibling();
	}
}

void FileGroupsWidget::removeFiles ( const QStringList& fileList )
{
	QStringList::ConstIterator it;
	
	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		removeFile ( *it );
	}
}

#include "filegroupswidget.moc"
