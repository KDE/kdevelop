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
#ifndef __DOCUMENTATION_WIDGET_H__
#define __DOCUMENTATION_WIDGET_H__

#include <qwidget.h>

class KDevProject;
class DocumentationPart;
class ContentsView;
class IndexView;
class DocumentationCatalogItem;
class KTabWidget;
class KListBox;

class DocumentationWidget : public QWidget
{
    Q_OBJECT
public:  
    DocumentationWidget(DocumentationPart *part);
    ~DocumentationWidget();

    DocumentationPart *part() const { return m_part; }

    ContentsView *contents() const { return m_contents; }
    KListBox *index() const;

protected slots:
    void tabChanged(QWidget *w);

private:
    DocumentationPart *m_part;
    ContentsView *m_contents;
    IndexView *m_index;
    KTabWidget *m_tab;
};

#endif
