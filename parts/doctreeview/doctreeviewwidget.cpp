/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
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
#include <qvbox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevproject.h"

#include "doctreeviewwidget.h"
#include "doctreeviewpart.h"
#include "doctreeconfigwidget.h"
#include "misc.h"
#include "../../config.h"



class DocTreeItem : public QListViewItem
{
public:
    enum Type { Folder, Book, Doc };
    DocTreeItem( QListView *parent, Type type, const QString &text );
    DocTreeItem( DocTreeItem *parent, Type type, const QString &text );

    void setFileName(const QString &fn)
        { filename = fn; }
    virtual QString fileName()
        { return filename; }
    virtual void clear();
    
private:
    void init();
    Type typ;
    QString filename;
};


DocTreeItem::DocTreeItem(QListView *parent, Type type, const QString &text)
    : QListViewItem(parent, text), typ(type)
{
    init();
}


DocTreeItem::DocTreeItem(DocTreeItem *parent, Type type, const QString &text)
    : QListViewItem(parent, text), typ(type)
{
    init();
}


void DocTreeItem::init()
{
    QString icon;
    if (typ == Folder)
        icon = "folder";
    else if (typ == Book)
        icon = "contents";
    else
        icon = "document";
    setPixmap(0, SmallIcon(icon));
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


/*************************************/
/* Folder "Books"                    */
/*************************************/


/**
 * Currently the list of books is hardcoded. In the future,
 * the list should be collected (in analogy to the docindices
 * directory) from an installed list of tocs (which in
 * turn may be generated from texinfo or docbook sources
 * at build time.
 */
class DocTreeGeneralFolder : public DocTreeItem
{
public:
    DocTreeGeneralFolder(DocTreeViewWidget *parent)
        : DocTreeItem(parent, Folder, i18n("Books"))
        { setExpandable(true); }
    void refresh();
};


void DocTreeGeneralFolder::refresh()
{
    DocTreeItem::clear();

    DocTreeItem *stlbook = new DocTreeItem(this, Book, i18n("STL"));
    stlbook->setFileName(locate("html", "en/gideon/stl/index.html"));
    DocTreeItem *kde2book = new DocTreeItem(this, Book, i18n("KDE 2 Development"));
    kde2book->setFileName(locate("html", "en/gideon/kde2book/index.html"));
    DocTreeItem *libcbook = new DocTreeItem(this, Book, i18n("Libc"));
    libcbook->setFileName(locate("html", "en/gideon/libc/libc_toc.html"));
}


/*************************************/
/* Folder "Qt/KDE libraries"         */
/*************************************/


/**
 * Here we specialize on a Qt/kdelibs library. The constructor takes
 * the shown name and the index file name as argument.
 * The url of the book itself is read on demand.
 */
class DocTreeKDELibsBook : public DocTreeItem
{
public:
    DocTreeKDELibsBook( DocTreeItem *parent, const QString &name, const QString &idxfilename );
    ~DocTreeKDELibsBook();

    virtual QString fileName();
    virtual void setOpen(bool o);
    
private:
    void readContents();
    void readKdoc2Index(FILE *f);
    QString idx_filename;
};


DocTreeKDELibsBook::DocTreeKDELibsBook(DocTreeItem *parent, const QString &name, const QString &idxfilename)
    : DocTreeItem(parent, Book, name),
      idx_filename(idxfilename)
{
    setExpandable(true);
}


DocTreeKDELibsBook::~DocTreeKDELibsBook()
{}


QString DocTreeKDELibsBook::fileName()
{
    if (DocTreeItem::fileName().isNull())
        readContents();
    return DocTreeItem::fileName();
}


void DocTreeKDELibsBook::setOpen(bool o)
{
    //    kdDebug(9002) << (o? "Open kdelibs book" : "Close kdelibs book") << endl;
    if (o && DocTreeItem::fileName().isNull())
        readContents();
    DocTreeItem::setOpen(o);
}


void DocTreeKDELibsBook::readContents()
{
    FILE *f;
    bool success = false;
    if (idx_filename.right(3) != QString::fromLatin1(".gz")) {
        if ( (f = fopen(idx_filename, "r")) != 0) {
            readKdoc2Index(f);
            fclose(f);
            success = true;
        }
    } else {
        if ( (f = popen(QString("gzip -c -d ")
                        + idx_filename + " 2>/dev/null", "r")) != 0) {
            readKdoc2Index(f);
            pclose(f);
            success = true;
        }
    }
    setExpandable(success);
}


void DocTreeKDELibsBook::readKdoc2Index(FILE *f)
{
    char buf[1024];
    DocTreeItem *classItem = 0;
    int pos0;
    QString classname, membername, filename;
    
    while (fgets(buf, sizeof buf, f)) {
        QString s = buf;
        if (s.left(pos0=11) == "<BASE URL=\"") {
            int pos2 = s.find("\">", pos0);
            if (pos2 != -1)
                setFileName(s.mid(pos0, pos2-pos0));
        }
        else if (s.left(pos0=9) == "<C NAME=\"") {
            int pos1 = s.find("\" REF=\"", pos0);
            if (pos1 == -1)
                continue;
                int pos2 = s.find("\">", pos1+7);
                if (pos2 == -1)
                    continue;
                classname = s.mid(pos0, pos1-pos0);
                filename = s.mid(pos1+7, pos2-(pos1+7));
                filename.replace(QRegExp("::"), "__");
                classItem = new DocTreeItem(this, Doc, classname);
                classItem->setFileName(DocTreeItem::fileName() + "/" + filename);
        }
      else if (s.left(pos0=9) == "<M NAME=\"" || s.left(pos0=10) == "<ME NAME=\"")
          {
              int pos1 = s.find("\" REF=\"", pos0);
              if (pos1 == -1)
                  continue;
              int pos2 = s.find("\">", pos1+7);
              if (pos2 == -1)
                  continue;

              // Long version: membername = classname + "::" + s.mid(pos0, pos1-pos0);
              membername = s.mid(pos0, pos1-pos0);
              filename = s.mid(pos1+7, pos2-(pos1+7));
              filename.replace(QRegExp("::"), "__");
              if (classItem) {
                  DocTreeItem *item = new DocTreeItem(classItem, Doc, membername);
                  item->setFileName(DocTreeItem::fileName() + "/" + filename);
              }
          }
    }
    
    sortChildItems(0, true);
}


class DocTreeKDELibsFolder : public DocTreeItem
{
public:
    DocTreeKDELibsFolder(DocTreeViewWidget *parent)
        : DocTreeItem(parent, Folder, i18n("Qt/KDE Libraries"))
        { setExpandable(true); }
    void refresh();
};


void DocTreeKDELibsFolder::refresh()
{
    DocTreeItem::clear();

    QStringList itemNames, fileNames, hiddenNames;
    DocTreeViewTool::getAllLibraries(&itemNames, &fileNames);
    DocTreeViewTool::getHiddenLibraries(&hiddenNames);

    QStringList::Iterator it1, it2;
    for (it1 = itemNames.begin(), it2 = fileNames.begin();
         it1 != itemNames.end() && it2 != fileNames.end();
         ++it1, ++it2)
        if (!hiddenNames.contains(*it2)) {
            (void) new DocTreeKDELibsBook(this, *it1, *it2);
        }
}
    

/*************************************/
/* Folder "Documentation Base"       */
/*************************************/

#ifdef WITH_DOCBASE


class DocTreeDocbaseFolder : public DocTreeItem
{
public:
    DocTreeDocbaseFolder(DocTreeViewWidget *parent);
    ~DocTreeDocbaseFolder();
    virtual void setOpen(bool o);
private:
    void readDocbaseFile(FILE *f);
};


DocTreeDocbaseFolder::DocTreeDocbaseFolder(DocTreeViewWidget *parent)
    : DocTreeItem(parent, Folder, i18n("Documentation Base"))
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
            DocTreeItem *item = new DocTreeItem(this, Doc, title);
            item->setFileName(filename);
            break;
        }
        else if (s.left(9) == "Section: "
                 && s.find("programming", 9, false) == -1)
            break;
    }
}


void DocTreeDocbaseFolder::setOpen(bool o)
{
    //    kdDebug(9002) << (o? "Open docbase folder" : "Close docbase folder") << endl;
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
/* Folder "Bookmarks"                */
/*************************************/

class DocTreeBookmarksFolder : public DocTreeItem
{
public:
    DocTreeBookmarksFolder(DocTreeViewWidget *parent);
    void refresh();
};


DocTreeBookmarksFolder::DocTreeBookmarksFolder(DocTreeViewWidget *parent)
    : DocTreeItem(parent, Folder, i18n("Bookmarks"))
{}


void DocTreeBookmarksFolder::refresh()
{
    DocTreeItem::clear();

    QStringList othersTitle, othersURL;
    DocTreeViewTool::getBookmarks(&othersTitle, &othersURL);
    QStringList::Iterator it1, it2;
    for (it1 = othersTitle.begin(), it2 = othersURL.begin();
         it1 != othersTitle.end() && it2 != othersURL.end();
         ++it1, ++it2) {
        DocTreeItem *item = new DocTreeItem(this, Book, *it1);
        item->setFileName(*it2);
    }
}


/*************************************/
/* Folder "Current Project"          */
/*************************************/

class DocTreeProjectFolder : public DocTreeItem
{
public:
    DocTreeProjectFolder(DocTreeViewWidget *parent);
    void setProject(KDevProject *project)
        { m_project = project; }
    void refresh();
    
private:
    KDevProject *m_project;
};


DocTreeProjectFolder::DocTreeProjectFolder(DocTreeViewWidget *parent)
    : DocTreeItem(parent, Folder, i18n("Current Project"))
{}


void DocTreeProjectFolder::refresh()
{
    DocTreeItem::clear();

    if (m_project) {
        // API documentation
        QDir apidir(m_project->projectDirectory() + "/srcdoc");
        if (apidir.exists()) {
            QStringList entries = apidir.entryList("*", QDir::Dirs);
            for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
                QString filename = apidir.filePath(*it) + "/index.html";
                if (!QFileInfo(filename).exists())
                    continue;
                DocTreeItem *item = new DocTreeItem(this, Book, i18n("API of %1").arg(*it));
                item->setFileName(filename);
            }
        }
        // User documentation
        QDir userdir(m_project->projectDirectory() + "/doc");
        if (userdir.exists()) {
            QStringList entries = userdir.entryList("*", QDir::Dirs);
            for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
                QString filename = userdir.filePath(*it) + "/HTML/index.html";
                if (!QFileInfo(filename).exists())
                    continue;
                DocTreeItem *item = new DocTreeItem(this, Book, i18n("Userdoc for %1").arg(*it));
                item->setFileName(filename);
            }
        }
    }
    
    if (!firstChild())
        setExpandable(false);
}


/**************************************/
/* The DocTreeViewWidget itself           */
/**************************************/


DocTreeViewWidget::DocTreeViewWidget(DocTreeViewPart *part)
    : KListView(0, "doc tree widget")
{
    setFocusPolicy(ClickFocus);
    setRootIsDecorated(true);
    setSorting(-1);
    setFrameStyle(Panel | Sunken);
    setLineWidth(2); 
    header()->hide();
    addColumn("");
    
    folder_bookmarks = new DocTreeBookmarksFolder(this);
    folder_bookmarks->refresh();
    folder_project   = new DocTreeProjectFolder(this);
#ifdef WITH_DOCBASE
    folder_docbase   = new DocTreeDocbaseFolder(this);
#endif
    folder_kdelibs   = new DocTreeKDELibsFolder(this);
    folder_kdelibs->refresh();
    folder_general   = new DocTreeGeneralFolder(this);
    folder_general->refresh();

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    m_part = part;
}


DocTreeViewWidget::~DocTreeViewWidget()
{}


void DocTreeViewWidget::slotItemExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // We assume here that ALL items in the list view
    // are DocTreeItem's
    DocTreeItem *dtitem = static_cast<DocTreeItem*>(item);
    
    QString ident = dtitem->fileName();
    if (ident.isEmpty())
        return;
    
    m_part->core()->gotoDocumentationFile(ident);
}


void DocTreeViewWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;
    contextItem = item;
    KPopupMenu pop(i18n("Documentation Tree"));
    pop.insertItem(i18n("Customize..."), this, SLOT(slotConfigure()));
    pop.exec(p);
}


void DocTreeViewWidget::slotConfigure()
{
    DocTreeConfigWidget::Page page;
    if (contextItem == folder_kdelibs || contextItem->parent() == folder_kdelibs)
        page = DocTreeConfigWidget::Libraries;
    else
        page = DocTreeConfigWidget::Bookmarks;

    KDialogBase dlg(KDialogBase::TreeList, i18n("Customize documentation tree"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this,
                    "customization dialog");
    QVBox *vbox = dlg.addVBoxPage(i18n("Documentation tree"));
    DocTreeConfigWidget *w = new DocTreeConfigWidget(this, vbox, "doctreeview config widget");
    w->showPage(page);
    connect(&dlg, SIGNAL(okClicked()), w, SLOT(accept()));
    dlg.exec();
}


void DocTreeViewWidget::configurationChanged()
{
    QTimer::singleShot(0, this, SLOT(refresh()));
}


void DocTreeViewWidget::refresh()
{
    folder_general->refresh();
    folder_kdelibs->refresh();
    folder_bookmarks->refresh();
}


void DocTreeViewWidget::projectChanged(KDevProject *project)
{
    folder_project->setProject(project);
    folder_project->refresh();
}


QString DocTreeViewWidget::locatehtml(const QString &fileName)
{

    QString path = locate("html", KGlobal::locale()->language() + '/' + fileName);
    if (path.isNull())
       path = locate("html", "default/" + fileName);

    return path;
}

#include "doctreeviewwidget.moc"
