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
#ifndef DOCGLOBALCONFIGWIDGET_H
#define DOCGLOBALCONFIGWIDGET_H

#include "docglobalconfigwidgetbase.h"

#include <qmap.h>

class DocumentationPart;
class DocumentationWidget;
class DocumentationPlugin;
class KListView;

class DocGlobalConfigWidget : public DocGlobalConfigWidgetBase
{
    Q_OBJECT
public:
    DocGlobalConfigWidget(DocumentationPart *part, DocumentationWidget *widget,
        QWidget *parent = 0, const char *name = 0, WFlags fl = 0);
    ~DocGlobalConfigWidget();
    /*$PUBLIC_FUNCTIONS$*/

public slots:
    /*$PUBLIC_SLOTS$*/
    virtual void collectionsBoxCurrentChanged(int box);
    
    virtual void removeCollectionButtonClicked();
    virtual void editCollectionButtonClicked();
    virtual void addCollectionButtonClicked();
    
    virtual void accept();

protected:
    /*$PROTECTED_FUNCTIONS$*/
    DocumentationPlugin *activePlugin();
    KListView *activeView();
    
protected slots:
    /*$PROTECTED_SLOTS$*/

private:
    DocumentationPart *m_part;
    DocumentationWidget *m_widget;
    
    QMap<int, DocumentationPlugin*> m_pluginBoxes;
    QMap<int, KListView*> m_pluginViews;
};

#endif

