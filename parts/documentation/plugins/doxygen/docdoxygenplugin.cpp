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

#include "../../../../config.h"

static const KAboutData data("docdoxygenplugin", I18N_NOOP("Doxygen documentation plugin"), "1.0");
typedef KDevGenericFactory<DocDoxygenPlugin> DocDoxygenPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdocdoxygenplugin, DocDoxygenPluginFactory(&data) )

DocDoxygenPlugin::DocDoxygenPlugin(QObject* parent, const char* name, const QStringList)
    :DocumentationPlugin(DocDoxygenPluginFactory::instance()->config(), parent, name)
{
    setCapabilities(Index | FullTextSearch);
    autoSetup();
}

DocDoxygenPlugin::~DocDoxygenPlugin()
{
}

QPair<KFile::Mode, QString> DocDoxygenPlugin::catalogLocatorProps()
{
    return QPair<KFile::Mode, QString>(KFile::File, "index.html");
}

QString DocDoxygenPlugin::catalogTitle(const QString& url)
{
    QFileInfo fi(url);
    if (!fi.exists())
        return QString::null;
    
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
}

bool DocDoxygenPlugin::needRefreshIndex(DocumentationCatalogItem* item)
{
    QFileInfo fi(item->url().path());
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
    QString doxyDocDir(KDELIBS_DOXYDIR);
    doxyDocDir = URLUtil::envExpand(doxyDocDir);
    if (doxyDocDir.isEmpty())
    {
        QStringList apiDirs = DocDoxygenPluginFactory::instance()->dirs()->findDirs("html", "kdelibs-apidocs");
        if (apiDirs.count() > 0)
            doxyDocDir = apiDirs.first();
    }
    if (!doxyDocDir.isEmpty())
    {
        config->setGroup("Search Settings");
        config->writeEntry("The KDE API Reference (The KDE API Reference)", true);
        config->setGroup("Index Settings");
        config->writeEntry("The KDE API Reference (The KDE API Reference)", true);
        config->setGroup("Locations");
        config->writePathEntry("The KDE API Reference (The KDE API Reference)", doxyDocDir + QString("/index.html"));
    }
}

void DocDoxygenPlugin::createIndex(IndexBox* index, DocumentationCatalogItem* item)
{
    QFileInfo fi(item->url().path());
    if (!fi.exists())
        return;
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
            DocumentationItem *docItem = new DocumentationItem(DocumentationItem::Book, item, *it);
            docItem->setURL(KURL(d.absFilePath(*it) + "/html/index.html"));
            docItem->setExpandable(true);
            createBookTOC(docItem);
        }
    }
}

DocumentationCatalogItem *DocDoxygenPlugin::createCatalog(KListView *contents, const QString &title, const QString &url)
{
    DocumentationCatalogItem *item = new DocumentationCatalogItem(this, contents, title);
    item->setURL(url);
    return item;
}

void DocDoxygenPlugin::createBookTOC(DocumentationItem *item)
{
    QString tagName = item->url().upURL().directory(false) + item->text(0) + ".tag";
    //@todo list html files in the directory if tag was not found
    if (!QFile::exists(tagName))
        return;
    
    QFile f(tagName);
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

            if (QFile::exists(item->url().directory(false) + filename))
            {
                DocumentationItem *docItem = new DocumentationItem(DocumentationItem::Document,
                    item, classname);
                docItem->setURL(KURL(item->url().directory(false) + filename));
            }
        }
        childEl = childEl.nextSibling().toElement();
    }    
}

void DocDoxygenPlugin::createBookIndex(const QString &tagfile, IndexBox* index, DocumentationCatalogItem* item)
{
    QString tagName = tagfile;
    kdDebug() << tagfile << endl;
    //@todo list html files in the directory if tag was not found
    if (!QFile::exists(tagName))
        return;
    
    QFile f(tagName);
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
    createIndexFromTag(dom, index, item, docEl, KURL(tagfile).directory(false));
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
            indexItem->addURL(KURL(prefix + "html/" + filename));
            
            createIndexFromTag(dom, index, item, childEl, prefix + "html/" + filename);
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
            
            IndexItemProto *indexItem = new IndexItemProto(this, item, index, membername,
                i18n("%1::%2%3 Member Reference").arg(classname).arg(membername).arg(arglist));
            indexItem->addURL(KURL(prefix + "#" + anchor));
        }
        childEl = childEl.nextSibling().toElement();
    }
}

#include "docdoxygenplugin.moc"
