/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann and the KDevelop Team       *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "makewidget.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "processlinemaker.h"
#include "makeviewpart.h"
#include "makeitem.h"

#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kprocess.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kstatusbar.h>
#include <kapplication.h>
#include <kconfig.h>

#include <qmessagebox.h>
#include <qapplication.h>
#include <qdir.h>
#include <qimage.h>
#include <qstylesheet.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <qclipboard.h>
#include <qpopupmenu.h>
#include <qstylesheet.h>
#include <private/qrichtext_p.h>

static const char *const error_xpm[] =
    {
        "11 11 5 1",
        ". c None",
        "# c #313062",
        "a c #6261cd",
        "b c #c50000",
        "c c #ff8583",
        "...........",
        "...####....",
        ".a#bbbb#a..",
        ".#ccbbbb#..",
        "#bccbbbbb#.",
        "#bbbbbbbb#.",
        "#bbbbbbcb#.",
        "#bbbbbccb#.",
        ".#bbbccb#..",
        ".a#bbbb#a..",
        "...####...."
    };


static const char *const warning_xpm[] =
    {
        "11 11 5 1",
        ". c None",
        "# c #313062",
        "a c #6261cd",
        "b c #c5c600",
        "c c #ffff41",
        "...........",
        "...####....",
        ".a#bbbb#a..",
        ".#ccbbbb#..",
        "#bccbbbbb#.",
        "#bbbbbbbb#.",
        "#bbbbbbcb#.",
        "#bbbbbccb#.",
        ".#bbbccb#..",
        ".a#bbbb#a..",
        "...####...."
    };


static const char *const message_xpm[] =
    {
        "11 11 5 1",
        ". c None",
        "b c #3100c5",
        "# c #313062",
        "c c #3189ff",
        "a c #6265cd",
        "...........",
        "...####....",
        ".a#bbbb#a..",
        ".#ccbbbb#..",
        "#bccbbbbb#.",
        "#bbbbbbbb#.",
        "#bbbbbbcb#.",
        "#bbbbbccb#.",
        ".#bbbccb#..",
        ".a#bbbb#a..",
        "...####...."
    };


MakeWidget::MakeWidget(MakeViewPart *part)
	: QTextEdit(0, "make widget")
	, m_directoryStatusFilter( m_errorFilter )
	, m_errorFilter( m_continuationFilter )
	, m_continuationFilter( m_actionFilter )
	, m_actionFilter( m_otherFilter )
	, moved(false)
	, m_part(part)
	, m_vertScrolling(false)
	, m_horizScrolling(false)
	, m_bCompiling(false)
{
	updateSettingsFromConfig();

	if ( m_bLineWrapping )
		setWordWrap(WidgetWidth);
	setWrapPolicy(Anywhere);
	setReadOnly(true);
	setMimeSourceFactory(new QMimeSourceFactory);
	mimeSourceFactory()->setImage("error", QImage((const char**)error_xpm));
	mimeSourceFactory()->setImage("warning", QImage((const char**)warning_xpm));
	mimeSourceFactory()->setImage("message", QImage((const char**)message_xpm));

	childproc = new KShellProcess("/bin/sh");
	procLineMaker = new ProcessLineMaker( childproc );

	connect( procLineMaker, SIGNAL(receivedStdoutLine(const QString&)),
	         this, SLOT(insertStdoutLine(const QString&) ));
	connect( procLineMaker, SIGNAL(receivedStderrLine(const QString&)),
	         this, SLOT(insertStderrLine(const QString&) ));

	connect( childproc, SIGNAL(processExited(KProcess*)),
	         this, SLOT(slotProcessExited(KProcess*) )) ;

	connect( &m_directoryStatusFilter, SIGNAL(item(EnteringDirectoryItem*)),
	         this, SLOT(slotEnteredDirectory(EnteringDirectoryItem*)) );
	connect( &m_directoryStatusFilter, SIGNAL(item(ExitingDirectoryItem*)),
	         this, SLOT(slotExitedDirectory(ExitingDirectoryItem*)) );
	connect( &m_errorFilter, SIGNAL(item(MakeItem*)),
	         this, SLOT(insertItem(MakeItem*)) );
	connect( &m_actionFilter, SIGNAL(item(MakeItem*)),
	         this, SLOT(insertItem(MakeItem*)) );
	connect( &m_otherFilter, SIGNAL(item(MakeItem*)),
	         this, SLOT(insertItem(MakeItem*)) );

	connect( verticalScrollBar(), SIGNAL(sliderPressed()),
	         this, SLOT(verticScrollingOn()) );
	connect( verticalScrollBar(), SIGNAL(sliderReleased()),
	         this, SLOT(verticScrollingOff()) );
	connect( horizontalScrollBar(), SIGNAL(sliderPressed()),
	         this, SLOT(horizScrollingOn()) );
	connect( horizontalScrollBar(), SIGNAL(sliderReleased()),
	         this, SLOT(horizScrollingOff()) );
}

MakeWidget::~MakeWidget()
{
	delete mimeSourceFactory();
	delete childproc;
	delete procLineMaker;
}

void MakeWidget::queueJob(const QString &dir, const QString &command)
{
	commandList.append(command);
	dirList.append(dir);
	if (!isRunning())
		startNextJob();
}

void MakeWidget::startNextJob()
{
	QStringList::Iterator it = commandList.begin();
	if ( it == commandList.end() )
		return;

	currentCommand = *it;
	commandList.remove(it);

	int i = currentCommand.findRev(" gmake");
	if ( i == -1 )
		i = currentCommand.findRev(" make");
	if ( i == -1 )
		m_bCompiling = false;
	else
	{
		QString s = currentCommand.right(currentCommand.length() - i);
		m_bCompiling =
			s.contains("configure ")        ||
			s.contains(" Makefile.cvs")     ||
			s.contains(" clean")            ||
			s.contains(" distclean")        ||
			s.contains(" package-messages") ||
			s.contains(" install");
	}

	it = dirList.begin();
	QString dir = *it;
	dirList.remove(it);

	clear(); // clear the widget
	m_items.clear(); // FIXME: memory leak
	moved = false;

	insertItem( new CommandItem( currentCommand ) );

	childproc->clearArguments();
	*childproc << currentCommand;
	childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);

	dirstack.clear();
	dirstack.push(new QString(dir));

	m_part->mainWindow()->raiseView(this);
	m_part->core()->running(m_part, true);
}

void MakeWidget::killJob()
{
	childproc->kill();
}

bool MakeWidget::isRunning()
{
	return childproc->isRunning();
}

void MakeWidget::copy()
{
	int parafrom=0, indexfrom=0, parato=0, indexto=0;
	getSelection(&parafrom, &indexfrom, &parato, &indexto);
	if(parafrom < 0 || indexfrom < 0 || parato < 0 || indexto < 0)
	QTextEdit::copy();

	QString selection;
	for(int i = parafrom; i<=parato; i++)
		selection += text(i) + "\n";

	selection.remove(0, indexfrom);
	int removeend = text(parato).length() - indexto;

	selection.remove((selection.length()-1) - removeend, removeend);

#if QT_VERSION >= 0x030100
	kapp->clipboard()->setText(selection, QClipboard::Clipboard);
#else
	kapp->clipboard()->setText(selection);
#endif
}

void MakeWidget::nextError()
{
	int parag, index;
	if (moved)
		getCursorPosition(&parag, &index);
	else
		parag = 0;

	for ( int it = parag; it < m_items.count(); ++it )
	{
		ErrorItem* item = dynamic_cast<ErrorItem*>( m_items[it] );
		if ( !item )
			continue;
		moved = true;
		parag = it;
		document()->removeSelection(0);
		setSelection(parag, 0, parag+1, 0, 0);
		setCursorPosition(parag, 0);
		ensureCursorVisible();
		m_part->partController()->editDocument(item->fileName, item->lineNum);
		m_part->mainWindow()->statusBar()->message( item->m_error, 10000 );
		m_part->mainWindow()->lowerView(this);
		return;
	}

	KNotifyClient::beep();
}

void MakeWidget::prevError()
{
	int parag, index;
	if (moved)
		getCursorPosition(&parag, &index);
	else
		parag = 0;

	for ( int it = parag; it >= 0; --it)
	{
		ErrorItem* item = dynamic_cast<ErrorItem*>( m_items[it] );
		if ( !item )
			continue;
		moved = true;
		parag = it;
		document()->removeSelection(0);
		setSelection(parag, 0, parag+1, 0, 0);
		setCursorPosition(parag, 0);
		ensureCursorVisible();
		m_part->partController()->editDocument(item->fileName, item->lineNum);
		m_part->mainWindow()->statusBar()->message( item->m_error, 10000 );
		m_part->mainWindow()->lowerView(this);
		return;
	}

	KNotifyClient::beep();
}

void MakeWidget::contentsMousePressEvent(QMouseEvent *e)
{
	QTextEdit::contentsMousePressEvent(e);
	if ( e->button() != LeftButton )
		return;
	int parag, index;
	getCursorPosition(&parag, &index);
	searchItem(parag);
}

void MakeWidget::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Key_Return || e->key() == Key_Enter)
	{
		int parag, index;
		getCursorPosition(&parag, &index);
		searchItem(parag);
	}
	else
		QTextEdit::keyPressEvent(e);
}

void MakeWidget::searchItem(int parag)
{
	ErrorItem* item = dynamic_cast<ErrorItem*>( m_items[parag] );
	if ( item )
	{
		m_part->partController()->editDocument(item->fileName, item->lineNum);
		m_part->mainWindow()->statusBar()->message( item->m_error, 10000 );
		m_part->mainWindow()->lowerView(this);
	}
}

void MakeWidget::insertStdoutLine( const QString& line )
{
	if ( !appendToLastLine( line ) )
		m_directoryStatusFilter.processLine( line );
}

void MakeWidget::insertStderrLine( const QString& line )
{
	if ( !appendToLastLine( line ) )
		m_errorFilter.processLine( line );
}

bool MakeWidget::appendToLastLine( const QString& text )
{
	if ( m_items.count() == 0 )
		return false;
	MakeItem* item = m_items[m_items.count() - 1];
	if ( !item->append( text ) )
		return false;
	removeParagraph( paragraphs() - 1 );
	append( item->formattedText( m_compilerOutputLevel, brightBg() ) );
	return true;
}

void MakeWidget::slotProcessExited(KProcess *)
{
	if (childproc->normalExit())
	{
		if (childproc->exitStatus())
		{
			KNotifyClient::event( "ProcessError", i18n("The process has finished with errors"));
		}
		else
		{
			KNotifyClient::event( "ProcessSuccess", i18n("The process has finished successfully"));
			emit m_part->commandFinished(currentCommand);
		}
	}

	MakeItem* item = new ExitStatusItem( childproc->normalExit(), childproc->exitStatus() );
	insertItem( item );

	m_part->mainWindow()->statusBar()->message( QString("%1: %2").arg(currentCommand).arg(item->m_text), 3000);
	m_part->core()->running(m_part, false);

	// Defensive programming: We emit this with a single shot timer so that we go once again
	// through the event loop. After that, we can be sure that the process is really finished
	// and its KProcess object can be reused.
	if (childproc->normalExit())
		QTimer::singleShot(0, this, SLOT(startNextJob()));
	else
	{
		commandList.clear();
		dirList.clear();
	}
}

void MakeWidget::slotEnteredDirectory( EnteringDirectoryItem* item )
{
	kdDebug(9004) << "Entering dir: " << item->directory << endl;
	QString* dir = new QString( item->directory );
	dirstack.push( dir );
	insertItem( item );
}

void MakeWidget::slotExitedDirectory( ExitingDirectoryItem* item )
{
	QString eDir = item->directory;
	kdDebug(9004) << "Leaving dir: " << eDir << endl;
	QString *dir = dirstack.pop();
	if (!dir)
	{
		kdWarning(9004) << "Left more directories than entered: " << eDir;
	}
	else if (dir->compare(eDir) != 0)
	{
		kdWarning(9004) << "Expected directory: \"" << *dir << "\" but got \"" << eDir << "\"" << endl;
	}
	insertItem( item );
	delete dir;
}

void MakeWidget::insertItem( MakeItem* item )
{
	m_items.push_back( item );

	int para, index;
	getCursorPosition( &para, &index );

	bool atEnd = para == paragraphs() - 1 && index == paragraphLength( para );

	int paraFrom, indexFrom, paraTo, indexTo;
	getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo, 0);

	append( item->formattedText( m_compilerOutputLevel, brightBg() ) );

	setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);

	if (atEnd && !m_vertScrolling && !m_horizScrolling)
	{
		moveCursor(MoveEnd, false);
		moveCursor(MoveLineStart, false);//if linewrap is off we must avoid the jumping of the vertical scrollbar
	}
}

bool MakeWidget::brightBg()
{
	int h,s,v;
	paletteBackgroundColor().hsv( &h, &s, &v );
	return (v > 127);
}

QPopupMenu* MakeWidget::createPopupMenu( const QPoint& pos )
{
	QPopupMenu* pMenu = QTextEdit::createPopupMenu(pos);
	pMenu->setCheckable(true);

	pMenu->insertSeparator();
	int id = pMenu->insertItem(i18n("Line wrapping"), this, SLOT(toggleLineWrapping()) );
	pMenu->setItemChecked(id, m_bLineWrapping);

	pMenu->insertSeparator();
	id = pMenu->insertItem(i18n("Very short compiler output"), this, SLOT(slotVeryShortCompilerOutput()) );
	pMenu->setItemChecked(id, m_compilerOutputLevel == eVeryShort);
	id = pMenu->insertItem(i18n("Short compiler output"), this, SLOT(slotShortCompilerOutput()) );
	pMenu->setItemChecked(id, m_compilerOutputLevel == eShort);
	id = pMenu->insertItem(i18n("Full compiler output"), this, SLOT(slotFullCompilerOutput()) );
	pMenu->setItemChecked(id, m_compilerOutputLevel == eFull);

	pMenu->insertSeparator();
	id = pMenu->insertItem(i18n("Show directory navigation messages"), this, SLOT(toggleShowDirNavigMessages()));
	pMenu->setItemChecked(id, DirectoryItem::getShowDirectoryMessages());

	return pMenu;
}

void MakeWidget::toggleLineWrapping()
{
	m_bLineWrapping = !m_bLineWrapping;
	KConfig *pConfig = kapp->config();
	pConfig->setGroup("MakeOutputView");
	pConfig->writeEntry("LineWrapping", m_bLineWrapping);
	pConfig->sync();
	if (m_bLineWrapping) {
		setWordWrap(WidgetWidth);
	}
	else
	{
		setWordWrap(NoWrap);
	}
}

void MakeWidget::slotVeryShortCompilerOutput() { setCompilerOutputLevel(eVeryShort); }
void MakeWidget::slotShortCompilerOutput() { setCompilerOutputLevel(eShort); }
void MakeWidget::slotFullCompilerOutput() { setCompilerOutputLevel(eFull); }

void MakeWidget::setCompilerOutputLevel(EOutputLevel level)
{
	m_compilerOutputLevel = level;
	KConfig *pConfig = kapp->config();
	pConfig->setGroup("MakeOutputView");
	pConfig->writeEntry("CompilerOutputLevel", (int) level);
	pConfig->sync();
	clear();
	for( uint i = 0; i < m_items.size(); i++ )
	{
		append( m_items[i]->formattedText( m_compilerOutputLevel, brightBg() ) );
	}
}

void MakeWidget::toggleShowDirNavigMessages()
{
	DirectoryItem::setShowDirectoryMessages( !DirectoryItem::getShowDirectoryMessages() );
	KConfig *pConfig = kapp->config();
	pConfig->setGroup("MakeOutputView");
	pConfig->writeEntry("ShowDirNavigMsg", DirectoryItem::getShowDirectoryMessages());
	pConfig->sync();
}

void MakeWidget::updateSettingsFromConfig()
{
	KConfig *pConfig = kapp->config();
	pConfig->setGroup("MakeOutputView");
	setFont(pConfig->readFontEntry("Messages Font"));
	m_bLineWrapping = pConfig->readBoolEntry("LineWrapping", true);
	m_compilerOutputLevel = (EOutputLevel) pConfig->readNumEntry("CompilerOutputLevel", (int) eVeryShort);
	DirectoryItem::setShowDirectoryMessages( pConfig->readBoolEntry("ShowDirNavigMsg", false) );
}

#include "makewidget.moc"
