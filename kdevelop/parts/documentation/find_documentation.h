/***************************************************************************
 *   Copyright (C) 2004 by Tobias Glaesser                                 *
 *   tobi.web@gmx.de                                                       *
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

#ifndef FINDDOCUMENTATION_H
#define FINDDOCUMENTATION_H

#include "find_documentationbase.h"

class DocumentationWidget;
class KListViewItem;
class FindDocumentationOptions;
class QProcess;

class FindDocumentation : public FindDocumentationBase
{
Q_OBJECT
public:
    FindDocumentation(DocumentationWidget* parent, const char* name = 0);
    ~FindDocumentation();
    /*$PUBLIC_FUNCTIONS$*/

public slots:
    /*$PUBLIC_SLOTS$*/
    virtual void buttonPressedOnItem( int button, QListViewItem * item, const QPoint & pos, int c );
    virtual void clickOnItem( QListViewItem * item );
    virtual void procInfoReadFromStdout();
    virtual void procManReadFromStdout();
    virtual void searchInInfo();
    virtual void searchInMan();
    virtual void searchInGoogle();
    virtual void searchInContents();
    virtual void searchInIndex();
    virtual void startSearch();
    virtual void setSearchTerm( const QString & term );
    virtual void clickOptions();

protected:
    /*$PROTECTED_FUNCTIONS$*/
    virtual void focusInEvent(QFocusEvent *e);

protected slots:
  /*$PROTECTED_SLOTS$*/

private:
    QProcess* proc_man;
    QProcess* proc_info;
    KListViewItem* man_item;
    KListViewItem* info_item;
    KListViewItem* index_item;
    KListViewItem* google_item;
    KListViewItem* contents_item;
    KListViewItem* last_item;
    DocumentationWidget* m_widget;
    FindDocumentationOptions* m_options;
    bool first_match_found;
    
};

#endif

