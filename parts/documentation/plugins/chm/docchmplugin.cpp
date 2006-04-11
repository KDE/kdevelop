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
#include <kio/netaccess.h>
#include <iostream>
#include <fstream>
#include <qvaluevector.h>
#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>
#include <qregexp.h>

#include "../../../../config.h"

static const KDevPluginInfo data("docchmplugin");
typedef KDevGenericFactory<DocCHMPlugin> DocCHMPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libdocchmplugin, DocCHMPluginFactory(data) )

DocCHMPlugin::DocCHMPlugin(QObject* parent, const char* name, QStringList // args
                           )
    :DocumentationPlugin(DocCHMPluginFactory::instance()->config(), parent, name)
{
    setCapabilities(CustomDocumentationTitles); // | Index | FullTextSearch | ProjectDocumentation
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


QString DocCHMPlugin::getSpecialData(const QString& name, KURL url) {
    QString ret = "";
    url.setProtocol("ms-its");
    
    url.addPath("/:" + name);
    QString tmpFile;
    if( KIO::NetAccess::download( url, tmpFile, 0 ) )
    {
        std::filebuf fb;
        fb.open (tmpFile.ascii(), std::ios::in);
        std::istream is(&fb);
        char buf[5000] = " ";
        while(is.good()) {
            is.getline(buf, 5000);
            ret += buf; ret += "\n";
        }
        fb.close();
        KIO::NetAccess::removeTempFile( tmpFile );
    } else {
        kdDebug(9002) << "DocCHMPlugin::getSpecialData could not download data from " << url.url() << endl;
    }
    return ret;
}


///this currently is useless, because htdig cannot use kioslaves
QStringList DocCHMPlugin::fullTextSearchLocations()
{
    //return QStringList::split( '\n', getSpecialData("contents", m_url));
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


static KListViewItem* chainEnd(KListViewItem *parent) {
    if(parent == 0) return 0;
    KListViewItem* ret = dynamic_cast<KListViewItem*>(parent->firstChild());
    if(ret == 0) return 0;
    while(ret->nextSibling() != 0) {
        ret = dynamic_cast<KListViewItem*>(ret->nextSibling());
    }
    return ret;
}

static QString decodeHTML(const QString& s) {
    QRegExp rx(QString::fromLatin1("&#(\\d+);|&nbsp;"));
    QString out = s;
    int pos = rx.search(out);
    while(pos > -1) {
        QString found = rx.cap(0);
        if(found != "&nbsp;") {
            out.replace(pos, found.length(), static_cast<char>(rx.cap(1).toInt()));
        }else{
            out.replace(pos, found.length(), " ");
        }
        pos = rx.search(out, pos+1);
    }
    return out;
}


void DocCHMPlugin::createTOC(DocumentationCatalogItem* item)
{
    QStringList lines = QStringList::split("\n", getSpecialData("catalog", item->url()) );
    if(lines.count() % 4 != 0) { kdDebug(9002) << "DocCHMPlugin::createTOC: wrong count of strings"; return;}
    
    QValueVector<DocumentationItem*> items;
    items.push_back(item);
    for(QStringList::Iterator it = lines.begin(); it != lines.end();) {
        bool ok1 = true, ok2 = true;
        int parent = (*it).toInt(&ok1);
        ++it;
        int current = (*it).toInt(&ok2);
        ++it;
        if(int(items.size()) != current || !ok1 || !ok2 || parent < 0 || parent >= int(items.size()) || current < 0 || current != int(items.size())) {
            kdDebug(9002) << "DocCHMPlugin::createTOC error while parsing output of ioslave" << endl;
            break;
        }
        
        QString& name(*it);
        ++it;
        KURL url(*it);
        ++it;
        
        items.push_back(new DocumentationItem(
                DocumentationItem::Document, items[parent], chainEnd(items[parent]), decodeHTML(name)));
        items[current]->setURL(url);
        if(parent != 0) items[parent]->setType(DocumentationItem::Book);
    }
    

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
