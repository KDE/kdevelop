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

#include "autoprojectwidget.h"

#include <qfile.h>
#include <qheader.h>
#include <qsplitter.h>
#include <qpainter.h>
#include <qptrstack.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qregexp.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kregexp.h>
#include <kurl.h>
#include <kfile.h>
#include <kaction.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"
#include "domutil.h"
#include "misc.h"
#include "subprojectoptionsdlg.h"
#include "targetoptionsdlg.h"
#include "addsubprojectdlg.h"
#include "addtargetdlg.h"
#include "addservicedlg.h"
#include "addapplicationdlg.h"
#include "addfiledlg.h"
#include "importexistingdlg.h"
#include "removefiledlg.h"
#include "autoprojectpart.h"


static QString nicePrimary(QCString primary)
{
    if (primary == "PROGRAMS")
        return i18n("Program");
    else if (primary == "LIBRARIES")
        return i18n("Library");
    else if (primary == "LTLIBRARIES")
        return i18n("Libtool Library");
    else if (primary == "SCRIPTS")
        return i18n("Script");
    else if (primary == "HEADERS")
        return i18n("Header");
    else if (primary == "DATA")
        return i18n("Data");
    else if (primary == "JAVA")
        return i18n("Java");
    else
        return QString::null;
}


static QCString cleanWhitespace(QCString str)
{
    QCString res;
    
    QStringList l = QStringList::split(QRegExp("[ \t]"), QString(str));
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        res += *it;
        res += " ";
    }

    return res.left(res.length()-1);
}


/**
 * Class ProjectItem
 */

ProjectItem::ProjectItem(Type type, QListView *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{
    bld = false;
}


ProjectItem::ProjectItem(Type type, ProjectItem *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{
    bld = false;
}


void ProjectItem::paintCell(QPainter *p, const QColorGroup &cg,
                            int column, int width, int alignment)
{
    if (isBold()) {
        QFont font(p->font());
        font.setBold(true);
        p->setFont(font);
    }
    QListViewItem::paintCell(p, cg, column, width, alignment);
}


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
    targets.setAutoDelete(true);
    setPixmap(0, SmallIcon("folder"));
}


/**
 * Class TargetItem
 */

TargetItem::TargetItem(QListView *lv, bool group, const QString &text)
    : ProjectItem(Target, lv, text)
{
    sources.setAutoDelete(true);
    setPixmap(0, group? SmallIcon("tar") : SmallIcon("binary"));
}


/**
 * Class FileItem
 */

FileItem::FileItem(QListView *lv, const QString &text)
    : ProjectItem(File, lv, text)
{
    setPixmap(0, SmallIcon("document"));
}


AutoProjectWidget::AutoProjectWidget(AutoProjectPart *part, bool kde)
    : QVBox(0, "auto project widget")
{
    widgetLayout = new QVBoxLayout ( this );

    QSplitter *splitter = new QSplitter(Vertical, this);

    overviewBox = new QVBox ( splitter, "vertical overview box" );

    overviewButtonBox = new QHBox ( overviewBox, "subproject button box" );
    overviewButtonBox->setMargin ( 2 );
    overviewButtonBox->setSpacing ( 2 );

    subProjectOptionsButton = new QToolButton ( overviewButtonBox );
	subProjectOptionsButton->setPixmap ( SmallIcon ( "configure" ) );
	QToolTip::add ( subProjectOptionsButton, i18n ( "Show options of currently selected subproject..." ) );

    addSubprojectButton = new QToolButton ( overviewButtonBox );
    addSubprojectButton->setPixmap ( SmallIcon ( "folder_new" ) );
    QToolTip::add ( addSubprojectButton, i18n ( "Add new subproject to currently selected subproject..." ) );

    addExistingSubprojectButton = new QToolButton ( overviewButtonBox );
    addExistingSubprojectButton->setPixmap ( SmallIcon ( "fileimport" ) );
    QToolTip::add ( addExistingSubprojectButton, i18n ( "Add existing subproject to currently selected subproject..." ) );

    addTargetButton = new QToolButton ( overviewButtonBox );
    addTargetButton->setPixmap ( SmallIcon ( "targetnew_kdevelop" ) );
    QToolTip::add ( addTargetButton, i18n ( "Add new target to currently selected subproject..." ) );

    addServiceButton = new QToolButton ( overviewButtonBox );
    addServiceButton->setPixmap ( SmallIcon ( "servicenew_kdevelop" ) );
    QToolTip::add ( addServiceButton, i18n ( "Add new service to currently selected subproject..." ) );

    addApplicationButton = new QToolButton ( overviewButtonBox );
    addApplicationButton->setPixmap ( SmallIcon ( "window_new" ) );
    QToolTip::add ( addApplicationButton, i18n ( "Add new application to currently selected subproject..." ) );

    buildSubprojectButton = new QToolButton ( overviewButtonBox );
    buildSubprojectButton->setPixmap ( SmallIcon ( "launch" ) );
    QToolTip::add ( buildSubprojectButton, i18n ( "Build currently selected subproject..." ) );

    overviewButtonBox->setMaximumHeight ( subProjectOptionsButton->height() );

/*    subProjectOptionsButton->setEnabled ( true );
    addSubprojectButton->setEnabled ( true );
    addTargetButton->setEnabled ( true );
    addServiceButton->setEnabled ( true );
    addApplicationButton->setEnabled ( true );
    buildSubprojectButton->setEnabled ( true );
*/
	// HACK
	QWidget* spacer1 = new QWidget ( overviewButtonBox );
	overviewButtonBox->setStretchFactor ( spacer1, 1 );

    overview = new KListView ( overviewBox, "project overview widget");
    overview->setResizeMode(QListView::LastColumn);
    overview->setSorting ( -1 );
    overview->header()->hide();
    overview->addColumn ( QString::null );

    widgetLayout->addWidget ( overviewBox );

    targetBox = new QVBox ( splitter, "vertical target box" );

    targetButtonBox = new QHBox ( targetBox, "target button box" );
    targetButtonBox->setMargin ( 2 );
    targetButtonBox->setSpacing ( 2 );

    targetOptionsButton = new QToolButton ( targetButtonBox );
    targetOptionsButton->setPixmap ( SmallIcon ( "configure" ) );
    QToolTip::add ( targetOptionsButton, i18n ( "Show options of currently target..." ) );

    addNewFileButton = new QToolButton ( targetButtonBox );
    addNewFileButton->setPixmap ( SmallIcon ( "filenew" ) );
    QToolTip::add ( addNewFileButton, i18n ( "Create new file and add it to currently selected target..." ) );

    addExistingFileButton = new QToolButton ( targetButtonBox );
    addExistingFileButton->setPixmap ( SmallIcon ( "fileimport" ) );
    QToolTip::add ( addExistingFileButton, i18n ( "Add existing files to the currently selected target..." ) );

    removeFileButton = new QToolButton ( targetButtonBox );
    removeFileButton->setPixmap ( SmallIcon ( "editdelete" ) );
    QToolTip::add ( removeFileButton, i18n ( "Remove currently selected file" ) );

    buildTargetButton = new QToolButton ( targetButtonBox );
    buildTargetButton->setPixmap ( SmallIcon ( "launch" ) );
    QToolTip::add ( buildTargetButton, i18n ( "Build currently selected target..." ) );

    targetButtonBox->setMaximumHeight ( addNewFileButton->height() );

    targetOptionsButton->setEnabled ( false );
    addNewFileButton->setEnabled ( false );
    addExistingFileButton->setEnabled ( false );
    removeFileButton->setEnabled ( false );
    buildTargetButton->setEnabled ( false );

	// HACK
	QWidget* spacer2 = new QWidget ( targetButtonBox );
	targetButtonBox->setStretchFactor ( spacer2, 1 );

    //widgetLayout->addWidget ( splitter );

    details = new KListView(targetBox, "project details widget");
    details->setRootIsDecorated(true);
    details->setResizeMode(QListView::LastColumn);
    details->setSorting(-1);
    details->header()->hide();

    widgetLayout->addWidget ( targetBox );

    KActionCollection* actions = new KActionCollection ( this );

    subProjectOptionsAction = new KAction ( i18n ( "Options..." ), "configure", 0, this, SLOT ( slotSubprojectOptions() ), actions, "subproject options" );
    addSubprojectAction = new KAction ( i18n ( "Add Subproject..." ), "folder_new", 0, this, SLOT ( slotAddSubproject() ), actions, "add subproject" );
    addExistingSubprojectAction = new KAction ( i18n ( "Add existing subproject..." ), "fileimport", 0, this, SLOT ( slotAddExistingSubproject() ), actions, "add existing subproject" );
    addTargetAction = new KAction ( i18n ( "Add Target..." ), "targetnew_kdevelop", 0, this, SLOT ( slotAddTarget() ), actions, "add target" );
    addServiceAction = new KAction ( i18n ( "Add Service..." ), "servicenew_kdevelop", 0, this, SLOT ( slotAddService() ), actions, "add service" );
    addApplicationAction = new KAction ( i18n ( "Add Application..." ), "window_new", 0, this, SLOT ( slotAddApplication() ), actions, "add application" );
    buildSubprojectAction = new KAction ( i18n ( "Build" ), "launch", 0, this, SLOT ( slotBuildSubproject() ), actions, "add build subproject" );

    targetOptionsAction = new KAction ( i18n ( "Options..." ), "configure", 0, this, SLOT ( slotTargetOptions() ), actions, "target options" );
    addNewFileAction = new KAction ( i18n ( "Create new file..." ), "filenew", 0, this, SLOT ( slotAddNewFile() ), actions, "add new file" );
    addExistingFileAction = new KAction ( i18n ( "Add existing file(s)..." ), "fileimport", 0, this, SLOT ( slotAddExistingFile() ), actions, "add existing file" );
    buildTargetAction = new KAction ( i18n ( "Build target..." ), "launch", 0, this, SLOT ( slotBuildTarget() ), actions, "build target" );
    setActiveTargetAction = new KAction ( i18n ( "Make target active..." ), "", 0, this, SLOT ( slotSetActiveTarget() ), actions, "set active target" );

    removeFileAction = new KAction ( i18n ( "Remove file..." ), "editdelete", 0, this, SLOT ( slotRemoveFile() ), actions, "remove file" );

    connect ( subProjectOptionsButton, SIGNAL ( clicked() ), this, SLOT ( slotSubprojectOptions() ) );
    connect ( addSubprojectButton, SIGNAL ( clicked() ), this, SLOT ( slotAddSubproject() ) );
    connect ( addExistingSubprojectButton, SIGNAL ( clicked() ), this, SLOT ( slotAddExistingSubproject() ) );
    connect ( addTargetButton, SIGNAL ( clicked() ), this, SLOT ( slotAddTarget() ) );
    connect ( addServiceButton, SIGNAL ( clicked() ), this, SLOT ( slotAddService() ) );
    connect ( addApplicationButton, SIGNAL ( clicked() ), this, SLOT ( slotAddApplication() ) );
    connect ( buildSubprojectButton, SIGNAL ( clicked() ), this, SLOT ( slotBuildSubproject() ) );

    connect ( targetOptionsButton, SIGNAL ( clicked() ), this, SLOT ( slotTargetOptions() ) );
    connect ( addNewFileButton, SIGNAL ( clicked() ), this, SLOT ( slotAddNewFile() ) );
    connect ( addExistingFileButton, SIGNAL ( clicked() ), this, SLOT ( slotAddExistingFile() ) );
    connect ( removeFileButton , SIGNAL ( clicked() ), this, SLOT ( slotRemoveFile() ) );
    connect ( buildTargetButton, SIGNAL ( clicked() ), this, SLOT ( slotBuildTarget() ) );

    details->addColumn(QString::null);

    connect ( overview, SIGNAL ( clicked ( QListViewItem* ) ), this, SLOT ( slotItemClicked ( QListViewItem* ) ) );
    connect( overview, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( overview, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( overview, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    connect ( details, SIGNAL ( clicked ( QListViewItem* ) ), this, SLOT ( slotItemClicked ( QListViewItem* ) ) );
    connect( details, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( details, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( details, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    m_part = part;
    m_kdeMode = kde;
    m_shownSubproject = 0;
    m_activeSubproject = 0;
    m_activeTarget = 0;
    m_activeFile = 0;
}


AutoProjectWidget::~AutoProjectWidget()
{}


void AutoProjectWidget::openProject(const QString &dirName)
{
    SubprojectItem *item = new SubprojectItem(overview, "/");
    item->subdir = "/";
    item->path = dirName;
    parse(item);
    item->setOpen(true);

    slotItemExecuted(item);
}


void AutoProjectWidget::closeProject()
{
    overview->clear();
    details->clear();
}


QStringList AutoProjectWidget::allSubprojects()
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


QStringList AutoProjectWidget::allLibraries()
{
    int prefixlen = projectDirectory().length()+1;
    QStringList res;
    
    QListViewItemIterator it(overview);
    for (; it.current(); ++it) {
        SubprojectItem *spitem = static_cast<SubprojectItem*>(it.current());
        QString path = spitem->path;
        QListIterator<TargetItem> tit(spitem->targets);
        for (; tit.current(); ++tit) {
            QString primary = (*tit)->primary;
            if (primary == "LIBRARIES" || primary == "LTLIBRARIES") {
                QString fullname = path + "/" + QString((*tit)->name);
                res.append(fullname.mid(prefixlen));
            }
        }
    }
    
    return res;
}


QStringList AutoProjectWidget::allSourceFiles()
{
    QStack<QListViewItem> s;
    QStringList res;
    
    for ( QListViewItem *item = overview->firstChild(); item;
          item = item->nextSibling()? item->nextSibling() : s.pop() ) {
        if (item->firstChild())
            s.push(item->firstChild());
        
        SubprojectItem *spitem = static_cast<SubprojectItem*>(item);
        QString path = spitem->path;
        QListIterator<TargetItem> tit(spitem->targets);
        for (; tit.current(); ++tit) {
            QString primary = (*tit)->primary;
            if (primary == "PROGRAMS" || primary == "LIBRARIES"
                || primary == "LTLIBRARIES" || primary == "JAVA") {
                QListIterator<FileItem> fit(tit.current()->sources);
                for (; fit.current(); ++fit) {
                    QString fullname = path + "/" + (*fit)->name;
                    res += fullname;
                }
            }
        }
    }
    
    return res;
}


QString AutoProjectWidget::projectDirectory()
{
    if (!overview->firstChild())
        return QString::null; //confused

    return static_cast<SubprojectItem*>(overview->firstChild())->path;
}


QString AutoProjectWidget::subprojectDirectory()
{
    if (!m_shownSubproject)
        return QString::null;

    return m_shownSubproject->path;
}


QString AutoProjectWidget::buildDirectory()
{
    QDomDocument &dom = *m_part->projectDom();

    QString dir = DomUtil::readEntry(dom, "/kdevautoproject/configure/builddir");
    return dir.isEmpty()? projectDirectory() : dir;
}


void AutoProjectWidget::slotItemClicked ( QListViewItem* item )
{
    if ( !item ) return;

    ProjectItem* pvItem = static_cast<ProjectItem*>(item);

    if ( pvItem->type() == ProjectItem::Subproject )
    {
        subProjectOptionsButton->setEnabled ( true );
        addSubprojectButton->setEnabled ( true );
        addTargetButton->setEnabled ( true );
        addServiceButton->setEnabled ( true );
        addApplicationButton->setEnabled ( true );
        buildSubprojectButton->setEnabled ( true );

        m_activeSubproject = static_cast<SubprojectItem*>(item);
    }
    else if ( pvItem->type() == ProjectItem::Target )
    {
        targetOptionsButton->setEnabled ( true );
        addNewFileButton->setEnabled ( true );
        addExistingFileButton->setEnabled ( true );
        removeFileButton->setEnabled ( false );
        buildTargetButton->setEnabled ( true );

        m_activeTarget = static_cast<TargetItem*> ( item );
    }
    else if ( pvItem->type() == ProjectItem::File )
    {
        targetOptionsButton->setEnabled ( false );
        addNewFileButton->setEnabled ( false );
        addExistingFileButton->setEnabled ( false );
        removeFileButton->setEnabled ( true );
        buildTargetButton->setEnabled ( false );

        m_activeFile = static_cast<FileItem*> ( item );
    }
}


void AutoProjectWidget::setActiveTarget(const QString &targetPath)
{
    int prefixlen = projectDirectory().length()+1;

    m_activeSubproject = 0;
    m_activeTarget = 0;
    
    QListViewItemIterator it(overview);
    for (; it.current(); ++it) {
        SubprojectItem *spitem = static_cast<SubprojectItem*>(it.current());
        QString path = spitem->path;
        QListIterator<TargetItem> tit(spitem->targets);
        for (; tit.current(); ++tit) {
            QString primary = (*tit)->primary;
            if (primary != "PROGRAMS" && primary != "LIBRARIES"
                && primary != "LTLIBRARIES")
                continue;
            QString currentTargetPath = (path + "/" + QString((*tit)->name)).mid(prefixlen);
            bool hasTarget = (targetPath == currentTargetPath);
            kdDebug(9020) << "Compare " << targetPath
                          << " with " << currentTargetPath
                          << ", name: " << (*tit)->name
                          << ", prefix: " << (*tit)->prefix
                          << ", primary: " << (*tit)->primary << endl;
            (*tit)->setBold(hasTarget);
            if (hasTarget) {
                m_activeSubproject = spitem;
                m_activeTarget = (*tit);
                if (m_shownSubproject != m_activeSubproject) {
                    overview->setSelected(spitem, true);
                    slotItemExecuted(spitem);
                }
            } else {
                details->viewport()->update();
            }
        }
    }
}


QString AutoProjectWidget::activeDirectory()
{
    return m_activeSubproject->path.mid(projectDirectory().length()+1);
}


void AutoProjectWidget::addFile(const QString &name)
{
    FileItem *fitem = createFileItem(name);
    m_activeTarget->sources.append(fitem);
    m_activeTarget->insertItem(fitem);

    // TODO: Merge with code in addfiledlg.cpp
    QCString canontargetname = AutoProjectTool::canonicalize(m_activeTarget->name);
    QCString varname = canontargetname + "_SOURCES";
    m_activeSubproject->variables[varname] += (QCString(" ") + name.latin1());
    
    QMap<QCString,QCString> replaceMap;
    replaceMap.insert(varname, m_activeSubproject->variables[varname]);
    
    AutoProjectTool::modifyMakefileam(m_activeSubproject->path + "/Makefile.am", replaceMap);

    emitAddedFile(m_activeSubproject->path + "/" + name);
}


void AutoProjectWidget::removeFile(const QString &fileName)
{
}


void AutoProjectWidget::slotItemExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // We assume here that ALL items in both list views
    // are ProjectItem's
    ProjectItem *pvitem = static_cast<ProjectItem*>(item);

    if (pvitem->type() == ProjectItem::Subproject) {
        if (m_shownSubproject) {
            // Remove all TargetItems and all of their children from the view
            QListIterator<TargetItem> it1(m_shownSubproject->targets);
            for (; it1.current(); ++it1) {
                // After AddTargetDialog, it can happen that an
                // item is not yet in the list view, so better check...
                if (it1.current()->parent())
                    while ((*it1)->firstChild())
                        (*it1)->takeItem((*it1)->firstChild());
                details->takeItem(*it1);
            }
        }
            
        m_shownSubproject = static_cast<SubprojectItem*>(item);
        m_activeSubproject = static_cast<SubprojectItem*>(item);

        // Insert all TargetItems and all of their children into the view
        QListIterator<TargetItem> it2(m_shownSubproject->targets);
        for (; it2.current(); ++it2) {
            details->insertItem(*it2);
            QListIterator<FileItem> it3((*it2)->sources);
            for (; it3.current(); ++it3)
                (*it2)->insertItem(*it3);
            QString primary = (*it2)->primary;
            if (primary == "PROGRAMS" || primary == "LIBRARIES"
                || primary == "LTLIBRARIES" || primary == "JAVA")
                (*it2)->setOpen(true);
        }
    } else if (pvitem->type() == ProjectItem::File) {
        QString dirName = m_shownSubproject->path;
        FileItem *fitem = static_cast<FileItem*>(pvitem);
        m_part->partController()->editDocument(KURL(dirName + "/" + QString(fitem->name)));
    	m_part->topLevel()->lowerView(this);
    }
}

void AutoProjectWidget::slotSubprojectOptions()
{
    kdDebug ( 9000 ) << "AutoProjectWidget::slotSubprojectOptions()" << endl;
    SubprojectOptionsDialog(m_part, this, m_activeSubproject, this, "subproject options dialog").exec();
}

void AutoProjectWidget::slotAddSubproject()
{
    AddSubprojectDialog dlg(m_part, this, m_activeSubproject, this, "add subproject dialog");
    dlg.exec();
}

void AutoProjectWidget::slotAddExistingSubproject()
{
    ImportExistingDialog ( m_part, KFile::Directory, m_activeSubproject->path, this, "Add existing subprojects to this subproject" ).exec();
}

void AutoProjectWidget::slotAddTarget()
{
    AddTargetDialog dlg(this, m_shownSubproject, this, "add target dialog");

    // Update the details view if a target was added
    if ( dlg.exec() && m_activeSubproject ) slotItemExecuted ( m_activeSubproject );
}

void AutoProjectWidget::slotAddService()
{
    AddServiceDialog dlg(this, m_activeSubproject, this, "add service dialog");

    // Update the details view if a service was added
    if ( dlg.exec() && m_activeSubproject ) slotItemExecuted ( m_activeSubproject );
}

void AutoProjectWidget::slotAddApplication()
{
    AddApplicationDialog dlg(this, m_activeSubproject, this, "add application dialog");

    // Update the details view if an application was added
    if ( dlg.exec() && m_activeSubproject ) slotItemExecuted ( m_activeSubproject );
}

void AutoProjectWidget::slotBuildSubproject()
{
    QString relpath = m_activeSubproject->path.mid(projectDirectory().length());

    m_part->startMakeCommand(buildDirectory() + relpath, QString::fromLatin1(""));

    m_part->topLevel()->lowerView ( this );
}

void AutoProjectWidget::slotTargetOptions()
{
    kdDebug ( 9000 ) << "AutoProjectWidget::slotTargetOptions()" << endl;
    TargetOptionsDialog(this, m_activeTarget, this, "target options dialog").exec();
}

void AutoProjectWidget::slotAddNewFile()
{
    AddFileDialog dlg(m_part, this, m_activeSubproject, m_activeTarget, this, "add file dialog");

    if (dlg.exec()) slotItemExecuted(m_activeSubproject); // update list view
}

void AutoProjectWidget::slotAddExistingFile()
{
    ImportExistingDialog ( m_part, KFile::Files, m_activeTarget->name, this, "Add existing files to this target" ).exec();
}

void AutoProjectWidget::slotBuildTarget()
{
    QString name = m_activeTarget->name;

    if (m_activeTarget->primary == "LIBRARIES") name + ".a";

    else if (m_activeTarget->primary == "LTLIBRARIES") name + ".la";

    QString relpath = m_activeSubproject->path.mid(projectDirectory().length());
    m_part->startMakeCommand(buildDirectory() + relpath, m_activeTarget->name);

    m_part->topLevel()->lowerView ( this );
}

void AutoProjectWidget::slotRemoveFile()
{
    RemoveFileDialog dlg(this, m_activeSubproject, m_activeTarget, m_activeFile->text(0), this, "remove file dialog");

    if (dlg.exec()) slotItemExecuted(m_activeSubproject);
}

void AutoProjectWidget::slotSetActiveTarget()
{
    QString targetPath = m_shownSubproject->path + "/" + QString(m_activeTarget->name);
    targetPath = targetPath.mid(projectDirectory().length()+1);
    kdDebug(9020) << "Setting active " << targetPath << endl;
    setActiveTarget(targetPath);
    QDomDocument &dom = *m_part->projectDom();
    DomUtil::writeEntry(dom, "/kdevautoproject/general/activetarget", targetPath);
}

void AutoProjectWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;
    
    ProjectItem *pvitem = static_cast<ProjectItem*>(item);

    if (pvitem->type() == ProjectItem::Subproject)
    {
        subProjectOptionsButton->setEnabled ( true );
        addSubprojectButton->setEnabled ( true );
        addTargetButton->setEnabled ( true );
        addServiceButton->setEnabled ( true );
        addApplicationButton->setEnabled ( true );
        buildSubprojectButton->setEnabled ( true );

        SubprojectItem *spitem = static_cast<SubprojectItem*>(pvitem);

        m_activeSubproject = spitem;

        KPopupMenu* popup = new KPopupMenu ( this, i18n ( "Subproject" ) );

        subProjectOptionsAction->plug ( popup );
        popup->insertSeparator();
        addSubprojectAction->plug ( popup );
        addExistingSubprojectAction->plug ( popup );
        popup->insertSeparator();
        addTargetAction->plug ( popup );
        popup->insertSeparator();
        addServiceAction->plug ( popup );
        popup->insertSeparator();
        addApplicationAction->plug ( popup );
        popup->insertSeparator();
        buildSubprojectAction->plug ( popup );

        popup->exec ( p );
    }
    else if (pvitem->type() == ProjectItem::Target)
    {
        targetOptionsButton->setEnabled ( true );
        addNewFileButton->setEnabled ( true );
        addExistingFileButton->setEnabled ( true );
        removeFileButton->setEnabled ( false );
        buildTargetButton->setEnabled ( true );

        TargetItem *titem = static_cast<TargetItem*>(pvitem);

        m_activeTarget = titem;

        KPopupMenu* popup = new KPopupMenu ( this, ( nicePrimary(titem->primary ) ) );

        if (titem->primary == "PROGRAMS" || titem->primary == "LIBRARIES"
            || titem->primary == "LTLIBRARIES")
        {
            targetOptionsAction->plug ( popup );
            setActiveTargetAction->plug ( popup );
            popup->insertSeparator();
        }

        addNewFileAction->plug ( popup );
        addExistingFileAction->plug ( popup );
        popup->insertSeparator();
        buildTargetAction->plug ( popup );

        popup->exec ( p );
    }
    else if (pvitem->type() == ProjectItem::File)
    {
        targetOptionsButton->setEnabled ( false );
        addNewFileButton->setEnabled ( false );
        addExistingFileButton->setEnabled ( false );
        removeFileButton->setEnabled ( true );
        buildTargetButton->setEnabled ( false );

        FileItem *fitem = static_cast<FileItem*>(pvitem);
        m_activeFile = fitem;
/*        TargetItem *titem = static_cast<TargetItem*>(fitem->parent());

        m_activeTarget = titem;*/

        KPopupMenu* popup = new KPopupMenu ( this, i18n ( "Remove file" ) );

        removeFileAction->plug ( popup );

        popup->exec ( p );

        FileContext context ( m_activeSubproject->path + "/" + fitem->name );
        m_part->core()->fillContextMenu ( popup, &context );
    }
}


TargetItem *AutoProjectWidget::createTargetItem(const QCString &name,
                                                const QCString &prefix, const QCString &primary)
{
    bool group = !(primary == "PROGRAMS" || primary == "LIBRARIES"
                   || primary == "LTLIBRARIES" || primary == "JAVA");
    QString text = group?
        i18n("%1 in %2").arg(nicePrimary(primary)).arg(prefix)
        : i18n("%1 (%2 in %3)").arg(name).arg(nicePrimary(primary)).arg(prefix);
    
    // Workaround because of QListView not being able to create
    // items without actually inserting them
    TargetItem *titem = new TargetItem(overview, group, text);
    overview->takeItem(titem);
    titem->name = name;
    titem->prefix = prefix;
    titem->primary = primary;

    return titem;
}


FileItem *AutoProjectWidget::createFileItem(const QString &name)
{
    FileItem *fitem = new FileItem(overview, name);
    overview->takeItem(fitem);
    fitem->name = name;

    return fitem;
}


void AutoProjectWidget::emitAddedFile(const QString &name)
{
    emit m_part->addedFileToProject(name);
}


void AutoProjectWidget::emitRemovedFile(const QString &name)
{
    emit m_part->removedFileFromProject(name);
}


void AutoProjectWidget::parsePrimary(SubprojectItem *item, QCString lhs, QCString rhs)
{
    // Parse line foo_bar = bla bla
    int pos = lhs.findRev('_');
    QCString prefix = lhs.left(pos);
    QCString primary = lhs.right(lhs.length()-pos-1);
    //    kdDebug(9020) << "Prefix:" << prefix << ",Primary:" << primary << endl;

#if 0
    QStrList prefixes;
    prefixes.append("bin");
    prefixes.append("pkglib");
    prefixes.append("pkgdata");
    prefixes.append("noinst");
    prefixes.append("check");
    prefixes.append("sbin");
    QStrList primaries;
    primaries.append("PROGRAMS");
    primaries.append("LIBRARIES");
    primaries.append("LTLIBRARIES");
    primaries.append("SCRIPTS");
    primaries.append("HEADERS");
    primaries.append("DATA");
#endif

    // Not all combinations prefix/primary are possible, so this
    // could also be checked... not trivial because the list of
    // possible prefixes can be extended dynamically (see below)
    if (primary == "PROGRAMS" || primary == "LIBRARIES" || primary == "LTLIBRARIES") {
        QStringList l = QStringList::split(QRegExp("[ \t\n]"), QString(rhs));
        QStringList::Iterator it1;
        for (it1 = l.begin(); it1 != l.end(); ++it1) {
            TargetItem *titem = createTargetItem((*it1).latin1(), prefix, primary);
            item->targets.append(titem);

            QCString canonname = AutoProjectTool::canonicalize(*it1);
            titem->ldflags = cleanWhitespace(item->variables[canonname + "_LDFLAGS"]);
            titem->ldadd = cleanWhitespace(item->variables[canonname + "_LDADD"]);
            titem->libadd = cleanWhitespace(item->variables[canonname + "_LIBADD"]);
            titem->dependencies = cleanWhitespace(item->variables[canonname + "_DEPENDENCIES"]);

            QCString sources = item->variables[canonname + "_SOURCES"];
            QStringList l2 = QStringList::split(QRegExp("[ \t\n]"), sources);
            QStringList::Iterator it2;
            for (it2 = l2.begin(); it2 != l2.end(); ++it2) {
                FileItem *fitem = createFileItem(*it2);
                titem->sources.append(fitem);
            }
        }
    } else if (primary == "SCRIPTS" || primary == "HEADERS" || primary == "DATA") {
        // See if we have already such a group
        for (uint i=0; i < item->targets.count(); ++i) {
            TargetItem *titem = item->targets.at(i);
            if (primary == titem->primary && prefix == titem->prefix) {
                item->targets.remove(i);
                break;
            }
        }
        // Create a new one
        TargetItem *titem = createTargetItem(QCString(""), prefix, primary);
        item->targets.append(titem);
        
        QStringList l = QStringList::split(QRegExp("[ \t]"), QString(rhs));
        QStringList::Iterator it3;
        for (it3 = l.begin(); it3 != l.end(); ++it3) {
            FileItem *fitem = createFileItem(*it3);
            titem->sources.append(fitem);
        }
    } else if (primary == "JAVA") {
        QStringList l = QStringList::split(QRegExp("[ \t\n]"), QString(rhs));
        QStringList::Iterator it1;
        TargetItem *titem = createTargetItem(QCString(""), prefix, primary);
        item->targets.append(titem);

        for (it1 = l.begin(); it1 != l.end(); ++it1) {
            FileItem *fitem = createFileItem(*it1);
            titem->sources.append(fitem);
        }
    }
}


void AutoProjectWidget::parsePrefix(SubprojectItem *item, QCString lhs, QCString rhs)
{
    // Parse a line foodir = bla bla
    QCString name = lhs.left(lhs.length()-3);
    QCString dir = rhs;
    item->prefixes.insert(name, dir);
}


void AutoProjectWidget::parseSubdirs(SubprojectItem *item, QCString /*lhs*/, QCString rhs)
{
    // Parse a line SUBDIRS = bla bla

    // Take care of KDE hacks
    if (rhs.find("$(TOPSUBDIRS)") != -1) {
        QString dirs;
        QFile subdirsfile(item->path + "/subdirs");
        if (subdirsfile.open(IO_ReadOnly)) {
            QTextStream subdirsstream(&subdirsfile);
            while (!subdirsstream.atEnd()) {
                QString dir = subdirsstream.readLine();
                dirs.append(dir);
                dirs.append(" ");
            }
            subdirsfile.close();
        }
        rhs.replace(QRegExp("\\$\\(TOPSUBDIRS\\)"), dirs);
    }
    // Do something smarter here
    rhs.replace(QRegExp("\\$\\(AUTODIRS\\)"), "");
    rhs.replace(QRegExp("\\$\\(COMPILE_FIRST\\)"), "");
    rhs.replace(QRegExp("\\$\\(COMPILE_LAST\\)"), "");
    QStringList l = QStringList::split(QRegExp("[ \t]"), QString(rhs));
    l.sort();
    QStringList::Iterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        if (*it == ".")
            continue;
        SubprojectItem *newitem = new SubprojectItem(item, (*it));
        newitem->subdir = (*it);
        newitem->path = item->path + "/" + (*it);
        parse(newitem);
        // Experience tells me this :-)
        bool open = true;
        if (newitem->subdir == "doc")
            open = false;
        if (newitem->subdir == "po")
            open = false;
        if (newitem->subdir == "pics")
            open = false;
        if (newitem && static_cast<SubprojectItem*>(newitem->parent())->subdir == "doc")
            open = false;
        if (newitem && static_cast<SubprojectItem*>(newitem->parent())->subdir == "po")
            open = false;
        if (newitem && static_cast<SubprojectItem*>(newitem->parent())->subdir == "pics")
            open = false;
        newitem->setOpen(open);

        // Move to the bottom of the list
        QListViewItem *lastItem = item->firstChild();
        while (lastItem->nextSibling())
            lastItem = lastItem->nextSibling();
        if (lastItem != newitem)
            newitem->moveItem(lastItem);
    }
}


void AutoProjectWidget::parse(SubprojectItem *item)
{
    AutoProjectTool::parseMakefileam(item->path + "/Makefile.am", &item->variables);
    
    QMap<QCString, QCString>::ConstIterator it;
    for (it=item->variables.begin(); it!=item->variables.end(); ++it) {
        QCString lhs = it.key();
        QCString rhs = it.data();
        if (lhs.find('_') > 0)
            parsePrimary(item, lhs, rhs);
        else if (lhs.right(3) == "dir")
            parsePrefix(item, lhs, rhs);
        else if (lhs == "SUBDIRS")
            parseSubdirs(item, lhs, rhs);
    }
}

#include "autoprojectwidget.moc"
