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
#ifndef DOCQTPLUGIN_H
#define DOCQTPLUGIN_H

#include <kdevdocumentationplugin.h>

class KConfig;

class DocQtPlugin : public DocumentationPlugin
{
    Q_OBJECT
public:
    DocQtPlugin(QObject* parent, const char* name, const QStringList args = QStringList());
    ~DocQtPlugin();

    virtual QString pluginName() const;
    
    virtual void init(KListView* contents);
    virtual void reinit(KListView *contents, KListBox *index, QStringList restrictions);
    virtual void createTOC(DocumentationCatalogItem *item);
    virtual void setCatalogURL(DocumentationCatalogItem *item);    

    virtual bool catalogEnabled(const QString &name) const;
    virtual void setCatalogEnabled(const QString &name, bool e);
        
    virtual bool needRefreshIndex(DocumentationCatalogItem *item);
    virtual void createIndex(KListBox *index, DocumentationCatalogItem *item);
    virtual bool indexEnabled(DocumentationCatalogItem *item) const;
    virtual void setIndexEnabled(DocumentationCatalogItem *item, bool e);

    virtual void loadCatalogConfiguration(KListView *configurationView);
    virtual void saveCatalogConfiguration(KListView *configurationView);
    
    virtual QStringList fullTextSearchLocations();

    virtual QPair<KFile::Mode, QString> catalogLocatorProps();
    virtual QString catalogTitle(const QString &url);
        
protected:
    void autoSetup();

private:
    KConfig *config;
};

#endif
