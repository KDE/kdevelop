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
#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpopmenu.h>
#include "cdoctreepropdlg.h"
#include "cproject.h"
#include "doctreeview.h"
#include <stdio.h>


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
    virtual void refresh();
};


ListViewFolderItem::ListViewFolderItem(KListView *parent, const char *text)
    : KListViewItem(parent, text, "")
{
    setPixmap(0, Icon("mini/folder.xpm"));
}


void ListViewFolderItem::refresh()
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
    DocTreeKDevelopBook( KListViewItem *parent, const char *text,
			 const char *filename, bool expandable=false )
	: ListViewBookItem(parent, text, locatehtml(filename))
	{ setExpandable(expandable); }
    virtual void setOpen(bool o);
private:
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


void DocTreeKDevelopBook::setOpen(bool o)
{
    if (o && childCount() == 0)
	{
	    // Preliminary hack, only for user manual
	    if (ident().find("kdevelop/index.html") != -1)
		{
		    QString pathbase = ident().left(ident().length()-5);
		    QString path;
		    path = pathbase + "-1.html";
		    new ListViewDocItem(this, i18n("Introduction"), path);
		    path = pathbase + "-2.html";
		    new ListViewDocItem(this, i18n("Installation"), path);
		    path = pathbase + "-3.html";
		    new ListViewDocItem(this, i18n("Programs"), path);
		    path = pathbase + "-4.html";
		    new ListViewDocItem(this, i18n("Development with KDevelop"), path);
		    path = pathbase + "-5.html";
		    new ListViewDocItem(this, i18n("Overview"), path);
		    path = pathbase + "-6.html";
		    new ListViewDocItem(this, i18n("The Help System"), path);
		    path = pathbase + "-7.html";
		    new ListViewDocItem(this, i18n("Working with the Editor"), path);
		    path = pathbase + "-9.html";
		    new ListViewDocItem(this, i18n("Projects"), path);
		    path = pathbase + "-9.html";
		    new ListViewDocItem(this, i18n("Build Settings"), path);
		    path = pathbase + "-10.html";
		    new ListViewDocItem(this, i18n("The Class Browser"), path);
		    path = pathbase + "-11.html";
		    new ListViewDocItem(this, i18n("The Dialog Editor"), path);
		    path = pathbase + "-12.html";
		    new ListViewDocItem(this, i18n("General Configuration"), path);
		    path = pathbase + "-13.html";
		    new ListViewDocItem(this, i18n("Questions and Answers"), path);
		    path = pathbase + "-14.html";
		    new ListViewDocItem(this, i18n("Authors"), path);
		    path = pathbase + "-15.html";
		    new ListViewDocItem(this, i18n("Thanks"), path);
		    path = pathbase + "-16.html";
		    new ListViewDocItem(this, i18n("Copyright"), path);
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

    (void) new DocTreeKDevelopBook(this, i18n("User Manual"),
				   "index.html", true);
    (void) new DocTreeKDevelopBook(this, i18n("Programming Handbook"),
				   "programming.html");
    (void) new DocTreeKDevelopBook(this, i18n("Tutorials"),
				   "tutorial.html");
    (void) new DocTreeKDevelopBook(this, i18n("KDE Library Reference"),
				   "kde_libref.html");
    (void) new DocTreeKDevelopBook(this, i18n("C/C++ Reference"),
				   "c-c++-reference.html");
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
    void readKdoc2Index(FILE *f);
    virtual void setOpen(bool o);
private:
    static QString locatehtml(const char *libname);
    QString idx_filename;
};


DocTreeKDELibsBook::DocTreeKDELibsBook( KListViewItem *parent, const char *text,
					const char *libname )
    : ListViewBookItem(parent, text, locatehtml(libname))
{
    KConfig *config = kapp->getConfig();
    config->setGroup("Doc_Location");
    QString idx_path = config->readEntry("kdoc_index", KDOC_INDEXDIR);
    if (!idx_path.isEmpty())
	{
	    // If we have a kdoc2 index in either uncompressed
	    // or compressed form, we read it in on demand.
	    idx_filename = idx_path + "/";
	    if (!libname)
		idx_filename += "qt";
	    else
		idx_filename += libname;
	    idx_filename += ".kdoc";
	    setExpandable(true);
	}
}


QString DocTreeKDELibsBook::locatehtml(const char *libname)
{
    KConfig *config = kapp->getConfig();
    config->setGroup("Doc_Location");
    QString kde_path = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    QString qt_path = config->readEntry("doc_qt", QT_DOCDIR);

    if (!libname)
	return qt_path + "/index.html";
    else
	return kde_path + "/" + libname + "/index.html";
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
	}
    ListViewBookItem::setOpen(o);
}


class DocTreeKDELibsFolder : public ListViewFolderItem
{
public:
    DocTreeKDELibsFolder(DocTreeView *parent)
	: ListViewFolderItem(parent, i18n("Qt/KDE Libraries"))
	{}
    virtual void refresh();
};


void DocTreeKDELibsFolder::refresh()
{
    ListViewFolderItem::refresh();
    
    (void) new DocTreeKDELibsBook(this, i18n("Qt Library"),         0);
    (void) new DocTreeKDELibsBook(this, i18n("KDE Core Library"),   "kdecore");
    (void) new DocTreeKDELibsBook(this, i18n("KDE UI Library"),     "kdeui");
    (void) new DocTreeKDELibsBook(this, i18n("KDE KFile Library"),  "kfile");
    (void) new DocTreeKDELibsBook(this, i18n("KDE HTMLW Library"),  "khtmlw");
    (void) new DocTreeKDELibsBook(this, i18n("KDE KAB Library"),    "kab");
    (void) new DocTreeKDELibsBook(this, i18n("KDE KSpell Library"), "kspell");
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
    ~DocTreeOthersFolder();
    virtual void refresh();
    void handleRightButtonPressed(QListViewItem *item,
				  const QPoint &p);
private:
    KPopupMenu *others_pop;
    KPopupMenu *doc_pop;

};


DocTreeOthersFolder::DocTreeOthersFolder(DocTreeView *parent)
    : ListViewFolderItem(parent, i18n("Others"))
{
    others_pop = new KPopupMenu(i18n("Others"));
    others_pop->insertItem(i18n("Add Entry..."),
			   parent, SLOT(slotAddDocumentation()));
    
    doc_pop = new KPopupMenu(i18n("Others"));
    doc_pop->insertItem(i18n("Add Entry..."),
			parent, SLOT(slotAddDocumentation()));
    doc_pop->insertItem(i18n("Remove Entry"),
			parent, SLOT(slotRemoveDocumentation()));
    doc_pop->insertSeparator();
    doc_pop->insertItem(i18n("Properties..."),
			parent, SLOT(slotDocumentationProp()));
}


DocTreeOthersFolder::~DocTreeOthersFolder()
{
    delete others_pop;
    delete doc_pop;
}


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
}


void DocTreeOthersFolder::handleRightButtonPressed(QListViewItem *item,
						   const QPoint &p)
{
    if (item == this)
	others_pop->popup(p);
    else if (item->parent() == this)
	doc_pop->popup(p);
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
	{ project = prj; }
    
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
    folder_project->setProject(prj);
    folder_kdevelop->refresh();
    folder_kdelibs->refresh();
    folder_others->refresh();
    folder_project->refresh();
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
    if (!kitem->ident().isEmpty())
	emit fileSelected(kitem->ident());
}
