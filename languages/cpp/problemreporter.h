/*
  Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  version 2, License as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#ifndef PROBLEMSREPORTER_H
#define PROBLEMSREPORTER_H

#include <klistview.h>
#include <qguardedptr.h>
#include <qdatetime.h>

class CppSupportPart;
class QTimer;
class QTabBar;
class QWidgetStack;
class QGridLayout;
class QLineEdit;
class KDialogBase;
class Problem;
class KURL;

namespace KParts
{
	class Part;
}

namespace KTextEditor
{
	class MarkInterface;
	class Document;
}

class ProblemReporter: public QWidget
{
	Q_OBJECT
public:
	ProblemReporter( CppSupportPart* part, QWidget* parent = 0, const char* name = 0 );
	virtual ~ProblemReporter();

	void removeAllProblems( const QString& filename );
	void reportProblem( const QString& fileName, const Problem& p );

public slots:
	void reparse();
	void configure();
	void configWidget( KDialogBase* );

private slots:
	void slotPartAdded( KParts::Part* );
	void slotPartRemoved( KParts::Part* );
	void slotActivePartChanged( KParts::Part* );
	void slotTextChanged();
	void slotSelected( QListViewItem* );
	void slotFileParsed( const QString& fileName );
	void slotTabSelected( int tabindex );
	void slotFilter();
	void closedFile( const KURL &fileName );
	void slotParseCheck();

private:
	QString levelToString( int level ) const;
	int levelToMarkType( int level ) const;
	void InitListView( KListView* listview );
	void removeAllItems( QListView* listview, const QString& filename );
	void filterList( KListView* listview, const QString& level );
	void updateCurrentWith( QListView* listview, const QString& level, const QString& filename );
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

	CppSupportPart* m_cppSupport;
	QGuardedPtr<KTextEditor::Document> m_document;
	KTextEditor::MarkInterface* m_markIface;
	QTimer* m_timer;
	QTimer* m_parseCheckTimeout;
	QTime m_timeout;
	QString m_fileName;
	int m_active;
	int m_delay;
	int m_canParseFile;
};

#endif 
// kate: indent-mode csands; tab-width 4;
