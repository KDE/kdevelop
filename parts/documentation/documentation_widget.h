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

class FindDocumentation;
class KDevProject;
class DocumentationPart;
class ContentsView;
class IndexView;
class DocumentationCatalogItem;
class QToolBox;
class KListBox;
class SearchView;
class BookmarkView;
class KListView;

class DocumentationWidget : public QWidget
{
    Q_OBJECT
public:
    DocumentationWidget(DocumentationPart *part);
    ~DocumentationWidget();

    DocumentationPart *part() const { return m_part; }

    KListView *contents() const;
    KListBox *index() const;

public slots:
    void lookInDocumentationIndex();
    void lookInDocumentationIndex(const QString &term);
    void searchInDocumentation();
    void searchInDocumentation(const QString &term);
    void findInDocumentation(const QString &term = "");
    
protected:
    virtual void focusInEvent(QFocusEvent *e);

protected slots:
    void tabChanged(int t);

private:
    DocumentationPart *m_part;
    ContentsView *m_contents;
    IndexView *m_index;
    SearchView *m_search;
    BookmarkView *m_bookmarks;
    FindDocumentation *m_finder;
    QToolBox *m_tab;

friend class FindDocumentation;
};

#endif
