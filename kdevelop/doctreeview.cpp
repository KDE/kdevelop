/***************************************************************************
                             doctreeview.cpp
                             -------------------

    begin                : 3 Oct 1998
    copyright            : (C) 1998,1999 by Sandy Meier
    email                : smeier@rz.uni-potsdam.de
    copyright            : (C) 1999 The KDevelop Team
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
#include <qlist.h>
#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpopmenu.h>
#include "cdoctreepropdlg.h"
#include "cproject.h"
#include "doctreeview.h"
#include <stdio.h>

// There are still problems with BEN's workaround in certain circumstances
// so for the time being I've used my solution. jbb 07-02-2000
//#define BEN_QLISTVIEW_BYPASS	//define this symbol for using my solution to the docview bug
                                //undefine it to use jbb's

/**
 * A list view item that is decorated with a doc icon.
 * This typically represents a section in a manual.
 */
class ListViewDocItem : public KListViewItem
{
public:
    ListViewDocItem( KListViewItem *parent,
                     const char *text, const char *filename );
};


ListViewDocItem::ListViewDocItem(KListViewItem *parent,
                                 const char *text, const char *filename)
    : KListViewItem(parent, text, filename)
{
    setPixmap(0, Icon("mini/mini-doc.xpm"));
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
                      const char *text, const char *filename );
    virtual void setOpen(bool o);
};


ListViewBookItem::ListViewBookItem(KListViewItem *parent,
                                   const char *text, const char *filename)
    : KListViewItem(parent, text, filename)
{
    setOpen(false);
}


void ListViewBookItem::setOpen(bool o)
{
    setPixmap(0, o? Icon("mini/mini-book2.xpm") : Icon("mini/mini-book1.xpm"));
    KListViewItem::setOpen(o);
}


/**
 * A list view item that is decorated with a folder icon.
 * This typically represents one major part in the help tree.
 * Its contents can be refresh()d. The default implementation
 * simply deletes all children.
 */
class ListViewFolderItem : public KListViewItem
{
public:
    ListViewFolderItem( KListView *parent, const char *text );
    virtual void setOpen(bool o);
    virtual void refresh();
};


ListViewFolderItem::ListViewFolderItem(KListView *parent, const char *text)
    : KListViewItem(parent, text, "")
{
    setOpen(false);
}


void ListViewFolderItem::setOpen(bool o)
{
    setPixmap(0, o? Icon("mini/folder_open.xpm") : Icon("mini/folder.xpm"));
    KListViewItem::setOpen(o);
}


void ListViewFolderItem::refresh()
{
#ifdef BEN_QLISTVIEW_BYPASS
    QListViewItem *child = firstChild();
    while (child)
        {
            QListViewItem *old = child;
            child = child->nextSibling();
            delete old;
        }
#endif //BEN_QLISTVIEW_BYPASS
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
    DocTreeKDevelopBook( KListViewItem *parent, const char *text,
                         const char *filename, bool expandable=false )
        : ListViewBookItem(parent, text, locatehtml(filename))
        { setExpandable(expandable); }
    virtual void setOpen(bool o);
private:
    void readSgmlIndex(FILE *f);
    static QString locatehtml(const char *filename);
};


QString DocTreeKDevelopBook::locatehtml(const char *filename)
{
    QString pathbase = KApplication::kde_htmldir() + "/";
    QString path = pathbase + klocale->language() + "/kdevelop/" + filename;
    if (!QFileInfo(path).exists())
        path = pathbase + "default/kdevelop/" + filename;
    return path;
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
            {
              setExpandable(false);
              o=false; // the book icon remains closed
            }
        }
    ListViewBookItem::setOpen(o);
}


class DocTreeKDevelopFolder : public ListViewFolderItem
{
public:
    DocTreeKDevelopFolder(DocTreeView *parent)
        : ListViewFolderItem(parent, i18n("KDevelop"))
        {}
    virtual void refresh();
};


void DocTreeKDevelopFolder::refresh()
{
    ListViewFolderItem::refresh();

    (void) new DocTreeKDevelopBook(this, i18n("Welcome !"),
                                   "welcome/index.html", false);
    (void) new DocTreeKDevelopBook(this, i18n("User Manual"),
                                   "index.html", true);
    (void) new DocTreeKDevelopBook(this, i18n("Programming Handbook"),
                                   "programming/index.html", true);
    (void) new DocTreeKDevelopBook(this, i18n("Tutorials"),
                                   "tutorial/index.html", true);
    (void) new DocTreeKDevelopBook(this, i18n("KDE Library Reference"),
                                   "kde_libref/index.html", true);
    (void) new DocTreeKDevelopBook(this, i18n("KDE 2 Developer Guide"),
                                   "addendum/index.html", true);
    (void) new DocTreeKDevelopBook(this, i18n("C/C++ Reference"),
                                   "reference/C/cref.html");

    //horrible hack to counter the QListView bug DO NOT CHANGE without thinking about it
    //and looking closely at the implementation of QListView, expacially how are the pointers
    //in QListView::d->drawables are managed!!!   Benoit Cerrina <benoit.cerrina@writeme.com>
    listView()->setOpen(this, !isOpen());
    listView()->setOpen(this, !isOpen());
    //end of the horrible hack
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
    DocTreeKDELibsBook( KListViewItem *parent, const char *text,
                        const char *libname);
    void relocatehtml();
    virtual void setOpen(bool o);
private:
    int readKdoc2Index(FILE *f);
    static QString locatehtml(const char *libname);
    QString name;
    QString idx_filename;
};

DocTreeKDELibsBook::DocTreeKDELibsBook( KListViewItem *parent, const char *text,
                                        const char *libname )
    : ListViewBookItem(parent, text, locatehtml(libname)), name(libname)
{
    KConfig *config = kapp->getConfig();
    config->setGroup("Doc_Location");

#ifdef WITH_KDOC2
    QString doc_dir, idx_path;
    doc_dir = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    idx_path= doc_dir + "/kdoc-reference";
    if (!doc_dir.isEmpty())
        {
            // If we have a kdoc2 index in either uncompressed
            // or compressed form, we read it in on demand.
            idx_filename = idx_path + "/";
            if (!libname)
                idx_filename += "qt";
            else
                idx_filename += libname;
            idx_filename += ".kdoc";
            setExpandable(QFile::exists(idx_filename) || QFile::exists(idx_filename+".gz"));
        }
#endif

}

void DocTreeKDELibsBook::relocatehtml()
{
  setIdent(locatehtml(name));
}

QString DocTreeKDELibsBook::locatehtml(const char *libname)
{
    KConfig *config = kapp->getConfig();
    config->setGroup("Doc_Location");
    QString kde_path = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    QString qt_path = config->readEntry("doc_qt", QT_DOCDIR);

    if (!libname)
    {
        if (qt_path.right(1) != "/")
          qt_path= qt_path+"/";
        return qt_path + "index.html";
    }
    else
    {
        if (kde_path.right(1) != "/")
          kde_path= kde_path+"/";
        return kde_path + libname + "/index.html";
    }
}

int DocTreeKDELibsBook::readKdoc2Index(FILE *f)
{
    char buf[512];
    int count=0;
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
  		    count++;
                }
        }
    sortChildItems(0, true);
    return count;
}


void DocTreeKDELibsBook::setOpen(bool o)
{
  int count=0;
    if (o && childCount() == 0)
        {
            FILE *f;
            if ( (f = fopen(idx_filename, "r")) != 0)
                {
                    count=readKdoc2Index(f);
                    fclose(f);
                }
            else if ( (f = popen(QString("gzip -c -d ")
                                 + idx_filename + ".gz 2>/dev/null", "r")) != 0)
                {
                    count=readKdoc2Index(f);
                    pclose(f);
                }

            if (count==0)
            {
              setExpandable(false);
              o=false;  // the book icons remains closed
            }
        }
    ListViewBookItem::setOpen(o);
}


class DocTreeKDELibsFolder : public ListViewFolderItem
{
    QList<DocTreeKDELibsBook> list;
public:
    DocTreeKDELibsFolder(DocTreeView *parent)
        : ListViewFolderItem(parent, i18n("Qt/KDE Libraries"))
        { list.setAutoDelete(false);}


    void changePathes();
    virtual void refresh();
};

void DocTreeKDELibsFolder::changePathes()
{
    DocTreeKDELibsBook *child;
    for ( child=list.first(); child != 0; child=list.next())
    {
       child->relocatehtml();
    }
}

void DocTreeKDELibsFolder::refresh()
{
    ListViewFolderItem::refresh();
    list.clear();
    list.append(new DocTreeKDELibsBook(this, i18n("Qt Library"),         0));
    list.append(new DocTreeKDELibsBook(this, i18n("KDE Core Library"),   "kdecore"));
    list.append(new DocTreeKDELibsBook(this, i18n("KDE UI Library"),     "kdeui"));
    list.append(new DocTreeKDELibsBook(this, i18n("KDE KFile Library"),  "kfile"));
    list.append(new DocTreeKDELibsBook(this, i18n("KDE HTMLW Library"),  "khtmlw"));
    list.append(new DocTreeKDELibsBook(this, i18n("KDE KAB Library"),    "kab"));
    list.append(new DocTreeKDELibsBook(this, i18n("KDE KSpell Library"), "kspell"));
//    setOpen(false);
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
    virtual void refresh();
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
    ListViewFolderItem::refresh();

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
    virtual void refresh();
    void handleRightButtonPressed(QListViewItem *item,
                                  const QPoint &p);
};


DocTreeOthersFolder::DocTreeOthersFolder(DocTreeView *parent)
    : ListViewFolderItem(parent, i18n("Others"))
{}


void DocTreeOthersFolder::refresh()
{
    QStrList others;
    
    ListViewFolderItem::refresh();

    KConfig *config = kapp->getConfig();
    config->setGroup("Other_Doc_Location");
    config->readListEntry("others_list", others);
    config->setGroup("Other_Doc_Location");
    for (char *s = others.first(); s != 0; s = others.next())
        {
            QString filename = config->readEntry(s);
            (void) new ListViewBookItem(this, s, filename);
        }
    //setOpen(false);
}


void DocTreeOthersFolder::handleRightButtonPressed(QListViewItem *item,
                                                   const QPoint &p)
{
    if (item == this)
        {
            KPopupMenu pop(i18n("Others"));
            pop.insertItem(i18n("Add Entry..."),
                           listView(), SLOT(slotAddDocumentation()));
            pop.exec(p);
        }
    else if (item->parent() == this)
        {
            KPopupMenu pop(i18n("Others"));
            pop.insertItem(i18n("Add Entry..."),
                           listView(), SLOT(slotAddDocumentation()));
            pop.insertItem(i18n("Remove Entry"),
                           listView(), SLOT(slotRemoveDocumentation()));
            pop.insertSeparator();
            pop.insertItem(i18n("Properties..."),
                           listView(), SLOT(slotDocumentationProp()));
            pop.exec(p);
        }
}
    

/*************************************/
/* Folder "Current Project"         */
/*************************************/

class DocTreeProjectFolder : public ListViewFolderItem
{
public:
    DocTreeProjectFolder(DocTreeView *parent)
        : ListViewFolderItem(parent, i18n("Current Project")), project(0)
        {}
    virtual void refresh();
    void setProject(CProject *prj)
        {
        	project = prj;
        }
    
private:
    CProject *project;
};


void DocTreeProjectFolder::refresh()
{
    ListViewFolderItem::refresh();

    setExpandable(false);
    if (project && project->valid)
        {
            setExpandable(true);

            (void) new ListViewBookItem(this, i18n("API documentation"),
                                        /* strange!? */ "API-Documentation");
            (void) new ListViewBookItem(this, i18n("User manual"),
                                        /* strange!? */ "User-Manual");
//                setOpen(false);
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
    
    connect( this,
             SIGNAL(rightButtonPressed(QListViewItem*,const QPoint&,int)),
             SLOT(slotRightButtonPressed(QListViewItem*,const QPoint&,int)) );
    connect( this,
             SIGNAL(selectionChanged(QListViewItem*)),
             SLOT(slotSelectionChanged(QListViewItem *)) );
}


DocTreeView::~DocTreeView()
{}


QString DocTreeView::selectedText()
{
    QString text;
    if (currentItem())
        text = currentItem()->text(0);
    return text;
}


void DocTreeView::refresh(CProject *prj)
{
#ifndef BEN_QLISTVIEW_BYPASS	
	clear();

    folder_kdevelop = new DocTreeKDevelopFolder(this);
    folder_kdelibs  = new DocTreeKDELibsFolder(this);
//#ifdef WITH_DOCBASE
//    folder_docbase  = new DocTreeDocbaseFolder(this);
//#endif
    folder_others   = new DocTreeOthersFolder(this);
    folder_project  = new DocTreeProjectFolder(this);

    folder_kdevelop->setOpen(true);
    folder_kdelibs->setOpen(true);
#endif //BEN_QLISTVIEW_BYPASS
    folder_project->setProject(prj);
    folder_kdevelop->refresh();
    folder_kdelibs->refresh();
    folder_others->refresh();
    folder_project->refresh();

}

void DocTreeView::changePathes()
{
    folder_kdelibs->changePathes();
}

void DocTreeView::slotRightButtonPressed(QListViewItem *item,
                                         const QPoint &p, int)
{
    if (!item)
        return;

    setSelected(item, true);

    // Not very oo style, but works for the first
    // (as long as other folders don't use right-clicks)
    folder_others->handleRightButtonPressed(item, p);
}


void DocTreeView::slotAddDocumentation()
{
    CDocTreePropDlg dlg;
    dlg.setCaption(i18n("Add Entry..."));
    if (!dlg.exec())
        return;
    
    KConfig *config = kapp->getConfig();
    config->setGroup("Other_Doc_Location");
    QStrList others;
    config->readListEntry("others_list", others);
    
    // find the correct place and add the entry to the list
    if (currentItem() == folder_others) 
        others.insert(0, dlg.name_edit->text());
    else
        {
            int pos = others.find(currentItem()->text(0));
            others.insert(pos+1, dlg.name_edit->text());
        }
    //write the list
    config->writeEntry("others_list", others);
    // write the props
    config->writeEntry(dlg.name_edit->text(), dlg.file_edit->text());
    config->sync();

    folder_others->refresh();
}


void DocTreeView::slotRemoveDocumentation()
{
    KConfig *config = kapp->getConfig();
    config->setGroup("Other_Doc_Location");
    QStrList others;
    
    config->readListEntry("others_list", others);
    others.remove(currentItem()->text(0));
    config->writeEntry("others_list", others);
    config->sync();
    
    folder_others->refresh();
}


void DocTreeView::slotDocumentationProp()
{
    KConfig *config = kapp->getConfig();
    config->setGroup("Other_Doc_Location");

    QString name = currentItem()->text(0);
    QString filename = config->readEntry(name);
    
    CDocTreePropDlg dlg;
    dlg.setCaption(i18n("Properties..."));
    dlg.name_edit->setText(name);
    dlg.name_edit->setEnabled(false);
    dlg.file_edit->setText(filename);
    
    if (!dlg.exec())
        return;
    
    config->setGroup("Other_Doc_Location");
    config->writeEntry(name, dlg.file_edit->text());
    config->sync();

    folder_others->refresh();
}


void DocTreeView::slotSelectionChanged(QListViewItem *item)
{
    // We assume here that ALL (!) items in the list view
    // are KListViewItem's
    KListViewItem *kitem = static_cast<KListViewItem*>(item);
    if (kitem && !kitem->ident().isEmpty())
    {
      QString item=kitem->ident();
      // strip file: if present....
      //  e.g. qt2kdoc inserts it
      if (item.left(5)=="file:")
	item=item.mid(5, item.length());
      emit fileSelected(item);
    }
}

