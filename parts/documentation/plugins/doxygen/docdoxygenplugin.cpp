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

#include <kurl.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <khtml_part.h>
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
        kdDebug() << "1-----------" << endl;
    QFileInfo fi(url);
    if (!fi.exists())
    {
        kdDebug() << "2-----------" << url << endl;
        return QString::null;
    }
   
    KHTMLPart part;
    if (!part.openURL(KURL(url)))
    {
        kdDebug() << "-----------" << endl;
        return QString::null;
    }
    
    return part.htmlDocument().title().string();
}

QString DocDoxygenPlugin::pluginName() const
{
    return i18n("Doxygen Documentation Collection");
}

QStringList DocDoxygenPlugin::fullTextSearchLocations()
{
    return QStringList();
}

void DocDoxygenPlugin::setCatalogURL(DocumentationCatalogItem* item)
{
}

bool DocDoxygenPlugin::needRefreshIndex(DocumentationCatalogItem* item)
{
    QFileInfo fi(item->url().url());
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
        config->setGroup("Locations");
        config->writePathEntry("The KDE API Reference (The KDE API Reference)", doxyDocDir + QString("/index.html"));
    }
}

void DocDoxygenPlugin::createIndex(KListBox* index, DocumentationCatalogItem* item)
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

void DocDoxygenPlugin::createBookIndex(const QString &tagfile, KListBox* index, DocumentationCatalogItem* item)
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

    QDomElement childEl = docEl.firstChild().toElement();
    while (!childEl.isNull())
    {
        if (childEl.tagName() == "compound" && childEl.attribute("kind") == "class")
        {
            QString classname = childEl.namedItem("name").firstChild().toText().data();
            QString filename = childEl.namedItem("filename").firstChild().toText().data();

            kdDebug() << "    ch: " << KURL(tagfile).directory(false) + "html/" + filename << endl;
            if (QFile::exists(KURL(tagfile).directory(false) + "html/" + filename))
            {
                IndexItem *indexItem = new IndexItem(this, item, index, classname);
                indexItem->addURL(KURL(KURL(tagfile).directory(false) + "html/" + filename));
            }
        }
        childEl = childEl.nextSibling().toElement();
    }    
}

#include "docdoxygenplugin.moc"
