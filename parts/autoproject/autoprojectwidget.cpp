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

#include <qfile.h>
#include <qheader.h>
#include <qsplitter.h>
#include <qptrstack.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qregexp.h>
#include <kdebug.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kregexp.h>
#include <kurl.h>

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
#include "removefiledlg.h"
#include "autoprojectpart.h"
#include "autoprojectwidget.h"


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


static QCString cleanwhitespace(QCString str)
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
    m_kdeMode = kde;
    activeSubproject = 0;
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
    if (!activeSubproject)
        return QString::null;

    return activeSubproject->path;
}


QString AutoProjectWidget::buildDirectory()
{
    QDomDocument &dom = *m_part->projectDom();

    QString dir = DomUtil::readEntry(dom, "/kdevautoproject/configure/builddir");
    return dir.isEmpty()? projectDirectory() : dir;
}


void AutoProjectWidget::slotItemExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // We assume here that ALL items in both list views
    // are ProjectItem's
    ProjectItem *pvitem = static_cast<ProjectItem*>(item);

    if (pvitem->type() == ProjectItem::Subproject) {
        if (activeSubproject) {
            // Remove all TargetItems and all of their children from the view
            QListIterator<TargetItem> it1(activeSubproject->targets);
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

        // Insert all TargetItems and all of their children into the view
        QListIterator<TargetItem> it2(activeSubproject->targets);
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
        QString dirName = activeSubproject->path;
        FileItem *fitem = static_cast<FileItem*>(pvitem);
        m_part->partController()->editDocument(KURL(dirName + "/" + QString(fitem->name)));
	m_part->topLevel()->lowerView(this);
    }
}


void AutoProjectWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;
    
    ProjectItem *pvitem = static_cast<ProjectItem*>(item);

    if (pvitem->type() == ProjectItem::Subproject) {
        SubprojectItem *spitem = static_cast<SubprojectItem*>(pvitem);
        KPopupMenu pop(i18n("Subproject"));
        int idOptions = pop.insertItem(i18n("Options..."));
        int idAddSubproject = pop.insertItem(i18n("Add subproject..."));
        int idAddTarget = pop.insertItem(i18n("Add target..."));
        int idAddService = pop.insertItem(i18n("Add service desktop file..."));
        int idAddApplication = pop.insertItem(i18n("Add application desktop file..."));
        int idBuild = pop.insertItem(i18n("Build"));
        int r = pop.exec(p);
        if (r == idOptions) {
            SubprojectOptionsDialog(m_part, this, spitem, this, "subproject options dialog").exec();
        }
        else if (r == idAddSubproject) {
            AddSubprojectDialog dlg(m_part, this, spitem, this, "add subproject dialog");
            dlg.exec();
        }
        else if (r == idAddTarget) {
            AddTargetDialog dlg(this, spitem, this, "add target dialog");
            // Update the details view if a target was added
            if (dlg.exec() && activeSubproject == spitem)
                slotItemExecuted(spitem);
        }
        else if (r == idAddService) {
            AddServiceDialog dlg(this, spitem, this, "add service dialog");
            // Update the details view if a service was added
            if (dlg.exec() && activeSubproject == spitem)
                slotItemExecuted(spitem);
        }
        else if (r == idAddApplication) {
            AddApplicationDialog dlg(this, spitem, this, "add application dialog");
            // Update the details view if an application was added
            if (dlg.exec() && activeSubproject == spitem)
                slotItemExecuted(spitem);
        }
        else if (r == idBuild) {
            QString relpath = spitem->path.mid(projectDirectory().length());
            m_part->startMakeCommand(buildDirectory() + relpath, QString::fromLatin1(""));
        }
    } else if (pvitem->type() == ProjectItem::Target) {
        TargetItem *titem = static_cast<TargetItem*>(pvitem);
        KPopupMenu pop(nicePrimary(titem->primary));
        int idOptions = 0;
        if (titem->primary == "PROGRAMS" || titem->primary == "LIBRARIES"
            || titem->primary == "LTLIBRARIES") {
            idOptions = pop.insertItem(i18n("Options..."));
        }
        int idAddFile = pop.insertItem(i18n("Add file..."));
        int idBuild = pop.insertItem(i18n("Build"));
        int r = pop.exec(p);
        if (r == idOptions) {
            TargetOptionsDialog(this, titem, this, "target options dialog").exec();
        }
        else if (r == idAddFile) {
            AddFileDialog dlg(this, activeSubproject, titem, this, "add file dialog");
            if (dlg.exec())
                slotItemExecuted(activeSubproject); // update list view
        } else if (r == idBuild) {
            QString name = titem->name;
            if (titem->primary == "LIBRARIES")
                name + ".a";
            else if (titem->primary == "LTLIBRARIES")
                name + ".la";
            QString relpath = activeSubproject->path.mid(projectDirectory().length());
            m_part->startMakeCommand(buildDirectory() + relpath, titem->name);
        }
    } else if (pvitem->type() == ProjectItem::File) {
        FileItem *fitem = static_cast<FileItem*>(pvitem);
        TargetItem *titem = static_cast<TargetItem*>(fitem->parent());
        KPopupMenu pop;
        int idRemoveFile = pop.insertItem(i18n("Remove file..."));
	FileContext context(activeSubproject->path + "/" + fitem->name);
	m_part->core()->fillContextMenu(&pop, &context);
        int r = pop.exec(p);
        if (r == idRemoveFile) {
            RemoveFileDialog dlg(this, activeSubproject, titem, fitem->text(0), this, "remove file dialog");
            if (dlg.exec())
                slotItemExecuted(activeSubproject);
        }
    }
}


TargetItem *AutoProjectWidget::createTargetItem(const QCString &name,
                                                const QCString &prefix, const QCString &primary)
{
    bool group = !(primary == "PROGRAMS" || primary == "LIBRARIES" || primary == "LTLIBRARIES" || primary == "JAVA");
    QString text = group?
        i18n("%1 in %2").arg(nicePrimary(primary)).arg(prefix)
        : i18n("%1 (%2 in %3)").arg(name).arg(nicePrimary(primary)).arg(prefix);
    
    // Workaround because for QListView not being able to create
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
            titem->ldflags = cleanwhitespace(item->variables[canonname + "_LDFLAGS"]);
            titem->ldadd = cleanwhitespace(item->variables[canonname + "_LDADD"]);
            titem->libadd = cleanwhitespace(item->variables[canonname + "_LIBADD"]);
            titem->dependencies = cleanwhitespace(item->variables[canonname + "_DEPENDENCIES"]);

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
