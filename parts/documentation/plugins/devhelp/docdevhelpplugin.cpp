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
#include "docdevhelpplugin.h"

#include <unistd.h>

#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdialog.h>

#include <kurl.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>

#include "../../../../config.h"

class DevHelpDocumentationCatalogItem: public DocumentationCatalogItem
{
public:
    DevHelpDocumentationCatalogItem(const QString &devHelpFile, DocumentationPlugin* plugin,
        KListView *parent, const QString &name)
        :DocumentationCatalogItem(plugin, parent, name), m_devHelpFile(devHelpFile)
    {
    }
    DevHelpDocumentationCatalogItem(const QString &devHelpFile, DocumentationPlugin* plugin,
        DocumentationItem *parent, const QString &name)
        :DocumentationCatalogItem(plugin, parent, name), m_devHelpFile(devHelpFile)
    {
    }
    QString devHelpFile() const { return m_devHelpFile; }
    
private:
    QString m_devHelpFile;
};


static const KDevPluginInfo data("docdevhelpplugin");
typedef KDevGenericFactory<DocDevHelpPlugin> DocDevHelpPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdocdevhelpplugin, DocDevHelpPluginFactory(data) )

DocDevHelpPlugin::DocDevHelpPlugin(QObject* parent, const char* name,
    const QStringList /*args*/)
    :DocumentationPlugin(DocDevHelpPluginFactory::instance()->config(), parent, name)
{
    setCapabilities(Index | FullTextSearch | ProjectDocumentation);
    autoSetup();
}

DocDevHelpPlugin::~DocDevHelpPlugin()
{
}

DocumentationCatalogItem* DocDevHelpPlugin::createCatalog(KListView* contents, const QString& title, const QString& url)
{
    return new DevHelpDocumentationCatalogItem(url, this, contents, title);
}

QPair<KFile::Mode, QString> DocDevHelpPlugin::catalogLocatorProps()
{
    return QPair<KFile::Mode, QString>(KFile::File, "*.devhelp");
}

QString DocDevHelpPlugin::catalogTitle(const QString& url)
{
    QFileInfo fi(url);
    if (!fi.exists())
        return QString::null;

    QFile f(url);
    if (!f.open(IO_ReadOnly))
        return QString::null;
    
    QDomDocument doc;
    if (!doc.setContent(&f))
        return QString::null;
    f.close();

    QDomElement docEl = doc.documentElement();

    return docEl.attribute("title", QString::null);
}

QString DocDevHelpPlugin::pluginName() const
{
    return i18n("Devhelp Documentation Collection");
}

QStringList DocDevHelpPlugin::fullTextSearchLocations()
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

bool DocDevHelpPlugin::needRefreshIndex(DocumentationCatalogItem* item)
{
    DevHelpDocumentationCatalogItem *dhItem = dynamic_cast<DevHelpDocumentationCatalogItem *>(item);
    if (!dhItem)
        return false;
    
    QFileInfo fi(dhItem->devHelpFile());
    config->setGroup("Index");
    if (fi.lastModified() > config->readDateTimeEntry(dhItem->text(0), new QDateTime()))
    {
        kdDebug() << "need rebuild index for " << dhItem->text(0) << endl;
        config->writeEntry(item->text(0), fi.lastModified());
        return true;
    }
    else
        return false;
}

void DocDevHelpPlugin::autoSetupPlugin()
{
    QValueStack<QString> scanStack;
    
    pushToScanStack(scanStack, getenv("DEVHELP_SEARCH_PATH"));
    pushToScanStack(scanStack, QString(getenv("HOME")) + "/.devhelp/books");
    
    QString dhexepath = DocDevHelpPluginFactory::instance()->dirs()->findExe("devhelp");
    if (!dhexepath.isEmpty())
    {
        QFileInfo fi(dhexepath);
        QString path = KURL(fi.dirPath(true)).upURL().path(1);
        pushToScanStack(scanStack, path + "share/devhelp/books");
        pushToScanStack(scanStack, path + "share/gtk-doc/html");
    }
    
    pushToScanStack(scanStack, "/usr/share/gtk-doc/html");
    pushToScanStack(scanStack, "/usr/share/devhelp/books/");
    pushToScanStack(scanStack, "/usr/local/share/devhelp/books");
    pushToScanStack(scanStack, "/usr/local/share/gtk-doc/html");
    pushToScanStack(scanStack, "/opt/gnome/share/devhelp/books");
    pushToScanStack(scanStack, "/opt/gnome/share/gtk-doc/html");
    pushToScanStack(scanStack, "/opt/gnome2/share/devhelp/books");
    pushToScanStack(scanStack, "/opt/gnome2/share/gtk-doc/html");

    //fill the list of scan dirs (with subdirectories)
    QStringList scanList;
    QDir dir;
    do 
    {
        dir.setPath(scanStack.pop());
        if (!dir.exists())
            continue;
        scanList << dir.path();

        const QFileInfoList *dirEntries = dir.entryInfoList();
        QPtrListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) 
        {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) 
            {
                scanStack.push(path);
            }
        }
    } while (!scanStack.isEmpty());

    for (QStringList::const_iterator it = scanList.begin(); it != scanList.end(); ++it)
    {
        scanDevHelpDir(*it);
    }
}

void DocDevHelpPlugin::scanDevHelpDir(const QString &path)
{
    QDir d(path);
    if (! d.exists())
        return;
    
    d.setFilter(QDir::Files);
    //scan for *.devhelp files
    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;
    while ( (fi = it.current()) != 0 ) 
    {
        if (fi->extension() == "devhelp")
        {
            config->setGroup("Locations");
            config->writePathEntry(catalogTitle(fi->absFilePath()), fi->absFilePath());
        }
        ++it;
    }
}

void DocDevHelpPlugin::pushToScanStack(QValueStack<QString> &stack, const QString &value)
{
    if ( (!value.isEmpty()) && (!stack.contains(value)) )
    {
        stack << value;
        kdDebug() << "Devhelp scan stack: +: " << value << endl;
    }
}

void DocDevHelpPlugin::createIndex(IndexBox* index, DocumentationCatalogItem* item)
{
    DevHelpDocumentationCatalogItem *dhItem = dynamic_cast<DevHelpDocumentationCatalogItem *>(item);
    if (!dhItem)
        return;
    
    QFileInfo fi(dhItem->devHelpFile());

    QFile f(dhItem->devHelpFile());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << dhItem->devHelpFile() << endl;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f))
    {
        kdDebug() << "Not a valid devhelp file: " << dhItem->devHelpFile() << endl;
        return;
    }
    f.close();
    
    QString baseUrl = KURL(dhItem->devHelpFile()).directory();
    
    QDomElement docEl = doc.documentElement();
    QDomElement chaptersEl = docEl.namedItem("functions").toElement();
    QDomElement childEl = chaptersEl.firstChild().toElement();
    while (!childEl.isNull())
    {
        if (childEl.tagName() == "function")
        {
            QString name = childEl.attribute("name");
            QString url = childEl.attribute("link");
            
            IndexItemProto *ii = new IndexItemProto(this, item, index, name, item->text(0));
            ii->addURL(KURL(baseUrl+"/"+url));
        }
        childEl = childEl.nextSibling().toElement();
    }
}

void DocDevHelpPlugin::createTOC(DocumentationCatalogItem* item)
{
    DevHelpDocumentationCatalogItem *dhItem = dynamic_cast<DevHelpDocumentationCatalogItem *>(item);
    if (!dhItem)
        return;
    
    QFileInfo fi(dhItem->devHelpFile());

    QFile f(dhItem->devHelpFile());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << dhItem->devHelpFile() << endl;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f))
    {
        kdDebug() << "Not a valid devhelp file: " << dhItem->devHelpFile() << endl;
        return;
    }
    f.close();
    
    QDomElement docEl = doc.documentElement();
    QDomElement chaptersEl = docEl.namedItem("chapters").toElement();

    QDomElement childEl = chaptersEl.lastChild().toElement();
    QString baseUrl = KURL(dhItem->devHelpFile()).directory();
    addTocSect(dhItem, childEl, baseUrl, true);
}

void DocDevHelpPlugin::addTocSect(DocumentationItem *parent, QDomElement childEl, 
    QString baseUrl, bool book)
{
    while (!childEl.isNull())
    {
        if ( (childEl.tagName() == "sub") || (childEl.tagName() == "chapter"))
        {
            QString name = childEl.attribute("name");
            QString url = childEl.attribute("link");
            
            if (name.isEmpty() && url.contains("ix"))
                name = "Index";
            
            DocumentationItem *item = new DocumentationItem(
                book ? DocumentationItem::Book : DocumentationItem::Document, parent, name);
            item->setURL(KURL(baseUrl+"/"+url));
            
            QDomElement grandchildEl = childEl.lastChild().toElement();
            addTocSect(item, grandchildEl, baseUrl);
        }
        childEl = childEl.previousSibling().toElement();
    }
}

void DocDevHelpPlugin::setCatalogURL(DocumentationCatalogItem* item)
{
    DevHelpDocumentationCatalogItem *dhItem = dynamic_cast<DevHelpDocumentationCatalogItem *>(item);
    if (!dhItem)
        return;
    
    QFileInfo fi(dhItem->devHelpFile());

    QFile f(dhItem->devHelpFile());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << dhItem->devHelpFile() << endl;
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f))
    {
        kdDebug(9002) << "Not a valid Devhelp file: " << dhItem->devHelpFile() << endl;
        return;
    }
    f.close();

    QDomElement docEl = doc.documentElement();
    QDomElement titleEl = docEl.namedItem("book").toElement();

    if (item->url().isEmpty())
    {
        KURL url(fi.dirPath(true) + "/" + docEl.attribute("link", QString::null));
        item->setURL(url);
    }
}

ProjectDocumentationPlugin *DocDevHelpPlugin::projectDocumentationPlugin(ProjectDocType type)
{
    if (type == APIDocs)
        return new ProjectDocumentationPlugin(this, type);
    return DocumentationPlugin::projectDocumentationPlugin(type);
}

#include "docdevhelpplugin.moc"
