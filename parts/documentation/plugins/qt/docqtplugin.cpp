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
#include "docqtplugin.h"

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

#include <urlutil.h>
#include <kdevgenericfactory.h>

#include "../../../../config.h"

class QtDocumentationCatalogItem: public DocumentationCatalogItem
{
public:
    QtDocumentationCatalogItem(const QString &dcfFile, DocumentationPlugin* plugin,
        KListView *parent, const QString &name)
        :DocumentationCatalogItem(plugin, parent, name), m_dcfFile(dcfFile)
    {
    }
    QtDocumentationCatalogItem(const QString &dcfFile, DocumentationPlugin* plugin,
        DocumentationItem *parent, const QString &name)
        :DocumentationCatalogItem(plugin, parent, name), m_dcfFile(dcfFile)
    {
    }
    QString dcfFile() const { return m_dcfFile; }
    
private:
    QString m_dcfFile;
};

static const KAboutData data("docqtplugin", I18N_NOOP("Qt DCF documentation plugin"), "1.0");
typedef KDevGenericFactory<DocQtPlugin> DocQtPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdocqtplugin, DocQtPluginFactory(&data) )

DocQtPlugin::DocQtPlugin(QObject* parent, const char* name, const QStringList)
    :DocumentationPlugin(parent, name)
{
    config = DocQtPluginFactory::instance()->config();
    config->setGroup("General");
    
    if ( ! config->readBoolEntry("Autosetup", false) )
        autoSetup();
    
    setCapabilities(Index | FullTextSearch);
}

DocQtPlugin::~DocQtPlugin()
{
}

void DocQtPlugin::createTOC(DocumentationCatalogItem *item)
{
    QtDocumentationCatalogItem *qtItem = dynamic_cast<QtDocumentationCatalogItem *>(item);
    if (!qtItem)
        return;
    
    QFileInfo fi(qtItem->dcfFile());

    QFile f(qtItem->dcfFile());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << qtItem->dcfFile() << endl;
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "DCF")
    {
        kdDebug(9002) << "Not a valid DCF file: " << qtItem->dcfFile() << endl;
        return;
    }
    f.close();

    QDomElement docEl = doc.documentElement();
    QDomElement titleEl = docEl.namedItem("DCF").toElement();

    QDomElement childEl = docEl.lastChild().toElement();
    while (!childEl.isNull())
    {
        if (childEl.tagName() == "section")
        {
            QString ref = childEl.attribute("ref");
            QString title = childEl.attribute("title");

            DocumentationItem *sectionItem = new DocumentationItem(DocumentationItem::Book, item, title);
            KURL sectionUrl(fi.dirPath(true) + "/" + ref);
            sectionItem->setURL(sectionUrl);

            QDomElement grandChild = childEl.lastChild().toElement();
            while(!grandChild.isNull())
            {
                if (grandChild.tagName() == "keyword")
                {
                    QString keyRef = grandChild.attribute("ref");
                    QString keyTitle = grandChild.text();

                    DocumentationItem *keyItem = new DocumentationItem(DocumentationItem::Document, sectionItem, keyTitle);
                    KURL keyUrl(fi.dirPath(true) + "/" + keyRef);
                    keyItem->setURL(keyUrl);
                }
                grandChild = grandChild.previousSibling().toElement();
            }
        }
        childEl = childEl.previousSibling().toElement();
    }
}

void DocQtPlugin::init(KListView* contents)
{
    config->setGroup("Locations");
    QMap<QString, QString> entryMap = config->entryMap("Locations");
    
    for (QMap<QString, QString>::const_iterator it = entryMap.begin();
        it != entryMap.end(); ++it)
    {
        new QtDocumentationCatalogItem(config->readPathEntry(it.key()), this,
            contents, it.key());
    }
}

void DocQtPlugin::reinit(KListView *contents, KListBox *index, QStringList restrictions)
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
        if (restrictions.contains(it.key()))
        {
            if (namedCatalogs.contains(it.key()))
                delete namedCatalogs[it.key()];
        }
        else
        {
            if (!namedCatalogs.contains(it.key()))    //create catalog if it does not exist
            {
                QtDocumentationCatalogItem *item = new QtDocumentationCatalogItem(
                    config->readPathEntry(it.key()), this, contents, it.key());
                loadIndex(index, item);
            }
            else if (!indexEnabled(namedCatalogs[it.key()]))    //clear index if it is disabled in configuration
                clearCatalogIndex(namedCatalogs[it.key()]);
            else if ( (indexEnabled(namedCatalogs[it.key()]))    //index is requested in configuration but does not yet exist
                && (!indexes.contains(namedCatalogs[it.key()])) )
                 createIndex(index, namedCatalogs[it.key()]);
        }
    }
}

void DocQtPlugin::autoSetup()
{
    QString qtDocDir(QT_DOCDIR);
    qtDocDir = URLUtil::envExpand(qtDocDir);
    if (qtDocDir.isEmpty())
    {
        qtDocDir = getenv("QTDIR");
    }
    if (!qtDocDir.isEmpty())
    {
        config->setGroup("Locations");
        config->writePathEntry("Qt Reference Documentation", qtDocDir + QString("/qt.dcf"));
        config->writePathEntry("Qt Assistant Manual", qtDocDir + QString("/assistant.dcf"));
        config->writePathEntry("Qt Designer Manual", qtDocDir + QString("/designer.dcf"));
        config->writePathEntry("Guide to the Qt Translation Tools", qtDocDir + QString("/linguist.dcf"));
        config->writePathEntry("qmake User Guide", qtDocDir + QString("/qmake.dcf"));
    }
    
    config->setGroup("General");
    config->writeEntry("Autosetup", true);
    config->sync();
}

void DocQtPlugin::setCatalogURL(DocumentationCatalogItem *item)
{
    QtDocumentationCatalogItem *qtItem = dynamic_cast<QtDocumentationCatalogItem *>(item);
    if (!qtItem)
        return;
    
    QFileInfo fi(qtItem->dcfFile());

    QFile f(qtItem->dcfFile());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << qtItem->dcfFile() << endl;
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "DCF")
    {
        kdDebug(9002) << "Not a valid DCF file: " << qtItem->dcfFile() << endl;
        return;
    }
    f.close();

    QDomElement docEl = doc.documentElement();
    QDomElement titleEl = docEl.namedItem("DCF").toElement();

    if (item->url().isEmpty())
    {
        KURL url(fi.dirPath(true) + "/" + docEl.attribute("ref", QString::null));
        item->setURL(url);
    }
}

QString DocQtPlugin::pluginName() const
{
    return i18n("Qt Documentation Collection");
}

bool DocQtPlugin::needRefreshIndex(DocumentationCatalogItem *item)
{
    QtDocumentationCatalogItem *qtItem = dynamic_cast<QtDocumentationCatalogItem *>(item);
    if (!qtItem)
        return false;
    
    QFileInfo fi(qtItem->dcfFile());
    config->setGroup("Index");
    if (fi.lastModified() > config->readDateTimeEntry(qtItem->text(0), new QDateTime()))
    {
        kdDebug() << "need rebuild index for " << qtItem->text(0) << endl;
        config->writeEntry(item->text(0), fi.lastModified());
        return true;
    }
    else
        return false;
}

void DocQtPlugin::createIndex(KListBox *index, DocumentationCatalogItem *item) 
{
    QtDocumentationCatalogItem *qtItem = dynamic_cast<QtDocumentationCatalogItem *>(item);
    if (!qtItem)
        return;
    
    QFileInfo fi(qtItem->dcfFile());

    QFile f(qtItem->dcfFile());
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(9002) << "Could not read" << qtItem->dcfFile() << endl;
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "DCF")
    {
        kdDebug(9002) << "Not a valid DCF file: " << qtItem->dcfFile() << endl;
        return;
    }
    f.close();

    QDomElement docEl = doc.documentElement();
    QDomElement titleEl = docEl.namedItem("DCF").toElement();

    QDomElement childEl = docEl.lastChild().toElement();
    while (!childEl.isNull())
    {
        if (childEl.tagName() == "section")
        {
            //adymo: do not load section to index for Qt reference documentation
            if (fi.fileName() != "qt.dcf")
            {
                QString ref = childEl.attribute("ref");
                QString title = childEl.attribute("title");
                
                IndexItem *ii = new IndexItem(this, item, index, title);
                ii->addURL(KURL(fi.dirPath(true) + "/" + ref));
            }

            QDomElement grandChild = childEl.firstChild().toElement();
            while(!grandChild.isNull())
            {
                if (grandChild.tagName() == "keyword")
                {
                    QString keyRef = grandChild.attribute("ref");
                    QString keyTitle = grandChild.text();

                    IndexItem *ii = new IndexItem(this, item, index, keyTitle);
                    ii->addURL(KURL(fi.dirPath(true) + "/" + keyRef));
                }
                //adymo: for qt.dcf load only first keyword with class name, not method names
                //FIXME: make this configurable?
                if (fi.fileName() == "qt.dcf")
                    break;
                grandChild = grandChild.nextSibling().toElement();
            }
        }
        childEl = childEl.previousSibling().toElement();
    }
}

QStringList DocQtPlugin::fullTextSearchLocations()
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

void DocQtPlugin::loadCatalogConfiguration(KListView *configurationView)
{
    config->setGroup("Locations");
    QMap<QString, QString> entryMap = config->entryMap("Locations");

    for (QMap<QString, QString>::const_iterator it = entryMap.begin();
        it != entryMap.end(); ++it)
    {
        ConfigurationItem *item = new ConfigurationItem(configurationView, it.key(), it.data(),
            hasCapability(Index), hasCapability(FullTextSearch));
        config->setGroup("Index Settings");
        item->setIndex(config->readBoolEntry(item->title(), false));
        config->setGroup("Search Settings");
        item->setFullTextSearch(config->readBoolEntry(item->title(), false));
    }
}

void DocQtPlugin::saveCatalogConfiguration(KListView *configurationView)
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

QPair<KFile::Mode, QString> DocQtPlugin::catalogLocatorProps()
{
    return QPair<KFile::Mode, QString>(KFile::File, "*.xml *.dcf");
}

QString DocQtPlugin::catalogTitle(const QString &url)
{
    QFileInfo fi(url);
    if (!fi.exists())
        return QString::null;

    QFile f(url);
    if (!f.open(IO_ReadOnly))
        return QString::null;
    
    QDomDocument doc;
    if (!doc.setContent(&f) || (doc.doctype().name() != "DCF"))
        return QString::null;
    f.close();

    QDomElement docEl = doc.documentElement();

    return docEl.attribute("title", QString::null);
}

bool DocQtPlugin::indexEnabled(DocumentationCatalogItem *item) const
{
    config->setGroup("Index Settings");
    return config->readBoolEntry(item->text(0), false);
}

void DocQtPlugin::setIndexEnabled(DocumentationCatalogItem *item, bool e)
{
    config->setGroup("Index Settings");
    config->writeEntry(item->text(0), e);
}

#include "docqtplugin.moc"
