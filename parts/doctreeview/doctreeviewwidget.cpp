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

#include "doctreeviewwidget.h"

#include <stdio.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qvbox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"

#include "../../config.h"
#include "misc.h"
#include "doctreeviewfactory.h"
#include "doctreeviewpart.h"
#include "doctreeconfigwidget.h"


class DocTreeItem : public QListViewItem
{
public:
    enum Type { Folder, Book, Doc };
    DocTreeItem( QListView *parent, Type type, const QString &text, const QString &context );
    DocTreeItem( DocTreeItem *parent, Type type, const QString &text, const QString &context);

    void setFileName(const QString &fn)
        { filename = fn; }
    virtual QString fileName()
        { return filename; }
    virtual void clear();
    virtual QString context() const { return m_context; }
    
private:
    void init();
    Type typ;
    QString filename, m_context;
};


DocTreeItem::DocTreeItem(QListView *parent, Type type, const QString &text, const QString &context)
    : QListViewItem(parent, text), typ(type), m_context(context)
{
    init();
}


DocTreeItem::DocTreeItem(DocTreeItem *parent, Type type, const QString &text, const QString &context)
    : QListViewItem(parent, text), typ(type), m_context(context)
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
    DocTreeKDELibsBook( DocTreeItem *parent, const QString &name, const QString &idxfilename, const QString &context);
    ~DocTreeKDELibsBook();

    virtual QString fileName();
    virtual void setOpen(bool o);
    
private:
    void readContents();
    void readKdoc2Index(FILE *f);
    QString idx_filename;
};


DocTreeKDELibsBook::DocTreeKDELibsBook(DocTreeItem *parent, const QString &name, const QString &idxfilename, const QString &context)
    : DocTreeItem(parent, Book, name, context),
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
                classItem = new DocTreeItem(this, Doc, classname, context());
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
                  DocTreeItem *item = new DocTreeItem(classItem, Doc, membername, context());
                  item->setFileName(DocTreeItem::fileName() + "/" + filename);
              }
          }
    }
    
    sortChildItems(0, true);
}


class DocTreeKDELibsFolder : public DocTreeItem
{
public:
    DocTreeKDELibsFolder(DocTreeViewWidget *parent, const QString &context)
        : DocTreeItem(parent, Folder, i18n("Qt/KDE Libraries"), context)
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
            (void) new DocTreeKDELibsBook(this, *it1, *it2, context());
        }
}
    

/***************************************/
/* Folder from the 'tocs' resource dir */
/***************************************/

class DocTreeTocFolder : public DocTreeItem
{
public:
    DocTreeTocFolder(DocTreeViewWidget *parent, const QString &fileName, const QString &context);
    ~DocTreeTocFolder();

    QString tocName() const
    { return toc_name; }

private:
    QString toc_name;
};


DocTreeTocFolder::DocTreeTocFolder(DocTreeViewWidget *parent, const QString &fileName, const QString &context)
    : DocTreeItem(parent, Folder, fileName, context)
{
    QFileInfo fi(fileName);
    toc_name = fi.baseName();
    
    QFile f(fileName);
    if (!f.open(IO_ReadOnly)) {
        kdDebug(9002) << "Could not read doc toc: " << fileName << endl;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "kdeveloptoc") {
        kdDebug() << "Not a valid kdeveloptoc file: " << fileName << endl;
        return;
    }
    
    f.close();
    
    QDomElement docEl = doc.documentElement();
    QDomElement titleEl = docEl.namedItem("title").toElement();
    setText(0, titleEl.firstChild().toText().data());

    QString base;
    QListViewItem *lastChildItem = 0;
    QDomElement childEl = docEl.firstChild().toElement();
    while (!childEl.isNull()) {
        if (childEl.tagName() == "tocsect1") {
            QString name = childEl.attribute("name");
            QString url = childEl.attribute("url");
            DocTreeItem *item = new DocTreeItem(this, Book, name, DocTreeItem::context());
            if (!url.isEmpty())
                item->setFileName(base + url);
            
            if (lastChildItem)
                item->moveItem(lastChildItem);
            lastChildItem = item;
            
            // Ok, this means we have two levels in the table of contents hardcoded
            // Eventually, this limitation should go, but at the moment it is simple to implement :-)
            QListViewItem *lastGrandchildItem = 0;
            QDomElement grandchildEl = childEl.firstChild().toElement();
            while (!grandchildEl.isNull()) {
                if (grandchildEl.tagName() == "tocsect2") {
                    QString name2 = grandchildEl.attribute("name");
                    QString url2 = grandchildEl.attribute("url");
                    DocTreeItem *item2 = new DocTreeItem(item, Doc, name2, DocTreeItem::context());
                    if (!url2.isEmpty())
                        item2->setFileName(base + url2);
                    if (lastGrandchildItem)
                        item2->moveItem(lastGrandchildItem);
                    lastGrandchildItem = item2;
		    // and nobody said there couldn't be another level ;-)
		    QListViewItem *last2GrandchildItem = 0;
		    QDomElement grand2childEl = grandchildEl.firstChild().toElement();
		    while (!grand2childEl.isNull()) {
			if (grand2childEl.tagName() == "tocsect3") {
			    QString name3 = grand2childEl.attribute("name");
			    QString url3 = grand2childEl.attribute("url");
			    DocTreeItem *item3 = new DocTreeItem(item2, Doc, name3, DocTreeItem::context());
                            if (!url3.isEmpty())
                                item3->setFileName(base + url3);
			    if (last2GrandchildItem)
				item3->moveItem(last2GrandchildItem);
			    last2GrandchildItem = item3;
			}
			grand2childEl = grand2childEl.nextSibling().toElement();
		    }
                }
                grandchildEl = grandchildEl.nextSibling().toElement();
            }
        } else if (childEl.tagName() == "base") {
            base = childEl.attribute("href");
            if (!base.isEmpty())
                base += "/";
        }

        childEl = childEl.nextSibling().toElement();
    }

}


DocTreeTocFolder::~DocTreeTocFolder()
{}


/*************************************/
/* Folder "Documentation Base"       */
/*************************************/

#ifdef WITH_DOCBASE


class DocTreeDocbaseFolder : public DocTreeItem
{
public:
    DocTreeDocbaseFolder(DocTreeViewWidget *parent, const QString &context);
    ~DocTreeDocbaseFolder();
    virtual void setOpen(bool o);
private:
    void readDocbaseFile(FILE *f);
};


DocTreeDocbaseFolder::DocTreeDocbaseFolder(DocTreeViewWidget *parent, const QString &context)
    : DocTreeItem(parent, Folder, i18n("Documentation Base"), context)
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
    DocTreeBookmarksFolder(DocTreeViewWidget *parent, const QString &context);
    void refresh();
};


DocTreeBookmarksFolder::DocTreeBookmarksFolder(DocTreeViewWidget *parent, const QString &context)
    : DocTreeItem(parent, Folder, i18n("Bookmarks"), context)
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
        DocTreeItem *item = new DocTreeItem(this, Book, *it1, context());
        item->setFileName(*it2);
    }
}


/*************************************/
/* Folder "Current Project"          */
/*************************************/

class DocTreeProjectFolder : public DocTreeItem
{
public:
    DocTreeProjectFolder(DocTreeViewWidget *parent, const QString &context);
    void setProject(KDevProject *project)
        { m_project = project; }
    void refresh();
    
private:
    KDevProject *m_project;
};


DocTreeProjectFolder::DocTreeProjectFolder(DocTreeViewWidget *parent, const QString &context)
    : DocTreeItem(parent, Folder, i18n("Current Project"), context)
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
                DocTreeItem *item = new DocTreeItem(this, Book, i18n("API of %1").arg(*it), context());
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
                DocTreeItem *item = new DocTreeItem(this, Book, i18n("Userdoc for %1").arg(*it), context());
                item->setFileName(filename);
            }
        }
    }
    
    if (!firstChild())
        setExpandable(false);
}


/**************************************/
/* The DocTreeViewWidget itself       */
/**************************************/


DocTreeViewWidget::DocTreeViewWidget(DocTreeViewPart *part)
    : KListView(0, "doc tree widget")
{
    setFocusPolicy(ClickFocus);
    setRootIsDecorated(true);
    setResizeMode(QListView::LastColumn);
    setSorting(-1);
    header()->hide();
    addColumn(QString::null);
    
    folder_bookmarks = new DocTreeBookmarksFolder(this, "ctx_bookmarks");
    folder_bookmarks->refresh();
    folder_project   = new DocTreeProjectFolder(this, "ctx_current");
#ifdef WITH_DOCBASE
    folder_docbase   = new DocTreeDocbaseFolder(this, "ctx_docbase");
#endif

    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
    QStringList tocs = dirs->findAllResources("doctocs", QString::null, false, true);
    QStringList::Iterator tit;
    for (tit = tocs.begin(); tit != tocs.end(); ++tit)
        folder_toc.append(new DocTreeTocFolder(this, *tit, QString("ctx_%1").arg(*tit)));

    folder_kdelibs   = new DocTreeKDELibsFolder(this, "ctx_kdelibs");
    folder_kdelibs->refresh();

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
    
    m_part->partController()->showDocument(KURL(ident), dtitem->context());
    m_part->topLevel()->lowerView(this);
}


void DocTreeViewWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;
    contextItem = item;
    KPopupMenu popup(i18n("Documentation Tree"), this);
    popup.insertItem(i18n("Customize..."), this, SLOT(slotConfigure()));
    popup.exec(p);
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
    folder_kdelibs->refresh();
    folder_bookmarks->refresh();
}


void DocTreeViewWidget::projectChanged(KDevProject *project)
{
    folder_project->setProject(project);
    folder_project->refresh();

    // Remove all...
    takeItem(folder_bookmarks);
    takeItem(folder_project);
#ifdef WITH_DOCBASE
    takeItem(folder_docbase);
#endif
    QListIterator<DocTreeTocFolder> it1(folder_toc);
    for (; it1.current(); ++it1)
        takeItem(it1.current());
    takeItem(folder_kdelibs);

    // .. and insert all again except for ignored items
    QDomElement docEl = m_part->projectDom()->documentElement();
    QDomElement doctreeviewEl = docEl.namedItem("kdevdoctreeview").toElement();

    QStringList ignoretocs;
    if (project) {
        QDomElement ignoretocsEl = doctreeviewEl.namedItem("ignoretocs").toElement();
        QDomElement tocEl = ignoretocsEl.firstChild().toElement();
        while (!tocEl.isNull()) {
            if (tocEl.tagName() == "toc")
                ignoretocs << tocEl.firstChild().toText().data();
            tocEl = tocEl.nextSibling().toElement();
        }
    }
    
    insertItem(folder_bookmarks);
    insertItem(folder_project);
#ifdef WITH_DOCBASE
    insertItem(folder_docbase);
#endif
    QListIterator<DocTreeTocFolder> it2(folder_toc);
    for (; it2.current(); ++it2) {
        if (!ignoretocs.contains(it2.current()->tocName()))
            insertItem(it2.current());
    }
    if (!ignoretocs.contains("kde"))
        insertItem(folder_kdelibs);

    triggerUpdate();
}


QString DocTreeViewWidget::locatehtml(const QString &fileName)
{

    QString path = locate("html", KGlobal::locale()->language() + '/' + fileName);
    if (path.isNull())
       path = locate("html", "default/" + fileName);

    return path;
}

#include "doctreeviewwidget.moc"
