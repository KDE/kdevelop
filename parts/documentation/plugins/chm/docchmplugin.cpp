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
#include "docchmplugin.h"

#include <kurl.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <klistview.h>

#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>

#include "../../../../config.h"

static const KDevPluginInfo data("docchmplugin");
typedef KDevGenericFactory<DocCHMPlugin> DocCHMPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdocchmplugin, DocCHMPluginFactory(data) )

DocCHMPlugin::DocCHMPlugin(QObject* parent, const char* name, QStringList // args
                           )
    :DocumentationPlugin(DocCHMPluginFactory::instance()->config(), parent, name)
{
    setCapabilities(CustomDocumentationTitles);
    autoSetup();
}

DocCHMPlugin::~DocCHMPlugin()
{
}

DocumentationCatalogItem* DocCHMPlugin::createCatalog(KListView* contents, const QString& title, const QString& url)
{
    DocumentationCatalogItem *item = new DocumentationCatalogItem(this, contents, title);
    item->setURL(KURL(url));
    return item;
}

QPair<KFile::Mode, QString> DocCHMPlugin::catalogLocatorProps()
{
    return QPair<KFile::Mode, QString>(KFile::File, "*.chm");
}

QString DocCHMPlugin::catalogTitle(const QString& // url
                                   )
{
    return QString::null;
}

QString DocCHMPlugin::pluginName() const
{
    return i18n("CHM Documentation Collection");
}

QStringList DocCHMPlugin::fullTextSearchLocations()
{
    return QStringList();
}

bool DocCHMPlugin::needRefreshIndex(DocumentationCatalogItem* // item
                                    )
{
    return false;
}

void DocCHMPlugin::autoSetupPlugin()
{
    return;
}

void DocCHMPlugin::createIndex(IndexBox* // index
                               , DocumentationCatalogItem* // item
                               )
{
    return;
}

void DocCHMPlugin::createTOC(DocumentationCatalogItem* // item
                             )
{
    return;
}

void DocCHMPlugin::setCatalogURL(DocumentationCatalogItem* item)
{
    KURL chmURL = item->url();
    chmURL.setProtocol("ms-its");
    chmURL.addPath("/");
    item->setURL(chmURL);
}

#include "docchmplugin.moc"
