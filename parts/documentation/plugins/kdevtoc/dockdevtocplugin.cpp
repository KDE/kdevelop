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
#include "dockdevtocplugin.h"

#include <unistd.h>

#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdialog.h>

#include <kurl.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <klistview.h>
#include <kstandarddirs.h>

#include <urlutil.h>
#include <kdevgenericfactory.h>

#include "../../../../config.h"

class TOCDocumentationCatalogItem: public DocumentationCatalogItem
{
public:
    TOCDocumentationCatalogItem(const QString &tocFile, DocumentationPlugin* plugin,
        KListView *parent, const QString &name)
        :DocumentationCatalogItem(plugin, parent, name), m_tocFile(tocFile)
    {
    }
    TOCDocumentationCatalogItem(const QString &tocFile, DocumentationPlugin* plugin,
        DocumentationItem *parent, const QString &name)
        :DocumentationCatalogItem(plugin, parent, name), m_tocFile(tocFile)
    {
    }
    QString tocFile() const { return m_tocFile; }
    
private:
    QString m_tocFile;
};


static const KAboutData data("dockdevtocplugin", I18N_NOOP("KDevelopTOC documentation plugin"), "1.0");
typedef KDevGenericFactory<DocKDevTOCPlugin> DocKDevTOCPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdockdevtocplugin, DocKDevTOCPluginFactory(&data) )

DocKDevTOCPlugin::DocKDevTOCPlugin(QObject* parent, const char* name,
    const QStringList /*args*/)
    :DocumentationPlugin(DocKDevTOCPluginFactory::instance()->config(), parent, name)
{
    setCapabilities(Index);
    autoSetup();
}

DocKDevTOCPlugin::~DocKDevTOCPlugin()
{
}

QString DocKDevTOCPlugin::pluginName() const
{
    return i18n("KDevelopTOC Documentation Collection");
}

DocumentationCatalogItem* DocKDevTOCPlugin::createCatalog(KListView* contents, const QString& title, const QString& url)
{
    return new TOCDocumentationCatalogItem(url, this, contents, title);
}

QPair<KFile::Mode, QString> DocKDevTOCPlugin::catalogLocatorProps()
{
    return QPair<KFile::Mode, QString>(KFile::File, "*.toc");
}

QString DocKDevTOCPlugin::catalogTitle(const QString& url)
{
    QFileInfo fi(url);
    if (!fi.exists())
        return QString::null;

    QFile f(url);
    if (!f.open(IO_ReadOnly))
        return QString::null;
    
    QDomDocument doc;
    if (!doc.setContent(&f) || (doc.doctype().name() != "kdeveloptoc"))
        return QString::null;
    f.close();

    QDomElement titleEl = doc.documentElement().namedItem("title").toElement();

    return titleEl.firstChild().toText().data();
}


QStringList DocKDevTOCPlugin::fullTextSearchLocations()
{
    return QStringList();
}

bool DocKDevTOCPlugin::needRefreshIndex(DocumentationCatalogItem* /*item*/)
{
    return false;
}

void DocKDevTOCPlugin::autoSetupPlugin()
{
    QStringList tocsDir = DocKDevTOCPluginFactory::instance()->dirs()->findAllResources("data", "kdevdocumentation/tocs/*.toc");
    
    for (QStringList::const_iterator it = tocsDir.begin(); it != tocsDir.end(); ++it)
    {
        config->setGroup("Locations");
        config->writePathEntry(catalogTitle(*it), *it);
    }
}

void DocKDevTOCPlugin::createIndex(KListBox* index, DocumentationCatalogItem* item)
{
    TOCDocumentationCatalogItem *tocItem = dynamic_cast<TOCDocumentationCatalogItem *>(item);
    if (!tocItem)
        return;
    
    QFileInfo fi(tocItem->tocFile());

    QFile f(tocItem->tocFile());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << tocItem->tocFile() << endl;
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "kdeveloptoc")
    {
        kdDebug(9002) << "Not a valid kdeveloptoc file: " << tocItem->tocFile() << endl;
        return;
    }
    f.close();

    QDomElement docEl = doc.documentElement();
    QDomElement baseEl = docEl.namedItem("base").toElement();
    
    QString base;
    if (!baseEl.isNull())
        base = baseEl.attribute("href", QString::null);
    
    QDomElement indexEl = docEl.namedItem("index").toElement();
    QDomElement childEl = indexEl.firstChild().toElement();
    while (!childEl.isNull())
    {
        if (childEl.tagName() == "entry")
        {
            QString name = childEl.attribute("name");
            QString url = childEl.attribute("url");
            
            IndexItem *ii = new IndexItem(this, item, index, name);
            ii->addURL(KURL(constructURL(base, url)));
        }
        childEl = childEl.nextSibling().toElement();
    }
}

void DocKDevTOCPlugin::createTOC(DocumentationCatalogItem* item)
{
    TOCDocumentationCatalogItem *tocItem = dynamic_cast<TOCDocumentationCatalogItem *>(item);
    if (!tocItem)
        return;
    
    QFileInfo fi(tocItem->tocFile());

    QFile f(tocItem->tocFile());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << tocItem->tocFile() << endl;
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "kdeveloptoc")
    {
        kdDebug(9002) << "Not a valid kdeveloptoc file: " << tocItem->tocFile() << endl;
        return;
    }
    f.close();

    QDomElement docEl = doc.documentElement();
    QDomElement baseEl = docEl.namedItem("base").toElement();
    
    QString base;
    if (!baseEl.isNull())
        base = baseEl.attribute("href", QString::null);
    
    QDomElement childEl = docEl.lastChild().toElement();
    addTocSect(tocItem, childEl, base, 1);
}

void DocKDevTOCPlugin::addTocSect(DocumentationItem *parent, QDomElement childEl, const QString &base, uint level)
{
    while (!childEl.isNull())
    {
        if (childEl.tagName() == QString("tocsect%1").arg(level))
        {
            QString name = childEl.attribute("name");
            QString url = childEl.attribute("url");
            
            DocumentationItem *item = new DocumentationItem(level == 1 ? DocumentationItem::Book : DocumentationItem::Document, parent, name);
            item->setURL(KURL(constructURL(base, url)));

            QDomElement grandchildEl = childEl.lastChild().toElement();
            addTocSect(item, grandchildEl, base, level+1);
        }
        childEl = childEl.previousSibling().toElement();
    }
}

void DocKDevTOCPlugin::setCatalogURL(DocumentationCatalogItem* item)
{
    TOCDocumentationCatalogItem *tocItem = dynamic_cast<TOCDocumentationCatalogItem *>(item);
    if (!tocItem)
        return;
    
    QFileInfo fi(tocItem->tocFile());

    QFile f(tocItem->tocFile());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << tocItem->tocFile() << endl;
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "kdeveloptoc")
    {
        kdDebug(9002) << "Not a valid kdeveloptoc file: " << tocItem->tocFile() << endl;
        return;
    }
    f.close();

    QDomElement docEl = doc.documentElement();
    QDomElement baseEl = docEl.namedItem("base").toElement();
    
    if (item->url().isEmpty())
    {
        if (baseEl.isNull())
            item->setURL(KURL());
        else
            item->setURL(KURL(constructURL(baseEl.attribute("href", QString::null),
                baseEl.attribute("url", QString::null))));
    }
}

QString DocKDevTOCPlugin::constructURL(const QString &base, const QString &url)
{
    if (base.isEmpty() && !url.isEmpty())
        return url;
    if (!url.isEmpty())
        return base.endsWith("/") ? base + url : base + "/" + url;
    else
        return base;
}

#include "dockdevtocplugin.moc"
