/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
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
#include "doccustomplugin.h"

#include <kurl.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <klistview.h>

#include <kdevgenericfactory.h>

static const KAboutData data("doccustomplugin", I18N_NOOP("Custom documentation plugin"), "1.0");
typedef KDevGenericFactory<DocCustomPlugin> DocCustomPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdoccustomplugin, DocCustomPluginFactory(&data) )

DocCustomPlugin::DocCustomPlugin(QObject* parent, const char* name, const QStringList args)
    :DocumentationPlugin(DocCustomPluginFactory::instance()->config(), parent, name)
{
    setCapabilities(CustomDocumentationTitles | ProjectUserManual);
    autoSetup();
}

DocCustomPlugin::~DocCustomPlugin()
{
}

bool DocCustomPlugin::needRefreshIndex(DocumentationCatalogItem* item)
{
    return false;
}

QString DocCustomPlugin::pluginName() const
{
    return i18n("Custom Documentation Collection");
}

DocumentationCatalogItem* DocCustomPlugin::createCatalog(KListView* contents, const QString& title, const QString& url)
{
    DocumentationCatalogItem *item = new DocumentationCatalogItem(this, contents, title);
    item->setURL(KURL(url));
    return item;
}

QPair<KFile::Mode, QString> DocCustomPlugin::catalogLocatorProps()
{
    return QPair<KFile::Mode, QString>(KFile::File, "all/allfiles");
}

QString DocCustomPlugin::catalogTitle(const QString& /*url*/)
{
    return QString::null;
}

QStringList DocCustomPlugin::fullTextSearchLocations()
{
    return QStringList();
}


void DocCustomPlugin::autoSetupPlugin()
{
    return;
}

void DocCustomPlugin::createIndex(IndexBox* /*index*/, DocumentationCatalogItem* /*item*/)
{
    return;
}

void DocCustomPlugin::createTOC(DocumentationCatalogItem* /*item*/)
{
    return;
}

void DocCustomPlugin::setCatalogURL(DocumentationCatalogItem* /*item*/)
{
    return;
}

ProjectDocumentationPlugin *DocCustomPlugin::projectDocumentationPlugin(ProjectDocType type)
{
    if (type == UserManual)
        return new ProjectDocumentationPlugin(this, type);
    return DocumentationPlugin::projectDocumentationPlugin(type);
}
