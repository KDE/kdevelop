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
#include <qdir.h>
#include <qheader.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <kdeversion.h>

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

class FileComparator {
public:
	virtual ~FileComparator(){
	};
	virtual bool matches(const QString& name) const = 0;
};

class RegExpComparator : public FileComparator {
public:
	RegExpComparator(const QString& pattern) : m_exp(pattern, true, true){
	}
	bool matches(const QString& name) const{
		return m_exp.exactMatch(name);
	}
private:
	const QRegExp m_exp;
};

class EndingComparator : public FileComparator {
public:
	EndingComparator(const QString& pattern) : m_pattern ( pattern){
	}
	bool matches(const QString& name) const{
		return name.endsWith(m_pattern);
	}
private:
	const QString m_pattern;
};

class FileViewFolderItem : public QListViewItem
{
public:
    FileViewFolderItem(QListView *parent, const QString &name, const QString &pattern);
    bool matches(const QString &fileName);

private:
    QPtrList<FileComparator> m_patterns;
};


FileViewFolderItem::FileViewFolderItem(QListView *parent, const QString &name, const QString &pattern)
    : QListViewItem(parent, name)
{
    setPixmap(0, SmallIcon("folder"));
    m_patterns.setAutoDelete(true);
    QStringList patternstring = QStringList::split(';', pattern);
    QStringList::ConstIterator theend = patternstring.end();
    for (QStringList::ConstIterator ci = patternstring.begin(); ci != theend; ++ci)
	{
		QString pattern = *ci;
		QString tail = pattern.right( pattern.length() - 1 );

		if ( (tail).contains('*') || pattern.contains('?') || pattern.contains('[') || pattern.contains(']') )
		{
			m_patterns.append( new RegExpComparator( pattern ) );
		}
		else
		{
			if ( pattern.startsWith("*") )
			{
				m_patterns.append( new EndingComparator( tail ) );
			}
			else
			{
				m_patterns.append( new EndingComparator( pattern ) );
			}
		}
    }
}


bool FileViewFolderItem::matches(const QString &fileName)
{
    // Test with the file path, so that "*ClientServer/*.h" patterns work
    QString fName = QFileInfo(fileName).filePath();

    QPtrList<FileComparator>::ConstIterator theend = m_patterns.end();
    for (QPtrList<FileComparator>::ConstIterator ci = m_patterns.begin(); ci != theend; ++ci)
    	if ((*ci)->matches(fName))
		return true;

    return false;
}


class FileGroupsFileItem : public QListViewItem
{
public:
    FileGroupsFileItem(QListViewItem *parent, const QString &fileName);
    QString fileName() const
    { return fullname; }

private:
    QString fullname;
};


FileGroupsFileItem::FileGroupsFileItem(QListViewItem *parent, const QString &fileName)
    : QListViewItem(parent), fullname(fileName)
{
    setPixmap(0, SmallIcon("document"));
    QFileInfo fi(fileName);
    setText(0, fi.fileName());
    setText(1, "./" + fi.dirPath());
}

FileGroupsWidget::FileGroupsWidget(FileGroupsPart *part)
    : KListView(0, "file view widget"),
    m_actionToggleShowNonProjectFiles( 0 ), m_actionToggleDisplayLocation( 0 )
{
    /*
	Setting Location ID to -1 so I can check if it has been loaded later.
	If I dont, it will remove the name column and this is not too good :-)
	Is there any better way to do this?
    */
    LocationID=-1;

    setFocusPolicy(ClickFocus);
    setRootIsDecorated(true);
    setResizeMode(QListView::LastColumn);
    setSorting(-1);
    addColumn(i18n("Name"));
//    addColumn(i18n("Location"));

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    m_actionToggleShowNonProjectFiles = new KToggleAction( i18n("Show Non Project Files"), KShortcut(),
	this, SLOT(slotToggleShowNonProjectFiles()), this, "actiontoggleshowshownonprojectfiles" );
#if KDE_IS_VERSION(3,2,90)
    m_actionToggleShowNonProjectFiles->setCheckedState(i18n("Hide Non Project Files"));
#endif
    m_actionToggleShowNonProjectFiles->setWhatsThis(i18n("<b>Show non project files</b><p>Shows files that do not belong to a project in a file tree."));

    m_actionToggleDisplayLocation = new KToggleAction( i18n("Display Location Column"), KShortcut(),
        this, SLOT(slotToggleDisplayLocation()), this, "actiontoggleshowlocation" );
    m_actionToggleDisplayLocation->setWhatsThis(i18n("<b>Display the Location Column</b><p>Displays a column with the location of the files."));

    m_part = part;
    (void) translations; // supress compiler warning

    QDomDocument &dom = *m_part->projectDom();
    m_actionToggleShowNonProjectFiles->setChecked( !DomUtil::readBoolEntry(dom, "/kdevfileview/groups/hidenonprojectfiles") );
    m_actionToggleDisplayLocation->setChecked( !DomUtil::readBoolEntry(dom, "/kdevfileview/groups/hidenonlocation") );
}


FileGroupsWidget::~FileGroupsWidget()
{
    QDomDocument &dom = *m_part->projectDom();
    DomUtil::writeBoolEntry( dom, "/kdevfileview/groups/hidenonprojectfiles", !m_actionToggleShowNonProjectFiles->isChecked() );
    DomUtil::writeBoolEntry( dom, "/kdevfileview/groups/hidenonlocation", !m_actionToggleDisplayLocation->isChecked() );
}


void FileGroupsWidget::slotItemExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // toggle open state for parents
    if (item->childCount() > 0)
        setOpen(item, !isOpen(item));

    // Is it a group item?
    if (!item->parent())
        return;

    FileGroupsFileItem *fgfitem = static_cast<FileGroupsFileItem*>(item);
    m_part->partController()->editDocument(KURL::fromPathOrURL( m_part->project()->projectDirectory() + "/" + fgfitem->fileName() ));
}


void FileGroupsWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    KPopupMenu popup(i18n("File Groups"), this);
    /// @todo Add, remove groups
    int customizeId = popup.insertItem(i18n("Customize..."));
    popup.setWhatsThis(customizeId, i18n("<b>Customize</b><p>Opens <b>Customize File Groups</b> dialog where the groups can be managed."));
    if (item) {
    if (item->parent()) {
        // Not for group items
        FileGroupsFileItem *fvfitem = static_cast<FileGroupsFileItem*>(item);
        QString pathName = m_part->project()->projectDirectory() + QDir::separator() + fvfitem->fileName();
        KURL::List urls;
        urls.append(pathName);
        FileContext context(urls);
        m_part->core()->fillContextMenu(&popup, &context);
    }
    else{
        QStringList file_list;
        QListViewItem* i = item->firstChild();
        while(i){
            FileGroupsFileItem *fvgitem = static_cast<FileGroupsFileItem*>(i);
            file_list << fvgitem->fileName();
            i = i->nextSibling();
        }
        FileContext context(file_list);
        m_part->core()->fillContextMenu(&popup, &context);
    }
    }
    m_actionToggleShowNonProjectFiles->plug( &popup );
    m_actionToggleDisplayLocation->plug( &popup );

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

QStringList FileGroupsWidget::allFilesRecursively( QString const & dir )
{
	QStringList filelist;
	QString reldir = dir.mid( m_part->project()->projectDirectory().length() +1 );

	// recursively fetch all files in subdirectories
	QStringList subdirs = QDir( dir ).entryList( QDir::Dirs );
	QValueListIterator<QString> it = subdirs.begin();
	while ( it != subdirs.end() )
	{
		if ( *it != "." && *it != ".." )
		{
			filelist += allFilesRecursively( dir + "/"+ *it );
		}
		++it;
	}

	// append the project relative directory path to all files in the current directory
	QStringList dirlist = QDir( dir ).entryList( QDir::Files );
	QValueListIterator<QString> itt = dirlist.begin();
	while ( itt != dirlist.end() )
	{
		if ( reldir.isEmpty() )
		{
			filelist << *itt;
		}
		else
		{
			filelist << reldir + "/" + *itt;
		}
		++itt;
	}

	return filelist;
}

void FileGroupsWidget::refresh()
{
    while (firstChild())
        delete firstChild();

    if (m_actionToggleDisplayLocation->isChecked()) {
        // Display the Location column
	LocationID=addColumn(i18n("Location"));
    }
    else {
        // Remove the Location column
	//Need to check if the ID exists, if not do nothing!!
	if (LocationID!=-1)
	    removeColumn(LocationID);
    }
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

    QStringList allFiles;
    if (m_actionToggleShowNonProjectFiles->isChecked()) {
        // get all files in the project directory
        allFiles = allFilesRecursively( m_part->project()->projectDirectory() );
    }
    else {
        // get all project files
        allFiles = m_part->project()->allFiles();
    }
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
        if (fvgitem->matches(fileName))
        {
            QString f = fileName;
            if (fileName.contains(m_part->project()->projectDirectory()))
                f = fileName.mid(m_part->project()->projectDirectory().length()+1);
            (void) new FileGroupsFileItem(fvgitem, f);
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

void FileGroupsWidget::slotToggleShowNonProjectFiles()
{
    refresh();
}

void FileGroupsWidget::slotToggleDisplayLocation()
{
    refresh();
}


#include "filegroupswidget.moc"

