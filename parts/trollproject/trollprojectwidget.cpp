/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2000-2001 by Trolltech AS.                              *
 *   info@trolltech.com                                                    *
 *                                                                         *
 *   Part of this file is taken from Qt Designer.                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "trollprojectwidget.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qsplitter.h>
#include <qptrstack.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <kdebug.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kregexp.h>
#include <kurl.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "domutil.h"
#include "trollprojectpart.h"


/**
 * Class ProjectViewItem
 */

ProjectItem::ProjectItem(Type type, QListView *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{}


ProjectItem::ProjectItem(Type type, ProjectItem *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{}


/**
 * Class SubprojectItem
 */

SubprojectItem::SubprojectItem(QListView *parent, const QString &text)
    : ProjectItem(Subproject, parent, text)
{
    init();
}


SubprojectItem::SubprojectItem(SubprojectItem *parent, const QString &text)
    : ProjectItem(Subproject, parent, text)
{
    init();
}


void SubprojectItem::init()
{
    groups.setAutoDelete(true);
    setPixmap(0, SmallIcon("folder"));
}


/**
 * Class GroupItem
 */

GroupItem::GroupItem(QListView *lv, GroupType groupType, const QString &text)
    : ProjectItem(Group, lv, text)
{
    files.setAutoDelete(true);
    setPixmap(0, SmallIcon("tar"));
}


/**
 * Class FileItem
 */

FileItem::FileItem(QListView *lv, const QString &text)
    : ProjectItem(File, lv, text)
{
    setPixmap(0, SmallIcon("document"));
}


TrollProjectWidget::TrollProjectWidget(TrollProjectPart *part)
    : QVBox(0, "troll project widget")
{
    QSplitter *splitter = new QSplitter(Vertical, this);

    overview = new KListView(splitter, "project overview widget");
    overview->setFrameStyle(Panel | Sunken);
    overview->setSorting(-1);
    overview->setLineWidth(2); 
    overview->header()->hide();
    overview->addColumn("");
    details = new KListView(splitter, "project details widget");
    details->setRootIsDecorated(true);
    details->setFrameStyle(Panel | Sunken);
    details->setSorting(-1);
    details->setLineWidth(2); 
    details->header()->hide();
    details->addColumn("");

    connect( overview, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( overview, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );
    connect( details, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( details, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    m_part = part;
    activeSubproject = 0;
}


TrollProjectWidget::~TrollProjectWidget()
{}


void TrollProjectWidget::openProject(const QString &dirName)
{
    SubprojectItem *item = new SubprojectItem(overview, "/");
    item->subdir = "/";
    item->path = dirName;
    parse(item);
    item->setOpen(true);
    
    slotItemExecuted(item);
}


void TrollProjectWidget::closeProject()
{
    overview->clear();
    details->clear();
}


QStringList TrollProjectWidget::allSubprojects()
{
    int prefixlen = projectDirectory().length()+1;
    QStringList res;
    
    QListViewItemIterator it(overview);
    for (; it.current(); ++it) {
        if (it.current() == overview->firstChild())
            continue;
        QString path = static_cast<SubprojectItem*>(it.current())->path;
        res.append(path.mid(prefixlen));
    }
    
    return res;
}


QStringList TrollProjectWidget::allSourceFiles()
{
    QStack<QListViewItem> s;
    QStringList res;
    
    for ( QListViewItem *item = overview->firstChild(); item;
          item = item->nextSibling()? item->nextSibling() : s.pop() ) {
        if (item->firstChild())
            s.push(item->firstChild());
        
        SubprojectItem *spitem = static_cast<SubprojectItem*>(item);
        QString path = spitem->path;
        QListIterator<GroupItem> tit(spitem->groups);
        for (; tit.current(); ++tit) {
            GroupItem::GroupType type = (*tit)->groupTyp;
            if (type == GroupItem::Sources || type == GroupItem::Headers) {
                QListIterator<FileItem> fit(tit.current()->files);
                for (; fit.current(); ++fit) {
                    QString fullname = path + "/" + (*fit)->name;
                    res += fullname;
                }
            }
        }
    }
    
    return res;
}


QString TrollProjectWidget::projectDirectory()
{
    if (!overview->firstChild())
        return QString::null; //confused

    return static_cast<SubprojectItem*>(overview->firstChild())->path;
}


QString TrollProjectWidget::subprojectDirectory()
{
    if (!activeSubproject)
        return QString::null;

    return activeSubproject->path;
}


void TrollProjectWidget::slotItemExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // We assume here that ALL items in both list views
    // are ProjectItem's
    ProjectItem *pvitem = static_cast<ProjectItem*>(item);

    if (pvitem->type() == ProjectItem::Subproject) {
        if (activeSubproject) {
            // Remove all GroupItems and all of their children from the view
            QListIterator<GroupItem> it1(activeSubproject->groups);
            for (; it1.current(); ++it1) {
                // After AddTargetDialog, it can happen that an
                // item is not yet in the list view, so better check...
                if (it1.current()->parent())
                    while ((*it1)->firstChild())
                        (*it1)->takeItem((*it1)->firstChild());
                details->takeItem(*it1);
            }
        }
            
        activeSubproject = static_cast<SubprojectItem*>(item);

        // Insert all GroupItems and all of their children into the view
        QListIterator<GroupItem> it2(activeSubproject->groups);
        for (; it2.current(); ++it2) {
            details->insertItem(*it2);
            QListIterator<FileItem> it3((*it2)->files);
            for (; it3.current(); ++it3)
                (*it2)->insertItem(*it3);
            (*it2)->setOpen(true);
        }
    } else if (pvitem->type() == ProjectItem::File) {
        QString dirName = activeSubproject->path;
        FileItem *fitem = static_cast<FileItem*>(pvitem);
        m_part->partController()->editDocument(KURL(dirName + "/" + QString(fitem->name)));
    }
}


void TrollProjectWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;
    
    ProjectItem *pvitem = static_cast<ProjectItem*>(item);

    if (pvitem->type() == ProjectItem::Subproject) {
        SubprojectItem *spitem = static_cast<SubprojectItem*>(pvitem);
        KPopupMenu pop(i18n("Subproject"));
        int idOptions = pop.insertItem(i18n("Options..."));
        int idAddSubproject = pop.insertItem(i18n("Add Subproject..."));
        int idBuild = pop.insertItem(i18n("Build"));
        int r = pop.exec(p);
        if (r == idOptions) {
            ;
        }
        else if (r == idAddSubproject) {
            ;
        }
        else if (r == idBuild) {
            QString relpath = spitem->path.mid(projectDirectory().length());
            m_part->startMakeCommand(projectDirectory() + relpath, QString::fromLatin1(""));
        }
    } else if (pvitem->type() == ProjectItem::Group) {
        GroupItem *titem = static_cast<GroupItem*>(pvitem);
        KPopupMenu pop;
        int idAddFile = pop.insertItem(i18n("Add File..."));
        int r = pop.exec(p);
        if (r == idAddFile) {
            ;
                //   slotItemExecuted(activeSubproject); // update list view
        }
    } else if (pvitem->type() == ProjectItem::File) {
        FileItem *fitem = static_cast<FileItem*>(pvitem);
        GroupItem *titem = static_cast<GroupItem*>(fitem->parent());
        KPopupMenu pop;
        int idRemoveFile = pop.insertItem(i18n("Remove File..."));
        int r = pop.exec(p);
        if (r == idRemoveFile) {
            ;
        }
    }
}


GroupItem *TrollProjectWidget::createGroupItem(GroupItem::GroupType groupType, const QString &name)
{
    // Workaround because for QListView not being able to create
    // items without actually inserting them
    GroupItem *titem = new GroupItem(overview, groupType, name);
    overview->takeItem(titem);

    return titem;
}


FileItem *TrollProjectWidget::createFileItem(const QString &name)
{
    FileItem *fitem = new FileItem(overview, name);
    overview->takeItem(fitem);
    fitem->name = name;

    return fitem;
}


void TrollProjectWidget::emitAddedFile(const QString &name)
{
    emit m_part->addedFileToProject(name);
}


void TrollProjectWidget::emitRemovedFile(const QString &name)
{
    emit m_part->removedFileFromProject(name);
}


// from Designer
static QString parse_part( const QString &part )
{
    QString res;
    bool inName = FALSE;
    QString currName;
    for ( int i = 0; i < (int)part.length(); ++i ) {
	QChar c = part[ i ];
	if ( !inName ) {
	    if ( c != ' ' && c != '\t' && c != '\n' && c != '=' && c != '\\' )
		inName = TRUE;
	    else
		continue;
	}
	if ( inName ) {
	    if ( c == '\n' )
		break;
	    res += c;
	}
    }
    return res;
}


void TrollProjectWidget::parse(SubprojectItem *item)
{
    QFileInfo fi(item->path);
    QString proname = item->path + "/" + fi.baseName() + ".pro";
    kdDebug(9024) << "Parsing " << proname << endl;

    QFile f(proname);
    if (!f.open(IO_ReadOnly))
        return;

    QTextStream ts(&f);
    QString contents = ts.read();
    f.close();

    // from Designer
    int i = contents.find( "INTERFACES" );
    if ( i != -1 ) {
	QString part = contents.mid( i + QString( "INTERFACES" ).length() );
	QStringList lst;
	bool inName = FALSE;
	QString currName;
	for ( i = 0; i < (int)part.length(); ++i ) {
	    QChar c = part[ i ];
	    if ( ( c.isLetter() || c.isDigit() || c == '.' || c == '/' || c == '_' ) &&
		 c != ' ' && c != '\t' && c != '\n' && c != '=' && c != '\\' ) {
		if ( !inName )
		    currName = QString::null;
		currName += c;
		inName = TRUE;
	    } else {
		if ( inName ) {
		    inName = FALSE;
		    if ( currName.right( 3 ).lower() == ".ui" )
			lst.append( currName );
		}
	    }
	}

	item->interfaces = lst;
    }

    i = contents.find( "SOURCES" );
    if ( i != -1 ) {
	QString part = contents.mid( i + QString( "SOURCES" ).length() );
	QString s = parse_part( part );
	item->sources = QStringList::split( ' ', s );
    }

    i = contents.find( "HEADERS" );
    if ( i != -1 ) {
	QString part = contents.mid( i + QString( "HEADERS" ).length() );
	QString s = parse_part( part );
	item->headers = QStringList::split( ' ', s );
    }

    // Create list view items
    if (!item->interfaces.isEmpty()) {
        GroupItem *titem = createGroupItem(GroupItem::Interfaces, "Interfaces");
        item->groups.append(titem);
        QStringList l = item->interfaces;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            titem->files.append(fitem);
        }
    }
    if (!item->sources.isEmpty()) {
        GroupItem *titem = createGroupItem(GroupItem::Sources, "Sources");
        item->groups.append(titem);
        QStringList l = item->sources;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            titem->files.append(fitem);
        }
    }
    if (!item->headers.isEmpty()) {
        GroupItem *titem = createGroupItem(GroupItem::Headers, "Headers");
        item->groups.append(titem);
        QStringList l = item->headers;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            titem->files.append(fitem);
        }
    }

    i = contents.find( "SUBDIRS" );
    if ( i != -1 ) {
	QString part = contents.mid( i + QString( "SUBDIRS" ).length() );
	QString s = parse_part( part );
	QStringList lst = QStringList::split( ' ', s );
        QStringList::Iterator it;
        for (it = lst.begin(); it != lst.end(); ++it) {
            SubprojectItem *newitem = new SubprojectItem(item, (*it));
            newitem->subdir = *it;
            newitem->path = item->path + "/" + (*it);
            parse(newitem);
        }
    }

}

#include "trollprojectwidget.moc"
