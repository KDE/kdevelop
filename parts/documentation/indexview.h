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
#ifndef INDEXVIEW_H
#define INDEXVIEW_H

#include <qwidget.h>

class KListBox;
class KLineEdit;
class QListBoxItem;
class DocumentationWidget;
class FindDocumentation;

class IndexView : public QWidget
{
    Q_OBJECT
public:
    IndexView(DocumentationWidget *parent = 0, const char *name = 0);
    ~IndexView();
    
    virtual bool eventFilter(QObject *watched, QEvent *e);
    KListBox *indexBox() const { return m_index; }

public slots:
    void searchInIndex();
    void setSearchTerm(const QString &term);
    void askSearchTerm();
    
protected slots:
    void searchInIndex(QListBoxItem *item);
    void showIndex(const QString &term);

    void itemMouseButtonPressed(int button, QListBoxItem *item, const QPoint &pos);

protected:
    virtual void focusInEvent(QFocusEvent *e);
    
private:
    DocumentationWidget *m_widget;
    
    KListBox *m_index;
    KLineEdit *m_edit;
    
friend class FindDocumentation;
};

#endif
