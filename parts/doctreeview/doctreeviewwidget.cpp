/***************************************************************************
 *   Copyright (C) 1999-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "doctreeviewwidget.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qlistview.h>

#include <kdebug.h>
#include <kdialogbase.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kprocess.h>
#include <kdeversion.h>

#include "kdevcore.h"
#include "domutil.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"

#include "../../config.h"
#include "misc.h"
#include "doctreeviewfactory.h"
#include "doctreeviewpart.h"
#include "doctreeglobalconfigwidget.h"
#include "doctreeprojectconfigwidget.h"

class DocTreeItem : public QListViewItem
{
public:
    enum Type { Folder, Book, Doc };
    DocTreeItem( KListView *parent, Type type, const QString &text, const QString &context );
    DocTreeItem( DocTreeItem *parent, Type type, const QString &text, const QString &context);

    void setFileName(const QString &fn)
        { filename = fn; }
    virtual QString fileName()
        { return filename; }
    virtual void clear();
    virtual QString context() const { return m_context; }
	virtual Type getType() const { return typ; }

private:
    void init();
    Type typ;
    QString filename, m_context;
};


DocTreeItem::DocTreeItem(KListView *parent, Type type, const QString &text, const QString &context)
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
/* Folder "Qt/KDE libraries (kdoc)"  */
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
    return DocTreeItem::fileName()+"/index.html";
}


void DocTreeKDELibsBook::setOpen(bool o)
{
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
        QString cmd = "gzip -c -d ";
#if (KDE_VERSION > 305)
        cmd += KProcess::quote(idx_filename);
#else
        cmd += KShellProcess::quote(idx_filename);
#endif
        cmd += " 2>/dev/null";
        if ( (f = popen(QFile::encodeName(cmd), "r")) != 0) {
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
      else if (/*s.left(pos0=9) == "<M NAME=\"" || */s.left(pos0=10) == "<ME NAME=\"")
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
//              kdDebug ( 9000 ) << "++++++++++++++ " << membername << endl;
                  item->setFileName(DocTreeItem::fileName() + "/" + filename);
              }
          }
    }

    sortChildItems(0, true);
}


class DocTreeKDELibsFolder : public DocTreeItem
{
public:
    DocTreeKDELibsFolder(QString location, QString name, KListView *parent, const QString &context)
        : DocTreeItem(parent, Folder, name, context), m_location(location)
        { setExpandable(true); }
    void refresh();
private:
    QString m_location;
};


void DocTreeKDELibsFolder::refresh()
{
    DocTreeItem::clear();

/*    QDir d(m_location);
    QStringList fileList = d.entryList("*", QDir::Dirs);

    QStringList::ConstIterator it;
    for (it = fileList.begin(); it != fileList.end(); ++it) {
        QString dirName = (*it);
        if (dirName == "." || dirName == ".." || dirName == "common")
            continue;
        qWarning("loading dir %s", dirName.latin1());*/
        QStringList itemNames, fileNames, hiddenNames;
        DocTreeViewTool::readLibraryDocs(m_location,&itemNames, &fileNames);
        QStringList::Iterator it1, it2;
        for (it1 = itemNames.begin(), it2 = fileNames.begin();
             it1 != itemNames.end() && it2 != fileNames.end();
            ++it1, ++it2) {
            new DocTreeKDELibsBook(this, *it1, *it2, context());
        }
    //}

    sortChildItems(0, true);
    
    //TODO: merge in default libraries and hidden options

    // Read in possible items for the Libraries tree
/*    QStringList libNames, docDirs, sourceDirs;
    DocTreeViewTool::getLibraries(&libNames, &docDirs, &sourceDirs);
    QStringList::Iterator libName, docDir, sourceDir;
    for (libName = libNames.begin(),
         docDir = docDirs.begin(),
         sourceDir = sourceDirs.begin() ;
         libName!=libNames.end() && docDir!=docDirs.end() && sourceDir!=sourceDirs.end();
         ++libName, ++docDir, ++sourceDir) {
        QStringList itemNames, fileNames, hiddenNames;
        DocTreeViewTool::readLibraryDocs(*docDir,&itemNames, &fileNames);
        QStringList::Iterator it1, it2;
        for (it1 = itemNames.begin(), it2 = fileNames.begin();
             it1 != itemNames.end() && it2 != fileNames.end();
            ++it1, ++it2) {
            new DocTreeKDELibsBook(this, *it1, *it2, context());
        }

    }*/
}


/***************************************/
/* Folder "Qt/KDE libraries (Doxygen)" */
/***************************************/


/**
 * API documentation created with the configure option --with-apidocs
 * in kdelibs.
 */
class DocTreeDoxygenBook : public DocTreeItem
{
public:
    DocTreeDoxygenBook( DocTreeItem *parent, const QString &name,
                        const QString &tagFileName, const QString &context);
    ~DocTreeDoxygenBook();
    static bool isInstallationOK(const QString& bookDir)
    {
      return QFile::exists(bookDir + "/html/index.html");
    }

    virtual void setOpen(bool o);

private:
    void readTagFile();
    QString dirname;
};


DocTreeDoxygenBook::DocTreeDoxygenBook(DocTreeItem *parent, const QString &name,
                                       const QString &dirName, const QString &context)
    : DocTreeItem(parent, Book, name, context),
      dirname(dirName)
{
    QString fileName = dirName + "/index.html";
    setFileName(fileName);
    setExpandable(true);
}


DocTreeDoxygenBook::~DocTreeDoxygenBook()
{}


void DocTreeDoxygenBook::setOpen(bool o)
{
    if (o && childCount() == 0)
        readTagFile();
    DocTreeItem::setOpen(o);
}


void DocTreeDoxygenBook::readTagFile()
{
    QString tagName = dirname + "/" + text(0) + ".tag";
    QFile f(tagName);
    if(!f.exists())
    {
#if QT_VERSION >= 0x030100
        tagName.remove("/html/");
#else
	tagName.replace( QRegExp( "\\/html\\/" ), QString() );
#endif
        f.setName( tagName );
    }
    if (!f.open(IO_ReadOnly)) {
        kdDebug(9002) << "Could not open tag file: " << f.name() << endl;
        return;
    }

    QDomDocument dom;
    if (!dom.setContent(&f) || dom.documentElement().nodeName() != "tagfile") {
        kdDebug(9002) << "No valid tag file" << endl;
        return;
    }
    f.close();

    QDomElement docEl = dom.documentElement();

    QDomElement childEl = docEl.firstChild().toElement();
    while (!childEl.isNull()) {
        if (childEl.tagName() == "compound" && childEl.attribute("kind") == "class") {
            QString classname = childEl.namedItem("name").firstChild().toText().data();
            QString filename = childEl.namedItem("filename").firstChild().toText().data();

            if (QFile::exists(dirname + filename)) { // don't create bad links
                DocTreeItem *item = new DocTreeItem(this, Doc, classname, context());
                item->setFileName(dirname + filename);
            }
        }
        childEl = childEl.nextSibling().toElement();
    }

    sortChildItems(0, true);
}


class DocTreeDoxygenFolder : public DocTreeItem
{
public:
    DocTreeDoxygenFolder(QString location, QString name, KListView *parent, const QString &context)
        : DocTreeItem(parent, Folder, name, context), m_location(location)
        { setExpandable(true); }
    void refresh();
private:
    QString m_location;
};

void DocTreeDoxygenFolder::refresh()
{
    DocTreeItem::clear();

/*    KConfig *config = DocTreeViewFactory::instance()->config();
    config->setGroup("General");
    QString docdir = config->readEntry("kdelibsdocdir", KDELIBS_DOXYDIR);

    //kdDebug(9002) << "docdir: " << docdir << endl;*/
    QDir d(m_location);
    QStringList fileList = d.entryList("*", QDir::Dirs);

    QStringList::ConstIterator it;
    for (it = fileList.begin(); it != fileList.end(); ++it) {
        QString dirName = (*it);
        //kdDebug(9002) << "dirname: " << dirName << endl;
        if (dirName == "." || dirName == ".." || dirName == "common")
            continue;
        if (DocTreeDoxygenBook::isInstallationOK(d.absFilePath(*it))) {
            new DocTreeDoxygenBook(this, *it, d.absFilePath(*it) + "/html/", context());
            //kdDebug(9002) << "foo: " << d.absFilePath(*it) + "/html/" + *it << endl;
        }
    }

    sortChildItems(0, true);
}


/***************************************/
/* Folder from the 'tocs' resource dir */
/***************************************/

class DocTreeTocFolder : public DocTreeItem
{
public:
    DocTreeTocFolder(KListView *parent, const QString &fileName, const QString &context);
    ~DocTreeTocFolder();

    QString tocName() const
    { return toc_name; }

private:
    QString base;
    QString toc_name;
};


DocTreeTocFolder::DocTreeTocFolder(KListView *parent, const QString &fileName, const QString &context)
    : DocTreeItem(parent, Folder, fileName, context)
{
    setFileName( fileName );

    QFileInfo fi(fileName);
    toc_name = fi.baseName();
    base = DocTreeViewTool::tocLocation( fileName );

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
    DocTreeDocbaseFolder(KListView *parent, const QString &context);
    ~DocTreeDocbaseFolder();
    virtual void setOpen(bool o);
private:
    void readDocbaseFile(FILE *f);
};


DocTreeDocbaseFolder::DocTreeDocbaseFolder(KListView *parent, const QString &context)
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
            DocTreeItem *item = new DocTreeItem(this, Doc, title, context());
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
    DocTreeBookmarksFolder(KListView *parent, const QString &context);
    void refresh();
};

DocTreeBookmarksFolder::DocTreeBookmarksFolder(KListView *parent, const QString &context)
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
    DocTreeProjectFolder(KListView *parent, const QString &context);
    void setProject(KDevProject *project)
        { m_project = project; }
    void refresh();

private:
    KDevProject *m_project;
    QString m_userdocDir, m_apidocDir;
};

DocTreeProjectFolder::DocTreeProjectFolder(KListView *parent, const QString &context)
    : DocTreeItem(parent, Folder, i18n("Current Project"), context), m_project(0)
{
}


void DocTreeProjectFolder::refresh()
{
    //TODO: use doxygen tags
    if( !m_project )
        return;

    m_userdocDir = DomUtil::readEntry(
        *m_project->projectDom() , "/kdevdoctreeview/projectdoc/userdocDir");
    m_apidocDir = DomUtil::readEntry(
        *m_project->projectDom() , "/kdevdoctreeview/projectdoc/apidocDir");


    DocTreeItem::clear();

        // API documentation
        QDir apidir( m_apidocDir );
        if (apidir.exists()) {
            QStringList entries = apidir.entryList("*.html", QDir::Files);
            QString filename = apidir.absPath() + "/index.html";
            if (!QFileInfo(filename).exists())
                return;
            DocTreeItem *item = new DocTreeItem(
                this, Book, i18n("API of %1").arg(m_project->projectName() ), context());
                item->setFileName(filename);
            for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
                filename = *it;
                DocTreeItem *ditem = new DocTreeItem(item,
                    Doc, QFileInfo(filename).baseName() , context());
                ditem->setFileName(apidir.absPath() +"/"+ filename);
            }
        }
        // User documentation
        QDir userdir( m_userdocDir );
        if (userdir.exists()) {
            QStringList entries = userdir.entryList("*.html", QDir::Files);
            QString filename = userdir.absPath() + "/index.html";
            if (!QFileInfo(filename).exists())
                return;
            DocTreeItem *item = new DocTreeItem(
                this, Book, i18n("Usedoc for %1").arg(m_project->projectName() ), context());
                item->setFileName(filename);
            for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
                filename = *it;
                DocTreeItem *ditem = new DocTreeItem(item,
                    Doc, QFileInfo(filename).baseName() , context());
                ditem->setFileName(userdir.absPath() +"/"+ filename);
            }
        }

    if (!firstChild())
        setExpandable(false);

}


/**************************************/
/* Qt Folder                                    */
/**************************************/

class DocTreeQtFolder : public DocTreeItem
{
public:
    DocTreeQtFolder(QString xml, QString name, KListView *parent, const QString &context);
    void refresh();
private:
    QString filename;
    QString m_xml;
};

DocTreeQtFolder::DocTreeQtFolder(QString xml, QString name, KListView *parent, 
    const QString &context)
    : DocTreeItem(parent, Folder, name, context), m_xml(xml)
{
//    filename = _fileName;
}

void DocTreeQtFolder::refresh()
{
    filename = m_xml;
    
    QFileInfo fi(filename);

    QFile f(filename);
    if (!f.open(IO_ReadOnly)) {
        kdDebug(9002) << "Could not read" << m_xml << endl;
        kdDebug(9002) << "  Filename was: " << filename << endl;
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "DCF") {
        kdDebug(9002) << "Not a valid DCF file: " << filename << endl;
        return;
    }
    DocTreeItem::clear();

    f.close();

    QDomElement docEl = doc.documentElement();
    QDomElement titleEl = docEl.namedItem("DCF").toElement();
    
    setFileName(fi.dirPath( true ) +"/"+ docEl.attribute("ref", QString::null));

    QDomElement childEl = docEl.lastChild().toElement();
    while (!childEl.isNull())
    {
        if (childEl.tagName() == "section")
        {
            QString ref = childEl.attribute("ref");
            QString title = childEl.attribute("title");

            //dymo: enable all qt docs: disable "Class Reference" check
/*            int i = title.find("Class Reference");
            if( i > 0 )
            {
                title = title.left(i);*/
                DocTreeItem* item = item = new DocTreeItem(this, Book, title, context());
                item->setFileName(fi.dirPath( true ) +"/"+ ref);

                QDomElement grandChild = childEl.lastChild().toElement();
                while(!grandChild.isNull())
                {
                    if (grandChild.tagName() == "keyword")
                    {
                        QString dref = grandChild.attribute("ref");
                        QString dtitle = grandChild.text();

                        DocTreeItem* dItem = new DocTreeItem(item, Doc, dtitle, context());
                        dItem->setFileName(fi.dirPath( true ) +"/"+ dref);
                    }
                    grandChild = grandChild.previousSibling().toElement();
               }
               //kdDebug(9002) <<"ref: "<< ref <<"  title: " << title << endl;
//            }
            childEl = childEl.previousSibling().toElement();
        }
    }
}

bool DocTreeViewWidget::initKDocKDELibs()
{
/*    KConfig *config = DocTreeViewFactory::instance()->config();
    config->setGroup( "General" );
    kdelibskdoc = config->readBoolEntry("displayKDELibsKDoc", false);

    if ( kdelibskdoc ) {
        if( folder_kdelibs ) return true;
        folder_kdelibs = new DocTreeKDELibsFolder(docView, "ctx_kdelibs");
        folder_kdelibs->refresh();
        return true;
    } else {
        if( folder_kdelibs )
            delete folder_kdelibs;
        folder_kdelibs = 0L;
        return false;
    }*/
    return true;
}
/**************************************/
/* The DocTreeViewWidget itself       */
/**************************************/

DocTreeViewWidget::DocTreeViewWidget(DocTreeViewPart *part)
    : QVBox(0, "doc tree widget"), m_activeTreeItem ( 0L )
{
    /* initializing the documentation toolbar */
    searchToolbar = new QHBox ( this, "search toolbar" );
    searchToolbar->setMargin ( 2 );
    searchToolbar->setSpacing ( 2 );

    completionCombo = new KHistoryCombo ( true, searchToolbar, "completion combo box" );
    
    startButton = new QToolButton ( searchToolbar, "start searching" );
    startButton->setSizePolicy ( QSizePolicy ( (QSizePolicy::SizeType)0, ( QSizePolicy::SizeType)0, 0, 0, startButton->sizePolicy().hasHeightForWidth()) );
    startButton->setPixmap ( SmallIcon ( "key_enter" ) );
    QToolTip::add ( startButton, i18n ( "Start searching" ) );

    nextButton = new QToolButton ( searchToolbar, "next match button" );
    nextButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType )0, ( QSizePolicy::SizeType) 0, 0, 0, nextButton->sizePolicy().hasHeightForWidth()) );
    nextButton->setPixmap ( SmallIcon ( "next" ) );
    QToolTip::add ( nextButton, i18n ( "Jump to next matching entry" ) );
    nextButton->setEnabled( false );

    prevButton = new QToolButton ( searchToolbar, "previous match button" );
    prevButton->setSizePolicy ( QSizePolicy ( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, prevButton->sizePolicy().hasHeightForWidth()) );
    prevButton->setPixmap ( SmallIcon ( "previous" ) );
    QToolTip::add ( prevButton, i18n ( "Jump to last matching entry" ) );
    prevButton->setEnabled( false );

    docView = new KListView ( this, "documentation list view" );

    docView->setFocusPolicy(ClickFocus);
    docView->setRootIsDecorated(true);
    docView->setResizeMode(QListView::LastColumn);
    docView->setSorting(-1);
    docView->header()->hide();
    docView->addColumn(QString::null);

    folder_bookmarks = new DocTreeBookmarksFolder(docView, "ctx_bookmarks");
    folder_bookmarks->refresh();

    folder_project   = new DocTreeProjectFolder(docView, "ctx_current");
    folder_project->refresh();

#ifdef WITH_DOCBASE
    folder_docbase   = new DocTreeDocbaseFolder(docView, "ctx_docbase");
#endif

    // doctocs
    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
    QStringList tocs = dirs->findAllResources("doctocs", QString::null, false, true);
    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit)
        folder_toc.append(new DocTreeTocFolder(docView, *tit, QString("ctx_%1").arg(*tit)));

//    initKDocKDELibs();

    KConfig *config = DocTreeViewFactory::instance()->config();
    if (config)
    {
        config->setGroup("General KDoc");
        QMap<QString, QString> dmap = config->entryMap("General KDoc");
        QString kdocdir(KDELIBS_DOCDIR);
        if (dmap.empty() && (!kdocdir.isEmpty()))
        {
            config->writePathEntry("KDE Libraries (KDoc)", QString(KDELIBS_DOCDIR));
            dmap["KDE Libraries (KDoc)"] = QString(KDELIBS_DOCDIR);
        }

        QMap<QString, QString>::Iterator it;
        for (it = dmap.begin(); it != dmap.end(); ++it)
        {
            DocTreeKDELibsFolder *kdf = new DocTreeKDELibsFolder(it.data(), it.key(), docView, "ctx_kdelibs");
            kdf->refresh();
            folder_kdoc.append(kdf);
        }
    }
    
    if (config)
    {
        config->setGroup("General Doxygen");
        QMap<QString, QString> xmap = config->entryMap("General Doxygen");
        QString doxydir(KDELIBS_DOXYDIR);
        if (xmap.empty() && (!doxydir.isEmpty()))
        {
            config->writePathEntry("KDE Libraries (Doxygen)", QString(KDELIBS_DOXYDIR));
            xmap["KDE Libraries (Doxygen)"] = QString(KDELIBS_DOXYDIR);
        }

        QMap<QString, QString>::Iterator it;
        for (it = xmap.begin(); it != xmap.end(); ++it)
        {
            DocTreeDoxygenFolder *dxf = new DocTreeDoxygenFolder(it.data(), it.key(), docView, "ctx_doxygen");
            dxf->refresh();
            folder_doxygen.append(dxf);
        }
    }

        
//    folder_doxygen   = new DocTreeDoxygenFolder(docView, "ctx_doxygen");
    
//    folder_doxygen->refresh();

    // eventually, Qt docu extra
/*    QListViewItem* pChild = folder_doxygen->firstChild();
    while (pChild && pChild->text(0) != "qt") {
        pChild = pChild->nextSibling();
    }
*/
    if (config)
    {
        config->setGroup("General Qt");
        QMap<QString, QString> emap = config->entryMap("General Qt");

        QString qtdocdir(config->readEntry("qtdocdir", QT_DOCDIR));
        if (emap.empty() && (!qtdocdir.isEmpty()))
        {
            config->writePathEntry("Qt Reference Documentation", QString(QT_DOCDIR) + QString("/qt.xml"));
            emap["Qt Reference Documentation"] = QString(QT_DOCDIR) + QString("/qt.xml");
            config->writePathEntry("Qt Assistant Manual", QString(QT_DOCDIR) + QString("/assistant.xml"));
            emap["Qt Assistant Manual"] = QString(QT_DOCDIR) + QString("/assistant.xml");
            config->writePathEntry("Qt Designer Manual", QString(QT_DOCDIR) + QString("/designer.xml"));
            emap["Qt Designer Manual"] = QString(QT_DOCDIR) + QString("/designer.xml");
            config->writePathEntry("Guide to the Qt Translation Tools", QString(QT_DOCDIR) + QString("/linguist.xml"));
            emap["Guide to the Qt Translation Tools"] = QString(QT_DOCDIR) + QString("/linguist.xml");
            config->writePathEntry("qmake User Guide", QString(QT_DOCDIR) + QString("/qmake.xml"));
            emap["qmake User Guide"] = QString(QT_DOCDIR) + QString("/qmake.xml");
        }

        QMap<QString, QString>::Iterator it;
        for (it = emap.begin(); it != emap.end(); ++it)
        {
            DocTreeQtFolder *qtf = new DocTreeQtFolder(it.data(), it.key(), docView, "ctx_qt");
            qtf->refresh();
            folder_qt.append(qtf);
        }
    }

    connect ( nextButton, SIGNAL ( clicked() ), this, SLOT ( slotJumpToNextMatch() ) );
    connect ( prevButton, SIGNAL ( clicked() ), this, SLOT ( slotJumpToPrevMatch() ) );
    connect ( startButton, SIGNAL ( clicked() ), this, SLOT ( slotStartSearching() ) );
    connect ( completionCombo, SIGNAL ( returnPressed ( const QString& ) ), this, SLOT ( slotHistoryReturnPressed ( const QString& ) ) );

    connect( docView, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( docView, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );
    connect ( docView, SIGNAL ( selectionChanged ( QListViewItem* ) ), this, SLOT ( slotSelectionChanged ( QListViewItem* ) ) );

    m_part = part;
}


DocTreeViewWidget::~DocTreeViewWidget()
{}

void DocTreeViewWidget::searchForItem ( const QString& currentText )
{
	completionCombo->addToHistory( currentText );
	QListViewItem* current = docView->currentItem();
	if( current->firstChild() )
	{  //only allow items with childs to be searched in
		QListViewItemIterator  docViewIterator( current );
		while( docViewIterator.current() )
		{// now we do the search
			if( docViewIterator.current()->text(0).find( currentText, false )>=0 )
			{
				searchResultList.append( docViewIterator.current() );
			}
			++docViewIterator;
		}
	}
}

void DocTreeViewWidget::slotJumpToNextMatch()
{
	if( searchResultList.next() )
	{
		docView->setSelected ( searchResultList.current(), true );
		docView->ensureItemVisible ( searchResultList.current() );
		slotItemExecuted ( searchResultList.current() );
		prevButton->setEnabled( true );

		if(searchResultList.current() == searchResultList.getLast() )
			nextButton->setEnabled( false );
	}
	else
	{
		searchResultList.last();
	}

}

void DocTreeViewWidget::slotJumpToPrevMatch()
{
	if( searchResultList.prev() )
	{
		docView->setSelected ( searchResultList.current(), true );
		docView->ensureItemVisible ( searchResultList.current() );
		slotItemExecuted ( searchResultList.current() );
		nextButton->setEnabled( true );

		if(searchResultList.current() == searchResultList.getFirst() )
			prevButton->setEnabled( false );
	}
	else
	{
		searchResultList.first();
	 }
}

void DocTreeViewWidget::slotStartSearching()
{
    QString currentText = completionCombo->currentText();
	slotHistoryReturnPressed ( currentText );
}

void DocTreeViewWidget::slotHistoryReturnPressed ( const QString& currentText )
{
    if( !docView->selectedItem() )
        docView->setCurrentItem( docView->firstChild() );

    nextButton->setEnabled( false );
	prevButton->setEnabled( false );
	searchResultList.clear();

	if( currentText.length() > 0 )
		searchForItem( currentText ); //fills searchResultList


	if ( searchResultList.count() )
	{
		kdDebug ( 9002 ) << "Found a matching entry!" << endl;
		docView->setSelected ( searchResultList.first(), true );
		docView->ensureItemVisible (  searchResultList.first() );
		slotItemExecuted ( searchResultList.first() );
	}
	if ( searchResultList.count() > 1 )
	{
		nextButton->setEnabled( true );
	}
}

void DocTreeViewWidget::slotSelectionChanged ( QListViewItem* item )
{
	contextItem = item;

	if( !item->parent() )
	{// current is a toplevel item, so we initialize all childs
		QListViewItem * myChild = item->firstChild();
		while( myChild && myChild->parent())
		{// only initialize current folder, not the below ones
			myChild->setOpen( true );
			myChild->setOpen( false );

			myChild = myChild->itemBelow();
		}
	}

}

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

    // avoid empty pages when clicking on .toc files, choose its first child instead
    if (ident.right(4) == ".toc") {
        dtitem = static_cast<DocTreeItem*>(dtitem->firstChild());
        if (!dtitem) return;
        ident = dtitem->fileName();
        if (ident.isEmpty()) return;
    }

    kdDebug(9002) << "Showing: " << ident << endl;
    m_part->partController()->showDocument(KURL(ident), dtitem->context());
    m_part->mainWindow()->lowerView(this);
}


void DocTreeViewWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;
    contextItem = item;
    KPopupMenu popup(i18n("Documentation Tree"), this);

    DocTreeItem *dItem = dynamic_cast<DocTreeItem*>( item );
    DocumentationContext dcontext( dItem->fileName(), "" );

    QListViewItem* i = contextItem;
    while(i->parent()) // go to folder
    {
        i = i->parent();
    }
    if ( i == folder_project ) {
        popup.insertItem(i18n("Project Properties"), this, SLOT(slotConfigureProject()));
    } else {
        popup.insertItem(i18n("Properties"), this, SLOT(slotConfigure()));
    }
    if ( i != folder_bookmarks && dItem && !dItem->fileName().isEmpty() )
    {
        popup.insertItem(i18n("Add to Bookmarks"), this, SLOT(slotAddBookmark()));
        dcontext = DocumentationContext( dItem->fileName(), dItem->text(0) );
    }
    if (  contextItem->parent() && dItem && contextItem->parent() == folder_bookmarks )
    {
        popup.insertItem(i18n("Remove"), this, SLOT(slotRemoveBookmark()));
        dcontext = DocumentationContext( dItem->fileName(), dItem->text(0) );
    }
    m_part->core()->fillContextMenu( &popup , &dcontext );
    popup.exec(p);
}


void DocTreeViewWidget::slotConfigure()
{
    KDialogBase dlg(KDialogBase::Tabbed, i18n("Customize Documentation Tree"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this,
                    "customization dialog");

    QVBox *vbox1 = dlg.addVBoxPage(i18n("Documentation Tree: Global"));
    DocTreeGlobalConfigWidget *w1 = new DocTreeGlobalConfigWidget( m_part, this, vbox1, "doctreeview global config widget");
    connect(&dlg, SIGNAL(okClicked()), w1, SLOT(accept()));

    dlg.exec();

    delete w1;
}

void DocTreeViewWidget::slotConfigureProject()
{
    KDialogBase dlg(KDialogBase::Tabbed, i18n("Customize Documentation Tree"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this,
                    "customization dialog");

    DocTreeProjectConfigWidget *w2 = 0;

    if(m_part->project()) {
      QVBox *vbox2 = dlg.addVBoxPage(i18n("Documentation Tree: Project"));
      w2 = new DocTreeProjectConfigWidget(this, vbox2, m_part->project(),"doctreeview project config widget");
      connect(&dlg, SIGNAL(okClicked()), w2, SLOT(accept()));
      dlg.exec();
    }
    if(w2)
      delete w2;
}


void DocTreeViewWidget::configurationChanged()
{
    kdDebug(9002) << "DocTreeViewWidget::configurationChanged()" << endl;
    initKDocKDELibs();
    QTimer::singleShot(0, this, SLOT(refresh()));
}


void DocTreeViewWidget::refresh()
{
    kdDebug(9002) << "DocTreeViewWidget::refresh()" << endl;
//    folder_doxygen->refresh();
    folder_bookmarks->refresh();
    folder_project->refresh();
    
/*    if( folder_kdelibs )
        folder_kdelibs->refresh();*/

   
    DocTreeTocFolder *item;
    for ( item = folder_toc.first(); item; item = folder_toc.next() )
        delete item;

    folder_toc.clear();

    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
    QStringList tocs = dirs->findAllResources("doctocs", QString::null, false, true);
    QStringList ignore( DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoretocs", "toc") );

    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit)
    {
        if( !ignore.contains( QFileInfo(*tit).baseName() ) )
            folder_toc.append(new DocTreeTocFolder(docView, *tit, QString("ctx_%1").arg(*tit)));
    }

    folder_kdoc.setAutoDelete(true);
    folder_kdoc.clear();
    folder_kdoc.setAutoDelete(false);
    
    KConfig *config = DocTreeViewFactory::instance()->config();
    if (config)
    {
        config->setGroup("General KDoc");
        QMap<QString, QString> emap = config->entryMap("General KDoc");
        QMap<QString, QString>::Iterator it;
        for (it = emap.begin(); it != emap.end(); ++it)
        {
            DocTreeKDELibsFolder *kdf = new DocTreeKDELibsFolder(it.data(), it.key(), docView, "ctx_kdelibs");
            kdf->refresh();
            folder_kdoc.append(kdf);
        }
    }
   
    folder_doxygen.setAutoDelete(true);
    folder_doxygen.clear();
    folder_doxygen.setAutoDelete(false);
    
    if (config)
    {
        config->setGroup("General Doxygen");
        QMap<QString, QString> emap = config->entryMap("General Doxygen");
        QMap<QString, QString>::Iterator it;
        for (it = emap.begin(); it != emap.end(); ++it)
        {
            DocTreeDoxygenFolder *dxf = new DocTreeDoxygenFolder(it.data(), it.key(), docView, "ctx_doxygen");
            dxf->refresh();
            folder_doxygen.append(dxf);
        }
    }

        
    folder_qt.setAutoDelete(true);
    folder_qt.clear();
    folder_qt.setAutoDelete(false);
    
    if (config)
    {
        config->setGroup("General Qt");
        QMap<QString, QString> emap = config->entryMap("General Qt");
        QMap<QString, QString>::Iterator it;
        for (it = emap.begin(); it != emap.end(); ++it)
        {
            DocTreeQtFolder *qtf = new DocTreeQtFolder(it.data(), it.key(), docView, "ctx_qt");
            qtf->refresh();
            folder_qt.append(qtf);
        }
    }
}


void DocTreeViewWidget::projectChanged(KDevProject *project)
{
    folder_project->setProject(project);
    folder_project->refresh();
    if(!project) {
        kdDebug(9002) << "No Project...." << endl;
        return;
    }


    // Remove all...
    docView->takeItem(folder_bookmarks);
    docView->takeItem(folder_project);
#ifdef WITH_DOCBASE
    docView->takeItem(folder_docbase);
#endif
    QListIterator<DocTreeTocFolder> it1(folder_toc);
    for (; it1.current(); ++it1)
        docView->takeItem(it1.current());

    QListIterator<DocTreeKDELibsFolder> itk(folder_kdoc);
    for (; itk.current(); ++itk)
        docView->takeItem(itk.current());
//    if(folder_doxygen) docView->takeItem(folder_doxygen);
    QListIterator<DocTreeDoxygenFolder> itx(folder_doxygen);
    for (; itx.current(); ++itx)
        docView->takeItem(itx.current());
    
    QListIterator<DocTreeQtFolder> itq(folder_qt);
    for (; itq.current(); ++itq)
        docView->takeItem(itq.current());
//    if(folder_qt) docView->takeItem(folder_qt);
//    if(folder_kdelibs) docView->takeItem(folder_kdelibs);
//    docView->takeItem(folder_kdevelop);

    // .. and insert all again except for ignored items
    QStringList ignoretocs = DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoretocs", "toc");

    docView->insertItem(folder_bookmarks);
    docView->insertItem(folder_project);
#ifdef WITH_DOCBASE
    docView->insertItem(folder_docbase);
#endif
    QListIterator<DocTreeTocFolder> it2(folder_toc);
//    it2.toLast();
//    for (; it2.current(); --it2) {
    for (; it2.current(); ++it2) {
        if (!ignoretocs.contains(it2.current()->tocName()))
            docView->insertItem(it2.current());
    }

//    docView->insertItem(folder_doxygen);
    QListIterator<DocTreeKDELibsFolder> itk2(folder_kdoc);
//    itk2.toLast();
//    for (; itk2.current(); --itk2)
    for (; itk2.current(); ++itk2)
        docView->insertItem(itk2.current());
    
    QListIterator<DocTreeDoxygenFolder> itx2(folder_doxygen);
//    itx2.toLast();
//    for (; itx2.current(); --itx2)
    for (; itx2.current(); ++itx2)
        docView->insertItem(itx2.current());
    
/*    if(folder_kdelibs && kdelibskdoc )
        if (!ignoretocs.contains("kde"))
            docView->insertItem(folder_kdelibs);
*/
    QListIterator<DocTreeQtFolder> itq2(folder_qt);
//    itq2.toLast();
//    for (; itq2.current(); --itq2)
    for (; itq2.current(); ++itq2)
        docView->insertItem(itq2.current());
//    if(folder_qt) docView->insertItem(folder_qt);

    docView->triggerUpdate();
}


QString DocTreeViewWidget::locatehtml(const QString &fileName)
{

    QString path = locate("html", KGlobal::locale()->language() + '/' + fileName);
    if (path.isNull())
       path = locate("html", "default/" + fileName);

    return path;
}


void DocTreeViewWidget::slotAddBookmark()
{
	DocTreeItem *item = dynamic_cast<DocTreeItem*>( contextItem );
	if( item )
	{
		DocTreeViewTool::addBookmark( item->text(0), item->fileName() );
		folder_bookmarks->refresh();
	}
}

void DocTreeViewWidget::slotRemoveBookmark()
{
	DocTreeItem *item = dynamic_cast<DocTreeItem*>( contextItem );
	if( item )
	{
		int posFolder = docView->itemIndex( folder_bookmarks );
		int i = docView->itemIndex( item ) - posFolder;
		//kdDebug(9002) << "remove item: " << i << endl;

		DocTreeViewTool::removeBookmark( i );

		folder_bookmarks->refresh();
	}
}

#include "doctreeviewwidget.moc"
