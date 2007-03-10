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
#include <klineedit.h>
#include <qguardedptr.h>
#include <qdatetime.h>
#include <map>
#include <ext/hash_map>
#include "hashedstring.h"

class CppSupportPart;
class QTimer;
class QTabBar;
class QWidgetStack;
class QGridLayout;
class KDialogBase;
class Problem;
class KURL;

class EfficientKListView {
public:
    typedef __gnu_cxx::hash_multimap<HashedString, QListViewItem*> Map;
    typedef std::pair< Map::const_iterator, Map::const_iterator > Range;
    EfficientKListView( KListView* list = 0 ) : m_list( list ), m_insertionNumber( 0 ) {
    }
    
    EfficientKListView& operator = ( KListView* list ) {
        m_list = list;
        return *this;
    }
    
    operator KListView* () {
        return m_list;
    }
    
    operator const KListView* () const {
        return m_list;
    }
    
    KListView* operator -> () {
        return m_list;
    }
    
    const KListView* operator -> () const {
        return m_list;
    }
    
    void addItem( const QString& str, QListViewItem* item ) {
        HashedString h( str );
        m_insertionNumbers[h] = ++m_insertionNumber;
        m_map.insert( std::make_pair( h, item ) );
    }
    
    Range getRange( const QString& str ) const {
        return m_map.equal_range( HashedString(str) );
    }

    ///If the list has more then size items, the first items are removed until the size fits.
    void limitSize( int size );
  
    void removeAllItems( const QString& str );
    
    bool hasItem( const QString& str ) const {
        Map::const_iterator it = m_map.find( HashedString(str) );
        return it != m_map.end();
    }
private:
    int m_insertionNumber;
    Map m_map;
    typedef __gnu_cxx::hash_map<HashedString, int> InsertionMap;
  InsertionMap m_insertionNumbers; //This is used to count which file was inserted first(higher insertion-number -> inserted later)
    KListView* m_list;
};

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
	bool hasErrors(const QString& file);

public slots:

private slots:
	void slotPartAdded( KParts::Part* );
	void slotActivePartChanged( KParts::Part* );
	void slotSelected( QListViewItem* );
	void slotTabSelected( int tabindex );
	void slotFilter();
	void initCurrentList();

private:
	QString levelToString( int level ) const;
	int levelToMarkType( int level ) const;
	void InitListView( KListView* listview );
	void filterList( KListView* listview, const QString& level );
    void updateCurrentWith( EfficientKListView& listview, const QString& level, const QString& filename );

private:
	QGridLayout* m_gridLayout;
	QTabBar* m_tabBar;
	QWidgetStack* m_widgetStack;
	KListView* m_currentList;
    QTimer* m_initCurrentTimer;
	EfficientKListView m_errorList;
	EfficientKListView m_fixmeList;
	EfficientKListView m_todoList;
    EfficientKListView m_warningList;
	KListView* m_filteredList;
	KLineEdit* m_filterEdit;

	CppSupportPart* m_cppSupport;
	KTextEditor::MarkInterface* m_markIface;
	QString m_fileName;
	
	///@todo move these to cppsupportpart
	int m_active;
	int m_delay;
};

#endif 
// kate: indent-mode csands; tab-width 4;
