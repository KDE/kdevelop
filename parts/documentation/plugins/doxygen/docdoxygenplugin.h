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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef DOCDOXYGENPLUGIN_H
#define DOCDOXYGENPLUGIN_H

#include <kdevdocumentationplugin.h>

class QDomDocument;
class QDomElement;

class DocDoxygenPlugin: public DocumentationPlugin
{
    Q_OBJECT
public:
    DocDoxygenPlugin(QObject* parent, const char* name, const QStringList args = QStringList());
    ~DocDoxygenPlugin();

    virtual QString pluginName() const;

    virtual DocumentationCatalogItem *createCatalog(KListView *contents, const QString &title, const QString &url);
    
    virtual void createTOC(DocumentationCatalogItem* item);
    virtual void setCatalogURL(DocumentationCatalogItem* item);

    virtual bool needRefreshIndex(DocumentationCatalogItem* item);
    virtual void createIndex(IndexBox* index, DocumentationCatalogItem* item);

    virtual QStringList fullTextSearchLocations();
                    
    virtual QPair<KFile::Mode, QString> catalogLocatorProps();
    virtual QString catalogTitle(const QString& url);
    
    virtual void autoSetupPlugin();

    virtual ProjectDocumentationPlugin *projectDocumentationPlugin(ProjectDocType type);
        
protected:
    void createBookTOC(DocumentationItem* item, const QString &tagUrl = QString::null, 
        const QString &baseHtmlUrl = QString::null);
    void createBookIndex(const QString &tagfile, IndexBox* index, DocumentationCatalogItem* item, const QString &baseHtmlUrl = QString::null);
    
    void createIndexFromTag(QDomDocument &dom, IndexBox* index, DocumentationCatalogItem* item,
        QDomElement &parentEl, const QString &prefix);
    
    void autoSetupDocs(const QString &defaultDir, const QString &searchDir, 
        const QString &name);
    
};


#endif
