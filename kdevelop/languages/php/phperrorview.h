/*
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2005 by Nicolas Escuder <n.escuder@intra-links.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PHPERRORVIEW_H
#define PHPERRORVIEW_H

#include <klistview.h>
#include <qguardedptr.h>
#include "phpfile.h"

class PHPSupportPart;
class QTimer;
class QTabBar;
class QWidgetStack;
class QGridLayout;
class QLineEdit;
class KDialogBase;
class Problem;
class KURL;

namespace KParts{
    class Part;
}

namespace KTextEditor{
    class MarkInterface;
    class Document;
}


enum Errors
{
   Error = 0,
   ErrorParse = 1,
   ErrorNoSuchFunction = 2,
   Warning = 3,
   Todo = 4,
   Fixme = 5,
};

class PHPErrorView: public QWidget {
Q_OBJECT
public:
   PHPErrorView( PHPSupportPart* part, QWidget* parent=0, const char* name=0 );
   virtual ~PHPErrorView();

   void removeAllProblems( const QString& filename );
   void reportProblem( int level,  const QString& fileName, int line, const QString& text);

private slots:
   void slotPartAdded( KParts::Part* );
   void slotPartRemoved( KParts::Part* );
   void slotActivePartChanged( KParts::Part* );
   void slotSelected( QListViewItem* );
   void slotTabSelected( int tabindex );
   void slotFilter();

private:
   QString levelToString( int level ) const;
   int levelToMarkType( int level ) const;
   void InitListView(KListView* listview);
   void removeAllItems( QListView* listview, const QString& filename );
   void filterList(KListView* listview, const QString& level);
   void updateCurrentWith(QListView* listview, const QString& level, const QString& filename);
   void initCurrentList();

private:
   QGridLayout* m_gridLayout;
   QTabBar* m_tabBar;
   QWidgetStack* m_widgetStack;
   KListView* m_currentList;
   KListView* m_errorList;
   KListView* m_fixmeList;
   KListView* m_warningList;
   KListView* m_todoList;
   KListView* m_filteredList;
   QLineEdit* m_filterEdit;

   PHPSupportPart* m_phpSupport;
   QGuardedPtr<KTextEditor::Document> m_document;
   KTextEditor::MarkInterface* m_markIface;
   QTimer* m_timer;
   QString m_fileName;
   int m_active;
   int m_delay;
};

#endif
