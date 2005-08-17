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
#include <q3header.h>
#include <qtextstream.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <klocale.h>

//class DocumentationItem

DocumentationItem::DocumentationItem(DocumentationItem::Type type, KListView *parent,
    const QString &name)
    :KListViewItem(parent, name), m_type(type)
{
    init();
}

DocumentationItem::DocumentationItem(DocumentationItem::Type type, KListViewItem *parent,
    const QString &name)
    :KListViewItem(parent, name), m_type(type)
{
    init();
}

DocumentationItem::DocumentationItem(DocumentationItem::Type type, KListView *parent, 
    KListViewItem *after, const QString &name)
    :KListViewItem(parent, after, name), m_type(type)
{
    init();
}

DocumentationItem::DocumentationItem(DocumentationItem::Type type, KListViewItem * parent,
    KListViewItem * after, const QString & name )
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
    isLoaded(false), isActivated(false), m_isProjectDocumentationItem(false)
{
    setExpandable(true);
    m_plugin->addCatalog(this);
}

DocumentationCatalogItem::DocumentationCatalogItem(DocumentationPlugin* plugin,
    DocumentationItem *parent, const QString &name)
    :DocumentationItem(DocumentationItem::Catalog, parent, name), m_plugin(plugin),
    isLoaded(false), isActivated(false), m_isProjectDocumentationItem(false)
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
    if (o)
    {
        load();
    }
    DocumentationItem::setOpen(o);
}

void DocumentationCatalogItem::load()
{
if(isLoaded)
return;

        plugin()->createTOC(this);
        isLoaded = true;
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




//class IndexItemProto

IndexItemProto::IndexItemProto(DocumentationPlugin *plugin, DocumentationCatalogItem *catalog, 
    IndexBox *listbox, const QString &text, const QString &description)
    : m_listbox(listbox), m_text(text), m_description(description)
{
    plugin->indexes[catalog].append(this);
    m_listbox->addIndexItem(this);
}

IndexItemProto::~IndexItemProto()
{
    m_listbox->removeIndexItem(this);
}


//class IndexItem

IndexItem::IndexItem(IndexBox *listbox, const QString &text)
    :Q3ListBoxText(listbox, text), m_listbox(listbox)
{
}

IndexItem::List IndexItem::urls() const
{
    List urlList;
    Q3ValueList<IndexItemProto*> itemProtos = m_listbox->items[text()];
    for (Q3ValueList<IndexItemProto*>::const_iterator it = itemProtos.begin();
        it != itemProtos.end(); ++it)
        urlList.append(qMakePair((*it)->description(), (*it)->url()));
    return urlList;
}




//class ConfigurationItem

ConfigurationItem::ConfigurationItem(Q3ListView *parent, DocumentationPlugin * plugin, const QString &title, const QString &url,
    bool indexPossible, bool fullTextSearchPossible)
    :Q3CheckListItem(parent, "", Q3CheckListItem::CheckBox), m_title(title), m_url(url),
    m_origTitle(title), m_contents(true), m_index(false), m_fullTextSearch(false),
    m_indexPossible(indexPossible), m_fullTextSearchPossible(fullTextSearchPossible),
    m_docPlugin( plugin )
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
    
        Q3ListView *lv = listView();
        if ( !lv )
            return;
    
        const BackgroundMode bgmode = lv->viewport()->backgroundMode();
        const QColorGroup::ColorRole crole = QPalette::backgroundRoleFromMode( bgmode );
        p->fillRect(0, 0, width, height(), cg.brush(crole));

        QFontMetrics fm(lv->fontMetrics());
        int boxsize = lv->style().pixelMetric(QStyle::PM_CheckListButtonSize, lv);
        int marg = lv->itemMargin();
        int styleflags = QStyle::State_None;
                
        if (((column == 0) && m_contents) || ((column == 1) && m_index) || ((column == 2) && m_fullTextSearch))
            styleflags |= QStyle::State_On;
        else
            styleflags |= QStyle::State_Off;
        if ((column == 0) || ((column == 1) && m_indexPossible) || ((column == 2) && m_fullTextSearchPossible))
            styleflags |= QStyle::State_Enabled;

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
    Q3ListViewItem::paintCell(p, cg, column, width, align);
}

int ConfigurationItem::width(const QFontMetrics &fm, const Q3ListView *lv, int c) const
{
    if ((c == 0) || (c == 1) || (c == 2))
        return lv->style().pixelMetric(QStyle::PM_CheckListButtonSize, lv) + 4;
    return Q3ListViewItem::width(fm, lv, c);
}








//class DocumentationPlugin

DocumentationPlugin::DocumentationPlugin(KConfig *pluginConfig, QObject *parent, const char *name)
    :QObject(parent, name), config(pluginConfig), m_indexCreated(false)
{
}

DocumentationPlugin::~DocumentationPlugin()
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
    for (Q3ValueList<DocumentationCatalogItem *>::iterator it = catalogs.begin();
        it != catalogs.end(); ++it)
    {
        createTOC(*it);
    }
}

void DocumentationPlugin::clear()
{
    for (Q3ValueList<DocumentationCatalogItem *>::iterator it = catalogs.begin();
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
    Q3ValueList<IndexItemProto *> idx = indexes[item];
    for (Q3ValueList<IndexItemProto *>::iterator it = idx.begin(); it != idx.end(); ++it)
    {
        delete *it;
    }
    indexes.remove(item);

    //remove catalog
    catalogs.remove(item);
}

void DocumentationPlugin::createIndex(IndexBox *index)
{
    if (m_indexCreated)
        return;
    
    for (Q3ValueList<DocumentationCatalogItem *>::iterator it = catalogs.begin();
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
    if (!cacheFile.open(QIODevice::WriteOnly))
        return;
    
    QTextStream str(&cacheFile);
    str.setEncoding(QTextStream::Unicode);
    str << CACHE_VERSION << endl;

    Q3ValueList<IndexItemProto*> catalogIndexes = indexes[item];
    for (Q3ValueList<IndexItemProto*>::const_iterator it = catalogIndexes.constBegin();
        it != catalogIndexes.constEnd(); ++it)
    {
        str << (*it)->text() << endl;
        str << (*it)->description() << endl;
        str << (*it)->url().url() << endl;
    }
   
    cacheFile.close();
}

bool DocumentationPlugin::loadCachedIndex(IndexBox *index, DocumentationCatalogItem *item)
{
    QString cacheName = locateLocal("data", QString("kdevdocumentation/index/cache_") + item->text(0));
    QFile cacheFile(cacheName);
    if (!cacheFile.open(QIODevice::ReadOnly))
        return false;

    kdDebug() << "Using cached index for item: " << item->text(0) << endl;
    
    QTextStream str(&cacheFile);
    str.setEncoding(QTextStream::Unicode);
    QString cache = str.read();
    QStringList cacheList = QStringList::split("\n", cache, true);
    QString ver = cacheList.first();
    if (ver != CACHE_VERSION)
    {
        kdDebug() << "Wrong cache version: " << ver << endl;
        return false;
    }
    QStringList::const_iterator it = cacheList.begin();
    it++;
    QString s[3]; int c = 0;
    for (; it != cacheList.end(); ++it)
    {
        s[c] = *it;
        if (c == 2)
        {
            IndexItemProto *ii = new IndexItemProto(this, item, index, s[0], s[1]);
            ii->addURL(KURL(s[2]));
            c = 0;
        }
        else c++;
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
    new ConfigurationItem(configurationView, this, title, url, 
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
    Q3ValueList<IndexItemProto *> idx = indexes[item];
    for (Q3ValueList<IndexItemProto *>::iterator it = idx.begin(); it != idx.end(); ++it)
    {
        delete *it;
    }
    indexes.remove(item);
}

void DocumentationPlugin::loadIndex(IndexBox *index, DocumentationCatalogItem *item)
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

void DocumentationPlugin::reinit(KListView *contents, IndexBox *index, QStringList restrictions)
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
        config->setGroup("Locations");
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
                index->setDirty(true);
//                index->refill(indexes[item]);
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
                index->setDirty(true);
            }
            m_indexCreated = true;
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
        if (namedCatalogs.contains(it.key())
            && namedCatalogs[it.key()]->isProjectDocumentationItem())
            continue;
        
        ConfigurationItem *item = new ConfigurationItem(configurationView, this, it.key(), it.data(),
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
    
    Q3ListViewItemIterator it(configurationView);
    while (it.current())
    {
        ConfigurationItem *confItem = dynamic_cast<ConfigurationItem*>(it.current());
        if ( confItem->docPlugin() != this ) 
        {
            ++it;
            continue;
        }

        config->setGroup("Locations");
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




//class IndexBox

IndexBox::IndexBox(QWidget *parent, const char *name)
    :KListBox(parent, name), m_dirty(false)
{
}

void IndexBox::addIndexItem(IndexItemProto *item)
{
    items[item->text()].append(item);
}

void IndexBox::removeIndexItem(IndexItemProto *item)
{
    QString text = item->text();
    items[text].remove(item);
    if (items[text].count() == 0)
    {
        items.remove(text);
        Q3ListBoxItem *item = findItem(text, Qt::CaseSensitive | Qt::ExactMatch);
        if (item)
            delete item;
    }
}

void IndexBox::fill()
{
    for (QMap<QString, Q3ValueList<IndexItemProto*> >::const_iterator it = items.begin();
        it != items.end(); ++it)
    {
        new IndexItem(this, it.key());
    }
}

void IndexBox::setDirty(bool dirty)
{
    m_dirty = dirty;
}

void IndexBox::refill()
{
    if (m_dirty)
    {
        clear();
        fill();
        setDirty(false);
    }
}


ProjectDocumentationPlugin::ProjectDocumentationPlugin(DocumentationPlugin *docPlugin, DocumentationPlugin::ProjectDocType type)
    :QObject(0, 0), m_docPlugin(docPlugin), m_catalog(0), m_type(type), m_contents(0), m_index(0)
{
    kdDebug() << "ProjectDocumentationPlugin::ProjectDocumentationPlugin for type " << type << endl;
    
    m_watch = new KDirWatch(this);
    connect(m_watch, SIGNAL(dirty(const QString&)), this, SLOT(reinit()));
    m_watch->startScan();
}

ProjectDocumentationPlugin::~ProjectDocumentationPlugin()
{
    deinit();
}

void ProjectDocumentationPlugin::init(KListView *contents, IndexBox *index, const QString &url)
{
    m_contents = contents;
    m_index = index;
    m_url = url;
    
    if (m_catalog)
        deinit();
    m_catalog = m_docPlugin->createCatalog(contents, 
        m_type == DocumentationPlugin::APIDocs ? i18n("Project API Documentation")
        : i18n("Project User Manual"), url);
    if (m_catalog)
    {
        m_catalog->setProjectDocumentationItem(true);
        m_watch->addFile(url);
    }
}

void ProjectDocumentationPlugin::reinit()
{
    deinit();
    if (m_contents != 0 && m_index != 0 && m_url != 0)
        init(m_contents, m_index, m_url);
}

void ProjectDocumentationPlugin::deinit()
{
    m_watch->removeFile(m_url);
    delete m_catalog;
    m_catalog = 0;
}

QString ProjectDocumentationPlugin::pluginName() const
{
    return m_docPlugin->pluginName();
}

QString ProjectDocumentationPlugin::catalogURL() const
{
    return m_url;
}

#include "kdevdocumentationplugin.moc"
