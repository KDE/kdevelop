/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "docdoxygenplugin.h"

#include <unistd.h>

#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdialog.h>
#include <qregexp.h>

#include <kurl.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <urlutil.h>
#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>

#include "../../../../config.h"

class DoxyDocumentationCatalogItem: public DocumentationCatalogItem
{
public:
    DoxyDocumentationCatalogItem(const QString &origUrl, DocumentationPlugin* plugin,
        KListView *parent, const QString &name)
        :DocumentationCatalogItem(plugin, parent, name), m_origUrl(origUrl)
    {
    }
    DoxyDocumentationCatalogItem(const QString &origUrl, DocumentationPlugin* plugin,
        DocumentationItem *parent, const QString &name)
        :DocumentationCatalogItem(plugin, parent, name), m_origUrl(origUrl)
    {
    }
    QString origUrl() const { return m_origUrl; }
    
private:
    QString m_origUrl;
};


static const KDevPluginInfo data("docdoxygenplugin");
typedef KDevGenericFactory<DocDoxygenPlugin> DocDoxygenPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdocdoxygenplugin, DocDoxygenPluginFactory(data) )

DocDoxygenPlugin::DocDoxygenPlugin(QObject* parent, const char* name, const QStringList)
    :DocumentationPlugin(DocDoxygenPluginFactory::instance()->config(), parent, name)
{
    setCapabilities(Index | FullTextSearch | ProjectDocumentation);
    autoSetup();
}

DocDoxygenPlugin::~DocDoxygenPlugin()
{
}

QPair<KFile::Mode, QString> DocDoxygenPlugin::catalogLocatorProps()
{
    return QPair<KFile::Mode, QString>(KFile::File, "index.html *.tag");
}

QString DocDoxygenPlugin::catalogTitle(const QString& url)
{
    QFileInfo fi(url);
    if (!fi.exists())
        return QString::null;
    
    if (fi.extension(false) == "html")
    {
        QFile f(url);
        if (!f.open(IO_ReadOnly))
            return QString::null;
        
        QTextStream ts(&f);
        QString contents = ts.read();
        QRegExp re(".*<title>(.*)</title>.*");
        re.setCaseSensitive(false);
        re.search(contents);
        return re.cap(1);   
    }
    else if (fi.extension(false) == "tag")
    {
        QFile *f = 0;
        QFile f1(fi.dirPath(true) + "/html/index.html");
        if (f1.open(IO_ReadOnly))
            f = &f1;
        QFile f2(fi.dirPath(true) + "/index.html");
        if (f2.open(IO_ReadOnly))
            f = &f2;
        if (f != 0)
        {
            QTextStream ts(f);
            QString contents = ts.read();
            QRegExp re(".*<title>(.*)</title>.*");
            re.setCaseSensitive(false);
            re.search(contents);
            return re.cap(1);   
        }
    }
    return QString::null;
}

QString DocDoxygenPlugin::pluginName() const
{
    return i18n("Doxygen Documentation Collection");
}

QStringList DocDoxygenPlugin::fullTextSearchLocations()
{
    QStringList locs;
        
    QMap<QString, QString> entryMap = config->entryMap("Locations");

    for (QMap<QString, QString>::const_iterator it = entryMap.begin();
        it != entryMap.end(); ++it)
    {
        config->setGroup("Search Settings");
        if (config->readBoolEntry(it.key(), false))
        {
            config->setGroup("Locations");
            QFileInfo fi(config->readPathEntry(it.key()));
            locs << fi.dirPath(true);
        }
    }
    
    return locs;
}

void DocDoxygenPlugin::setCatalogURL(DocumentationCatalogItem* item)
{
    if (item->url().url().endsWith("tag"))
    {
        QFileInfo fi(item->url().directory(false) + "html/index.html");
        if (fi.exists())
        {
            item->setURL(KURL::fromPathOrURL(fi.absFilePath()));
            return;
        }
        QFileInfo fi2(item->url().directory(false) + "index.html");
        if (fi2.exists())
        {
            item->setURL(KURL::fromPathOrURL(fi2.absFilePath()));
            return;
        }
        item->setURL(KURL());
    }
}

bool DocDoxygenPlugin::needRefreshIndex(DocumentationCatalogItem* item)
{
    DoxyDocumentationCatalogItem *doxyItem = dynamic_cast<DoxyDocumentationCatalogItem*>(item);
    if (!doxyItem)
        return false;
    
    QFileInfo fi(doxyItem->origUrl());
    config->setGroup("Index");
    if (fi.lastModified() > config->readDateTimeEntry(item->text(0), new QDateTime()))
    {
        kdDebug() << "need rebuild index for " << item->text(0) << endl;
        config->writeEntry(item->text(0), fi.lastModified());
        return true;
    }
    else
        return false;
}

void DocDoxygenPlugin::autoSetupPlugin()
{
    autoSetupDocs(KDELIBS_DOXYDIR, "en/kdelibs-apidocs", "The KDE API Reference (The KDE API Reference)");
    autoSetupDocs("", "en/kdevelop-apidocs", "The KDevelop Platform API Documentation (KDevelop)");
    
}

void DocDoxygenPlugin::autoSetupDocs(const QString &defaultDir, const QString &searchDir,
    const QString &name)
{
    QString doxyDocDir(defaultDir);
    doxyDocDir = URLUtil::envExpand(doxyDocDir);
    if (doxyDocDir.isEmpty())
    {
        QStringList apiDirs = DocDoxygenPluginFactory::instance()->dirs()->findDirs("html", searchDir);
        for (QStringList::const_iterator it = apiDirs.begin(); it != apiDirs.end(); ++it )
        {
            doxyDocDir = *it;
            QString indexFile = doxyDocDir + "index.html";
            if (QFile::exists(indexFile))
            {
                doxyDocDir = doxyDocDir + "/" + searchDir;
                break;
            }
            doxyDocDir = "";
        }
    }
    if (!doxyDocDir.isEmpty())
    {
        config->setGroup("Search Settings");
        config->writeEntry(name, true);
        config->setGroup("Index Settings");
        config->writeEntry(name, true);
        config->setGroup("Locations");
        config->writePathEntry(name, doxyDocDir + QString("/index.html"));
    }
}

void DocDoxygenPlugin::createIndex(IndexBox* index, DocumentationCatalogItem* item)
{
    QFileInfo fi(item->url().path());
    if (!fi.exists())
        return;

    DoxyDocumentationCatalogItem *doxyItem = dynamic_cast<DoxyDocumentationCatalogItem*>(item);
    if (!doxyItem)
        return;
    
    //doxygen documentation mode (if catalog points to a .tag)
    if (doxyItem->origUrl().endsWith("tag"))
    {
        QString htmlUrl;
        QFileInfo fi2(item->url().directory(false) + "index.html");
        if (fi2.exists())
            htmlUrl = fi2.dirPath(true) + "/";
        QFileInfo fi(item->url().directory(false) + "html/index.html");
        if (fi.exists())
            htmlUrl = fi.dirPath(true) + "/";
        
        createBookIndex(doxyItem->origUrl(), index, item, htmlUrl);
    }
    
    //KDE doxygen documentation mode (if catalog points to a index.html)
    QDir d(fi.dirPath(true));
    QStringList fileList = d.entryList("*", QDir::Dirs);
    
    QStringList::ConstIterator it;
    for (it = fileList.begin(); it != fileList.end(); ++it)
    {
        QString dirName = (*it);
        if (dirName == "." || dirName == ".." || dirName == "common")
            continue;
        if (QFile::exists(d.absFilePath(*it) + "/html/index.html"))
        {
            createBookIndex(d.absFilePath(*it) + "/" + *it + ".tag", index, item);
        }
    }
}

void DocDoxygenPlugin::createTOC(DocumentationCatalogItem* item)
{
    QFileInfo fi(item->url().path());
    if (!fi.exists())
        return;
    
    DoxyDocumentationCatalogItem *doxyItem = dynamic_cast<DoxyDocumentationCatalogItem*>(item);
    if (!doxyItem)
        return;
    
    //doxygen documentation mode (if catalog points to a .tag)
    if (doxyItem->origUrl().endsWith("tag"))
    {
        QString htmlUrl;
        QFileInfo fi2(item->url().directory(false) + "index.html");
        if (fi2.exists())
            htmlUrl = fi2.dirPath(true) + "/";
        QFileInfo fi(item->url().directory(false) + "html/index.html");
        if (fi.exists())
            htmlUrl = fi.dirPath(true) + "/";
        if (!htmlUrl.isEmpty())
            createBookTOC(item, doxyItem->origUrl(), htmlUrl);
    }
    
    //KDE doxygen documentation mode (if catalog points to a index.html)
    QDir d(fi.dirPath(true));
    QStringList fileList = d.entryList("*", QDir::Dirs, QDir::Name | QDir::Reversed);
    QStringList::ConstIterator it;
    for (it = fileList.begin(); it != fileList.end(); ++it)
    {
        QString dirName = (*it);
        if (dirName == "." || dirName == ".." || dirName == "common")
            continue;
        if (QFile::exists(d.absFilePath(*it) + "/html/index.html"))
        {
            DocumentationItem *docItem = new DocumentationItem(DocumentationItem::Book, item, *it);
            docItem->setURL(KURL(d.absFilePath(*it) + "/html/index.html"));
            docItem->setExpandable(true);
            createBookTOC(docItem);
        }
    }
}

DocumentationCatalogItem *DocDoxygenPlugin::createCatalog(KListView *contents, const QString &title, const QString &url)
{
    kdDebug() << "DocDoxygenPlugin::createCatalog: url=" << url << endl;
    DocumentationCatalogItem *item = new DoxyDocumentationCatalogItem(url, this, contents, title);
    item->setURL(url);
    return item;
}

void DocDoxygenPlugin::createBookTOC(DocumentationItem *item, const QString &tagUrl, const QString &baseHtmlUrl)
{
    QString tagName;
    if (tagUrl.isEmpty())
        tagName = item->url().upURL().directory(false) + item->text(0) + ".tag";
    else
        tagName = tagUrl;
    
    QString baseUrl;
    if (baseHtmlUrl.isEmpty())
        baseUrl = item->url().directory(false);
    else
        baseUrl = baseHtmlUrl;
            
    //@todo list html files in the directory if tag was not found
    if (!QFile::exists(tagName))
        return;

    QStringList tagFileList;
    if (tagName.endsWith(".tag"))
        tagFileList = tagFiles(QFileInfo(tagName).dirPath() + "/");
    else
        tagFileList += tagName;

    QStringList::ConstIterator end = tagFileList.constEnd();
    for (QStringList::ConstIterator it = tagFileList.constBegin(); it != end; ++it)
    {
        QFile f(*it);
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
    
        QDomElement childEl = docEl.lastChild().toElement();
        while (!childEl.isNull())
        {
            if (childEl.tagName() == "compound" && childEl.attribute("kind") == "class")
            {
                QString classname = childEl.namedItem("name").firstChild().toText().data();
                QString filename = childEl.namedItem("filename").firstChild().toText().data();
    
                if (QFile::exists(baseUrl + filename))
                {
                    DocumentationItem *docItem = new DocumentationItem(DocumentationItem::Document,
                        item, classname);
                    docItem->setURL(KURL(baseUrl + filename));
                }
            }
            childEl = childEl.previousSibling().toElement();
        }
    }
}

void DocDoxygenPlugin::createBookIndex(const QString &tagfile, IndexBox* index, DocumentationCatalogItem* item, const QString &baseHtmlUrl)
{
    QString tagName = tagfile;
    kdDebug() << tagfile << endl;
    if (!QFile::exists(tagName))
        return;
    QString prefix = baseHtmlUrl.isEmpty() ? KURL(tagfile).directory(false) + "html/" : baseHtmlUrl;
    
    QStringList tagFileList = tagFiles(QFileInfo(tagName).dirPath() + "/");

    QStringList::ConstIterator end = tagFileList.constEnd();
    for (QStringList::ConstIterator it = tagFileList.constBegin(); it != end; ++it)
    {
        QFile f(*it);
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
        createIndexFromTag(dom, index, item, docEl, prefix);
  }
}

void DocDoxygenPlugin::createIndexFromTag(QDomDocument &dom, IndexBox *index,
    DocumentationCatalogItem *item, QDomElement &parentEl, const QString &prefix)
{
    QDomElement docEl = parentEl;

    QDomElement childEl = docEl.firstChild().toElement();
    while (!childEl.isNull())
    {
        if (childEl.tagName() == "compound" && 
            ((childEl.attribute("kind") == "class")
            || (childEl.attribute("kind") == "struct")
            || (childEl.attribute("kind") == "namespace") ))
        {
            QString classname = childEl.namedItem("name").firstChild().toText().data();
            QString filename = childEl.namedItem("filename").firstChild().toText().data();

            IndexItemProto *indexItem = new IndexItemProto(this, item, index, classname, 
            i18n("%1 Class Reference").arg(classname));
            indexItem->addURL(KURL(prefix + filename));
            
            createIndexFromTag(dom, index, item, childEl, prefix + filename);
        }
        else if ((childEl.tagName() == "member") && 
            ((childEl.attribute("kind") == "function")
            || (childEl.attribute("kind") == "slot")
            || (childEl.attribute("kind") == "signal") ))
        {
            QString classname = parentEl.namedItem("name").firstChild().toText().data();
            QString membername = childEl.namedItem("name").firstChild().toText().data();
            QString anchor = childEl.namedItem("anchor").firstChild().toText().data();
            QString arglist = childEl.namedItem("arglist").firstChild().toText().data();
            
            if (classname != membername)
            {
                IndexItemProto *indexItem = new IndexItemProto(this, item, index, membername,i18n("%1::%2%3 Member Reference").arg(classname).arg(membername).arg(arglist));
                indexItem->addURL(KURL(prefix + "#" + anchor));
            }
        }
        childEl = childEl.nextSibling().toElement();
    }
}

ProjectDocumentationPlugin *DocDoxygenPlugin::projectDocumentationPlugin(ProjectDocType type)
{
    if (type == APIDocs)
        return new ProjectDocumentationPlugin(this, type);
    return DocumentationPlugin::projectDocumentationPlugin(type);
}

QStringList DocDoxygenPlugin::tagFiles(const QString& path, int level)
{
  QStringList r;
  QDir dir(path);
  if (level > 10) return r;
  if (!dir.isReadable()) return r;
  if (!dir.exists()) return r;

  QStringList  dirList;
  QStringList  fileList;
  QStringList::Iterator it;

  dir.setFilter ( QDir::Dirs);
  dirList = dir.entryList();

  dirList.remove(".");
  dirList.remove("..");

  dir.setFilter(QDir::Files | QDir::Hidden | QDir::System);
  fileList = dir.entryList();
  QStringList::Iterator end = dirList.end();
  for ( it = dirList.begin(); it != end; ++it )
  {
    QString name = *it;
    if (QFileInfo( dir, *it ).isSymLink())
      continue;
    r += tagFiles(path + name + "/", level + 1 );
  }

  QStringList::Iterator fend = fileList.end();
  for ( it = fileList.begin(); it != fend; ++it )
  {
    QString name = *it;
    QFileInfo fi( dir, *it );
    if (fi.isSymLink() || !fi.isFile())
      continue;

    if (QDir::match(QString("*.tag"), name))
      r += (path+name);
  }

  return r;
}

#include "docdoxygenplugin.moc"
