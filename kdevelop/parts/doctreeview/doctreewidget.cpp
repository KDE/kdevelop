/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qlayout.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include "config.h"

#include "ctoolclass.h"
#include "cproject.h"
#include "doctreewidget.h"
#include "doctreeview.h"


class DocTreeItem : public QListViewItem
{
public:
    enum Type { Folder, Book, Doc };
    DocTreeItem( QListView *parent, Type type, const QString &text, const QString &id );
    DocTreeItem( DocTreeItem *parent, Type type, const QString &text, const QString &id );
    
    QString ident() const
        { return idnt; }
    virtual void clear();
    virtual void setOpen(bool o);
    
private:
    void setTypePixmap(bool o);
    Type typ;
    QString idnt;
};


DocTreeItem::DocTreeItem(QListView *parent, Type type, const QString &text, const QString &id)
    : QListViewItem(parent, text), typ(type), idnt(id)
{
    setTypePixmap(false);
}


DocTreeItem::DocTreeItem(DocTreeItem *parent, Type type, const QString &text, const QString &id)
    : QListViewItem(parent, text), typ(type), idnt(id)
{
    setTypePixmap(false);
}


void DocTreeItem::clear()
{
    QListViewItem *child = firstChild();
    while (child) {
        QListViewItem *old = child;
        child = child->nextSibling();
        delete old;
    }
}


void DocTreeItem::setTypePixmap(bool o)
{
    QPixmap pixmap;
    if (typ == Folder)
        pixmap = SmallIcon(o? "folder-open" : "folder");
    else if (typ == Book)
        pixmap = UserIcon(o? "mini-book2" : "mini-book1");
    else
        pixmap = UserIcon("mini-doc");
    
    setPixmap(0, pixmap);
}


void DocTreeItem::setOpen(bool o)
{
    setTypePixmap(o);
    QListViewItem::setOpen(o);
}


   
/*************************************/
/* Folder "KDevelop"                */
/*************************************/


/**
 * Here we specialize on a KDevelop book. The constructor takes
 * only the last part of the file name as argument and tries to
 * locate the file according to the locale set.
 * The constructor allows an argument expandable.
 */
class DocTreeKDevelopBook : public DocTreeItem
{
public:
    DocTreeKDevelopBook( DocTreeItem *parent, const QString &text,
                         const QString &filename, bool expandable=false );
    ~DocTreeKDevelopBook();
    virtual void setOpen(bool o);
private:
    void readSgmlIndex(FILE *f);
};


DocTreeKDevelopBook::DocTreeKDevelopBook(DocTreeItem *parent, const QString &text,
                                         const QString &filename, bool expandable)
    : DocTreeItem(parent, Book, text, filename)
{
    setExpandable(expandable);
}


DocTreeKDevelopBook::~DocTreeKDevelopBook()
{}


void DocTreeKDevelopBook::setOpen(bool o)
{
    kdDebug(9002) << (o? "Open kdevelop book" : "Close kdevelop book") << endl;
    if (o && childCount() == 0) {
        FILE *f;
        if ( (f = fopen(ident(), "r")) != 0) {
            readSgmlIndex(f);
            fclose(f);
        } else {
            setExpandable(false);
        }
    }
    DocTreeItem::setOpen(o);
}


void DocTreeKDevelopBook::readSgmlIndex(FILE *f)
{
    char buf[512];
    while (fgets(buf, sizeof buf, f)) {
        // HTML files produced by sgml2html have toc's like the following:
        // <H2><A NAME="toc1">1.</A> <A HREF="index-1.html">Introduction</A></H2>
        QString s = buf;
        if (s.find("<H2>") == -1)
            continue;
        int pos1 = s.find("A HREF=\"");
        if (pos1 == -1)
            continue;
        int pos2 = s.find('"', pos1+8);
        if (pos2 == -1)
            continue;
        int pos3 = s.find('<', pos2+1);
        if (pos3 == -1)
            continue;
        QString filename = s.mid(pos1+8, pos2-(pos1+8));
        QString title = s.mid(pos2+2, pos3-(pos2+2));
        QFileInfo fi(ident());
        QString path = fi.dirPath() + "/" + filename;
        new DocTreeItem(this, Doc, title, path);
    }
}


class DocTreeKDevelopFolder : public DocTreeItem
{
public:
    DocTreeKDevelopFolder(DocTreeWidget *parent)
        : DocTreeItem(parent, Folder, i18n("KDevelop"), "")
        { setExpandable(true); }
    void refresh();
};


void DocTreeKDevelopFolder::refresh()
{
    DocTreeItem::clear();

    QString path = locate("appdata", "tools/documentation");
    KSimpleConfig docconfig(path);
    docconfig.setGroup("Contents");
    QStringList entries = docconfig.readListEntry("KDevelopEntries");
    KConfig *config = KGlobal::config();
    config->setGroup("DocTree");
    QStringList nonEntries = config->readListEntry("KDevelopNotShown");
    
    for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
        if (!nonEntries.contains(*it)) {
            docconfig.setGroup("KDevelop-" + (*it));
            QString name = docconfig.readEntry("Name");
            QString filename = CToolClass::locatehtml("kdevelop/" + docconfig.readEntry("Path"));
            bool expandable = docconfig.readBoolEntry("Expandable");
            (void) new DocTreeKDevelopBook(this, name, filename, expandable);
            kdDebug(9002) << "Insert " << name << " from file " << filename << endl;
        }
}
    

/*************************************/
/* Folder "Qt/KDE libraries"         */
/*************************************/


/**
 * Here we specialize on a Qt/kdelibs book. The constructor takes
 * only the library name as argument and tries to locate the
 * according file. To simplify things, we treat Qt as a special
 * KDE library, namely that with an empty name (sorry trolls :-)
 */
class DocTreeKDELibsBook : public DocTreeItem
{
public:
    DocTreeKDELibsBook( DocTreeItem *parent, const QString &text,
                        const QString &filename, const QString &idxfilename );
    ~DocTreeKDELibsBook();
    virtual void setOpen(bool o);
private:
    void readKdoc2Index(FILE *f);
    QString idx_filename;
};


DocTreeKDELibsBook::DocTreeKDELibsBook(DocTreeItem *parent, const QString &text,
                                       const QString &filename, const QString &idxfilename)
    : DocTreeItem(parent, Book, text, filename), idx_filename(idxfilename)
{
    if (!idxfilename.isNull())
        // If we have a kdoc2 index in either uncompressed
        // or compressed form, we read it in on demand.
        setExpandable(true);
}


DocTreeKDELibsBook::~DocTreeKDELibsBook()
{}


void DocTreeKDELibsBook::readKdoc2Index(FILE *f)
{
    char buf[1024];
    QString baseurl;
    while (fgets(buf, sizeof buf, f)) {
        QString s = buf;
        if (s.left(11) == "<BASE URL=\"") {
            int pos2 = s.find("\">", 11);
            if (pos2 != -1)
                baseurl = s.mid(11, pos2-11);
        }
        else if (s.left(9) == "<C NAME=\"") {
            int pos1 = s.find("\" REF=\"", 9);
            if (pos1 == -1)
                continue;
                int pos2 = s.find("\">", pos1+7);
                if (pos2 == -1)
                    continue;
                QString classname = s.mid(9, pos1-9);
                QString filename = s.mid(pos1+7, pos2-(pos1+7));
                new DocTreeItem(this, Doc, classname, baseurl + "/" + filename);
        }
    }
    sortChildItems(0, true);
}


void DocTreeKDELibsBook::setOpen(bool o)
{
    kdDebug(9002) << (o? "Open kdelibs book" : "Close kdelibs book") << endl;
    if (o && childCount() == 0) {
        FILE *f;
        if ( (f = fopen(idx_filename, "r")) != 0) {
            readKdoc2Index(f);
            fclose(f);
        }
        else if ( (f = popen(QString("gzip -c -d ")
                             + idx_filename + ".gz 2>/dev/null", "r")) != 0) {
            readKdoc2Index(f);
            pclose(f);
        }
        else
            setExpandable(false);
    }
    DocTreeItem::setOpen(o);
}


class DocTreeKDELibsFolder : public DocTreeItem
{
public:
    DocTreeKDELibsFolder(DocTreeWidget *parent)
        : DocTreeItem(parent, Folder, i18n("Qt/KDE Libraries"), "")
        { setExpandable(true); }
    void refresh();
};


void DocTreeKDELibsFolder::refresh()
{
    DocTreeItem::clear();

    KConfig *config = KGlobal::config();
    config->setGroup("Doc_Location");
    QString kde_path = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    QString qt_path = config->readEntry("doc_qt", QT_DOCDIR);
    QString idx_path = kde_path + "/kdoc-reference";
    
    QString path = locate("appdata", "tools/documentation");
    KSimpleConfig docconfig(path);
    docconfig.setGroup("Contents");
    QStringList entries = docconfig.readListEntry("LibrariesEntries");
    config->setGroup("DocTree");
    QStringList nonEntries = config->readListEntry("LibrariesNotShown");
    
    for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
        if (!nonEntries.contains(*it)) {
            docconfig.setGroup("Libraries-" + (*it));
            QString name = docconfig.readEntry("Name");
            QString filename = (*it == "qt")?
                (qt_path + "/index.html") : (kde_path + "/" + (*it) + "/index.html");
            QString idxfilename = idx_path.isEmpty()?
                QString::null : (idx_path + "/" + (*it) + ".kdoc");
            if (QFileInfo(filename).exists())
                (void) new DocTreeKDELibsBook(this, name, filename, idxfilename);
            kdDebug(9002) << "Insert " << name << " from file " << filename << endl;
        }
}
    

/*************************************/
/* Folder "Documentation Base"       */
/*************************************/

#ifdef WITH_DOCBASE


class DocTreeDocbaseFolder : public DocTreeItem
{
public:
    DocTreeDocbaseFolder(DocTreeWidget *parent);
    ~DocTreeDocbaseFolder();
    virtual void setOpen(bool o);
private:
    void readDocbaseFile(FILE *f);
};


DocTreeDocbaseFolder::DocTreeDocbaseFolder(DocTreeWidget *parent)
    : DocTreeItem(parent, Folder, i18n("Documentation Base"), "")
{
    setExpandable(true);
}


DocTreeDocbaseFolder::~DocTreeDocbaseFolder()
{}


void DocTreeDocbaseFolder::readDocbaseFile(FILE *f)
{
    char buf[1024];
    QString title;
    bool html = false;
    while (fgets(buf, sizeof buf, f)) {
        QString s = buf;
        if (s.right(1) == "\n")
            s.truncate(s.length()-1); // chop
        
        if (s.left(7) == "Title: ")
            title = s.mid(7, s.length()-7);
        else if (s.left(8) == "Format: ")
            html = s.find("HTML", 8, false) != -1;
        else if (s.left(7) == "Index: "
                 && html && !title.isEmpty()) {
            QString filename = s.mid(7, s.length()-7);
            (void) new DocTreeItem(this, Doc, title, filename);
            break;
        }
        else if (s.left(9) == "Section: "
                 && s.find("programming", 9, false) == -1)
            break;
    }
}


void DocTreeDocbaseFolder::setOpen(bool o)
{
    kdDebug(9002) << (o? "Open docbase folder" : "Close docbase folder") << endl;
    if (o && childCount() == 0) {
        QDir d("/usr/share/doc-base");
        QStringList fileList = d.entryList("*", QDir::Files);
        QStringList::Iterator it;
        for (it = fileList.begin(); it != fileList.end(); ++it) {
            FILE *f;
            if ( (f = fopen(d.filePath(*it), "r")) != 0) {
                readDocbaseFile(f);
                fclose(f);
            }
        }
    }
    DocTreeItem::setOpen(o);
}


#endif


/*************************************/
/* Folder "Others"                  */
/*************************************/

class DocTreeOthersFolder : public DocTreeItem
{
public:
    DocTreeOthersFolder(DocTreeWidget *parent);
    void refresh();
};


DocTreeOthersFolder::DocTreeOthersFolder(DocTreeWidget *parent)
    : DocTreeItem(parent, Folder, i18n("Others"), "")
{}


void DocTreeOthersFolder::refresh()
{
    DocTreeItem::clear();

    KConfig *config = KGlobal::config();
    config->setGroup("DocTree");
    QStringList othersShownTitle = config->readListEntry("OthersShownTitle");
    QStringList othersShownURL = config->readListEntry("OthersShownURL");
    QStringList::Iterator it1 = othersShownTitle.begin();
    QStringList::Iterator it2 = othersShownURL.begin();
    for (; it1 != othersShownTitle.end() && it2 != othersShownURL.end(); ++it1, ++it2)
        (void) new DocTreeItem(this, Book, *it1, *it2);
}


/*************************************/
/* Folder "Current Project"          */
/*************************************/

class DocTreeProjectFolder : public DocTreeItem
{
public:
    DocTreeProjectFolder(DocTreeWidget *parent);
    void setProject(CProject *prj)
        { project = prj; }
    void refresh();
    
private:
    CProject *project;
};


DocTreeProjectFolder::DocTreeProjectFolder(DocTreeWidget *parent)
    : DocTreeItem(parent, Folder, i18n("Current Project"), "")
{}


void DocTreeProjectFolder::refresh()
{
    DocTreeItem::clear();

    setExpandable(false);
    if (project && project->valid) {
        setExpandable(true);
        
        (void) new DocTreeItem(this, Book, i18n("API documentation"), "internal:projectAPI");
        (void) new DocTreeItem(this, Book, i18n("User manual"), "internal:projectManual");
    }
}


/**************************************/
/* The DocTreeWidget itself           */
/**************************************/


DocTreeWidget::DocTreeWidget(DocTreeView *view)
    : KListView(0, "doc tree widget")
{
    setRootIsDecorated(true);
    setSorting(-1);
    setFrameStyle(Panel | Sunken);
    setLineWidth(2); 
    header()->hide();
    addColumn("");
    
    folder_project  = new DocTreeProjectFolder(this);
    folder_others   = new DocTreeOthersFolder(this);
#ifdef WITH_DOCBASE
    folder_docbase  = new DocTreeDocbaseFolder(this);
    //    folder_docbase->refresh();
#endif
    folder_kdelibs  = new DocTreeKDELibsFolder(this);
    folder_kdelibs->refresh();   
    folder_kdevelop = new DocTreeKDevelopFolder(this);
    folder_kdevelop->refresh();

    connect( this, SIGNAL(pressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );

    m_part = view;
}


DocTreeWidget::~DocTreeWidget()
{}


void DocTreeWidget::slotItemExecuted(QListViewItem *item)
{
    // We assume here that ALL (!) items in the list view
    // are DocTreeItem's
    DocTreeItem *dtitem = static_cast<DocTreeItem*>(item);
    if (!dtitem)
        return;

    QString ident = dtitem->ident();
    if (ident == "internal:projectAPI")
        emit m_part->gotoProjectApiDoc();
    else if (ident == "internal:projectManual")
        emit m_part->gotoProjectManual();
    else if (!ident.isEmpty())
        emit m_part->gotoDocumentationFile(ident);
}


void DocTreeWidget::slotRightButtonPressed(QListViewItem *item, const QPoint &p)
{
    contextItem = item;
    KPopupMenu pop(i18n("Documentation Tree"));
    pop.insertItem(i18n("Configure..."), this, SLOT(slotConfigure()));
    pop.exec(p);
}


void DocTreeWidget::slotConfigure()
{
#if 0
    DocTreeConfigWidget::Page page;
    if (contextItem == folder_kdevelop || contextItem->parent() == folder_kdevelop)
        page = DocTreeWidgetConfigWidget::KDevelop;
    else if (contextItem == folder_kdelibs || contextItem->parent() == folder_kdelibs)
        page = DocTreeWidgetConfigWidget::Libraries;
    else
        page = DocTreeWidgetConfigWidget::Others;

    CustomizeDialog *dlg = new CustomizeDialog(this, "customize doctreeview");
    QFrame *frame = dlg->addPage(i18n("Documentation Tree"));
    QBoxLayout *vbox = new QVBoxLayout(frame);
    DocTreeWidgetConfigWidget *w =
        new DocTreeConfigWidget(this, frame, "doctreeview config widget");
    w->showPage(page);
    vbox->addWidget(w);
    connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
    dlg->exec();
#endif
}


void DocTreeWidget::configurationChanged()
{
    folder_kdevelop->refresh();
    folder_kdelibs->refresh();
    folder_others->refresh();
}


void DocTreeWidget::docPathChanged()
{
    folder_kdelibs->refresh();
}


void DocTreeWidget::projectOpened(CProject *prj)
{
    folder_project->setProject(prj);
    folder_kdevelop->refresh();
    folder_kdelibs->refresh();
    folder_others->refresh();
    folder_project->refresh();
}


void DocTreeWidget::projectClosed()
{
    folder_project->setProject(0);
}

