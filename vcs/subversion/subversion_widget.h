/* Copyright (C) 2003
	 Mickael Marchand <marchand@kde.org>

	 This program is free software; you can redistribute it and/or
	 modify it under the terms of the GNU General Public
	 License as published by the Free Software Foundation; either
	 version 2 of the License, or (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; see the file COPYING.  If not, write to
	 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	 Boston, MA 02111-1307, USA.
	 */

#ifndef __SUBVERSION_WIDGET_H__
#define __SUBVERSION_WIDGET_H__

#include <qlistview.h>
#include "svn_blamewidget.h"
#include "svn_logviewwidget.h"
#include <qvaluelist.h>

class subversionPart;
#include <ktabwidget.h>
#include <kio/defaultprogress.h>
#include <qguardedptr.h>
class KTextEdit;
class SvnLogHolder;
class SvnBlameHolder;
class SvnLogViewWidget;
class QToolButton;
class QPushButton;

/** The main Subversion DockWidget. Contains logview-output, blame-output, status and etc */
// class subversionWidget : public SvnOutputWidgetBase
class subversionWidget : public KTabWidget
{
    Q_OBJECT
public:
    subversionWidget(subversionPart *part, QWidget *parent, const char* name);
    ~subversionWidget();

    // append what?. Append any text status outputs
    void append( QString notifications );
    void showLogResult( QValueList<SvnLogHolder> *holderList, QString reqUrl );
    void showBlameResult( QValueList<SvnBlameHolder> *blamelist );

protected slots:
    void closeCurrentTab();

private:
    KTabWidget* tab(){ return this; }
	subversionPart *m_part;
    
    QGuardedPtr<KTextEdit> m_edit;
    QPushButton *m_closeButton;

};
/**
 * reimplement compare(), to be able to sort any item by integer
 */
class SvnIntSortListItem : public QListViewItem {
public:
	SvnIntSortListItem ( QListView* parent=0 );
	~SvnIntSortListItem ();
	/** Returns < 0 if this item is less than i, 0 if they are equal and > 0 if this item is greater than i. 
	 */
	virtual int compare( QListViewItem* i, int col, bool ascending ) const;
};

class SvnLogViewItem : public SvnIntSortListItem {
    public:
        SvnLogViewItem( QListView* parent );
        ~SvnLogViewItem();

        QString m_pathList;
        QString m_message;
};

/////////////////////////////////////////////////////////////
/// Subversion Progress Display Widget
class SvnProgressDlg : public KIO::DefaultProgress {
public:
	SvnProgressDlg( bool showNow = true );
	~SvnProgressDlg();
	void setSourceUrl( const QString & );
	void setDestUrl( const QString & );
};

#endif
