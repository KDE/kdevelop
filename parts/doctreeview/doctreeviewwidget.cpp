/***************************************************************************
 *   Copyright (C) 1999-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
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
#include <qmessagebox.h>
#include <qlabel.h>
#include <qptrlist.h>
#include <qprogressdialog.h>
#include <qwhatsthis.h>
#include <qxml.h>
#include <qdatastream.h>

#include <kdebug.h>
#include <kapplication.h>
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

#include "choosedlg.h"

#include "kdevcore.h"
#include "domutil.h"
#include "urlutil.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"

#include "../../config.h"
#include "misc.h"
#include "doctreeitem.h"
#include "doctreeviewfactory.h"
#include "doctreeviewpart.h"
#include "doctreeglobalconfigwidget.h"
#include "doctreeprojectconfigwidget.h"
#include "doclineedit.h"

#include "docsearchdlg.h"

IndexTreeData::IndexTreeData(const QString &text, const QString &parent, const QString &filename) :
        m_text(text), m_parent(parent), m_filename(filename), m_visible(true)
{
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
    virtual void refresh();

private:
    void readContents();
    void readKdoc2Index(FILE *f);
};


DocTreeKDELibsBook::DocTreeKDELibsBook(DocTreeItem *parent, const QString &name, const QString &idxfilename, const QString &context)
        : DocTreeItem(parent, Book, name, context, true)
{
    setIndexFileName(idxfilename);
}


DocTreeKDELibsBook::~DocTreeKDELibsBook()
{}


QString DocTreeKDELibsBook::fileName()
{
    if (DocTreeItem::fileName().isNull())
        readContents();
    
    return DocTreeItem::fileName() + "/index.html";
}


void DocTreeKDELibsBook::refresh()
{
    DocTreeItem::refresh();
    
    if (DocTreeItem::fileName().isNull())
        readContents();
}


void DocTreeKDELibsBook::readContents()
{
    FILE *f;
    bool success = false;
    if (indexFileName().right(3) != QString::fromLatin1(".gz"))
    {
        if ( (f = fopen(QFile::encodeName( indexFileName() ).data(), "r")) != 0)
        {
            readKdoc2Index(f);
            fclose(f);
            success = true;
        }
    }
    else
    {
        QString cmd = "gzip -c -d ";
#if (KDE_VERSION > 305)
        cmd += KProcess::quote(indexFileName());
#else
        cmd += KShellProcess::quote(indexFileName());
#endif
        cmd += " 2>/dev/null";
        if ( (f = popen(QFile::encodeName(cmd), "r")) != 0)
        {
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

    while (fgets(buf, sizeof buf, f))
    {
        QString s = buf;
        if (s.left(pos0=11) == "<BASE URL=\"")
        {
            int pos2 = s.find("\">", pos0);
            if (pos2 != -1)
                setFileName(s.mid(pos0, pos2-pos0));
        }
        else if (s.left(pos0=9) == "<C NAME=\"")
        {
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
            classItem->postInit();
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
            if (classItem)
            {
                DocTreeItem *item = new DocTreeItem(classItem, Doc, membername, context());
                //              kdDebug ( 9000 ) << "++++++++++++++ " << membername << endl;
                item->postInit();
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
            : DocTreeItem(parent, Folder, name, context, true), m_location(location)
    {}
    virtual void refresh();
private:
    QString m_location;
};


void DocTreeKDELibsFolder::refresh()
{
    DocTreeItem::refresh();
    
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
            ++it1, ++it2)
    {
        (new DocTreeKDELibsBook(this, *it1, *it2, context()))->postInit();
    }
    //}

    sortChildItems(0, true);

    /// @todo merge in default libraries and hidden options

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
                (new DocTreeKDELibsBook(this, *it1, *it2, context()))->postInit();;
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
    
    static bool isInstallationOK(const QString& bookDir)
    {
        return QFile::exists(bookDir + "/html/index.html");
    }

    virtual void refresh();

private:
    QString dirname;
};


DocTreeDoxygenBook::DocTreeDoxygenBook(DocTreeItem *parent, const QString &name,
                                       const QString &dirName, const QString &context)
        : DocTreeItem(parent, Book, name, context, true),
        dirname(dirName)
{
    QString fileName = dirName + "index.html";
    setFileName(fileName);

    QString tagName = dirname + "/" + text(0) + ".tag";
    if (!QFile::exists(tagName))
    {
#if QT_VERSION >= 0x030100
        tagName.remove("/html/");
#else
        tagName.replace( QRegExp( "\\/html\\/" ), QString() );
#endif
    }
    
    setIndexFileName(tagName);
}

void DocTreeDoxygenBook::refresh()
{
    DocTreeItem::refresh();
    
    QFile f(indexFileName());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not open tag file: " << f.name() << endl;
        return;
    }

    QDomDocument dom;
    if (!dom.setContent(&f) || dom.documentElement().nodeName() != "tagfile")
    {
        kdDebug(9002) << "No valid tag file" << endl;
        return;
    }
    f.close();

    QDomElement docEl = dom.documentElement();

    QDomElement childEl = docEl.firstChild().toElement();
    while (!childEl.isNull())
    {
        if (childEl.tagName() == "compound" && childEl.attribute("kind") == "class")
        {
            QString classname = childEl.namedItem("name").firstChild().toText().data();
            QString filename = childEl.namedItem("filename").firstChild().toText().data();

            if (QFile::exists(dirname + filename))
            { // don't create bad links
                DocTreeItem *item = new DocTreeItem(this, Doc, classname, context());
                item->postInit();
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
            : DocTreeItem(parent, Folder, name, context, true), m_location(location)
    {}
    void refresh();
private:
    QString m_location;
};

void DocTreeDoxygenFolder::refresh()
{
    DocTreeItem::refresh();
    
    /*    KConfig *config = DocTreeViewFactory::instance()->config();
        config->setGroup("General");
        QString docdir = config->readPathEntry("kdelibsdocdir", KDELIBS_DOXYDIR);
     
        //kdDebug(9002) << "docdir: " << docdir << endl;*/
    QDir d(m_location);
    QStringList fileList = d.entryList("*", QDir::Dirs);

    QStringList::ConstIterator it;
    for (it = fileList.begin(); it != fileList.end(); ++it)
    {
        QString dirName = (*it);
        //kdDebug(9002) << "dirname: " << dirName << endl;
        if (dirName == "." || dirName == ".." || dirName == "common")
            continue;
        if (DocTreeDoxygenBook::isInstallationOK(d.absFilePath(*it)))
        {
            (new DocTreeDoxygenBook(this, *it, d.absFilePath(*it) + "/html/", context()))->postInit();
            //kdDebug(9002) << "foo: " << d.absFilePath(*it) + "/html/" + *it << endl;
        }
    }

    QFileInfo fi(m_location +"/index.html");
    if (fi.exists())
        setFileName(m_location +"/index.html");

    sortChildItems(0, true);
}


/***************************************/
/* Folder from the 'tocs' resource dir */
/***************************************/

class DocTreeTocFolder : public DocTreeItem
{
public:
    DocTreeTocFolder(const QString& name, KListView *parent, const QString &fileName, const QString &context);
    DocTreeTocFolder(KListView *parent, const QString &fileName, const QString &context);

    QString tocName() const { return toc_name; }

    virtual void refresh();
    
private:
    //void init();
    void addTocSect(DocTreeItem *parent, QDomElement childEl, uint level);
    
    QString base;
    QString toc_name;
};

#if 0
class TocNameExtractor : public QXmlDefaultHandler
{
public:
    TocNameExtractor(DocTreeTocFolder* parent)
        : m_parent(parent)
        , m_titleNext(false)
    {
    }
    
    virtual bool notationDecl(const QString& name, const QString& /*publicId*/, const QString& /*systemId*/)
    {
        if (name != "kdeveloptoc") {
            kdDebug() << "Not a valid kdeveloptoc file: " << m_parent->indexFileName() << endl;
            return false;
        }
        return true;
    }
            
    virtual bool startElement(const QString& /*namespaceURI*/, const QString& localName, const QString& /*qName*/, const QXmlAttributes& /*atts*/)
    {
        if (localName == "title")
            m_titleNext = true;   
        return true;
    }
    
    virtual bool characters(const QString& ch)
    {
        if (m_titleNext) {
            m_parent->setText(0, ch);
            return false;
        }
        return true;
    }
    
private:
    DocTreeTocFolder* m_parent;
    bool m_titleNext;
};
#endif

DocTreeTocFolder::DocTreeTocFolder(const QString& name, KListView *parent, const QString &fileName, const QString &context)
        : DocTreeItem(parent, Folder, fileName, context, true)
{
    setFileName( fileName );
    setIndexFileName( fileName );
    setText(0, name);

    QFileInfo fi(indexFileName());
    toc_name = fi.baseName();
}

DocTreeTocFolder::DocTreeTocFolder(KListView *parent, const QString &fileName, const QString &context)
        : DocTreeItem(parent, Folder, fileName, context, true)
{
    setFileName( fileName );
    setIndexFileName( fileName );
    
    QFileInfo fi(indexFileName());
    toc_name = fi.baseName();

    refresh();
}

#if 0
void DocTreeTocFolder::init()
{
    QFileInfo fi(indexFileName());
    toc_name = fi.baseName();
    base = DocTreeViewTool::tocLocation( indexFileName() );

    QFile f(indexFileName());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read doc toc: " << indexFileName() << endl;
        return;
    }
    
    QXmlInputSource s(&f);
    QXmlSimpleReader r;
    TocNameExtractor t(this);
    r.setContentHandler(&t);
    r.setDTDHandler(&t);
    r.parse(&s);
}
#endif

void DocTreeTocFolder::refresh()
{
    DocTreeItem::refresh();
    
    QFileInfo fi(indexFileName());
    toc_name = fi.baseName();
    base = DocTreeViewTool::tocLocation( indexFileName() );

    QFile f(indexFileName());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read doc toc: " << indexFileName() << endl;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "kdeveloptoc")
    {
        kdDebug() << "Not a valid kdeveloptoc file: " << indexFileName() << endl;
        return;
    }
    f.close();
    
    QDomElement docEl = doc.documentElement();
    QDomElement titleEl = docEl.namedItem("title").toElement();
    setText(0, titleEl.firstChild().toText().data());

    QDomElement childEl = docEl.firstChild().toElement();
    
    //!!! finally infinite tocsect depth implemented
    addTocSect(0, childEl, 1);
}


void DocTreeTocFolder::addTocSect(DocTreeItem *parent, QDomElement childEl, uint level)
{
    QListViewItem *lastChildItem = 0;
    while (!childEl.isNull())
    {
        if (childEl.tagName() == QString("tocsect%1").arg(level))
        {
            QString name = childEl.attribute("name");
            QString url = childEl.attribute("url");
            DocTreeItem *item = 0;
            if (parent == 0) {
                item = new DocTreeItem(this, Book, name, DocTreeItem::context());
                item->postInit();
            } else {
                item = new DocTreeItem(parent, Doc, name, DocTreeItem::context());
                item->postInit();
            }
            
            if (!url.isEmpty())
                item->setFileName(base + url);

            if (lastChildItem)
                item->moveItem(lastChildItem);
            lastChildItem = item;

            QDomElement grandchildEl = childEl.firstChild().toElement();
            addTocSect(item, grandchildEl, level+1);
        }
        childEl = childEl.nextSibling().toElement();
    }
}

/****************************************************/
/* Folder from the DevHelp documentation collection */
/****************************************************/
class DocTreeDevHelpFolder : public DocTreeItem
{
public:
    DocTreeDevHelpFolder(KListView *parent, const QString &fileName, const QString &context);

    QString tocName() const { return toc_name; }

    virtual void refresh();
    
private:
    void addTocSect(DocTreeItem *parent, QDomElement childEl);
    
    QString base;
    QString toc_name;
};

DocTreeDevHelpFolder::DocTreeDevHelpFolder(KListView *parent, const QString &fileName, const QString &context)
        : DocTreeItem(parent, Folder, fileName, context)
{
    setIndexFileName(fileName);
}

void DocTreeDevHelpFolder::refresh()
{
    DocTreeItem::refresh();
    
    QTime t;
    t.start();
    
    QFileInfo fi(indexFileName());
    toc_name = fi.baseName();
    base = DocTreeViewTool::devhelpLocation( indexFileName() );

    QFile f(indexFileName());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read devhelp toc: " << indexFileName() << endl;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f))
    {
        kdDebug() << "Not a valid devhelp file: " << indexFileName() << endl;
        return;
    }
    f.close();
    
    QDomElement docEl = doc.documentElement();
    QDomElement chaptersEl = docEl.namedItem("chapters").toElement();
    setText(0, docEl.attribute("title"));
    setFileName( base + docEl.attribute("link") );

    QDomElement childEl = chaptersEl.firstChild().toElement();
    addTocSect(0, childEl);
}

void DocTreeDevHelpFolder::addTocSect(DocTreeItem *parent, QDomElement childEl)
{
    QListViewItem *lastChildItem = 0;
    while (!childEl.isNull())
    {
        if ( (childEl.tagName() == "sub") || (childEl.tagName() == "chapter"))
        {
            QString name = childEl.attribute("name");
            QString url = childEl.attribute("link");
            DocTreeItem *item = 0;
            if (parent == 0) {
                item = new DocTreeItem(this, Book, name, DocTreeItem::context());
                item->postInit();
            } else {
                item = new DocTreeItem(parent, Doc, name, DocTreeItem::context());
                item->postInit();
            }
            if (!url.isEmpty())
                item->setFileName(base + url);

            if (lastChildItem)
                item->moveItem(lastChildItem);
            lastChildItem = item;

            QDomElement grandchildEl = childEl.firstChild().toElement();
            addTocSect(item, grandchildEl);
        }
        childEl = childEl.nextSibling().toElement();
    }
}

/*************************************/
/* Folder "Documentation Base"       */
/*************************************/

#ifdef WITH_DOCBASE


class DocTreeDocbaseFolder : public DocTreeItem
{
public:
    DocTreeDocbaseFolder(KListView *parent, const QString &context);
    virtual void refresh();
private:
    void readDocbaseFile(FILE *f);
};


DocTreeDocbaseFolder::DocTreeDocbaseFolder(KListView *parent, const QString &context)
        : DocTreeItem(parent, Folder, i18n("Documentation Base"), context)
{
}


void DocTreeDocbaseFolder::readDocbaseFile(FILE *f)
{
    char buf[1024];
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
            DocTreeItem *item = new DocTreeItem(this, Doc, title, context());
            item->postInit();
            item->setFileName(filename);
            break;
        }
        else if (s.left(9) == "Section: "
                 && s.find("programming", 9, false) == -1)
            break;
    }
}


void DocTreeDocbaseFolder::refresh()
{
    DocTreeItem::refresh();
    
    QDir d("/usr/share/doc-base");
    QStringList fileList = d.entryList("*", QDir::Files);
    QStringList::Iterator it;
    for (it = fileList.begin(); it != fileList.end(); ++it)
    {
        FILE *f;
        if ( (f = fopen( QFile::encodeName(d.filePath(*it)), "r")) != 0)
        {
            readDocbaseFile(f);
            fclose(f);
        }
    }
}


#endif


/*************************************/
/* Folder "Bookmarks"                */
/*************************************/

class DocTreeBookmarksFolder : public DocTreeItem
{
public:
    DocTreeBookmarksFolder(KListView *parent, const QString &context);
    virtual void refresh();
};

DocTreeBookmarksFolder::DocTreeBookmarksFolder(KListView *parent, const QString &context)
        : DocTreeItem(parent, Folder, i18n("Bookmarks"), context)
{}

void DocTreeBookmarksFolder::refresh()
{
    DocTreeItem::refresh();

    QStringList othersTitle, othersURL;
    DocTreeViewTool::getBookmarks(&othersTitle, &othersURL);
    QStringList::Iterator it1, it2;
    for (it1 = othersTitle.begin(), it2 = othersURL.begin();
            it1 != othersTitle.end() && it2 != othersURL.end();
            ++it1, ++it2)
    {
        DocTreeItem *item = new DocTreeItem(this, Book, *it1, context());
        item->postInit();
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
    virtual void refresh();

private:
    KDevProject *m_project;
    QString m_userdocDir, m_apidocDir;
};

DocTreeProjectFolder::DocTreeProjectFolder(KListView *parent, const QString &context)
        : DocTreeItem(parent, Folder, i18n("Current Project"), context), m_project(0)
{}


void DocTreeProjectFolder::refresh()
{
    DocTreeItem::refresh();
    
    /// @todo use doxygen tags
    if( !m_project )
        return;

    m_userdocDir = DomUtil::readEntry(
                       *m_project->projectDom() , "/kdevdoctreeview/projectdoc/userdocDir");
    m_apidocDir = DomUtil::readEntry(
                      *m_project->projectDom() , "/kdevdoctreeview/projectdoc/apidocDir");
    
    // API documentation
    QDir apidir( m_apidocDir );
    if (apidir.exists())
    {
        QStringList entries = apidir.entryList("*.html", QDir::Files);
        QString filename = apidir.absPath() + "/index.html";
        if (!QFileInfo(filename).exists())
            return;
        DocTreeItem *item = new DocTreeItem(
                                this, Book, i18n("API of %1").arg(m_project->projectName() ), context());
        item->postInit();
        item->setFileName(filename);
        for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
        {
            filename = *it;
            DocTreeItem *ditem = new DocTreeItem(item,
                                                 Doc, QFileInfo(filename).baseName() , context());
            ditem->postInit();
            ditem->setFileName(apidir.absPath() +"/"+ filename);
        }
    }
    // User documentation
    QDir userdir( m_userdocDir );
    if (userdir.exists())
    {
        QStringList entries = userdir.entryList("*.html", QDir::Files);
        QString filename = userdir.absPath() + "/index.html";
        if (!QFileInfo(filename).exists())
            return;
        DocTreeItem *item = new DocTreeItem(
                                this, Book, i18n("Usedoc for %1").arg(m_project->projectName() ), context());
        item->postInit();
        item->setFileName(filename);
        for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
        {
            filename = *it;
            DocTreeItem *ditem = new DocTreeItem(item,
                                                 Doc, QFileInfo(filename).baseName() , context());
            ditem->postInit();
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
};

DocTreeQtFolder::DocTreeQtFolder(QString xml, QString name, KListView *parent,
                                 const QString &context)
        : DocTreeItem(parent, Folder, name, context, true)
{
    setIndexFileName(xml);
}

void DocTreeQtFolder::refresh()
{
    DocTreeItem::refresh();
    
    QFileInfo fi(indexFileName());

    QFile f(indexFileName());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << indexFileName() << endl;
        //kdDebug(9002) << "  Filename was: " << filename << endl;
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "DCF")
    {
        kdDebug(9002) << "Not a valid DCF file: " << indexFileName() << endl;
        return;
    }

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
            item->postInit();
            item->setFileName(fi.dirPath( true ) +"/"+ ref);

            QDomElement grandChild = childEl.lastChild().toElement();
            while(!grandChild.isNull())
            {
                if (grandChild.tagName() == "keyword")
                {
                    QString dref = grandChild.attribute("ref");
                    QString dtitle = grandChild.text();

                    DocTreeItem* dItem = new DocTreeItem(item, Doc, dtitle, context());
                    dItem->postInit();
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
    return true;
}

/**************************************/
/* The DocTreeViewWidget itself       */
/**************************************/

DocTreeViewWidget::DocTreeViewWidget(DocTreeViewPart *part)
        : QVBox(0, "doc tree widget"), m_activeTreeItem ( 0L ), indexMode ( filteredMode ), // will be switched
        subStringSearch( false )
{

    /* initializing the tree/index switch */
    modeSwitch = new KTabCtl(this, "mode switch");

    /* create the 2 tabs */
    treeWidget = new QVBox(modeSwitch, "tree mode widget");
    indexWidget = new QVBox(modeSwitch, "index mode widget");

    // INDEX MODE INIT
    QHBox *hbo = new QHBox(indexWidget, "label + edit");
    hbo->setMargin( 2 );
    QLabel *l = new QLabel( 0, i18n("Se&arch:"), hbo, "search-label" );
    filterEdit = new DocLineEdit( hbo, "index mode filter line edit" );
    l->setBuddy(filterEdit);

    subSearchButton = new QToolButton ( hbo, "sub search check" );
    subSearchButton->setSizePolicy ( QSizePolicy ( (QSizePolicy::SizeType)0, ( QSizePolicy::SizeType)0, 0, 0, 0) );
    subSearchButton->setPixmap ( SmallIcon ( "grep" ) );
    subSearchButton->setToggleButton(true);
    QToolTip::add ( subSearchButton, i18n ( "Search substrings" ) );
    QWhatsThis::add(subSearchButton, i18n("<b>Search substrings</b><p>Index view searches for substrings in index items if toggled."));

    indexModeSwitch = new QToolButton ( hbo, "index mode switch" );
    indexModeSwitch->setSizePolicy ( QSizePolicy ( (QSizePolicy::SizeType)0, ( QSizePolicy::SizeType)0, 0, 0, 0) );
    indexModeSwitch->setPixmap ( SmallIcon ( "contents" ) );
    indexModeSwitch->setToggleButton(true);
    indexModeSwitch->setOn( true );
    QToolTip::add ( indexModeSwitch, i18n ( "Show topics for index items" ) );
    QWhatsThis::add(indexModeSwitch, i18n("<b>Show topics for index items</b><p>Index view shows topics to which index items belong if toggled."));

    indexView = new KListView ( indexWidget, "documentation index list view" );

    indexView->setFocusPolicy(ClickFocus);
    indexView->setResizeMode(QListView::LastColumn);
    indexView->addColumn(QString::null);
    indexView->setSorting(0);
    indexView->header()->hide();

    connect ( filterEdit, SIGNAL ( textChanged(const QString &) ), this, SLOT ( slotFilterTextChanged(const QString &) ) );
    connect ( filterEdit, SIGNAL ( returnPressed() ), this, SLOT ( slotFilterReturn() ) );
    connect ( filterEdit, SIGNAL ( upPressed() ), this, SLOT ( slotIndexPrevMatch() ) );
    connect ( filterEdit, SIGNAL ( downPressed() ), this, SLOT ( slotIndexNextMatch() ) );
    connect ( filterEdit, SIGNAL ( pgupPressed() ), this, SLOT ( slotIndexPgUp() ) );
    connect ( filterEdit, SIGNAL ( pgdownPressed() ), this, SLOT ( slotIndexPgDown() ) );
    connect ( filterEdit, SIGNAL ( homePressed() ), this, SLOT ( slotIndexHome() ) );
    connect ( filterEdit, SIGNAL ( endPressed() ), this, SLOT ( slotIndexEnd() ) );
    connect ( indexView, SIGNAL ( executed(QListViewItem *) ), this, SLOT ( slotIndexItemExecuted(QListViewItem *) ) );
    connect ( indexView, SIGNAL ( returnPressed(QListViewItem *) ), this, SLOT ( slotIndexItemExecuted(QListViewItem *) ) );

    connect ( modeSwitch, SIGNAL ( tabSelected(int) ), this, SLOT ( slotCurrentTabChanged(int) ) );

    connect ( subSearchButton, SIGNAL ( clicked() ), this, SLOT ( slotSubstringCheckClicked() ) );
    connect ( indexModeSwitch, SIGNAL ( clicked() ), this, SLOT ( slotIndexModeCheckClicked() ) );

    // TREE MODE INIT

    /* initializing the documentation tree toolbar */
    searchToolbar = new QHBox ( treeWidget, "search toolbar" );
    searchToolbar->setMargin ( 2 );
    searchToolbar->setSpacing ( 2 );

    completionCombo = new KHistoryCombo ( true, searchToolbar, "completion combo box" );

    startButton = new QToolButton ( searchToolbar, "start searching" );
    startButton->setSizePolicy ( QSizePolicy ( (QSizePolicy::SizeType)0, ( QSizePolicy::SizeType)0, 0, 0, startButton->sizePolicy().hasHeightForWidth()) );
    startButton->setPixmap ( SmallIcon ( "key_enter" ) );
    QToolTip::add ( startButton, i18n ( "Start searching" ) );
    QWhatsThis::add(startButton, i18n("<b>Start searching</b><p>Searches through the documentation topics for a given term and shows the topic found."));

    nextButton = new QToolButton ( searchToolbar, "next match button" );
    nextButton->setSizePolicy ( QSizePolicy ( ( QSizePolicy::SizeType )0, ( QSizePolicy::SizeType) 0, 0, 0, nextButton->sizePolicy().hasHeightForWidth()) );
    nextButton->setPixmap ( SmallIcon ( "next" ) );
    QToolTip::add ( nextButton, i18n ( "Jump to next matching entry" ) );
    QWhatsThis::add(nextButton, i18n("<b>Jump to next matching entry</b><p>Shows the next topic found."));
    nextButton->setEnabled( false );

    prevButton = new QToolButton ( searchToolbar, "previous match button" );
    prevButton->setSizePolicy ( QSizePolicy ( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, prevButton->sizePolicy().hasHeightForWidth()) );
    prevButton->setPixmap ( SmallIcon ( "previous" ) );
    QToolTip::add ( prevButton, i18n ( "Jump to previous matching entry" ) );
    QWhatsThis::add(prevButton, i18n("<b>Jump to previous matching entry</b><p>Shows the previous topic found."));
    prevButton->setEnabled( false );

    docView = new KListView ( treeWidget, "documentation list view" );

    docView->setFocusPolicy(ClickFocus);
    docView->setRootIsDecorated(true);
    docView->setResizeMode(QListView::LastColumn);
    docView->setSorting(-1);
    docView->header()->hide();
    docView->addColumn(QString::null);

    folder_bookmarks = new DocTreeBookmarksFolder(docView, "ctx_bookmarks");
    folder_bookmarks->postInit();

    folder_project   = new DocTreeProjectFolder(docView, "ctx_current");
    folder_project->postInit();

#ifdef WITH_DOCBASE
    folder_docbase   = new DocTreeDocbaseFolder(docView, "ctx_docbase");
    folder_docbase->postInit();
#endif

    // devhelp docs
    KConfig *configdh = DocTreeViewFactory::instance()->config();
    if (configdh)
    {
        configdh->setGroup("TocDevHelp");
        QString firstScan = configdh->readEntry("FirstScan", "yes");
        if (firstScan != "no")
        {
            DocTreeViewTool::scanDevHelpDirs();
            configdh->writeEntry("FirstScan", "no");
        }
    }

    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
    QStringList dhtocs = dirs->findAllResources("docdevhelp", QString::null, false, true);
    for (QStringList::Iterator tit = dhtocs.begin(); tit != dhtocs.end(); ++tit) {
        DocTreeDevHelpFolder* item = new DocTreeDevHelpFolder(docView, *tit, QString("ctx_%1").arg(*tit));
        item->postInit();
        folder_devhelp.append(item);
    }

    // doctocs
    // We're caching title only because it is a huge startup speed gain to not have to extract the title from the XML
    QStringList tocs = dirs->findAllResources("doctocs", QString::null, false, true);
    QString cache = dirs->findResource("data", "kdevdoctreeview/docpartcache");
    bool regenerateCache = false;
    QFile cacheFile(cache);
    if (!cache.isEmpty() && cacheFile.open(IO_ReadOnly)) {
      QDataStream ds(&cacheFile);
      int version;
      ds >> version;
      // Opening cache
      if (version == 1) {
        QString fileName, title;
        while (!ds.atEnd()) {
          ds >> fileName >> title;
          if (tocs.contains(fileName) && QFileInfo(fileName).lastModified() < QFileInfo(cacheFile).lastModified()) {
            // Cache hit!
            DocTreeTocFolder* item = new DocTreeTocFolder(title, docView, fileName, QString("ctx_%1").arg(fileName));
            item->postInit();
            folder_toc.append(item);
            tocs.remove(fileName);
          } else {
            // couldn't find toc, may have been uninstalled. don't need to regenerate.
          }
        }
      } else {
        // Incorrect cache version
        regenerateCache = true;
      }
      cacheFile.close();
    }
    
    if (tocs.count()) {
      regenerateCache = true;
      QStringList tocs = dirs->findAllResources("doctocs", QString::null, false, true);
      for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit) {
          DocTreeTocFolder* item = new DocTreeTocFolder(docView, *tit, QString("ctx_%1").arg(*tit));
          item->postInit();
          folder_toc.append(item);
      }
    }
    
    if (regenerateCache) {
      // update cache here
      if (cache.isEmpty()) {
        cache = dirs->saveLocation("data");
        cache += "kdevdoctreeview/docpartcache";
        cacheFile.setName(cache);
      }
      // Creating cache
      cacheFile.open(IO_WriteOnly);
      QDataStream ds(&cacheFile);
      ds << 1;
      for (DocTreeTocFolder* f = folder_toc.first(); f; f = folder_toc.next())
        ds << f->fileName() << f->text(0);
    }

    //    initKDocKDELibs();

    KConfig *config = DocTreeViewFactory::instance()->config();
    if (config)
    {
        config->setGroup("General KDoc");
        QMap<QString, QString> dmap = config->entryMap("General KDoc");
        QString kdocdir(KDELIBS_DOCDIR);
        kdocdir = URLUtil::envExpand(kdocdir);
        if (dmap.empty() && (!kdocdir.isEmpty()))
        {
            config->writePathEntry("KDE Libraries (KDoc)", kdocdir);
            dmap["KDE Libraries (KDoc)"] = kdocdir;
        }

        QMap<QString, QString>::Iterator it;
        for (it = dmap.begin(); it != dmap.end(); ++it)
        {
            DocTreeKDELibsFolder *kdf = new DocTreeKDELibsFolder(it.data(), it.key(), docView, "ctx_kdelibs");
            kdf->postInit();
            folder_kdoc.append(kdf);
        }
    }

    if (config)
    {
        config->setGroup("General Doxygen");
        QMap<QString, QString> xmap = config->entryMap("General Doxygen");
        QString doxydir(KDELIBS_DOXYDIR);
        doxydir = URLUtil::envExpand(doxydir);
        if (xmap.empty() && (!doxydir.isEmpty()))
        {
            config->writePathEntry("KDE Libraries (Doxygen)", doxydir);
            xmap["KDE Libraries (Doxygen)"] = doxydir;
        }

        QMap<QString, QString>::Iterator it;
        for (it = xmap.begin(); it != xmap.end(); ++it)
        {
            DocTreeDoxygenFolder *dxf = new DocTreeDoxygenFolder(config->readPathEntry(it.key()), it.key(), docView, "ctx_doxygen");
            dxf->postInit();
            folder_doxygen.append(dxf);
        }
    }


    //    folder_doxygen   = new DocTreeDoxygenFolder(docView, "ctx_doxygen");

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

        QString qtdocdir(config->readPathEntry("qtdocdir", QT_DOCDIR));
        qtdocdir = URLUtil::envExpand(qtdocdir);
        if (emap.empty() && (!qtdocdir.isEmpty()))
        {
#if QT_VERSION >= 0x030200
            config->writePathEntry("Qt Reference Documentation", qtdocdir + QString("/qt.dcf"));
            emap["Qt Reference Documentation"] = qtdocdir + QString("/qt.dcf");
            config->writePathEntry("Qt Assistant Manual", qtdocdir + QString("/assistant.dcf"));
            emap["Qt Assistant Manual"] = qtdocdir + QString("/assistant.dcf");
            config->writePathEntry("Qt Designer Manual", qtdocdir + QString("/designer.dcf"));
            emap["Qt Designer Manual"] = qtdocdir + QString("/designer.dcf");
            config->writePathEntry("Guide to the Qt Translation Tools", qtdocdir + QString("/linguist.dcf"));
            emap["Guide to the Qt Translation Tools"] = qtdocdir + QString("/linguist.dcf");
            config->writePathEntry("qmake User Guide", qtdocdir + QString("/qmake.dcf"));
            emap["qmake User Guide"] = qtdocdir + QString("/qmake.dcf");
#else
            config->writePathEntry("Qt Reference Documentation", qtdocdir + QString("/qt.xml"));
            emap["Qt Reference Documentation"] = qtdocdir + QString("/qt.xml");
            config->writePathEntry("Qt Assistant Manual", qtdocdir + QString("/assistant.xml"));
            emap["Qt Assistant Manual"] = qtdocdir + QString("/assistant.xml");
            config->writePathEntry("Qt Designer Manual", qtdocdir + QString("/designer.xml"));
            emap["Qt Designer Manual"] = qtdocdir + QString("/designer.xml");
            config->writePathEntry("Guide to the Qt Translation Tools", qtdocdir + QString("/linguist.xml"));
            emap["Guide to the Qt Translation Tools"] = qtdocdir + QString("/linguist.xml");
            config->writePathEntry("qmake User Guide", qtdocdir + QString("/qmake.xml"));
            emap["qmake User Guide"] = qtdocdir + QString("/qmake.xml");
#endif
        }

        QMap<QString, QString>::Iterator it;
        for (it = emap.begin(); it != emap.end(); ++it)
        {
            DocTreeQtFolder *qtf = new DocTreeQtFolder(it.data(), it.key(), docView, "ctx_qt");
            qtf->postInit();
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

//    modeSwitch->setShape(QTabBar::TriangularAbove);
    modeSwitch->setBorder(false);
    modeSwitch->addTab(treeWidget, i18n("Co&ntents"));
    modeSwitch->addTab(indexWidget, i18n("&Index"));
}


DocTreeViewWidget::~DocTreeViewWidget()
{}

void DocTreeViewWidget::searchForItem ( const QString& currentText )
{
    completionCombo->addToHistory( currentText );

    QListViewItemIterator docViewIterator( docView );
    while( docViewIterator.current() )
    {
        // Load this category
        docViewIterator.current()->setOpen(true);
        docViewIterator.current()->setOpen(false);

        if( docViewIterator.current()->text(0).find( currentText, false ) >= 0 )
        {
            searchResultList.append( docViewIterator.current() );
        }
        ++docViewIterator;
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
    if (ident.right(4) == ".toc")
    {
        dtitem = static_cast<DocTreeItem*>(dtitem->firstChild());
        if (!dtitem) return;
        ident = dtitem->fileName();
        if (ident.isEmpty()) return;
    }

    kdDebug(9002) << "Showing: " << ident << endl;
    m_part->partController()->showDocument(KURL(ident));
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
    if ( i == folder_project )
    {
        int id = popup.insertItem(i18n("Project Properties"), this, SLOT(slotConfigureProject()));
        popup.setWhatsThis(id, i18n("<b>Project properties</b><p>Displays <b>Project Documentation</b> properties dialog."));
    }
    else
    {
        int id = popup.insertItem(i18n("Properties"), this, SLOT(slotConfigure()));
        popup.setWhatsThis(id, i18n("<b>Properties</b><p>Displays <b>Documentation Tree</b> properties dialog."));
    }
    if ( i != folder_bookmarks && dItem && !dItem->fileName().isEmpty() )
    {
        int id = popup.insertItem(i18n("Add to Bookmarks"), this, SLOT(slotAddBookmark()));
        dcontext = DocumentationContext( dItem->fileName(), dItem->text(0) );
        popup.setWhatsThis(id, i18n("<b>Add to bookmarks</b><p>Adds currently selected topic to the bookmarks list."));
    }
    if (  contextItem->parent() && dItem && contextItem->parent() == folder_bookmarks )
    {
        int id = popup.insertItem(i18n("Remove"), this, SLOT(slotRemoveBookmark()));
        popup.setWhatsThis(id, i18n("<b>Remove</b><p>Removes currently selected bookmark from the bookmarks list."));
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

    if(m_part->project())
    {
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
    folder_bookmarks->refresh();
    folder_project->refresh();

    /*    if( folder_kdelibs )
            folder_kdelibs->refresh();*/

    folder_devhelp.setAutoDelete(true);
    folder_devhelp.clear();
    folder_devhelp.setAutoDelete(false);

    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();
    QStringList dhtocs = dirs->findAllResources("docdevhelp", QString::null, false, true);
    QStringList ignoredh( DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoredevhelp", "toc") );

    for (QStringList::Iterator tit = dhtocs.begin(); tit != dhtocs.end(); ++tit)
    {
        if( !ignoredh.contains( QFileInfo(*tit).baseName() ) ) {
            DocTreeDevHelpFolder* item = new DocTreeDevHelpFolder(docView, *tit, QString("ctx_%1").arg(*tit));
            item->postInit();
            folder_devhelp.append(item);
        }
    }


    DocTreeTocFolder *item;
    for ( item = folder_toc.first(); item; item = folder_toc.next() )
        delete item;

    folder_toc.clear();

    QStringList tocs = dirs->findAllResources("doctocs", QString::null, false, true);
    QStringList ignore( DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoretocs", "toc") );

    for (QStringList::Iterator tit = tocs.begin(); tit != tocs.end(); ++tit)
    {
        if( !ignore.contains( QFileInfo(*tit).baseName() ) ) {
            DocTreeTocFolder* item = new DocTreeTocFolder(docView, *tit, QString("ctx_%1").arg(*tit));
            item->postInit();
            folder_toc.append(item);
        }
    }

    folder_kdoc.setAutoDelete(true);
    folder_kdoc.clear();
    folder_kdoc.setAutoDelete(false);

    KConfig *config = DocTreeViewFactory::instance()->config();
    if (config)
    {
        QStringList ignorekdocs( DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignorekdocs", "toc") );
        config->setGroup("General KDoc");
        QMap<QString, QString> emap = config->entryMap("General KDoc");
        QMap<QString, QString>::Iterator it;
        for (it = emap.begin(); it != emap.end(); ++it)
        {
            if (!ignorekdocs.contains(it.key()))
            {
                DocTreeKDELibsFolder *kdf = new DocTreeKDELibsFolder(it.data(), it.key(), docView, "ctx_kdelibs");
                kdf->postInit();
                folder_kdoc.append(kdf);
            }
        }
    }

    folder_doxygen.setAutoDelete(true);
    folder_doxygen.clear();
    folder_doxygen.setAutoDelete(false);

    if (config)
    {
        QStringList ignoredoxygen( DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoredoxygen", "toc") );
        config->setGroup("General Doxygen");
        QMap<QString, QString> emap = config->entryMap("General Doxygen");
        QMap<QString, QString>::Iterator it;
        for (it = emap.begin(); it != emap.end(); ++it)
        {
            if (!ignoredoxygen.contains(it.key()))
            {
                DocTreeDoxygenFolder *dxf = new DocTreeDoxygenFolder(it.data(), it.key(), docView, "ctx_doxygen");
                dxf->postInit();
                folder_doxygen.append(dxf);
            }
        }
    }


    folder_qt.setAutoDelete(true);
    folder_qt.clear();
    folder_qt.setAutoDelete(false);

    if (config)
    {
        QStringList ignoreqt_xml( DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoreqt_xml", "toc") );
        config->setGroup("General Qt");
        QMap<QString, QString> emap = config->entryMap("General Qt");
        QMap<QString, QString>::Iterator it;
        for (it = emap.begin(); it != emap.end(); ++it)
        {
            if (!ignoreqt_xml.contains(it.key()))
            {
                DocTreeQtFolder *qtf = new DocTreeQtFolder(it.data(), it.key(), docView, "ctx_qt");
                qtf->postInit();
                folder_qt.append(qtf);
            }
        }
    }
}


void DocTreeViewWidget::projectChanged(KDevProject *project)
{
    folder_project->setProject(project);

    //FIXME: this takes more time to load but don't cause crashes somewhere in KStyle
//    QTimer::singleShot(0, this, SLOT(refresh()));
    
    folder_project->refresh();
    if(!project)
    {
        kdDebug(9002) << "No Project...." << endl;
        return;
    }


    // Remove all...
    docView->takeItem(folder_bookmarks);
    docView->takeItem(folder_project);
#ifdef WITH_DOCBASE
    docView->takeItem(folder_docbase);
#endif
    QPtrListIterator<DocTreeDevHelpFolder> itdh(folder_devhelp);
    for (; itdh.current(); ++itdh)
        docView->takeItem(itdh.current());

    QPtrListIterator<DocTreeTocFolder> it1(folder_toc);
    for (; it1.current(); ++it1)
        docView->takeItem(it1.current());

    QPtrListIterator<DocTreeKDELibsFolder> itk(folder_kdoc);
    for (; itk.current(); ++itk)
        docView->takeItem(itk.current());
    //    if(folder_doxygen) docView->takeItem(folder_doxygen);
    QPtrListIterator<DocTreeDoxygenFolder> itx(folder_doxygen);
    for (; itx.current(); ++itx)
        docView->takeItem(itx.current());

    QPtrListIterator<DocTreeQtFolder> itq(folder_qt);
    for (; itq.current(); ++itq)
        docView->takeItem(itq.current());
    //    if(folder_qt) docView->takeItem(folder_qt);
    //    if(folder_kdelibs) docView->takeItem(folder_kdelibs);
    //    docView->takeItem(folder_kdevelop);

    // .. and insert all again except for ignored items
    QStringList ignoretocs = DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoretocs", "toc");
//    kdDebug() << "        !!!!!! IGNORE TOCS: " << ignoretocs << endl;
    QStringList ignoredh = DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoredevhelp", "toc");
    QStringList ignoredoxygen = DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoredoxygen", "toc");
    QStringList ignorekdocs = DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignorekdocs", "toc");
    QStringList ignoreqt_xml = DomUtil::readListEntry(*m_part->projectDom(), "/kdevdoctreeview/ignoreqt_xml", "toc");

    docView->insertItem(folder_bookmarks);
    docView->insertItem(folder_project);
#ifdef WITH_DOCBASE
    docView->insertItem(folder_docbase);
#endif
    QPtrListIterator<DocTreeDevHelpFolder> itdh2(folder_devhelp);
    for (; itdh2.current(); ++itdh2)
    {
        if (!ignoredh.contains(itdh2.current()->tocName()))
            docView->insertItem(itdh2.current());
    }

    QPtrListIterator<DocTreeTocFolder> it2(folder_toc);
    //    it2.toLast();
    //    for (; it2.current(); --it2) {
    for (; it2.current(); ++it2)
    {
//        kdDebug() << "            -- name " << it2.current()->tocName() << endl;
        if (!ignoretocs.contains(it2.current()->tocName()))
            docView->insertItem(it2.current());
    }

    //    docView->insertItem(folder_doxygen);
    QPtrListIterator<DocTreeKDELibsFolder> itk2(folder_kdoc);
    //    itk2.toLast();
    //    for (; itk2.current(); --itk2)
    for (; itk2.current(); ++itk2)
    {
        if (!ignorekdocs.contains(itk2.current()->text(0)))
            docView->insertItem(itk2.current());
    }

    QPtrListIterator<DocTreeDoxygenFolder> itx2(folder_doxygen);
    //    itx2.toLast();
    //    for (; itx2.current(); --itx2)
    for (; itx2.current(); ++itx2)
    {
        if (!ignoredoxygen.contains(itx2.current()->text(0)))
            docView->insertItem(itx2.current());
    }

    /*    if(folder_kdelibs && kdelibskdoc )
            if (!ignoretocs.contains("kde"))
                docView->insertItem(folder_kdelibs);
    */
    QPtrListIterator<DocTreeQtFolder> itq2(folder_qt);
    //    itq2.toLast();
    //    for (; itq2.current(); --itq2)
    for (; itq2.current(); ++itq2)
    {
        if (!ignoreqt_xml.contains(itq2.current()->text(0)))
            docView->insertItem(itq2.current());
    }
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

/* INDEX MODE SLOTS */

void DocTreeViewWidget::slotFilterTextChanged(const QString &nt)
{
    int res;
    QListViewItemIterator docViewIterator( indexView );
    QListViewItem *it = 0;

    while( docViewIterator.current() )
    {
        res = docViewIterator.current()->text(0).find(nt, 0, false);
        //  Not listed  OR  substring but !bSub
        if( (res == -1) || (!subStringSearch && (res > 0)) ) docViewIterator.current()->setVisible(false);
        else
        {
            if (it == 0) it = docViewIterator.current();
            docViewIterator.current()->setVisible(true);
        }

        ++docViewIterator;
    }
    if (it)
        indexView->setCurrentItem(it);
}

void DocTreeViewWidget::slotFilterReturn()
{
    indexView->setFocus();
/*    if (indexView->currentItem())
    {
        indexView->setFocus();
        slotItemExecuted(indexView->currentItem());
    }*/
}

void DocTreeViewWidget::slotIndexItemExecuted(QListViewItem *item)
{
    IndexTreeData *itd;
    QPtrList<IndexTreeData> *resultList;

    if (!item) return;

    QString ident = item->text(0);
    if (ident.isEmpty()) return;

    QPtrListIterator<IndexTreeData> ptrListIterator( indexItems );
    resultList = new QPtrList<IndexTreeData>();

    while( ptrListIterator.current() )
    {
        itd = static_cast<IndexTreeData *>(ptrListIterator.current());

        if((indexMode == filteredMode) && (itd->text() == ident)) resultList->append(itd);
        else if((indexMode == plainListMode) && ((itd->text() + " (" + itd->parent() + ")") == ident)) resultList->append(itd);
        ++ptrListIterator;
    }

    if(resultList->count() == 0)
    {
    }
    else if(resultList->count() == 1)
    {
        m_part->partController()->showDocument(KURL::fromPathOrURL( resultList->first()->fileName() ));
        m_part->mainWindow()->lowerView(this);
    }
    else
    {
        ChooseDlg chooseDlg(this, "choose dlg", m_part);
        chooseDlg.setList(resultList);
        chooseDlg.exec();
    }

    delete resultList;
}

void DocTreeViewWidget::filterMultiReferences()
{
    bool bFound;
    IndexTreeData *itd;
    IndexTreeData *itd2;

    QPtrListIterator<IndexTreeData> ptrListIterator( indexItems );
    while( ptrListIterator.current() )
    {
        itd = static_cast<IndexTreeData *>(ptrListIterator.current());
        bFound = false;

        QPtrListIterator<IndexTreeData> ptrListIterator2( indexItems );
        while( ptrListIterator2.current() )
        {
            itd2 = static_cast<IndexTreeData *>(ptrListIterator2.current());

            if( itd2->isVisible() && (itd2->text() == itd->text()) )
            {
                bFound = true;
                break;
            }
            ++ptrListIterator2;
        }

        // Make it visible !
        if(bFound == false) itd->setVisible(true);

        qApp->processEvents();
        ++ptrListIterator;
    }
}

void DocTreeViewWidget::slotCurrentTabChanged(int curtab)
{
    IndexTreeData *iI;
    
    if((curtab == 1) && (indexView->childCount() == 0))
    {
        // Index list not created yet
        QProgressDialog progress(i18n("Generating Index..."), 0, 100, this, "progDialog", true);

        indexItems.clear();
        progress.setProgress(0);

        /* Load Data out of the DocView -> easier changes later, but bit slower*/
        QListViewItemIterator docViewIterator( docView );
        while( docViewIterator.current() )
        {
            // Load this category
            docViewIterator.current()->setOpen(true);
            docViewIterator.current()->setOpen(false);

            DocTreeItem *dtitem = static_cast<DocTreeItem*>(docViewIterator.current());

            // Save them in a list
            iI = new IndexTreeData(dtitem->text(0), (dtitem->parent() != 0) ? dtitem->parent()->text(0) : QString(" "), dtitem->fileName());
            indexItems.append(iI);

            qApp->processEvents();
            ++docViewIterator;
        }
        progress.setProgress(30);

//        filterMultiReferences();

        progress.setProgress(90);

        // Put them in the list
        slotIndexModeCheckClicked();

        progress.setProgress(100);
    }
}

void DocTreeViewWidget::slotSubstringCheckClicked()
{
    subStringSearch = !subStringSearch;
    slotFilterTextChanged(filterEdit->text());
}

void DocTreeViewWidget::slotIndexModeCheckClicked()
{
    QString s;

    if(indexMode == filteredMode)   indexMode = plainListMode;
    else                            indexMode = filteredMode;

    indexView->clear();

    QPtrListIterator<IndexTreeData> ptrListIterator( indexItems );
    while( ptrListIterator.current() )
    {
        IndexTreeData *itd = static_cast<IndexTreeData *>(ptrListIterator.current());

        if(indexMode == plainListMode) s = itd->text() + " (" + itd->parent() + ")";
        else s = itd->text();

        if((indexMode == plainListMode) || itd->isVisible()) new QListViewItem(indexView, s);

        ++ptrListIterator;
    }

    slotFilterTextChanged(filterEdit->text());
}

void DocTreeViewWidget::slotIndexNextMatch( )
{
    if (indexView->currentItem())
    {
        QListViewItem *below = indexView->currentItem()->itemBelow();
        if (below)
        {
            indexView->setCurrentItem(below);
            indexView->ensureItemVisible(below);
        }
    }
}

void DocTreeViewWidget::slotIndexPrevMatch( )
{
    if (indexView->currentItem())
    {
        QListViewItem *above = indexView->currentItem()->itemAbove();
        if (above)
        {
            indexView->setCurrentItem(above);
            indexView->ensureItemVisible(above);
        }
    }
}

void DocTreeViewWidget::slotIndexPgUp( )
{
}

void DocTreeViewWidget::slotIndexPgDown( )
{
/*    if (indexView->currentItem())
    {
        QListViewItem *below = indexView->currentItem()->itemBelow();
        while ( (below) && (below->isVisible()) )
        {
            below = below->itemBelow();
        }
        if (below)
        {
            indexView->setCurrentItem(below);
            indexView->ensureItemVisible(below);
        }
    }*/
}

void DocTreeViewWidget::slotIndexHome( )
{
/*    QListViewItem *item = indexView->firstChild();
    if (item)
    {
        indexView->setCurrentItem(item);
        indexView->ensureItemVisible(item);
    }*/
}

void DocTreeViewWidget::slotIndexEnd( )
{
/*    QListViewItem *item = indexView->lastItem();
    if (item)
    {
        indexView->setCurrentItem(item);
        indexView->ensureItemVisible(item);
    }*/
}

#include "doctreeviewwidget.moc"
