/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qpainter.h>
#include <qtoolbutton.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kprocess.h>
#include <kparts/part.h>
#include <ktexteditor/selectioninterface.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <ktabwidget.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kmessagebox.h>
using namespace KTextEditor;

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"

#include "grepdlg.h"
#include "grepviewpart.h"
#include "grepviewwidget.h"


class GrepListBoxItem : public ProcessListBoxItem
{
public:
	GrepListBoxItem(const QString &fileName, const QString &lineNumber, const QString &text, bool showFilename);
	QString filename()
	{ return fileName; }
	int linenumber()
	{ return lineNumber.toInt(); }
	virtual bool isCustomItem();

private:
	virtual void paint(QPainter *p);
	QString fileName, lineNumber, text;
	bool show;
};


GrepListBoxItem::GrepListBoxItem(const QString &fileName, const QString &lineNumber, const QString &text, bool showFilename)
		: ProcessListBoxItem( QString::null, Normal),
		fileName(fileName), lineNumber(lineNumber), text(text.stripWhiteSpace()),
		show(showFilename)
{}


bool GrepListBoxItem::isCustomItem()
{
	return true;
}


void GrepListBoxItem::paint(QPainter *p)
{
	QFontMetrics fm = p->fontMetrics();
	QString stx = lineNumber + ":  ";
	int y = fm.ascent()+fm.leading()/2;
	int x = 3;
	if (show)
	{
		p->setPen(Qt::darkGreen);
		p->drawText(x, y, fileName);
		x += fm.width(fileName);
	}
	else
	{
		p->setPen(Qt::black);
		QFont font1(p->font());
		QFont font2(font1);
		font2.setBold(true);
		p->setFont(font2);
		p->drawText(x, y, stx);
		p->setFont(font1);
		x += fm.width(stx);

		p->setPen(Qt::blue);
		p->drawText(x, y, text);
	}
}


GrepViewWidget::GrepViewWidget(GrepViewPart *part) :  QWidget(0, "grepview widget")
{
	m_layout = new QHBoxLayout(this, 0, -1, "greplayout");

	m_tabWidget = new KTabWidget(this);

	m_layout->add(m_tabWidget);

	m_curOutput = new GrepViewProcessWidget(m_tabWidget);

	m_tabWidget->addTab(m_curOutput, i18n("Search Results"));

	grepdlg = new GrepDialog( part, this, "grep widget");
	connect( grepdlg, SIGNAL(searchClicked()), this, SLOT(searchActivated()) );
	connect( m_curOutput, SIGNAL(processExited(KProcess* )), this, SLOT(slotSearchProcessExited()) );

	connect( m_tabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotOutputTabChanged()) );

	connect( m_curOutput, SIGNAL(clicked(QListBoxItem*)),
	         this, SLOT(slotExecuted(QListBoxItem*)) );
	connect( m_curOutput, SIGNAL(returnPressed(QListBoxItem*)),
	         this, SLOT(slotExecuted(QListBoxItem*)) );


	connect( m_curOutput, SIGNAL(contextMenuRequested( QListBoxItem*, const QPoint&)), this, SLOT(popupMenu(QListBoxItem*, const QPoint&)));
	m_part = part;

	m_closeButton = new QToolButton(m_tabWidget);//@todo change text/icon
	m_closeButton->setIconSet(SmallIconSet("tab_remove"));
	m_closeButton->setEnabled(false);

	connect (m_closeButton, SIGNAL(clicked()), this, SLOT(slotCloseCurrentOutput()));
	m_tabWidget->setCornerWidget(m_closeButton);
}


GrepViewWidget::~GrepViewWidget()
{}


void GrepViewWidget::showDialog()
{
	// Get the selected text if there is any
	KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(m_part->partController()->activePart());
	if (ro_part)
	{
		SelectionInterface *selectIface = dynamic_cast<SelectionInterface*>(ro_part);
		if(selectIface && selectIface->hasSelection())
		{
			QString selText = selectIface->selection();
			if(!selText.contains('\n'))
			{
				grepdlg->setPattern(selText);
			}
		}
	}
	grepdlg->show();
}

// @todo - put this somewhere common - or just use Qt if possible
static QString escape(const QString &str)
{
	QString escaped("[]{}()\\^$?.+-*|");
	QString res;

	for (uint i=0; i < str.length(); ++i)
	{
		if (escaped.find(str[i]) != -1)
			res += "\\";
		res += str[i];
	}

	return res;
}


void GrepViewWidget::showDialogWithPattern(QString pattern)
{
	// Before anything, this removes line feeds from the
	// beginning and the end.
	int len = pattern.length();
	if (len > 0 && pattern[0] == '\n')
	{
		pattern.remove(0, 1);
		len--;
	}
	if (len > 0 && pattern[len-1] == '\n')
		pattern.truncate(len-1);
	grepdlg->setPattern( pattern );
	grepdlg->show();
}


void GrepViewWidget::searchActivated()
{
	if ( grepdlg->keepOutputFlag() )
		slotKeepOutput();
		
	m_tabWidget->showPage( m_curOutput );

	m_curOutput->setLastFileName("");
	m_curOutput->setMatchCount( 0 );

	QString files;
	// waba: code below breaks on filenames containing a ',' !!!
	QStringList filelist = QStringList::split(",", grepdlg->filesString());
	if (!filelist.isEmpty())
	{
		QStringList::Iterator it(filelist.begin());
		files = KShellProcess::quote(*it);
		++it;
		for (; it != filelist.end(); ++it)
			files += " -o -name " + KShellProcess::quote(*it);
	}

	m_lastPattern = grepdlg->patternString();
	QString pattern = grepdlg->templateString();
	if (grepdlg->regexpFlag())
	    pattern.replace(QRegExp("%s"), grepdlg->patternString());
        else
	    pattern.replace(QRegExp("%s"), escape( grepdlg->patternString() ) );

	QString filepattern = "find ";
	filepattern += KShellProcess::quote(grepdlg->directoryString());
	if (!grepdlg->recursiveFlag())
		filepattern += " -maxdepth 1";
	filepattern += " \\( -name ";
	filepattern += files;
	filepattern += " \\) -print -follow";

	QString command = filepattern + " " ;
	if (grepdlg->ignoreSCMDirsFlag())
	{
		command += "| grep -v -e \"SCCS/\" ";
		command += "-e \"CVS/\" ";
		command += "-e \"\\.svn/\" ";
		command += "-e \"_darcs/\" ";
	}

	// quote spaces in filenames going to xargs
	command += "| sed \"s/ /\\\\\\ /g\" ";

	command += "| xargs " ;

#ifndef USE_SOLARIS
	command += "egrep -H -n ";
	if (!grepdlg->caseSensitiveFlag())
	{
		command += "-i ";
	}
	command += "-e ";
#else
	// -H reported as not being available on Solaris,
	// but we're buggy without it on Linux.
	command += "egrep -n ";
	if (!grepdlg->caseSensitiveFlag())
	{
		command += "-i ";
	}
	command += "-e ";
#endif

	command += KShellProcess::quote(pattern);
	m_curOutput->startJob("", command);

	m_part->mainWindow()->raiseView(this);
	m_part->core()->running(m_part, true);
}


void GrepViewWidget::slotExecuted(QListBoxItem* item)
{
	ProcessListBoxItem *i = static_cast<ProcessListBoxItem*>(item);
	if (!i || !i->isCustomItem())
		return;

	GrepListBoxItem *gi = static_cast<GrepListBoxItem*>(i);
	m_part->partController()->editDocument( KURL( gi->filename() ), gi->linenumber()-1 );
}


void GrepViewProcessWidget::insertStdoutLine(const QString &line)
{
	int pos;
	QString filename, linenumber, rest;

	QString str = line;
	if ( (pos = str.find(':')) != -1)
	{
		filename = str.left(pos);
		str.remove( 0, pos+1 );
		if ( ( pos = str.find(':') ) != -1)
		{
			linenumber = str.left(pos);
			str.remove( 0, pos+1 );
			// filename will be displayed only once
			// selecting filename will display line 1 of file,
			// otherwise, line of requested search
			if ( _lastfilename != filename )
			{
				_lastfilename = filename;
				insertItem(new GrepListBoxItem(filename, "0", str, true));
				insertItem(new GrepListBoxItem(filename, linenumber, str, false));
			}
			else
			{
				insertItem(new GrepListBoxItem(filename, linenumber, str, false));
			}
			maybeScrollToBottom();
		}
		m_matchCount++;
	}
}


void GrepViewWidget::projectChanged(KDevProject *project)
{
	QString dir = project? project->projectDirectory() : QDir::homeDirPath();
	grepdlg->setDirectory(dir);
}

void GrepViewWidget::popupMenu(QListBoxItem*, const QPoint& p)
{
	if(m_curOutput->isRunning()) return;

	KPopupMenu rmbMenu;
	
	if(KAction *findAction = m_part->actionCollection()->action("edit_grep"))
	{
		rmbMenu.insertTitle(i18n("Find in Files"));
		findAction->plug(&rmbMenu);
		rmbMenu.exec(p);
	}
}

void GrepViewWidget::slotKeepOutput( )
{
	if ( m_lastPattern == QString::null ) return;
	
	m_tabWidget->changeTab(m_curOutput, m_lastPattern);

	m_curOutput = new GrepViewProcessWidget(m_tabWidget);
	m_tabWidget->insertTab(m_curOutput, i18n("Search Results"), 0);

	connect( m_curOutput, SIGNAL(clicked(QListBoxItem*)), this, SLOT(slotExecuted(QListBoxItem*)) );
	connect( m_curOutput, SIGNAL(returnPressed(QListBoxItem*)), this, SLOT(slotExecuted(QListBoxItem*)) );
	connect( m_curOutput, SIGNAL(processExited(KProcess* )), this, SLOT(slotSearchProcessExited()) );
	connect( m_curOutput, SIGNAL(contextMenuRequested( QListBoxItem*, const QPoint&)), this, SLOT(popupMenu(QListBoxItem*, const QPoint&)));
}

void GrepViewWidget::slotCloseCurrentOutput( )
{
	ProcessWidget* pw = static_cast<ProcessWidget*>(m_tabWidget->currentPage());
	if (pw == m_curOutput)
		return;

	m_tabWidget->removePage(pw);
	delete pw;

	if (m_tabWidget->count() == 1)
		m_closeButton->setEnabled( false );
}

void GrepViewWidget::killJob( int signo )
{
	m_curOutput->killJob( signo );
}

bool GrepViewWidget::isRunning( ) const
{
	return m_curOutput->isRunning();
}

void GrepViewWidget::slotOutputTabChanged( )
{
	ProcessWidget* pw = static_cast<ProcessWidget*>(m_tabWidget->currentPage());
	if (pw == m_curOutput)
		m_closeButton->setEnabled( false );
	else
		m_closeButton->setEnabled( true );
}

void GrepViewWidget::slotSearchProcessExited( )
{
	m_part->core()->running(m_part, false);
}

void GrepViewProcessWidget::childFinished( bool normal, int status )
{
	// When xargs executes grep several times (because the command line
	// generated would be too large for a single grep) and one of those
	// sets of files passed to grep does not contain a match, then an
	// error status of 123 is set by xargs, even if there is a match in
	// another set of files.
	// Reset this false status here.

	if (status == 123 && numRows() > 1)
		status = 0;

	insertItem(new ProcessListBoxItem(i18n("*** %n match found. ***", "*** %n matches found. ***", m_matchCount), ProcessListBoxItem::Diagnostic));
	maybeScrollToBottom();

	ProcessWidget::childFinished(normal, status);
}


#include "grepviewwidget.moc"
