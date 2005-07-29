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
#ifndef DOCCHMPLUGIN_H
#define DOCCHMPLUGIN_H

#include <kdevdocumentationplugin.h>

class DocCHMPlugin :public DocumentationPlugin
{
    Q_OBJECT
public:
    DocCHMPlugin(QObject* parent, const char* name, QStringList args = QStringList());
    ~DocCHMPlugin();
    
    virtual QString pluginName() const;

    virtual void setCatalogURL(DocumentationCatalogItem* item);
    virtual QString catalogTitle(const QString& url);
    virtual QPair<KFile::Mode, QString> catalogLocatorProps();
    virtual DocumentationCatalogItem* createCatalog(KListView* contents, const QString& title, const QString& url);
    virtual void createTOC(DocumentationCatalogItem* item);
    
    virtual void createIndex(IndexBox* index, DocumentationCatalogItem* item);
    virtual bool needRefreshIndex(DocumentationCatalogItem* item);
    
    virtual QStringList fullTextSearchLocations();
    
    virtual void autoSetupPlugin();
};

#endif
