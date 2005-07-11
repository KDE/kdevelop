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
#include <kdevplugininfo.h>

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

static const KDevPluginInfo data("docqtplugin");
typedef KDevGenericFactory<DocQtPlugin> DocQtPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdocqtplugin, DocQtPluginFactory(data) )

DocQtPlugin::DocQtPlugin(QObject* parent, const char* name, const QStringList)
    :DocumentationPlugin(DocQtPluginFactory::instance()->config(), parent, name)
{
    setCapabilities(Index | FullTextSearch);
    autoSetup();
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
                if (grandChild.tagName() == "section")
                {
                    QString keyRef = grandChild.attribute("ref");
                    QString keyTitle = grandChild.attribute("title");
                    DocumentationItem *keyItem = new DocumentationItem(DocumentationItem::Book, sectionItem, keyTitle);
                    KURL keyUrl(fi.dirPath(true) + "/" + keyRef);
                    keyItem->setURL(keyUrl);
                }
                grandChild = grandChild.previousSibling().toElement();
            }
        }
        childEl = childEl.previousSibling().toElement();
    }
}

void DocQtPlugin::autoSetupPlugin()
{
    QString qtDocDir(QT_DOCDIR);
    qtDocDir = URLUtil::envExpand(qtDocDir);
    if (qtDocDir.isEmpty())
    {
        qtDocDir = getenv("QTDIR");
    }
    if (!qtDocDir.isEmpty())
    {
        config->setGroup("Search Settings");
        config->writeEntry("Qt Reference Documentation", true);
        config->setGroup("Index Settings");
        config->writeEntry("Qt Reference Documentation", true);
        config->setGroup("Locations");
        config->writePathEntry("Qt Reference Documentation", qtDocDir + QString("/qt.dcf"));
        config->writePathEntry("Qt Assistant Manual", qtDocDir + QString("/assistant.dcf"));
        config->writePathEntry("Qt Designer Manual", qtDocDir + QString("/designer.dcf"));
        config->writePathEntry("Guide to the Qt Translation Tools", qtDocDir + QString("/linguist.dcf"));
        config->writePathEntry("qmake User Guide", qtDocDir + QString("/qmake.dcf"));
    }
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

void DocQtPlugin::createIndex(IndexBox *index, DocumentationCatalogItem *item)
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

    QDomElement childEl = docEl.firstChild().toElement();
    while (!childEl.isNull())
    {
        if (childEl.tagName() == "section")
        {
            createSectionIndex(fi, index, item, childEl);
        }
        childEl = childEl.nextSibling().toElement();
    }
}

void DocQtPlugin::createSectionIndex(QFileInfo &fi, IndexBox *index, DocumentationCatalogItem *item,
                                     QDomElement section)
{
    //adymo: do not load section to index for Qt reference documentation
    QString title = section.attribute("title");
    if (fi.fileName() != "qt.dcf")
    {
        QString ref = section.attribute("ref");

        IndexItemProto *ii = new IndexItemProto(this, item, index, title, item->text(0));
        ii->addURL(KURL(fi.dirPath(true) + "/" + ref));
    }

    QDomElement grandChild = section.firstChild().toElement();
    while(!grandChild.isNull())
    {
        if (grandChild.tagName() == "keyword")
        {
            QString keyRef = grandChild.attribute("ref");
            QString keyTitle = grandChild.text();

                    //adymo: a little hack to avoid unwanted elements
            if (keyRef != "qdir-example.html")
            {
                IndexItemProto *ii = new IndexItemProto(this, item, index, keyTitle, title);
                ii->addURL(KURL(fi.dirPath(true) + "/" + keyRef));
            }
        }
        if (grandChild.tagName() == "section")
        {
            createSectionIndex(fi, index, item, grandChild);
        }
        grandChild = grandChild.nextSibling().toElement();
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

DocumentationCatalogItem *DocQtPlugin::createCatalog(KListView *contents, const QString &title, const QString &url)
{
    return new QtDocumentationCatalogItem(url, this, contents, title);
}

#include "docqtplugin.moc"
