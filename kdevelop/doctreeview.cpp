/***************************************************************************
                             doctreeview.cpp
                             -------------------
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <qfileinfo.h>
#include <qheader.h>
#include <qdir.h>
#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kpopmenu.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <stdio.h>

#include "cproject.h"
#include "ctoolclass.h"
#include "doctreeconfdlg.h"
#include "doctreeview.h"

#if HAVE_CONFIG_H
#include "../config.h"
#endif

/**
 * A list view item that is decorated with a doc icon.
 * This typically represents a section in a manual.
 */
class ListViewDocItem : public KListViewItem
{
public:
    ListViewDocItem( KListViewItem *parent,
                     const QString &text, const QString &filename );
};


ListViewDocItem::ListViewDocItem(KListViewItem *parent,
                                 const QString &text, const QString &filename)
    : KListViewItem(parent, text, filename)
{
    setPixmap(0, BarIcon("mini-doc"));
}


/**
 * A list view item that is decorated with a book icon.
 * This typically represents one manual. When the user "opens"
 * the book, the according icon is changed.
 */
class ListViewBookItem : public KListViewItem
{
public:
    ListViewBookItem( KListViewItem *parent,
                      const QString &text, const QString &filename );
    virtual void setOpen(bool o);
};


ListViewBookItem::ListViewBookItem(KListViewItem *parent,
                                   const QString &text, const QString &filename)
    : KListViewItem(parent, text, filename)
{
    setOpen(false);
}


void ListViewBookItem::setOpen(bool o)
{
    setPixmap(0, o? BarIcon("mini-book2") : BarIcon("mini-book1"));
    KListViewItem::setOpen(o);
}


/**
 * A list view item that is decorated with a folder icon.
 * This typically represents one major part in the help tree.
 * Its contents can be clear()d, which is usually the first
 * thing done in the refresh() methods below.
 */
class ListViewFolderItem : public KListViewItem
{
public:
    ListViewFolderItem( KListView *parent, const QString &text );
    virtual void setOpen(bool o);
    void clear();
};


ListViewFolderItem::ListViewFolderItem(KListView *parent, const QString &text)
    : KListViewItem(parent, text, QString::null)
{
    setOpen(false);
}


void ListViewFolderItem::setOpen(bool o)
{
    QString fn(o? "mini/folder_open.png" : "mini/folder.png");
    setPixmap(0, kapp->iconLoader()->loadApplicationIcon(fn));
    KListViewItem::setOpen(o);
}


void ListViewFolderItem::clear()
{
    QListViewItem *child = firstChild();
    while (child)
        {
            QListViewItem *old = child;
            child = child->nextSibling();
            delete old;
        }
}


    
/*************************************/
/* Folder "KDevelop"                */
/*************************************/


/**
 * Here we specialize on a KDevelop book. The constructor takes
 * only the last part of the file name as argument and tries to
 * locate the file according to the locale set.
 * The constructor allows an argument expandable, but
 * the setOpen() implementation is currently nothing more than
 * a dirty hack.
 */
class DocTreeKDevelopBook : public ListViewBookItem
{
public:
    DocTreeKDevelopBook( KListViewItem *parent, const QString &text,
                         const QString &filename, bool expandable=false );
    virtual void setOpen(bool o);
private:
    void readSgmlIndex(FILE *f);
};


DocTreeKDevelopBook::DocTreeKDevelopBook(KListViewItem *parent, const QString &text,
                    const QString &filename, bool expandable)
    : ListViewBookItem(parent, text, filename)
{
    setExpandable(expandable);
}


void DocTreeKDevelopBook::readSgmlIndex(FILE *f)
{
    char buf[512];
    while (fgets(buf, sizeof buf, f))
        {
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
            new ListViewDocItem(this, title, path);
        }
}


void DocTreeKDevelopBook::setOpen(bool o)
{
    if (o && childCount() == 0)
        {
            FILE *f;
            if ( (f = fopen(ident(), "r")) != 0)
                {
                    readSgmlIndex(f);
                    fclose(f);
                }
            else
                setExpandable(false);
        }
    ListViewBookItem::setOpen(o);
}


class DocTreeKDevelopFolder : public ListViewFolderItem
{
public:
    DocTreeKDevelopFolder(DocTreeView *parent)
        : ListViewFolderItem(parent, i18n("KDevelop"))
        {}
    void refresh();
};


void DocTreeKDevelopFolder::refresh()
{
    ListViewFolderItem::clear();

    QString path = locate("appdata", "tools/documentation");
    KSimpleConfig docconfig(path);
    docconfig.setGroup("Contents");
    QStringList entries = docconfig.readListEntry("KDevelopEntries");
    KConfig *config = kapp->config();
    config->setGroup("DocTree");
    QStringList nonEntries = config->readListEntry("KDevelopNotShown");
    
    for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
        if (!nonEntries.contains(*it))
            {
                docconfig.setGroup("KDevelop-" + (*it));
                QString name = docconfig.readEntry("Name");
                QString filename = CToolClass::locatehtml("kdevelop/" + docconfig.readEntry("Path"));
                bool expandable = docconfig.readBoolEntry("Expandable");
                (void) new DocTreeKDevelopBook(this, name, filename, expandable);
                qDebug( "Insert %s", name.ascii() );
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
class DocTreeKDELibsBook : public ListViewBookItem
{
public:
    DocTreeKDELibsBook( KListViewItem *parent, const QString &text,
                        const QString &filename, const QString &idxfilename );
    virtual void setOpen(bool o);
private:
    void readKdoc2Index(FILE *f);
    QString idx_filename;
};


DocTreeKDELibsBook::DocTreeKDELibsBook(KListViewItem *parent, const QString &text,
                                       const QString &filename, const QString &idxfilename)
    : ListViewBookItem(parent, text, filename), idx_filename(idxfilename)
{
    if (!idxfilename.isNull())
        // If we have a kdoc2 index in either uncompressed
        // or compressed form, we read it in on demand.
        setExpandable(true);
}


void DocTreeKDELibsBook::readKdoc2Index(FILE *f)
{
    char buf[512];
    QString baseurl;
    while (fgets(buf, sizeof buf, f))
        {
            QString s = buf;
            if (s.left(11) == "<BASE URL=\"")
                {
                    int pos2 = s.find("\">", 11);
                    if (pos2 != -1)
                        baseurl = s.mid(11, pos2-11);
                }
            else if (s.left(9) == "<C NAME=\"")
                {
                    int pos1 = s.find("\" REF=\"", 9);
                    if (pos1 == -1)
                        continue;
                    int pos2 = s.find("\">", pos1+7);
                    if (pos2 == -1)
                        continue;
                    QString classname = s.mid(9, pos1-9);
                    QString filename = s.mid(pos1+7, pos2-(pos1+7));
                    new ListViewDocItem(this, classname,
                                        baseurl + "/" + filename);
                }
        }
    sortChildItems(0, true);
}


void DocTreeKDELibsBook::setOpen(bool o)
{
    if (o && childCount() == 0)
        {
            FILE *f;
            if ( (f = fopen(idx_filename, "r")) != 0)
                {
                    readKdoc2Index(f);
                    fclose(f);
                }
            else if ( (f = popen(QString("gzip -c -d ")
                                 + idx_filename + ".gz 2>/dev/null", "r")) != 0)
                {
                    readKdoc2Index(f);
                    pclose(f);
                }
            else
                setExpandable(false);
        }
    ListViewBookItem::setOpen(o);
}


class DocTreeKDELibsFolder : public ListViewFolderItem
{
public:
    DocTreeKDELibsFolder(DocTreeView *parent)
        : ListViewFolderItem(parent, i18n("Qt/KDE Libraries"))
        {}
    void refresh();
};


void DocTreeKDELibsFolder::refresh()
{
    ListViewFolderItem::clear();

    KConfig *config = kapp->config();
    config->setGroup("Doc_Location");
    QString kde_path = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    QString qt_path = config->readEntry("doc_qt", QT_DOCDIR);
    QString idx_path = config->readEntry("kdoc_index", KDOC_INDEXDIR);

    QString path = locate("appdata", "tools/documentation");
    KSimpleConfig docconfig(path);
    docconfig.setGroup("Contents");
    QStringList entries = docconfig.readListEntry("LibrariesEntries");
    config->setGroup("DocTree");
    QStringList nonEntries = config->readListEntry("LibrariesNotShown");
    
    for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
        if (!nonEntries.contains(*it))
            {
                docconfig.setGroup("Libraries-" + (*it));
                QString name = docconfig.readEntry("Name");
                QString filename = (*it == "qt")?
                    (qt_path + "/index.html") : (kde_path + "/" + (*it) + "/index.html");
                QString idxfilename = idx_path.isEmpty()?
                    QString::null : (idx_path + "/" + (*it) + ".kdoc");
                if (QFileInfo(filename).exists())
                    (void) new DocTreeKDELibsBook(this, name, filename, idxfilename);
                qDebug( "Insert %s", name.ascii() );
            }
}


/*************************************/
/* Folder "Documentation Base"       */
/*************************************/

#ifdef WITH_DOCBASE


class DocTreeDocbaseFolder : public ListViewFolderItem
{
public:
    DocTreeDocbaseFolder(DocTreeView *parent)
        : ListViewFolderItem(parent, i18n("Documentation Base"))
        { setExpandable(true); }
    virtual void setOpen(bool o);
    void refresh();
private:
    void readDocbaseFile(FILE *f);
};


void DocTreeDocbaseFolder::readDocbaseFile(FILE *f)
{
    char buf[512];
    QString title;
    bool html = false;
    while (fgets(buf, sizeof buf, f))
        {
            QString s = buf;
            if (s.right(1) == "\n")
                s.truncate(s.length()-1); // chop
            
            if (s.left(7) == "Title: ")
                title = s.mid(7, s.length()-7);
            else if (s.left(8) == "Format: ")
                html = s.find("HTML", 8, false) != -1;
            else if (s.left(7) == "Index: "
                     && html && !title.isEmpty())
                {
                    QString filename = s.mid(7, s.length()-7);
                    (void) new ListViewBookItem(this, title, filename);
                    break;
                }
            else if (s.left(9) == "Section: "
                     && s.find("programming", 9, false) == -1)
                break;
        }
}


void DocTreeDocbaseFolder::setOpen(bool o)
{
    if (o && childCount() == 0)
        {
            QDir d("/usr/share/doc-base");
            QStrListIterator it(*d.entryList("*", QDir::Files));
            for (; it.current(); ++it)
                {
                    FILE *f;
                    if ( (f = fopen(d.filePath(it.current()), "r")) != 0)
                        {
                            readDocbaseFile(f);
                            fclose(f);
                        }
                }
        }
    ListViewFolderItem::setOpen(o);
}


void DocTreeDocbaseFolder::refresh()
{
    ListViewFolderItem::clear();

    setOpen(isOpen()); // should reparse the doc-base
}

#endif


/*************************************/
/* Folder "Others"                  */
/*************************************/

class DocTreeOthersFolder : public ListViewFolderItem
{
public:
    DocTreeOthersFolder(DocTreeView *parent);
    void refresh();
};


DocTreeOthersFolder::DocTreeOthersFolder(DocTreeView *parent)
    : ListViewFolderItem(parent, i18n("Others"))
{}


void DocTreeOthersFolder::refresh()
{
    ListViewFolderItem::clear();

    KConfig *config = kapp->config();
    config->setGroup("DocTree");
    QStringList othersShownTitle = config->readListEntry("OthersShownTitle");
    QStringList othersShownURL = config->readListEntry("OthersShownURL");
    QStringList::Iterator it1 = othersShownTitle.begin();
    QStringList::Iterator it2 = othersShownURL.begin();
    for (; it1 != othersShownTitle.end() && it2 != othersShownURL.end(); ++it1, ++it2)
        (void) new ListViewBookItem(this, *it1, *it2);
}


/*************************************/
/* Folder "Current Project"          */
/*************************************/

class DocTreeProjectFolder : public ListViewFolderItem
{
public:
    DocTreeProjectFolder(DocTreeView *parent)
        : ListViewFolderItem(parent, i18n("Current Project")), project(0)
        {}
    void refresh();
    void setProject(CProject *prj)
        { project = prj; }
    
private:
    CProject *project;
};


void DocTreeProjectFolder::refresh()
{
    ListViewFolderItem::clear();

    setExpandable(false);
    if (project && project->valid)
        {
            setExpandable(true);

            (void) new ListViewBookItem(this, i18n("API documentation"), "internal:projectAPI");
            (void) new ListViewBookItem(this, i18n("User manual"), "internal:projectManual");
        }
}


/**************************************/
/* The DocTreeView itself              */
/**************************************/


DocTreeView::DocTreeView(QWidget *parent, const char *name)
    : KListView(parent, name)
{
    setRootIsDecorated(true);
    setSorting(-1);
    setFrameStyle(Panel | Sunken);
    setLineWidth(2); 
    header()->hide();
    addColumn("");
    
    folder_kdevelop = new DocTreeKDevelopFolder(this);
    folder_kdelibs  = new DocTreeKDELibsFolder(this);
#ifdef WITH_DOCBASE
    folder_docbase  = new DocTreeDocbaseFolder(this);
#endif
    folder_others   = new DocTreeOthersFolder(this);
    folder_project  = new DocTreeProjectFolder(this);

    folder_kdevelop->setOpen(true);
    folder_kdelibs->setOpen(true);
    projectOpened(0); // should perhaps be done in the constructors
    
    connect( this,
             SIGNAL(rightButtonPressed(QListViewItem*,const QPoint&,int)),
             SLOT(slotRightButtonPressed(QListViewItem*,const QPoint&,int)) );
    connect( this,
             SIGNAL(pressed(QListViewItem*)),
             SLOT(slotLeftButtonPressed(QListViewItem*)) );
}


DocTreeView::~DocTreeView()
{}


void DocTreeView::docPathChanged()
{
    folder_kdelibs->refresh();
}


void DocTreeView::projectOpened(CProject *prj)
{
    folder_project->setProject(prj);
    folder_kdevelop->refresh();
    folder_kdelibs->refresh();
    folder_others->refresh();
    folder_project->refresh();
}


void DocTreeView::projectClosed()
{
    folder_project->setProject(0);
}

void DocTreeView::slotLeftButtonPressed(QListViewItem *item)
{
    // We assume here that ALL (!) items in the list view
    // are KListViewItem's
    KListViewItem *kitem = static_cast<KListViewItem*>(item);
    if (!kitem)
        return;

    QString ident = kitem->ident();
    if (ident == "internal:projectAPI")
        emit projectAPISelected();
    else if (ident == "internal:projectManual")
        emit projectManualSelected();
    else if (!ident.isEmpty())
        emit fileSelected(kitem->ident());
}


void DocTreeView::slotRightButtonPressed(QListViewItem *item, const QPoint &p, int)
{
    contextItem = item;
    KPopupMenu pop(i18n("Documentation Tree"));
    pop.insertItem(i18n("Configure..."), this, SLOT(slotConfigure()));
    pop.exec(p);
}


void DocTreeView::slotConfigure()
{
    DocTreeConfigDialog::Page page;
    if (contextItem == folder_kdevelop || contextItem->parent() == folder_kdevelop)
        page = DocTreeConfigDialog::KDevelop;
    else if (contextItem == folder_kdelibs || contextItem->parent() == folder_kdelibs)
        page = DocTreeConfigDialog::Libraries;
    else
        page = DocTreeConfigDialog::Others;
    DocTreeConfigDialog dlg(page, this);
    if (dlg.exec() == QDialog::Accepted)
        {
            folder_kdevelop->refresh();
            folder_kdelibs->refresh();
            folder_others->refresh();
        }
}
