/* This file is part of the KDE project
   Copyright (C) 2004 by Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kdevdocumentationplugin.h"

#include <qfile.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qheader.h>
#include <qtextstream.h>

#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kdebug.h>

//class DocumentationItem

DocumentationItem::DocumentationItem(DocumentationItem::Type type, KListView *parent,
    const QString &name)
    :KListViewItem(parent, name), m_type(type)
{
    init();
}

DocumentationItem::DocumentationItem(DocumentationItem::Type type, DocumentationItem *parent,
    const QString &name)
    :KListViewItem(parent, name), m_type(type)
{
    init();
}

DocumentationItem::DocumentationItem(DocumentationItem::Type type, KListView *parent, 
    DocumentationItem *after, const QString &name)
    :KListViewItem(parent, after, name), m_type(type)
{
    init();
}

void DocumentationItem::init( )
{
    QString icon;
    
    switch (m_type)
    {
        case Collection:
        case Catalog:
            icon = "folder";
            break;
        case Book:
            icon = "contents";
            break;
        default:
            icon = "document";
    }
    
    setPixmap(0, SmallIcon(icon));
}





//class DocumentationCatalogItem

  
DocumentationCatalogItem::DocumentationCatalogItem(DocumentationPlugin* plugin,
    KListView *parent, const QString &name)
    :DocumentationItem(DocumentationItem::Catalog, parent, name), m_plugin(plugin),
    isLoaded(false), isActivated(false)
{
    setExpandable(true);
    m_plugin->addCatalog(this);
}

DocumentationCatalogItem::DocumentationCatalogItem(DocumentationPlugin* plugin,
    DocumentationItem *parent, const QString &name)
    :DocumentationItem(DocumentationItem::Catalog, parent, name), m_plugin(plugin),
    isLoaded(false), isActivated(false)
{
    setExpandable(true);
    m_plugin->addCatalog(this);
}

DocumentationCatalogItem::~ DocumentationCatalogItem( )
{
    m_plugin->clearCatalog(this);
}

void DocumentationCatalogItem::setOpen(bool o)
{
    if (o && !isLoaded)
    {
        plugin()->createTOC(this);
        isLoaded = true;
    }
    DocumentationItem::setOpen(o);
}

void DocumentationCatalogItem::activate()
{
    if (!isActivated)
    {
        plugin()->setCatalogURL(this);
        isActivated = true;
    }
    DocumentationItem::activate();
}






//class IndexItem

IndexItem::IndexItem(DocumentationPlugin *plugin, DocumentationCatalogItem *catalog, 
    QListBox *listbox, const QString &text)
    :QListBoxText(listbox, text)
{
    plugin->indexes[catalog].append(this);
}






//class ConfigurationItem

ConfigurationItem::ConfigurationItem(QListView *parent, const QString &title, const QString &url,
    bool indexPossible, bool fullTextSearchPossible)
    :QCheckListItem(parent, "", QCheckListItem::CheckBox), m_title(title), m_url(url),
    m_origTitle(title), m_contents(true), m_index(false), m_fullTextSearch(false),
    m_indexPossible(indexPossible), m_fullTextSearchPossible(fullTextSearchPossible)
{
    setText(3, m_title);
    setText(4, m_url);
}

void ConfigurationItem::paintCell(QPainter *p, const QColorGroup &cg, int column,
    int width, int align)
{
    if ( (column == 0) || (column == 1) || (column == 2) )
    {
        if ( !p )
            return;
    
        QListView *lv = listView();
        if ( !lv )
            return;
    
        const BackgroundMode bgmode = lv->viewport()->backgroundMode();
        const QColorGroup::ColorRole crole = QPalette::backgroundRoleFromMode( bgmode );
        p->fillRect(0, 0, width, height(), cg.brush(crole));

        QFontMetrics fm(lv->fontMetrics());
        int boxsize = lv->style().pixelMetric(QStyle::PM_CheckListButtonSize, lv);
        int marg = lv->itemMargin();
        int styleflags = QStyle::Style_Default;
                
        if (((column == 0) && m_contents) || ((column == 1) && m_index) || ((column == 2) && m_fullTextSearch))
            styleflags |= QStyle::Style_On;
        else
            styleflags |= QStyle::Style_Off;
        if ((column == 0) || ((column == 1) && m_indexPossible) || ((column == 2) && m_fullTextSearchPossible))
            styleflags |= QStyle::Style_Enabled;

        int x = 0;
        int y = 0;
        x += 3;
        if (align & AlignVCenter)
            y = ((height() - boxsize) / 2) + marg;
        else
            y = (fm.height() + 2 + marg - boxsize) / 2;

        QStyleOption opt(this);
        lv->style().drawPrimitive(QStyle::PE_CheckListIndicator, p,
            QRect(x, y, boxsize, fm.height() + 2 + marg), cg, styleflags, opt);
               
        return;
    }
    QListViewItem::paintCell(p, cg, column, width, align);
}

int ConfigurationItem::width(const QFontMetrics &fm, const QListView *lv, int c) const
{
    if ((c == 0) || (c == 1) || (c == 2))
        return lv->style().pixelMetric(QStyle::PM_CheckListButtonSize, lv) + 4;
    return QListViewItem::width(fm, lv, c);
}








//class DocumentationPlugin

DocumentationPlugin::DocumentationPlugin(KConfig *pluginConfig, QObject *parent, const char *name)
    :QObject(parent, name), config(pluginConfig), m_indexCreated(false)
{
}

void DocumentationPlugin::autoSetup()
{
    config->setGroup("General");    
    if ( ! config->readBoolEntry("Autosetup", false) )
    {
        autoSetupPlugin();
        config->setGroup("General");
        config->writeEntry("Autosetup", true);
        config->sync();
    }
}

void DocumentationPlugin::reload()
{
    clear();
    for (QValueList<DocumentationCatalogItem *>::iterator it = catalogs.begin();
        it != catalogs.end(); ++it)
    {
        createTOC(*it);
    }
}

void DocumentationPlugin::clear()
{
    for (QValueList<DocumentationCatalogItem *>::iterator it = catalogs.begin();
        it != catalogs.end(); ++it)
    {
        clearCatalog(*it);
    }
}

void DocumentationPlugin::clearCatalog(DocumentationCatalogItem *item)
{
    //clear named catalog map
    for (QMap<QString, DocumentationCatalogItem*>::iterator it = namedCatalogs.begin();
        it != namedCatalogs.end(); ++it)
    {
        if (it.data() == item)
            namedCatalogs.remove(it);
    }
    //clear indexes for catalog
    QValueList<IndexItem *> idx = indexes[item];
    for (QValueList<IndexItem *>::iterator it = idx.begin(); it != idx.end(); ++it)
    {
        delete *it;
    }
    indexes.remove(item);

    //remove catalog
    catalogs.remove(item);
}

void DocumentationPlugin::createIndex(KListBox *index)
{
    if (m_indexCreated)
        return;
    
    for (QValueList<DocumentationCatalogItem *>::iterator it = catalogs.begin();
        it != catalogs.end(); ++it)
    {
        loadIndex(index, *it);
    }
    m_indexCreated = true;
}

void DocumentationPlugin::cacheIndex(DocumentationCatalogItem *item)
{
    kdDebug() << "Creating index cache for " << item->text(0) << endl;
    
    QString cacheName = locateLocal("data", QString("kdevdocumentation/index/cache_") + item->text(0));
    QFile cacheFile(cacheName);
    if (!cacheFile.open(IO_WriteOnly))
        return;
    
    QTextStream str(&cacheFile);
    str.setEncoding(QTextStream::UnicodeUTF8);

    QValueList<IndexItem*> catalogIndexes = indexes[item];
    for (QValueList<IndexItem*>::const_iterator it = catalogIndexes.constBegin();
        it != catalogIndexes.constEnd(); ++it)
    {
        str << (*it)->text() << "::::" << (*it)->urls().toStringList().join("::::") << endl;
    }
   
    cacheFile.close();
}

bool DocumentationPlugin::loadCachedIndex(KListBox *index, DocumentationCatalogItem *item)
{
    QString cacheName = locateLocal("data", QString("kdevdocumentation/index/cache_") + item->text(0));
    QFile cacheFile(cacheName);
    if (!cacheFile.open(IO_ReadOnly))
        return false;

    kdDebug() << "Using cached index for item: " << item->text(0) << endl;
    
    QTextStream str(&cacheFile);
    str.setEncoding(QTextStream::UnicodeUTF8);

    while (!str.eof())
    {
        QStringList cacheItem = QStringList::split("::::", str.readLine(), false);
        IndexItem *ii = new IndexItem(this, item, index, cacheItem.first());
        QStringList::const_iterator it = cacheItem.constBegin();
        ++it;
        for (; it != cacheItem.constEnd(); ++it)
            ii->addURL(KURL(*it));
    }

    cacheFile.close();    
    return true;
}

void DocumentationPlugin::addCatalog(DocumentationCatalogItem *item)
{
    catalogs.append(item);
    namedCatalogs[item->text(0)] = item;
//    indexes[item] = QValueList<IndexItem*>();
}

void DocumentationPlugin::addCatalogConfiguration(KListView *configurationView,
    const QString &title, const QString &url)
{
    new ConfigurationItem(configurationView, title, url, 
        hasCapability(Index), hasCapability(FullTextSearch));
}

void DocumentationPlugin::editCatalogConfiguration(ConfigurationItem *configurationItem,
    const QString &title, const QString &url)
{
    configurationItem->setTitle(title);
    configurationItem->setURL(url);
}

void DocumentationPlugin::deleteCatalogConfiguration(const ConfigurationItem *const configurationItem)
{
    deletedConfigurationItems << configurationItem->title();
}

void DocumentationPlugin::clearCatalogIndex(DocumentationCatalogItem *item)
{
    //clear indexes for catalog
    QValueList<IndexItem *> idx = indexes[item];
    for (QValueList<IndexItem *>::iterator it = idx.begin(); it != idx.end(); ++it)
    {
        delete *it;
    }
    indexes.remove(item);
}

void DocumentationPlugin::loadIndex(KListBox *index, DocumentationCatalogItem *item)
{
    if (!indexEnabled(item))
        return;
    if (!needRefreshIndex(item) && loadCachedIndex(index, item))
        return;
    createIndex(index, item);
    cacheIndex(item);
}

void DocumentationPlugin::init(KListView *contents)
{
    config->setGroup("Locations");
    QMap<QString, QString> entryMap = config->entryMap("Locations");
    
    for (QMap<QString, QString>::const_iterator it = entryMap.begin();
        it != entryMap.end(); ++it)
    {
        if (catalogEnabled(it.key()))
            createCatalog(contents, it.key(), config->readPathEntry(it.key()));
    }
}

void DocumentationPlugin::reinit(KListView *contents, KListBox *index, QStringList restrictions)
{
    config->setGroup("Locations");
    QMap<QString, QString> entryMap = config->entryMap("Locations");

    //remove deleted in configuration catalogs
    for (QStringList::const_iterator it = deletedConfigurationItems.constBegin();
        it != deletedConfigurationItems.constEnd(); ++it)
    {
        if (namedCatalogs.contains(*it))
            delete namedCatalogs[*it];
    }
    deletedConfigurationItems.clear();

    //update configuration
    for (QMap<QString, QString>::const_iterator it = entryMap.begin();
        it != entryMap.end(); ++it)
    {
        if (restrictions.contains(it.key()) || (!catalogEnabled(it.key())))
        {
            if (namedCatalogs.contains(it.key()))
                delete namedCatalogs[it.key()];
        }
        else
        {
            kdDebug() << "updating 1" << endl;
            if (!namedCatalogs.contains(it.key()))    //create catalog if it does not exist
            {
                DocumentationCatalogItem * item = createCatalog(contents, it.key(), config->readPathEntry(it.key()));
                loadIndex(index, item);
            }
            else if (!indexEnabled(namedCatalogs[it.key()]))    //clear index if it is disabled in configuration
            {
                kdDebug() << "    updating: clearCatalogIndex" << endl;
                clearCatalogIndex(namedCatalogs[it.key()]);
            }
            else if ( (indexEnabled(namedCatalogs[it.key()]))    //index is requested in configuration but does not yet exist
                && (!indexes.contains(namedCatalogs[it.key()])) )
            {
                 kdDebug() << "    index requested " << endl;
                 loadIndex(index, namedCatalogs[it.key()]);
            }
            else if (indexEnabled(namedCatalogs[it.key()]))
                kdDebug() << "    1" << endl;
            else if (!indexes.contains(namedCatalogs[it.key()]))
                kdDebug() << "    2" << endl;
            else
                kdDebug() << "    3" << endl;                
        }
    }
}

void DocumentationPlugin::loadCatalogConfiguration(KListView *configurationView)
{
    config->setGroup("Locations");
    QMap<QString, QString> entryMap = config->entryMap("Locations");

    for (QMap<QString, QString>::const_iterator it = entryMap.begin();
        it != entryMap.end(); ++it)
    {
        ConfigurationItem *item = new ConfigurationItem(configurationView, it.key(), it.data(),
            hasCapability(Index), hasCapability(FullTextSearch));
        config->setGroup("TOC Settings");
        item->setContents(config->readBoolEntry(item->title(), true));
        config->setGroup("Index Settings");
        item->setIndex(config->readBoolEntry(item->title(), false));
        config->setGroup("Search Settings");
        item->setFullTextSearch(config->readBoolEntry(item->title(), false));
    }
}

void DocumentationPlugin::saveCatalogConfiguration(KListView *configurationView)
{
    config->setGroup("Locations");
    
    for (QStringList::const_iterator it = deletedConfigurationItems.constBegin();
        it != deletedConfigurationItems.constEnd(); ++it)
    {
        config->deleteEntry(*it);
    }
    
    QListViewItemIterator it(configurationView);
    while (it.current())
    {
        config->setGroup("Locations");
        ConfigurationItem *confItem = dynamic_cast<ConfigurationItem*>(it.current());
        if (confItem->isChanged())
            config->deleteEntry(confItem->origTitle());
        config->writePathEntry(confItem->title(), confItem->url());
        
        config->setGroup("TOC Settings");
        if (confItem->isChanged())
            config->deleteEntry(confItem->origTitle());
        config->writeEntry(confItem->title(), confItem->contents());
        
        config->setGroup("Index Settings");
        if (confItem->isChanged())
            config->deleteEntry(confItem->origTitle());
        config->writeEntry(confItem->title(), confItem->index());

        config->setGroup("Search Settings");
        if (confItem->isChanged())
            config->deleteEntry(confItem->origTitle());
        config->writeEntry(confItem->title(), confItem->fullTextSearch());

        ++it;
    }
    config->sync();
}

void DocumentationPlugin::setIndexEnabled( DocumentationCatalogItem * item, bool e )
{
    QString group = config->group();
    config->setGroup("Index Settings");
    config->writeEntry(item->text(0), e);
    config->setGroup(group);
}

bool DocumentationPlugin::indexEnabled( DocumentationCatalogItem * item ) const
{
    QString group = config->group();
    config->setGroup("Index Settings");
    bool b = config->readBoolEntry(item->text(0), false);
    config->setGroup(group);
    return b;
}

bool DocumentationPlugin::catalogEnabled(const QString &name) const
{
    QString group = config->group();
    config->setGroup("TOC Settings");
    bool b = config->readBoolEntry(name, true);
    config->setGroup(group);
    return b;
}

void DocumentationPlugin::setCatalogEnabled(const QString &name, bool e)
{
    QString group = config->group();
    config->setGroup("TOC Settings");
    config->writeEntry(name, e);
    config->setGroup(group);
}

#include "kdevdocumentationplugin.moc"
