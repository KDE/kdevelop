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
#ifndef KDEV_DOC_PLUGIN_H
#define KDEV_DOC_PLUGIN_H

#include <qmap.h>
#include <qvaluelist.h>
#include <qpair.h>

#include <klistbox.h>
#include <klistview.h>
#include <kurl.h>
#include <kfile.h>

class DocumentationItem: public KListViewItem
{
public:
    enum Type { Collection, Catalog, Book, Document };

    DocumentationItem(Type type, KListView *parent, const QString &name);
    DocumentationItem(Type type, DocumentationItem *parent, const QString &name);

    virtual void setURL(const KURL &url) { m_url = url; }
    virtual KURL url() const { return m_url; }
    
    Type type() const { return m_type; }

private:
    void init();
        
    KURL m_url;
    Type m_type;
};

class DocumentationPlugin;

class DocumentationCatalogItem: public DocumentationItem
{
public:
    DocumentationCatalogItem(DocumentationPlugin* plugin, KListView *parent, const QString &name);
    DocumentationCatalogItem(DocumentationPlugin* plugin, DocumentationItem *parent, const QString &name);
    virtual ~DocumentationCatalogItem();
    
    DocumentationPlugin* plugin() const { return m_plugin; }
    
    virtual void setOpen(bool o);
    
protected:
    virtual void activate();
    
private:
    DocumentationPlugin* m_plugin;
    bool isLoaded;
    bool isActivated;
};

class IndexItem: public QListBoxText
{
public:
    IndexItem(DocumentationPlugin *plugin, DocumentationCatalogItem *catalog, QListBox *listbox,
        const QString &text = QString::null);

    void addURL(const KURL &url) { m_urls.append(url); }
    KURL::List urls() const { return m_urls; }
    
private:
    KURL::List m_urls;
};

class QPainter;
class QColorGroup;
class QFontMetrics;

class ConfigurationItem: public QCheckListItem
{
public:
    ConfigurationItem(QListView *parent, const QString &title, const QString &url,
        bool indexPossible, bool fullTextSearchPossible);

    virtual QString title() const { return m_title; }
    virtual void setTitle(const QString title) { setText(2, m_title = title); }
    virtual QString url() const { return m_url; }
    virtual void setURL(const QString url) { setText(3, m_url = url); }

    virtual bool isChanged() const { return m_title == m_origTitle; }
    virtual QString origTitle() const {return m_origTitle; }

    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
    virtual int width(const QFontMetrics &fm, const QListView *lv, int c) const;

    bool index() const { return m_index; }
    void setIndex(bool index) { m_index = index; }
    bool fullTextSearch() const { return m_fullTextSearch; }
    void setFullTextSearch(bool fullTextSearch) { m_fullTextSearch = fullTextSearch; }
    
    bool indexPossible() const { return m_indexPossible; }
    bool fullTextSearchPossible() const { return m_fullTextSearchPossible; }

private:
    QString m_title;
    QString m_url;
    QString m_origTitle;
    
    bool m_index;
    bool m_fullTextSearch;
    
    bool m_indexPossible;
    bool m_fullTextSearchPossible;
};


/**
@short Documentation Plugin Interface

All KDevelop documentation plugins must implement this interface.
Documentation plugin handles certain documentation type. It provides
methods to load documentation catalogs and indexes for a documentation
of that type. It also has methods to configure catalogs and provides
a list of URL's for the full text search tool.
*/
class DocumentationPlugin: public QObject
{
    Q_OBJECT
public:
    /**Capability of documentation plugin.*/
    enum Capability { Index=1 /**<index can be built for catalogs*/,
                      FullTextSearch=2 /**<full text search is possible in catalog locations*/,
                      CustomDocumentationTitles=4 /**<user can specify titles for documentation catalogs*/ };
    
    /**Constructor. Should initialize capabilities of the plugin by using setCapabilities
    protected method. For example,
    @code
    setCapabilities(Index | FullTextSearch);
    @endcode
    */
    DocumentationPlugin(QObject *parent =0, const char *name =0);
    
    /**Returns the i18n name of the plugin.*/
    virtual QString pluginName() const = 0;
    
    /**Initialize a list of catalogs.
    @param contents the listview to fill with catalogs
    */
    virtual void init(KListView *contents) = 0;
    /**Reloads a list of catalogs. This method should add missing catalogs to the view,
    update index for added catalogs and also delete restricted catalogs.
    @param contents the listview to fill with catalogs
    @param index the listbox with index to update
    @param restrictions the list of catalogs names to remove
    */
    virtual void reinit(KListView *contents, KListBox *index, QStringList restrictions) = 0;
    /**Initializes plugin configuration. Documentation plugins should be able to
    initialize the default configuration on startup without any user interaction.
    Do not forget that automatic setup should be called only once.
    
    Call this method in plugin's constructor and check if previous setup was made,
    for example
    @code
    config->setGroup("General");
    
    if ( ! config->readBoolEntry("Autosetup", false) )
        autoSetup();    
    @endcode
    Then in the end of autoSetup() body you can use
    @code
    config->setGroup("General");
    config->writeEntry("Autosetup", true);
    config->sync();
    @endcode
    to disable automatic setup in the future.
    */
    virtual void autoSetup() = 0;

    /**Indicates if an index of given catalog should be rebuilt. This method
    is used by index caching algorythm to make a descision to rebuild index
    or to load it from cache.*/
    virtual bool needRefreshIndex(DocumentationCatalogItem *item) = 0;
    /**Builds index for given catalog. This method should fill index with
    IndexItem objects.
    @param index the listbox which contains index items
    */
    virtual void createIndex(KListBox *index, DocumentationCatalogItem *item) = 0;

    /**Creates a table of contents for given catalog. Documentation part uses
    lazy loading of toc's to reduce startup time. This means that createTOC
    will be called on expand event of catalog item.*/
    virtual void createTOC(DocumentationCatalogItem *item) = 0;
    /**Sets the URL to the catalog.*/
    virtual void setCatalogURL(DocumentationCatalogItem *item) = 0;
    virtual KURL::List fullTextSearchLocations() = 0;

    /**Loads catalog configuration and fills configurationView with ConfigurationItem objects.*/
    virtual void loadCatalogConfiguration(KListView *configurationView) = 0;
    /**Saves catalog configuration basing on configurationView and 
    deletedConfigurationItems contents.*/
    virtual void saveCatalogConfiguration(KListView *configurationView) = 0;
    /**Adds new catalog to a configuration.*/
    virtual void addCatalogConfiguration(KListView *configurationView, 
        const QString &title, const QString &url);
    /**Edits catalog configuration.*/
    virtual void editCatalogConfiguration(ConfigurationItem *configurationItem,
        const QString &title, const QString &url);
    /**Removes catalog from configuration. configurationItem should not be removed here.*/
    virtual void deleteCatalogConfiguration(const ConfigurationItem *const configurationItem);
    
    /**Returns a mode and a filter for catalog locator dialogs.*/
    virtual QPair<KFile::Mode, QString> catalogLocatorProps() = 0;
    /**Returns a title of catalog defined by an url parameter.*/
    virtual QString catalogTitle(const QString &url) = 0;
    
    /**Reloads documentation catalogs and indices.*/
    virtual void reload();
    /**Clears documentation catalogs and indices.*/
    virtual void clear();
    
    /**Checks if documentation plugin has given capability.*/
    bool hasCapability(Capability cap) const { return m_capabilities & cap; }
    
    /**Sets dirty flag for all indices. Index caching algorythm will update
    the cache next time @ref createIndex is called.*/
    void setDirtyIndex(bool dirty) { m_indexCreated = dirty; }

    /**Caches index for documentation catalog. Reimplement this only if custom
    caching algorythm is used (do not forget to reimplement also @ref loadCachedIndex
    and @ref createIndex).*/
    virtual void cacheIndex(DocumentationCatalogItem *item);
    /**Loads index from the cache. Reimplement this only if custom
    caching algorythm is used (do not forget to reimplement also @ref cacheIndex
    and @ref createIndex).*/
    virtual bool loadCachedIndex(KListBox *index, DocumentationCatalogItem *item);
    
public slots:
    /**Creates index and fills index listbox. Reimplement this only if custom
    caching algorythm is used (do not forget to reimplement also @ref cacheIndex
    and @ref loadCachedIndex).*/
    virtual void createIndex(KListBox *index);
        
protected:
    /**A list of loaded documentation catalogs.*/
    QValueList<DocumentationCatalogItem*> catalogs;
    /**A map of names of loaded documentation catalogs.*/
    QMap<QString, DocumentationCatalogItem*> namedCatalogs;
    /**A map of indices of loaded documentation catalogs.*/
    QMap<DocumentationCatalogItem*, QValueList<IndexItem*> > indexes;
    
    /**Sets capabilities of documentation plugin.*/
    void setCapabilities(int caps) { m_capabilities = caps; }
    
    /**Stores items deleted from configuration. @ref saveCatalogConfiguration
    uses this to remove entries from configuration file.*/
    QStringList deletedConfigurationItems;
    
private:
    /**Adds catalog item to catalogs, namedCatalogs and indexes lists and maps.*/
    virtual void addCatalog(DocumentationCatalogItem *item);
    /**Removes catalog item from catalogs, namedCatalogs and indexes lists and maps.*/
    virtual void clearCatalog(DocumentationCatalogItem *item);
    
    int m_capabilities;
    bool m_indexCreated;
    
friend class IndexItem;
friend class DocumentationCatalogItem;
};

#endif
