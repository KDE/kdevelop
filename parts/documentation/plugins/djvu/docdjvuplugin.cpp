/***************************************************************************
 *   Copyright (C) 2004 by Mathieu Chouinard                               *
 *   mathieu.chouinard@kdemail.net                                         *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "docdjvuplugin.h"

#include <kurl.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <klistview.h>

#include <kdevgenericfactory.h>

static const KAboutData data("docdjvuplugin", I18N_NOOP("Djvu documentation plugin"), "1.0");
typedef KDevGenericFactory<DocDjvuPlugin> DocDjvuPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdocdjvuplugin, DocDjvuPluginFactory(&data) )

DocDjvuPlugin::DocDjvuPlugin(QObject* parent, const char* name, const QStringList args)
    :DocumentationPlugin(DocDjvuPluginFactory::instance()->config(), parent, name)
{
    setCapabilities(CustomDocumentationTitles | ProjectUserManual);
    autoSetup();
}

DocDjvuPlugin::~DocDjvuPlugin()
{
}

bool DocDjvuPlugin::needRefreshIndex(DocumentationCatalogItem* item)
{
    return false;
}

QString DocDjvuPlugin::pluginName() const
{
    return i18n("Djvu Documentation Collection");
}

DocumentationCatalogItem* DocDjvuPlugin::createCatalog(KListView* contents, const QString& title, const QString& url)
{
    DocumentationCatalogItem *item = new DocumentationCatalogItem(this, contents, title);
    item->setURL(KURL(url));
    return item;
}

QPair<KFile::Mode, QString> DocDjvuPlugin::catalogLocatorProps()
{
    return QPair<KFile::Mode, QString>(KFile::File, "all/allfiles");
}

QString DocDjvuPlugin::catalogTitle(const QString& /*url*/)
{
    return QString::null;
}

QStringList DocDjvuPlugin::fullTextSearchLocations()
{
    return QStringList();
}


void DocDjvuPlugin::autoSetupPlugin()
{
    return;
}

void DocDjvuPlugin::createIndex(IndexBox* /*index*/, DocumentationCatalogItem* /*item*/)
{
    return;
}

void DocDjvuPlugin::createTOC(DocumentationCatalogItem* /*item*/)
{
    return;
}

void DocDjvuPlugin::setCatalogURL(DocumentationCatalogItem* /*item*/)
{
    return;
}

ProjectDocumentationPlugin *DocDjvuPlugin::projectDocumentationPlugin(ProjectDocType type)
{
    if (type == UserManual)
        return new ProjectDocumentationPlugin(this, type);
    return DocumentationPlugin::projectDocumentationPlugin(type);
}
