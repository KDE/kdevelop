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

#define CACHE_VERSION "2"

class DocumentationItem: public KListViewItem
{
public:
    enum Type { Collection, Catalog, Book, Document };

    DocumentationItem(Type type, KListView *parent, const QString &name);
    DocumentationItem(Type type, KListView *parent, KListViewItem *after, const QString &name);
    DocumentationItem(Type type, KListViewItem *parent, const QString &name);
    DocumentationItem(Type type, KListViewItem *parent, KListViewItem *after, const QString &name);

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
    void load();
    
    virtual bool isProjectDocumentationItem() const { return m_isProjectDocumentationItem; }
    virtual void setProjectDocumentationItem(bool b) { m_isProjectDocumentationItem = b; }
    
protected:
    virtual void activate();
    
private:
    DocumentationPlugin* m_plugin;
    bool isLoaded;
    bool isActivated;
    bool m_isProjectDocumentationItem;
};

class IndexBox;

class IndexItemProto
{
public:
    IndexItemProto(DocumentationPlugin *plugin, DocumentationCatalogItem *catalog, IndexBox *listbox,
        const QString &text, const QString &description);
    ~IndexItemProto();

    void addURL(const KURL &url) { m_url = url; }
    KURL url() const { return m_url; }
    QString text() const { return m_text; }
    QString description() const { return m_description; }
    
private:
    KURL m_url;
    IndexBox *m_listbox;
    QString m_text;
    QString m_description;
};

class IndexItem: public QListBoxText {
public:
    typedef QPair<QString, KURL> URL;
    typedef QValueList<URL> List;
    
    IndexItem(IndexBox *listbox, const QString &text);

    List urls() const;

private:
    IndexBox *m_listbox;
};

class IndexBox: public KListBox{
public:
    IndexBox(QWidget *parent = 0, const char *name = 0);
    
    virtual void addIndexItem(IndexItemProto *item);
    virtual void removeIndexItem(IndexItemProto *item);
    virtual void fill();
    virtual void refill();
    virtual void setDirty(bool dirty);
//    virtual void refill(QValueList<IndexItemProto*> &items);
    
private:
    QMap<QString, QValueList<IndexItemProto*> > items;
    friend class IndexItem;
    
    bool m_dirty;
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
    virtual void setTitle(const QString title) { setText(3, m_title = title); }
    virtual QString url() const { return m_url; }
    virtual void setURL(const QString url) { setText(4, m_url = url); }

    virtual bool isChanged() const { return m_title == m_origTitle; }
    virtual QString origTitle() const {return m_origTitle; }

    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
    virtual int width(const QFontMetrics &fm, const QListView *lv, int c) const;

    bool contents() const { return m_contents; }
    void setContents(bool contents) { m_contents = contents; }
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

    bool m_contents;
    bool m_index;
    bool m_fullTextSearch;
    
    bool m_indexPossible;
    bool m_fullTextSearchPossible;
};


class ProjectDocumentationPlugin;

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
                      CustomDocumentationTitles=4 /**<user can specify titles for documentation catalogs*/,
                      ProjectDocumentation=8 /**<plugin can handle project API documentation*/,
                      ProjectUserManual=16 /**<plugin can handle project user manual*/ };
    /**Possible project documentation types.*/
    enum ProjectDocType { APIDocs, UserManual };
    
    /**Constructor. Should initialize capabilities of the plugin by using setCapabilities
    protected method. For example,
    @code
    setCapabilities(Index | FullTextSearch);
    @endcode
    */
    DocumentationPlugin(KConfig *pluginConfig, QObject *parent =0, const char *name =0);
    virtual ~DocumentationPlugin();
    
    /**Returns the i18n name of the plugin.*/
    virtual QString pluginName() const = 0;
    
    /**Creates documentation catalog with given title and url.*/
    virtual DocumentationCatalogItem *createCatalog(KListView *contents, const QString &title, const QString &url) = 0;
    
    /**Initialize a list of catalogs.
    @param contents the listview to fill with catalogs
    */
    virtual void init(KListView *contents);
    /**Reloads a list of catalogs. This method should add missing catalogs to the view,
    update index for added catalogs and also delete restricted catalogs.
    @param contents the listview to fill with catalogs
    @param index the listbox with index to update
    @param restrictions the list of catalogs names to remove
    */
    virtual void reinit(KListView *contents, IndexBox *index, QStringList restrictions);
    /**Initializes plugin configuration. Documentation plugins should be able to
    initialize the default configuration on startup without any user interaction.
    Call this in the constructor of your plugin.*/
    virtual void autoSetup();
    /**Plugin specific automatic setup code. This method is called by @ref autoSetup.*/
    virtual void autoSetupPlugin() = 0;

    /**Indicates if a catalog with specified name is enabled. Documentation plugin
    should check this and do not load catalogs disabled in configuration.
    All catalogs are enabled by default.*/
    virtual bool catalogEnabled(const QString &name) const;
    /**Enables or disables documentation catalog.*/
    virtual void setCatalogEnabled(const QString &name, bool e);
    
    /**Indicates if an index of given catalog should be rebuilt. This method
    is used by index caching algorythm to make a descision to rebuild index
    or to load it from cache.*/
    virtual bool needRefreshIndex(DocumentationCatalogItem *item) = 0;
    /**Indicates if an index is enabled for given catalog. If documentation plugin
    has Index capability, indices for it's catalogs can be enabled/disabled
    in configuration dialog.*/
    virtual bool indexEnabled(DocumentationCatalogItem *item) const;
    /**Enables or disables index for documentation catalog.*/
    virtual void setIndexEnabled(DocumentationCatalogItem *item, bool e);
    /**Builds index for given catalog. This method should fill index with
    IndexItem objects.
    @param index the listbox which contains index items
    */
    virtual void createIndex(IndexBox *index, DocumentationCatalogItem *item) = 0;

    /**Creates a table of contents for given catalog. Documentation part uses
    lazy loading of toc's to reduce startup time. This means that createTOC
    will be called on expand event of catalog item.*/
    virtual void createTOC(DocumentationCatalogItem *item) = 0;
    /**Sets the URL to the catalog. This method will be called each time user
    clicks the documentation item. If it is too much overhead to determine the
    documentation catalog url in @ref createCatalog method then you can set it here.*/
    virtual void setCatalogURL(DocumentationCatalogItem *item) = 0;
    virtual QStringList fullTextSearchLocations() = 0;

    /**Loads catalog configuration and fills configurationView with ConfigurationItem objects.*/
    virtual void loadCatalogConfiguration(KListView *configurationView);
    /**Saves catalog configuration basing on configurationView and 
    deletedConfigurationItems contents. If you use KConfig to store configuration,
    it is important that you call KConfig::sync() method after saving.*/
    virtual void saveCatalogConfiguration(KListView *configurationView);
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
    virtual bool loadCachedIndex(IndexBox *index, DocumentationCatalogItem *item);
    
    /**Returns associated project documentation plugin. Default implementation returns zero.
    Reimplement this if the documentation plugin can also handle project documentation.*/
    virtual ProjectDocumentationPlugin *projectDocumentationPlugin(ProjectDocType type) { return 0; }
    
public slots:
    /**Creates index and fills index listbox. Reimplement this only if custom
    caching algorythm is used (do not forget to reimplement also @ref cacheIndex
    and @ref loadCachedIndex).*/
    virtual void createIndex(IndexBox *index);
        
protected:
    /**A list of loaded documentation catalogs.*/
    QValueList<DocumentationCatalogItem*> catalogs;
    /**A map of names of loaded documentation catalogs.*/
    QMap<QString, DocumentationCatalogItem*> namedCatalogs;
    /**A map of indices of loaded documentation catalogs.*/
    QMap<DocumentationCatalogItem*, QValueList<IndexItemProto*> > indexes;

    /**Sets capabilities of documentation plugin.*/
    void setCapabilities(int caps) { m_capabilities = caps; }
    /**Clears index of given catalog.*/
    virtual void clearCatalogIndex(DocumentationCatalogItem *item);
    /**Loads index from cache or creates and caches it if does not exist.*/
    void loadIndex(IndexBox *index, DocumentationCatalogItem *item);
    
    /**Stores items deleted from configuration. @ref saveCatalogConfiguration
    uses this to remove entries from configuration file.*/
    QStringList deletedConfigurationItems;

    /**Configuration object used by a plugin.*/
    KConfig *config;
        
private:
    /**Adds catalog item to catalogs, namedCatalogs and indexes lists and maps.*/
    virtual void addCatalog(DocumentationCatalogItem *item);
    /**Removes catalog item from catalogs, namedCatalogs and indexes lists and maps.*/
    virtual void clearCatalog(DocumentationCatalogItem *item);
    
    int m_capabilities;
    bool m_indexCreated;
    

friend class IndexItemProto;
friend class DocumentationCatalogItem;
};


/**
@short Project documentation plugin

Represents functionality to display project documentation catalog and index in documentation browser.
*/
class ProjectDocumentationPlugin: public QObject {
    Q_OBJECT
public:
    ProjectDocumentationPlugin(DocumentationPlugin *docPlugin, DocumentationPlugin::ProjectDocType type);
    virtual ~ProjectDocumentationPlugin();

    /**Initializes project documentation plugin - creates documentation catalog.*/
    virtual void init(KListView *contents, IndexBox *index, const QString &url);
    /**Deinitializes project documentation plugin - removes documentation catalog.*/
    virtual void deinit();
    
    QString pluginName() const;
    QString catalogURL() const;

public slots:
    /**Performs reinitialization if project documentation has changed (after building api documentation).*/
    virtual void reinit();
        
protected:
    DocumentationPlugin *m_docPlugin;
    DocumentationCatalogItem *m_catalog;
    
private:
    DocumentationPlugin::ProjectDocType m_type;

    class KDirWatch *m_watch;
    class KListView *m_contents;
    class IndexBox *m_index;
    QString m_url;
};

#endif
