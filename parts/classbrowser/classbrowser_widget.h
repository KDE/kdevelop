/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef CLASSBROWSER_WIDGET_H
#define CLASSBROWSER_WIDGET_H

#include <klistview.h>
#include <qmap.h>

class ClassBrowserPart;
class KComboBox;
class Catalog;
class KDevLanguageSupport;

class ClassBrowserWidget : public KListView
{
    Q_OBJECT
public:
    ClassBrowserWidget( ClassBrowserPart* part );
    virtual ~ClassBrowserWidget();

    void init();

    KDevLanguageSupport* languageSupport();

public slots:
    void addCatalog( Catalog* catalog );
    void removeCatalog( Catalog* catalog );

private slots:
    void slotStartSearch();
    void slotCatalogChanged( Catalog* catalog );
    void slotItemExecuted( QListViewItem* item );

private:
    ClassBrowserPart* m_part;
    QMap<Catalog*, QListViewItem*> m_items;
};

#endif
