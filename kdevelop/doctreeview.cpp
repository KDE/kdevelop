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


#include "doctreeview.h"

#include "cdoctreepropdlg.h"
#include "cproject.h"

//#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstddirs.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kfilterdev.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qlist.h>
#include <qlineedit.h>
#include <qregexp.h>
#include <qdom.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// There are still problems with BEN's workaround in certain circumstances
// so for the time being I've used my solution. jbb 07-02-2000
//#define BEN_QLISTVIEW_BYPASS	//define this symbol for using my solution to the docview bug
                                //undefine it to use jbb's

/**
 * A list view item that is decorated with a doc icon.
 * This typically represents a section in a manual.
 */
class ListViewDocItem : public KDevListViewItem
{
public:
    ListViewDocItem( KDevListViewItem *parent,
                     const QString &text, const QString &filename );

    virtual void setOpen(bool o);
};


ListViewDocItem::ListViewDocItem(KDevListViewItem *parent,
                                 const QString &text, const QString &filename)
    : KDevListViewItem(parent, text, filename)
{
    setOpen(false);
//    setPixmap(0, BarIcon("mini-doc"));
}

void ListViewDocItem::setOpen(bool o)
{
    setPixmap(0, o? SmallIcon("document") : SmallIcon("document2"));
    KDevListViewItem::setOpen(o);
}

//
//
///**
// * A list view item that is decorated with a book icon.
// * This typically represents one manual. When the user "opens"
// * the book, the according icon is changed.
// */
//class ListViewBookItem : public KDevListViewItem
//{
//public:
//    ListViewBookItem( KDevListViewItem *parent,
//                      const QString &text, const char *filename );
//    virtual void setOpen(bool o);
//};


ListViewBookItem::ListViewBookItem(KDevListViewItem *parent,
                                   const QString &text, const char *filename)
    : KDevListViewItem(parent, text, filename)
{
    setOpen(false);
}


void ListViewBookItem::setOpen(bool o)
{
    setPixmap(0, o? SmallIcon("contents") : SmallIcon("contents2"));
    KDevListViewItem::setOpen(o);
}


/**
 * A list view item that is decorated with a folder icon.
 * This typically represents one major part in the help tree.
 * Its contents can be refresh()d. The default implementation
 * simply deletes all children.
 */
class ListViewFolderItem : public KDevListViewItem
{
public:
    ListViewFolderItem( KDevListView *parent, const QString &text );
    virtual void setOpen(bool o);
    virtual void refresh();
};


ListViewFolderItem::ListViewFolderItem(KDevListView *parent, const QString &text)
    : KDevListViewItem(parent, text, "")
{
    setOpen(false);
}


void ListViewFolderItem::setOpen(bool o)
{
    setPixmap(0, o? SmallIcon("folder_open") : SmallIcon("folder"));
    KDevListViewItem::setOpen(o);
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
// moved class declaration to doctreeview.h because I need the static members in CKDevelop -Ralf Nolden

QString DocTreeKDevelopBook::locatehtml(const QString& filename)
{
  QString path=locate("html", KGlobal::locale()->language() +"/kdevelop/"+filename);
  if (!path.isEmpty())
    return path;

  path=locate("html", "default/kdevelop/"+filename);
  if (!path.isEmpty())
    return path;

  path=locate("html", "en/kdevelop/"+filename);
  if (!path.isEmpty())
    return path;

  // The file isn't where we wanted it. We now look for an index file
  // based on the relative path of the original file. This should contain
  // and error message to put up.
  if (filename.contains("/index.html"))
    return QString::null;

  QFileInfo fileInfo(filename);
  QString errInfo = fileInfo.dirPath() + "/index.html";

  path=locate("html", KGlobal::locale()->language() +"/kdevelop/"+errInfo);
  if (!path.isEmpty())
    return path;

  path=locate("html", "default/kdevelop/"+errInfo);
  if (!path.isEmpty())
    return path;

  path=locate("html", "en/kdevelop/"+errInfo);
  return path;
}


void DocTreeKDevelopBook::readSgmlIndex(FILE *f)
{
  char buf[512];
  ListViewDocItem* chapt=0;
  while (fgets(buf, sizeof buf, f))
  {
    // HTML files produced by sgml2html have toc's like the following:
    // <H2><A NAME="toc1">1.</A> <A HREF="index-1.html">Introduction</A></H2>
    QString s = QString::fromLocal8Bit(buf);
    if (s.left(4) == "<H2>")
    {
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
      chapt=new ListViewDocItem(this, title, path);
    }
    else if (s.left(4)=="<LI>")
    {
      int pos1 = s.find("A HREF=\"");
      if (pos1 == -1)
          continue;
      int pos2 = s.find("\">", pos1+8);
      if (pos2 == -1)
          continue;
      int pos3 = s.find(' ', pos2+1);             
      if(pos3 == -1)
          continue;
      int pos4 = s.find('<', pos2+1);             
      if(pos4 == -1)
          continue;
      
      QString filename = s.mid(pos1+8, pos2-(pos1+8)); 
      QString sectname = s.mid(pos3+1, pos4-(pos3+1));
      QFileInfo fi(ident());
      QString path = fi.dirPath() + "/" + filename;
      if(chapt)
        new ListViewDocItem(chapt, sectname,path);
    }
  }
}

QString DocTreeKDevelopBook::readIndexTitle(const QString &book)
{
  FILE *f;
  QString title="";
  if ( (f = fopen(QFile::encodeName(book).data(), "r")) != 0)
  {
    char buf[512];
    while (fgets(buf, sizeof buf, f))
    {
      // search for the TITLE start and end tag, store title between the two positions minus the tag length
      QString s = QString::fromLocal8Bit(buf);
      int pos1 = s.find("<TITLE>");
      if (pos1 == -1)
          continue;
      int pos2 = s.find("</TITLE>", pos1+7);
      if (pos2 == -1)
          continue;
      title = s.mid(pos1+7, pos2-(pos1+7));
    }    
    fclose(f);
  }
  return title;
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

  QString manual=DocTreeKDevelopBook::readIndexTitle(DocTreeKDevelopBook::locatehtml("index.html"));
  QString programming=DocTreeKDevelopBook::readIndexTitle(DocTreeKDevelopBook::locatehtml("programming/index.html"));    
  QString tutorial=DocTreeKDevelopBook::readIndexTitle(DocTreeKDevelopBook::locatehtml("tutorial/index.html"));    
  QString kdelibref=DocTreeKDevelopBook::readIndexTitle(DocTreeKDevelopBook::locatehtml("kde_libref/index.html"));    

  (void) new DocTreeKDevelopBook(this, i18n("Welcome to KDevelop!"), "about/intro.html");
  (void) new DocTreeKDevelopBook(this, manual,                  "index.html",             true);
  (void) new DocTreeKDevelopBook(this, programming,             "programming/index.html", true);
  (void) new DocTreeKDevelopBook(this, tutorial,                "tutorial/index.html",    true);
  (void) new DocTreeKDevelopBook(this, kdelibref,               "kde_libref/index.html",  true);
  KConfig* config=KGlobal::config();
  config->setGroup("Doc_Location");
  QString designer = config->readEntry("doc_qt", QT_DOCDIR)+"designer/book1.html";
  if(QFileInfo(designer).exists())
  (void) new ListViewBookItem(this, i18n("Qt Designer Manual"), designer);
  (void) new DocTreeKDevelopBook(this, i18n("C/C++ Reference"), "reference/C/cref.html");

    //horrible hack to counter the QListView bug DO NOT CHANGE without thinking about it
    //and looking closely at the implementation of QListView, expacially how are the pointers
    //in QListView::d->drawables are managed!!!   Benoit Cerrina <benoit.cerrina@writeme.com>
//    listView()->setOpen(this, !isOpen());
//    listView()->setOpen(this, !isOpen());
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

    #if KDE_QTVER >= 3
    DocTreeKDELibsBook( KDevListViewItem *parent, const QString &text,
                        const QString &libname, const QString &);
    #else
    DocTreeKDELibsBook( KDevListViewItem *parent, const QString &text,
                        const QString &libname);
    #endif
    
    void relocatehtml();
    virtual void setOpen(bool o);
private:

    #if KDE_QTVER >= 3
    QString tagFile;
    int readDoxygenTag(const QDomDocument &d);
    #else
    int readKdoc2Index(FILE *f);
    QString idx_filename;
    #endif

    static QString locatehtml(const QString& libname);
    QString name;
};

#if KDE_QTVER >= 3
DocTreeKDELibsBook::DocTreeKDELibsBook( KDevListViewItem *parent, const QString & text,
                                        const QString &libname, const QString & tag = 0 )
    : ListViewBookItem(parent, text, locatehtml(libname)), name(libname)
{
     tagFile = tag;
     setExpandable( QFile::exists(tagFile) );
}
#else
DocTreeKDELibsBook::DocTreeKDELibsBook( KDevListViewItem *parent, const QString & text,
                                        const QString &libname )
    : ListViewBookItem(parent, text, locatehtml(libname)), name(libname)
{
    KConfig *config = KGlobal::config();
    config->setGroup("Doc_Location");

    QString doc_dir, idx_path;
    doc_dir = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    idx_path= doc_dir + "/kdoc-reference";
    QDir dir;
    if(!dir.exists(idx_path))
      idx_path= QDir::homeDirPath ()+ "/.kdoc";

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

}
#endif

void DocTreeKDELibsBook::relocatehtml()
{
  setIdent(locatehtml(name));
}

#if KDE_QTVER >= 3
QString DocTreeKDELibsBook::locatehtml(const QString& libname)
{
    KConfig *config = KGlobal::config();
    config->setGroup("Doc_Location");
    QString kde_path = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    QString qt_path = config->readEntry("doc_qt", QT_DOCDIR);

    if (!libname)
    {
        if (qt_path.right(1) != "/")
          qt_path= qt_path+"/";
        return qt_path + "index.html";
    }

    return kde_path + libname + "/html/index.html";
}
#else
QString DocTreeKDELibsBook::locatehtml(const QString& libname)
{
    KConfig *config = KGlobal::config();
    config->setGroup("Doc_Location");
    QString kde_path = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    QString qt_path = config->readEntry("doc_qt", QT_DOCDIR);

    if (!libname)
    {
        if (qt_path.right(1) != "/")
          qt_path= qt_path+"/";
        return qt_path + "index.html";
    }
    QString indexFile;
    indexFile =  kde_path + "/kdoc-reference/" + libname +".kdoc";
    if(!(QFile::exists(indexFile) || QFile::exists(indexFile+".gz"))){
      indexFile =  QDir::homeDirPath ()+ "/.kdoc/" + libname +".kdoc";
    }
    if(!(QFile::exists(indexFile) || QFile::exists(indexFile+".gz")))
    {
    // return the standard way to get to the index file if the kdoc file doesn´t exist    
        if (kde_path.right(1) != "/")
          kde_path= kde_path+"/";
        return kde_path + libname + "/index.html";
    }
    FILE *f;
    if ( (f = fopen(indexFile, "r")) != 0)
    {
      char buf[512];
//      int count=0;
      QString baseurl;
      while (fgets(buf, sizeof buf, f))
      {
        QString s = QString::fromLocal8Bit(buf);
        if (s.left(11) == "<BASE URL=\"")
        {
            int pos2 = s.find("\">", 11);
            if (pos2 != -1)
                baseurl = s.mid(11, pos2-11);
        }
      }
      fclose(f);
      return baseurl+"/index.html";
    }
    else if ( (f = popen(QString("gzip -c -d ")
                         + KShellProcess::quote(indexFile+".gz") + " 2>/dev/null", "r")) != 0)
    {
      char buf[512];
//      int count=0;
      QString baseurl;
      while (fgets(buf, sizeof buf, f))
      {
        QString s = QString::fromLocal8Bit(buf);
        if (s.left(11) == "<BASE URL=\"")
        {
            int pos2 = s.find("\">", 11);
            if (pos2 != -1)
                baseurl = s.mid(11, pos2-11);
        }
      }
      fclose(f);
      return baseurl+"/index.html";
    }
  return "";  // only to kill warnings - IMHO there should be only 1 return at the end - W. Tasin
}
#endif

#if KDE_QTVER >= 3
int DocTreeKDELibsBook::readDoxygenTag(const QDomDocument &d)
{
    int count=0;
    QString classname;
    ListViewDocItem* class_doc=0;

    KConfig *config = KGlobal::config();
    config->setGroup("Doc_Location");
    QString baseurl;
    if (!name.isEmpty()){
        baseurl = config->readEntry("doc_kde", KDELIBS_DOCDIR);
        baseurl += name + "/html/";
    }
    else
        baseurl = config->readEntry("doc_qt", QT_DOCDIR);
    
    QDomNodeList classes = d.elementsByTagName("compound");
    for (int i = 0; i < classes.count(); ++i)
    {
        QDomNode item = classes.item(i);
        QString filename = item.namedItem("filename").toElement().text();
        classname = item.namedItem("name").toElement().text();
        class_doc= new ListViewDocItem(this, classname, baseurl + filename);
        count++;
        
        QDomNodeList members = item.childNodes();
        //kdDebug() << "members " << members.count() << endl;
        for (int m = 0; m < members.count(); ++m)
        {
            QDomNode member = members.item(m);
            //kdDebug() << "member " << member.nodeName() << endl;
            if (member.nodeName() == "member")
            {
                QString membername = member.namedItem("name").toElement().text();
                QString anchor = member.namedItem("anchor").toElement().text();
                // QString arglist = member.namedItem("arglist").toElement().text();  //doesn't look too good, commented out
                if(class_doc && !classname.isEmpty())
                  new ListViewDocItem(class_doc, membername /* + arglist */, baseurl + filename + "#" + anchor); // here the arglist, too
             }
         }
    }
    sortChildItems(0, true);
    
	return count;
}
#else
int DocTreeKDELibsBook::readKdoc2Index(FILE *f)
{
    char buf[512];
    int count=0;
    QString baseurl;
    QString classname;
    ListViewDocItem* class_doc=0L;
    while (fgets(buf, sizeof buf, f))
    {
      QString s = QString::fromLocal8Bit(buf);
      if (s.left(11) == "<BASE URL=\"")
      {
          int pos2 = s.find("\">", 11);
          if (pos2 != -1)
              baseurl = s.mid(11, pos2-11);
          {
          QString other=baseurl+"/hier.html";
          if(QFileInfo(other).exists())
           new ListViewDocItem(this,i18n("Hierarchy"),
                              other);
          other=baseurl+"/header-list.html";
          if(QFileInfo(other).exists())
           new ListViewDocItem(this,i18n("Header Files"),
                              other);
          other=baseurl+"/all-globals.html";
          if(QFileInfo(other).exists())
           new ListViewDocItem(this,i18n("Globals"),
                              other);
          other=baseurl+"/index-long.html";
          if(QFileInfo(other).exists())
           new ListViewDocItem(this,i18n("Annotated List"),
                              other);                    
          }
      }
      else if (s.left(9) == "<C NAME=\"")
      {
        int pos1 = s.find("\" REF=\"", 9);
        if (pos1 == -1)
            continue;
        int pos2 = s.find("\">", pos1+7);
        if (pos2 == -1)
            continue;
        classname = s.mid(9, pos1-9);
        QString filename = s.mid(pos1+7, pos2-(pos1+7));
        if(filename.contains("::"))
          filename.replace( QRegExp("::"), "__" );           // becomes "b"

        class_doc= new ListViewDocItem(this, classname,
                            baseurl + "/" + filename);
        count++;
      }  
      //////////////////////////////////////////
      else if (s.left(10) == "<ME NAME=\"")
      {
        int pos1 = s.find("\" REF=\"", 10);
        if (pos1 == -1)
            continue;
        int pos2 = s.find("\">", pos1+7);
        if (pos2 == -1)
            continue;
        QString membername = s.mid(10, pos1-10);
        membername=classname+"::"+membername;
        QString memberfilename = s.mid(pos1+7, pos2-(pos1+7));
        if(memberfilename.contains("::"))
          memberfilename.replace( QRegExp("::"), "__" );           // becomes "b"
        if(class_doc && classname)
          new ListViewDocItem(class_doc, membername,
                            baseurl + "/" + memberfilename);
      }
      else if (s.left(9) == "<M NAME=\"")
      {
        int pos1 = s.find("\" REF=\"", 9);
        if (pos1 == -1)
            continue;
        int pos2 = s.find("\">", pos1+7);
        if (pos2 == -1)
            continue;
        QString membername = s.mid(9, pos1-9);
        QString memberfilename = s.mid(pos1+7, pos2-(pos1+7));
        if(memberfilename.contains("::"))
          memberfilename.replace( QRegExp("::"), "__" );           // becomes "b"
        if(class_doc && classname)
          new ListViewDocItem(class_doc, membername,
                            baseurl + "/" + memberfilename);
      }
    }
    sortChildItems(0, true);

    return count;
}
#endif

#if KDE_QTVER >= 3
void DocTreeKDELibsBook::setOpen(bool o)
{
  int count=0;
  if (o && childCount() == 0)
  {
    QIODevice *dev = KFilterDev::deviceForFile( tagFile );
    QDomDocument d;
    d.setContent( dev );
    count=readDoxygenTag(d);
    delete dev;

    if (count==0)
    {
      setExpandable(false);
      o=false;
    }

  }
  ListViewBookItem::setOpen(o);
}
#else
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
    else  if ( (f = popen(QString("gzip -c -d ")
                                 + KShellProcess::quote(idx_filename + ".gz") + " 2>/dev/null", "r")) != 0)
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
#endif


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
#if KDE_QTVER >= 3
void DocTreeKDELibsFolder::refresh()
{
    ListViewFolderItem::refresh();
    list.clear();
    //list.append(new DocTreeKDELibsBook(this, i18n("Qt Library"), 0));

    // if we have kdoc2 index files, get the reference directory
    QString docu_dir, libname, msg, tagF;
    KConfig* config=KGlobal::config();
    config->setGroup("Doc_Location");
    docu_dir = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    if (!docu_dir.isEmpty())
    {
      QDir d;
      d.setPath(docu_dir);

      if(!d.exists())
        return;

      const QFileInfoList *subDirList = d.entryInfoList(QDir::Dirs); // get the subdirs
      QFileInfoListIterator it( *subDirList );
      QFileInfo *subDir;
      while ( (subDir=it.current()) ) {
        
        if( (subDir->fileName()!=".") && (subDir->fileName()!="..") )
        {
            tagF = docu_dir + subDir->fileName() + "/" + subDir->fileName() + ".tag" ;
            if ( QFile::exists(tagF) )  ;
            else if( QFile::exists(tagF + ".gz")) tagF += ".gz";

            if( QFile::exists(tagF) ){
                libname = subDir->fileName();
                if ( libname!=QString("qt") ){
                    msg.sprintf( i18n("%s-Library"),libname.ascii() );
                    list.append(new DocTreeKDELibsBook(this, msg, libname, tagF));
                }
                else
                    list.append(new DocTreeKDELibsBook(this, i18n("Qt Library"), 0, tagF));
            }
        }
        ++it;
      }
    }
}
#else
void DocTreeKDELibsFolder::refresh()
{
    ListViewFolderItem::refresh();
    list.clear();
    list.append(new DocTreeKDELibsBook(this, i18n("Qt Library"), 0));

    // if we have kdoc2 index files, get the reference directory
    QString docu_dir, index_path, libname, msg;
    KConfig* config=KGlobal::config();
    config->setGroup("Doc_Location");
    docu_dir = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    if (!docu_dir.isEmpty())
    {
      index_path= docu_dir + "/kdoc-reference";
      QDir d;
      d.setPath(index_path);
      if(!d.exists()){
        index_path=QDir::homeDirPath ()+ "/.kdoc";
        d.setPath(index_path);
      }
      if(!d.exists())
        return;

      const QFileInfoList *fileList = d.entryInfoList(); // get the file info list
      QFileInfoListIterator it( *fileList ); // iterator
      QFileInfo *fi; // the current file info
      while ( (fi=it.current()) ) {  // traverse all kdoc reference files
        libname=fi->fileName();  // get the filename
        if(!libname.contains("qt.kdoc") && fi->isFile()) // exclude qt.kdoc || qt.kdoc.gz and everything except files
        {
          libname=fi->baseName();  // get only the base of the filename as library name
          msg.sprintf(i18n("%s-Library"),libname.data());
          list.append(new DocTreeKDELibsBook(this, msg, libname)); // append to the doctree
        }
        ++it; // increase the iterator
      }
    }
}

#endif

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
            QStringList fileList = d.entryList("*", QDir::Files);
            QStringList::Iterator it;
            for (it = fileList.begin(); it != fileList.end(); ++it)
                {
                    FILE *f;
                    if ( (f = fopen(d.filePath(*it), "r")) != 0)
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
    
    // remove the child-items because the items are regenareted below,
    // otherwise they would appear twice
    QListViewItem* child1 = firstChild();
    QListViewItem* child2 = 0;

    while( child1 ) {
	child2 = child1->nextSibling();
	delete child1;
	child1 = child2;
    }

    ListViewFolderItem::refresh();

    KConfig *config = KGlobal::config();
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
    void handleRightButtonPressed(QListViewItem *item,
                                  const QPoint &p);
    virtual void refresh();
    void setProject(CProject *prj)
        {
        	project = prj;
        }
    
private:
    CProject *project;
    ListViewBookItem *api_item;
};


void DocTreeProjectFolder::refresh()
{
    ListViewFolderItem::refresh();

    setExpandable(false);
    if (project && project->isValid())
        {
            setExpandable(true);
            api_item = new ListViewBookItem(this, i18n("API documentation"),
                                        /* strange!? */ "API-Documentation");
            (void) new ListViewBookItem(this, i18n("User manual"),
                                        /* strange!? */ "User-Manual");
//                setOpen(false);
        }
}
void DocTreeProjectFolder::handleRightButtonPressed(QListViewItem *item,
                                                   const QPoint &p)
{
    if (project && project->isValid()){
      if ( item == this)
      {
          KPopupMenu pop(i18n("Project"));
          pop.insertItem(i18n("Update API"), listView(), SLOT(slotUpdateAPI()));
          pop.insertItem(i18n("Generate User Manual"), listView(), SLOT(slotUpdateUserManual()));
          pop.exec(p);
      }
      else if (item->parent() == this)
      {
          KPopupMenu pop(i18n("Project"));
          if(item==api_item)
            pop.insertItem(i18n("Update API"), listView(), SLOT(slotUpdateAPI()));
          else
            pop.insertItem(i18n("Generate User Manual"), listView(), SLOT(slotUpdateUserManual()));
          
          pop.exec(p);
      }
    }
}


/**************************************/
/* The DocTreeView itself              */
/**************************************/

DocTreeView::DocToolTip::DocToolTip( QWidget *parent )
  : QToolTip( parent )
{
}

void DocTreeView::DocToolTip::maybeTip( const QPoint &p )
{
  DocTreeView *dt;
  QString str;
  QRect r;
  
  dt = (DocTreeView *)parentWidget();
  
  dt->tip( p, r, str );

  if( !str.isEmpty() && r.isValid() )
    tip( r, str );
}

void DocTreeView::tip( const QPoint &p, QRect &r, QString &str )
{
  QListViewItem *i;

  i = itemAt( p );
  r = itemRect( i );
  
  if( i != NULL && r.isValid() )
    str = i->text( 0 );
  else
    str = "";
}

DocTreeView::DocTreeView(QWidget *parent, const char *name)
    : KDevListView(parent, name)
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

    folder_kdevelop->refresh();
    folder_kdelibs->refresh();
    folder_others->refresh();
    folder_project->refresh();

    toolTip = new DocToolTip( this );
    
    connect( this,
             SIGNAL(rightButtonPressed(QListViewItem*,const QPoint&,int)),
             SLOT(slotRightButtonPressed(QListViewItem*,const QPoint&,int)) );
    connect( this,
             SIGNAL(executed(QListViewItem *)),
             SLOT(slotSelectionChanged(QListViewItem *)) );
    connect( this,
             SIGNAL(returnPressed(QListViewItem *)),
             SLOT(slotSelectionChanged(QListViewItem *)) );
}


DocTreeView::~DocTreeView()
{
  delete toolTip; toolTip = NULL;
}


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
#ifdef WITH_DOCBASE
    folder_docbase  = new DocTreeDocbaseFolder(this);
#endif
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
    folder_project->handleRightButtonPressed(item, p);

}


void DocTreeView::slotAddDocumentation()
{
    CDocTreePropDlg dlg;
    dlg.setCaption(i18n("Add Entry..."));
    if (!dlg.exec())
        return;
    
    KConfig *config = KGlobal::config();
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
    KConfig *config = KGlobal::config();
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
    KConfig *config = KGlobal::config();
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
    // are KDevListViewItem's
    KDevListViewItem *kitem = static_cast<KDevListViewItem*>(item);
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

#include "doctreeview.moc"
