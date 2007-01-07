/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   Copyright (C) 2003 by Hamish Rodda                                    *
 *   meddie@yoyo.its.monash.edu.au                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "appoutputwidget.h"

#include <qregexp.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qclipboard.h>

#include <klocale.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <klineedit.h>
#include <kfiledialog.h>

#include "appoutputviewpart.h"
#include "filterdlg.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"

AppOutputWidget::AppOutputWidget(AppOutputViewPart* part)
    : ProcessWidget(0, "app output widget"), m_part(part)
{
	connect(this, SIGNAL(executed(QListBoxItem*)), SLOT(slotRowSelected(QListBoxItem*)));
	connect(this, SIGNAL(rightButtonClicked( QListBoxItem *, const QPoint & )), 
		SLOT(slotContextMenu( QListBoxItem *, const QPoint & )));
	KConfig *config = kapp->config();
	config->setGroup("General Options");
	setFont(config->readFontEntry("OutputViewFont"));
  setSelectionMode(QListBox::Extended);
}

void AppOutputWidget::clearViewAndContents()
{
	m_contentList.clear();
	clear();
}

AppOutputWidget::~AppOutputWidget()
{}


void AppOutputWidget::childFinished(bool normal, int status)
{
    ProcessWidget::childFinished(normal, status);
}


void AppOutputWidget::slotRowSelected(QListBoxItem* row)
{
	static QRegExp assertMatch("ASSERT: \\\"([^\\\"]+)\\\" in ([^\\( ]+) \\(([\\d]+)\\)");
	static QRegExp lineInfoMatch("\\[([^:]+):([\\d]+)\\]");
	static QRegExp rubyErrorMatch("([^:\\s]+\\.rb):([\\d]+):?.*$");

	if (row) {
		if (assertMatch.exactMatch(row->text())) {
			m_part->partController()->editDocument(KURL( assertMatch.cap(2) ), assertMatch.cap(3).toInt() - 1);
			m_part->mainWindow()->statusBar()->message(i18n("Assertion failed: %1").arg(assertMatch.cap(1)), 10000);
			m_part->mainWindow()->lowerView(this);

		} else if (lineInfoMatch.search(row->text()) != -1) {
			m_part->partController()->editDocument(KURL( lineInfoMatch.cap(1) ), lineInfoMatch.cap(2).toInt() - 1);
			m_part->mainWindow()->statusBar()->message(row->text(), 10000);
			m_part->mainWindow()->lowerView(this);
		} else if (rubyErrorMatch.search(row->text()) != -1) {
			QString file;
			if (rubyErrorMatch.cap(1).startsWith("/")) {
				file = rubyErrorMatch.cap(1);
			} else {
				file = m_part->project()->projectDirectory() + "/" + rubyErrorMatch.cap(1);
			}
			m_part->partController()->editDocument(KURL(rubyErrorMatch.cap(1)), rubyErrorMatch.cap(2).toInt() - 1);
			m_part->mainWindow()->statusBar()->message(row->text(), 10000);
			m_part->mainWindow()->lowerView(this);
		}
	}
}


void AppOutputWidget::insertStdoutLine(const QString &line)
{
// 	kdDebug(9004) << k_funcinfo << line << endl;

	if ( !m_part->isViewVisible() )
	{
		m_part->showView();
	}

	m_contentList.append(QString("o-")+line);
	if ( filterSingleLine( line ) )
	{
		ProcessWidget::insertStdoutLine(line);
	}
}


void AppOutputWidget::insertStderrLine(const QString &line)
{
// 	kdDebug(9004) << k_funcinfo << line << endl;

	if ( !m_part->isViewVisible() )
	{
		m_part->showView();
	}

	m_contentList.append(QString("e-")+line);
	if ( filterSingleLine( line ) )
	{
		ProcessWidget::insertStderrLine(line);
	}
}

void AppOutputWidget::editFilter()
{
	FilterDlg dlg( this );
	dlg.caseSensitive->setChecked( m_filter.m_caseSensitive );
	dlg.regularExpression->setChecked( m_filter.m_isRegExp );
	dlg.filterString->setText( m_filter.m_filterString );

	if ( dlg.exec() == QDialog::Accepted )
	{
		m_filter.m_caseSensitive = dlg.caseSensitive->isChecked();
		m_filter.m_isRegExp = dlg.regularExpression->isChecked();
		m_filter.m_filterString = dlg.filterString->text();

		m_filter.m_isActive = !m_filter.m_filterString.isEmpty();

		reinsertAndFilter();
	}
	
}
bool AppOutputWidget::filterSingleLine(const QString & line)
{
	if ( !m_filter.m_isActive ) return true;

	if ( m_filter.m_isRegExp )
	{
		return ( line.find( QRegExp( m_filter.m_filterString, m_filter.m_caseSensitive, false ) ) != -1 );
	}
	else
	{
		return ( line.find( m_filter.m_filterString, 0, m_filter.m_caseSensitive ) != -1 );
	}	
}

void AppOutputWidget::reinsertAndFilter()
{
	//copy the first item from the listbox
	//if a programm was started, this contains the issued command
	QString issuedCommand;
	if ( count() > 0 ) 
	{
		setTopItem(0);
		issuedCommand = item(topItem())->text();
	}

	clear();

	//write back the issued command
	if ( !issuedCommand.isEmpty() )
	{
		insertItem( new ProcessListBoxItem( issuedCommand, ProcessListBoxItem::Diagnostic ) );
	}

	//grep through the list for items matching the filter...
	QStringList strListFound;
	if ( m_filter.m_isActive )
	{	
		if ( m_filter.m_isRegExp )
		{
			strListFound = m_contentList.grep( QRegExp(m_filter.m_filterString, m_filter.m_caseSensitive, false ) );
		}
		else
		{
			strListFound = m_contentList.grep( m_filter.m_filterString, m_filter.m_caseSensitive );
		}
	}
	else
	{
		strListFound = m_contentList;
	}

	//... and reinsert the found items into the listbox
	for ( QStringList::Iterator it = strListFound.begin(); it != strListFound.end(); ++it ) 
	{
		if ((*it).startsWith("o-"))
		 {
			(*it).remove(0,2);
			insertItem(new ProcessListBoxItem(*it, ProcessListBoxItem::Normal));
		} 
		else if ((*it).startsWith("e-")) 
		{
			(*it).remove(0,2);
			insertItem(new ProcessListBoxItem(*it, ProcessListBoxItem::Error));
		}
	}
}

void AppOutputWidget::clearFilter()
{
	m_filter.m_isActive = false;
	reinsertAndFilter();
}

void AppOutputWidget::slotContextMenu( QListBoxItem *, const QPoint &p )
{
	KPopupMenu popup(this, "filter output");

	int id = popup.insertItem( i18n("Clear output"), this, SLOT(clearViewAndContents()) );
	popup.setItemEnabled( id, m_contentList.size() > 0 );

  popup.insertItem( i18n("Copy selected lines"), this, SLOT(copySelected()) );
  popup.insertSeparator();

	popup.insertItem( i18n("Save unfiltered"), this, SLOT(saveAll()) );
	id = popup.insertItem( i18n("Save filtered output"), this, SLOT(saveFiltered()) );
	popup.setItemEnabled( id, m_filter.m_isActive );
	popup.insertSeparator();

	id = popup.insertItem( i18n("Clear filter"), this, SLOT(clearFilter()) );
	popup.setItemEnabled( id, m_filter.m_isActive );

	popup.insertItem( i18n("Edit filter"), this, SLOT(editFilter() ) );

	popup.insertSeparator();
	popup.insertItem( i18n("Hide view"), this, SLOT(hideView()) );

	popup.exec(p);
}

void AppOutputWidget::hideView()
{
	m_part->hideView();
}

void AppOutputWidget::saveAll()
{
	saveOutputToFile( false );
}

void AppOutputWidget::saveFiltered()
{
	saveOutputToFile( true );
}

void AppOutputWidget::saveOutputToFile(bool useFilter)
{
	QString filename = KFileDialog::getSaveFileName();

	if ( filename.isEmpty() ) return;

	QStringList contents;
	if ( useFilter && m_filter.m_isActive )
	{	
		if ( m_filter.m_isRegExp )
		{
			contents = m_contentList.grep( QRegExp(m_filter.m_filterString, m_filter.m_caseSensitive, false ) );
		}
		else
		{
			contents = m_contentList.grep( m_filter.m_filterString, m_filter.m_caseSensitive );
		}
	}
	else
	{
		contents = m_contentList;
	}

	QFile file( filename );
	if ( file.open( IO_WriteOnly ) )
	{
		QTextStream ostream( &file );
		QStringList::ConstIterator it = contents.begin();
		while( it != contents.end() )
		{
			QString line = *it;
			if ( line.startsWith("o-") || line.startsWith("e-") )
			{
				line.remove( 0, 2 );
			}
			ostream << line << endl;
			++it;
		}
		file.close();
	}
}

void AppOutputWidget::copySelected()
{
  uint n = count();
  QString buffer;
  for (uint i = 0; i < n; i++)
  {
    if (isSelected(i))
      buffer += item(i)->text() + "\n";
  }
  kapp->clipboard()->setText(buffer, QClipboard::Clipboard);
}

#include "appoutputwidget.moc"
