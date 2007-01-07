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
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "processlinemaker.h"
#include "makeviewpart.h"
#include "makeitem.h"
#include "ktexteditor/document.h"
#include "ktexteditor/cursorinterface.h"
#include "ktexteditor/editinterface.h"
#include "urlutil.h"

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
#include <private/qrichtext_p.h>

#include <stdlib.h>
#include <limits.h>

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


class SelectionPreserver
{
public:
	SelectionPreserver( QTextEdit& textEdit, bool stayAtEnd )
		: m_textEdit( textEdit )
		, m_atEnd( false )
	{
		int para, index;
		m_textEdit.getCursorPosition( &para, &index );

		m_atEnd = stayAtEnd
		      && para == m_textEdit.paragraphs() - 1
		      && index == m_textEdit.paragraphLength( para );

		m_textEdit.getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo, 0);
	}

	~SelectionPreserver()
	{
		m_textEdit.setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);

		if ( m_atEnd )
		{
			m_textEdit.moveCursor(QTextEdit::MoveEnd, false);
			m_textEdit.moveCursor(QTextEdit::MoveLineStart, false);//if linewrap is off we must avoid the jumping of the vertical scrollbar
		}
	}

	QTextEdit& m_textEdit;
	bool m_atEnd;
	int paraFrom, indexFrom, paraTo, indexTo;
};

MakeWidget::MakeWidget(MakeViewPart *part)
	: QTextEdit(0, "make widget")
	, m_directoryStatusFilter( m_errorFilter )
	, m_errorFilter( m_continuationFilter )
	, m_continuationFilter( m_actionFilter )
	, m_actionFilter( m_otherFilter )
  , m_pendingItem(0)
	, m_paragraphs(0)
  , m_lastErrorSelected(-1)
	, m_part(part)
	, m_vertScrolling(false)
	, m_horizScrolling(false)
	, m_bCompiling(false)
{
	updateSettingsFromConfig();

	setTextFormat( Qt::RichText );
	
	if ( m_bLineWrapping )
		setWordWrap(WidgetWidth);
	else
		setWordWrap(NoWrap);

	setWrapPolicy(Anywhere);
	setReadOnly(true);
	setMimeSourceFactory(new QMimeSourceFactory);
	mimeSourceFactory()->setImage("error", QImage((const char**)error_xpm));
	mimeSourceFactory()->setImage("warning", QImage((const char**)warning_xpm));
	mimeSourceFactory()->setImage("message", QImage((const char**)message_xpm));

	childproc = new KProcess(this);
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

	connect( m_part->partController(), SIGNAL(loadedFile(const KURL&)),
	         this, SLOT(slotDocumentOpened(const KURL&)) );
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
	{
		// Store the current output view so that it
		// can be restored after a successful compilation
// 		m_part->mainWindow()->storeOutputViewTab();
		startNextJob();
	}
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
		if ( s.contains("configure ")        ||
		     s.contains(" Makefile.cvs")     ||
		     s.contains(" clean")            ||
		     s.contains(" distclean")        ||
		     s.contains(" package-messages") ||
		     s.contains(" install") )
		{
		    m_bCompiling = false;
		}
		else {
		    m_bCompiling = true;
		}
	}

	it = dirList.begin();
	QString dir = *it;
	m_lastBuildDir = dir;
	dirList.remove(it);

	clear(); // clear the widget
	for ( QValueVector<MakeItem*>::iterator it = m_items.begin(); it != m_items.end(); ++it )
		delete *it;
	m_items.clear();
	m_paragraphToItem.clear();
	m_paragraphs = 0;
	m_lastErrorSelected = -1;

	insertItem( new CommandItem( currentCommand ) );

	childproc->clearArguments();
	*childproc << currentCommand;
	childproc->setUseShell(true);
	childproc->start(KProcess::OwnGroup, KProcess::AllOutput);

	dirstack.clear();
	dirstack.push(new QString(dir));

	m_part->mainWindow()->raiseView(this);
	m_part->core()->running(m_part, true);
}

void MakeWidget::killJob()
{
	if (!childproc->kill(SIGINT))
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
    if( parafrom < 0 || indexfrom < 0 || parato < 0 || indexto < 0
    || ((parafrom == parato) && (indexfrom == indexto)) )
    {
        return;
    }

	QString selection;
	for(int i = parafrom; i<=parato; i++)
	   selection += text(i) + "\n";

   	   
	if(m_compilerOutputLevel == eShort || 
	   m_compilerOutputLevel == eVeryShort )
	{
	   QRegExp regexp("<.*>");
	   regexp.setMinimal(true);
	   selection.remove(regexp);
	}
	else
	{ //FIXME: Selections should be precise in the eShort and eVeryShort modes, too.
	  selection.remove(0, indexfrom);
	  int removeend = text(parato).length() - indexto;

	  selection.remove((selection.length()-1) -  removeend, removeend);	   
	}		
	   
	selection.replace("&lt;","<");
	selection.replace("&gt;",">");
	selection.replace("&quot;","\"");
	selection.replace("&amp;","&");	
	   
	kapp->clipboard()->setText(selection, QClipboard::Clipboard);
}

void MakeWidget::nextError()
{
	int parag;
	if (m_lastErrorSelected != -1)
		parag = m_lastErrorSelected;
	else
		parag = 0;

    //if there are no errors after m_lastErrorSelected try again from the beginning
    if (!scanErrorForward(parag))
        if (m_lastErrorSelected != -1)
        {
            m_lastErrorSelected = -1;
            if (!scanErrorForward(0))
                KNotifyClient::beep();
        }
        else
            KNotifyClient::beep();
}

void MakeWidget::prevError()
{
	int parag;
	if (m_lastErrorSelected != -1)
		parag = m_lastErrorSelected;
	else
		parag = 0;

    //if there are no errors before m_lastErrorSelected try again from the end
    if (!scanErrorBackward(parag))
        if (m_lastErrorSelected != -1)
        {
            m_lastErrorSelected = -1;
#if QT_VERSION >= 0x030100
            parag = (int)m_items.count();
#else
            parag = m_items.size();
#endif
            if (!scanErrorBackward(parag))
                KNotifyClient::beep();
        }
        else
            KNotifyClient::beep();
}

void MakeWidget::contentsMouseReleaseEvent( QMouseEvent* e )
{
	QTextEdit::contentsMouseReleaseEvent(e);
	if ( e->button() != LeftButton )
		return;
	searchItem(paragraphAt(e->pos()));
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

// returns the current directory for parag
QString MakeWidget::directory(int parag) const
{
	QValueVector<MakeItem*>::const_iterator it = qFind( m_items.begin(), m_items.end(), m_paragraphToItem[parag] );
	if ( it == m_items.end() )
		return QString::null;
	// run backwards over directories and figure out where we are
	while ( it != m_items.begin() ) {
		--it;
		EnteringDirectoryItem* edi = dynamic_cast<EnteringDirectoryItem*>( *it );
		if ( edi )
			return edi->directory + "/";
	}
	return QString::null;
}

// hackish function that will return true and put string "file" in "fName" if the file
// exists
static bool checkFileExists( const QString& file, QString& fName )
{
    if ( QFile::exists( file ) ) {
        fName = file;
        return true;
    }
    return false;
}

void MakeWidget::specialCheck( const QString& file, QString& fName ) const
{
    QString firstLine = text(0);
    QRegExp rx("cd \\'(.*)\\'.*");
    if (rx.search(firstLine) != -1)
    {
        KURL url(rx.cap(1)+"/", file);
        if (url.isValid())
        {
            kdDebug(9004) << "MakeWidget::specialCheck thinks that url is: " << url.url()
                << " origin: " << file << endl;
            fName = url.url();
        }
    }
}

QString MakeWidget::guessFileName( const QString& fName, int parag ) const
{
    // pathological case
    if ( ! m_part->project() ) return fName;

    QString name;
    QString dir = directory( parag );

    if ( fName.startsWith( "/" ) )
    {
        // absolute path given
        name = fName;
    }
    else if ( !dir.isEmpty() )
    {
        name = dir + fName;
    }
    else
    {
        // now it gets tricky - no directory navigation messages,
        // no absolute path - let's guess.
        name = fName;
        if ( !checkFileExists( m_lastBuildDir + "/" + fName, name) &&
            !checkFileExists( m_part->project()->projectDirectory() + "/" + fName, name ) &&
            !checkFileExists( m_part->project()->projectDirectory() + "/" + m_part->project()->activeDirectory() + "/" + fName, name ) &&
            !checkFileExists( m_part->project()->buildDirectory() + "/" + fName, name ) )
            specialCheck(fName, name);
    }

    kdDebug(9004) << "Opening file: " << name << endl;

    // GNU make resolves symlinks. if "name" is a real path to a file the
    // project know by symlink path, we need to return the symlink path
//    QStringList projectFiles = m_part->project()->allFiles();
    QStringList projectFiles = m_part->project()->symlinkProjectFiles();
    QStringList::iterator it = projectFiles.begin();
    while ( it != projectFiles.end() )
    {
        QString file = m_part->project()->projectDirectory() + "/" + *it;
        if ( name == URLUtil::canonicalPath( file ) )
        {
            kdDebug(9004) << "Found file in project - " << file << " == " << name << endl;
            return file;
        }
        ++it;
    }

    // this should only happen if the file is not in the project
    return name;
}

void MakeWidget::searchItem(int parag)
{
	ErrorItem* item = dynamic_cast<ErrorItem*>( m_paragraphToItem[parag] );
	if ( item )
	{
		// open the file
		kdDebug(9004) << "Opening file: " << guessFileName(item->fileName, parag) << endl;
		m_part->partController()->editDocument(KURL( guessFileName(item->fileName, parag) ), item->lineNum);
		m_part->mainWindow()->statusBar()->message( item->m_error, 10000 );
        m_lastErrorSelected = parag;
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
        {
            kdDebug() << "inserting stderr line: " << line << endl;
		m_errorFilter.processLine( line );
  }
}

void MakeWidget::slotProcessExited(KProcess *)
{
	if (childproc->normalExit())
	{
		if (childproc->exitStatus())
		{
			KNotifyClient::event( topLevelWidget()->winId(), "ProcessError", i18n("The process has finished with errors"));
			emit m_part->commandFailed(currentCommand);
		}
		else
		{
			KNotifyClient::event( topLevelWidget()->winId(), "ProcessSuccess", i18n("The process has finished successfully"));
			emit m_part->commandFinished(currentCommand);
		}
	}

	MakeItem* item = new ExitStatusItem( childproc->normalExit(), childproc->exitStatus() );
	insertItem( item );
  displayPendingItem();

	m_part->mainWindow()->statusBar()->message( QString("%1: %2").arg(currentCommand).arg(item->m_text), 3000);
	m_part->core()->running(m_part, false);

	// Defensive programming: We emit this with a single shot timer so that we go once again
	// through the event loop. After that, we can be sure that the process is really finished
	// and its KProcess object can be reused.
	if (childproc->normalExit() && !childproc->exitStatus())
	{
		QTimer::singleShot(0, this, SLOT(startNextJob()));
// 		if (commandList.isEmpty())
			// The last command on the list was successful so restore the
			// output view to what it had before the compilation process started
// 			m_part->mainWindow()->restoreOutputViewTab();
	}
	else
	{
		commandList.clear();
		dirList.clear();
	}
}

void MakeWidget::slotEnteredDirectory( EnteringDirectoryItem* item )
{
//	kdDebug(9004) << "Entering dir: " << item->directory << endl;
	QString* dir = new QString( item->directory );
	dirstack.push( dir );
	insertItem( item );
}

void MakeWidget::slotExitedDirectory( ExitingDirectoryItem* item )
{
	QString eDir = item->directory;
//	kdDebug(9004) << "Leaving dir: " << eDir << endl;
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

void MakeWidget::displayPendingItem()
{
  if (!m_pendingItem) return;
  // this handles the case of ImmDisplay|Append
  // We call displayPendingItem once in insertItem
  // and the appends are handled directly in
  // appendToLastLine
  if (!m_items.empty() 
      && m_items.last() == m_pendingItem) return;

  m_items.push_back(m_pendingItem);

  if ( m_bCompiling && !m_pendingItem->visible( m_compilerOutputLevel ) )
    return;

  SelectionPreserver preserveSelection( *this, !m_vertScrolling && !m_horizScrolling );
  m_paragraphToItem.insert( m_paragraphs++, m_pendingItem );
  append( m_pendingItem->formattedText( m_compilerOutputLevel, brightBg() ) );
}

bool MakeWidget::appendToLastLine( const QString& text )
{
	if ( !m_pendingItem ) return false;
	if ( !m_pendingItem->append( text ) )
  {
    displayPendingItem();
    m_pendingItem = 0;
		return false;
  }

  int mode = m_pendingItem -> displayMode();
  if ((mode & MakeItem::Append) && (mode & MakeItem::ImmDisplay))
  {
    removeParagraph(paragraphs() - 1);
    SelectionPreserver preserveSelection( *this, !m_vertScrolling && !m_horizScrolling );
    append( m_pendingItem->formattedText( m_compilerOutputLevel, brightBg() ) );
  }

	return true;
}

void MakeWidget::insertItem( MakeItem* new_item )
{
  displayPendingItem();
  m_pendingItem = new_item;

  if (!new_item) return;

  int mode = new_item -> displayMode();
  if (mode & MakeItem::ImmDisplay)
  {
    displayPendingItem();
    if (!(mode & MakeItem::Append))
      m_pendingItem = 0;
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
	int id = pMenu->insertItem(i18n("Line Wrapping"), this, SLOT(toggleLineWrapping()) );
	pMenu->setItemChecked(id, m_bLineWrapping);
    pMenu->setWhatsThis(id, i18n("<b>Line wrapping</b><p>Enables or disables wrapping of command lines displayed."));

	pMenu->insertSeparator();
	id = pMenu->insertItem(i18n("Very Short Compiler Output"), this, SLOT(slotVeryShortCompilerOutput()) );
    pMenu->setWhatsThis(id, i18n("<b>Very short compiler output</b><p>Displays only warnings, errors and the file names which are compiled."));
	pMenu->setItemChecked(id, m_compilerOutputLevel == eVeryShort);
	id = pMenu->insertItem(i18n("Short Compiler Output"), this, SLOT(slotShortCompilerOutput()) );
    pMenu->setWhatsThis(id, i18n("<b>Short compiler output</b><p>Suppresses all the compiler flags and formats to something readable."));
	pMenu->setItemChecked(id, m_compilerOutputLevel == eShort);
	id = pMenu->insertItem(i18n("Full Compiler Output"), this, SLOT(slotFullCompilerOutput()) );
    pMenu->setWhatsThis(id, i18n("<b>Full compiler output</b><p>Displays unmodified compiler output."));
	pMenu->setItemChecked(id, m_compilerOutputLevel == eFull);

	pMenu->insertSeparator();
	id = pMenu->insertItem(i18n("Show Directory Navigation Messages"), this, SLOT(toggleShowDirNavigMessages()));
    pMenu->setWhatsThis(id, i18n("<b>Show directory navigation messages</b><p>Shows <b>cd</b> commands that are executed while building."));
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

void MakeWidget::refill()
{
	clear();
	m_paragraphToItem.clear();
	m_paragraphs = 0;
	for( uint i = 0; i < m_items.size(); i++ )
	{
		if ( m_bCompiling && !m_items[i]->visible( m_compilerOutputLevel ) )
			continue;
		m_paragraphToItem.insert( m_paragraphs++, m_items[i] );
		append( m_items[i]->formattedText( m_compilerOutputLevel, brightBg() ) );
	}

}

void MakeWidget::slotVeryShortCompilerOutput() { setTextFormat( Qt::RichText ); setCompilerOutputLevel(eVeryShort); }
void MakeWidget::slotShortCompilerOutput() { setTextFormat( Qt::RichText ); setCompilerOutputLevel(eShort); }
void MakeWidget::slotFullCompilerOutput() { setTextFormat( Qt::RichText ); setCompilerOutputLevel(eFull);  }

void MakeWidget::setCompilerOutputLevel(EOutputLevel level)
{
	m_compilerOutputLevel = level;
	KConfig *pConfig = kapp->config();
	pConfig->setGroup("MakeOutputView");
	pConfig->writeEntry("CompilerOutputLevel", (int) level);
	pConfig->sync();
	refill();
}

void MakeWidget::toggleShowDirNavigMessages()
{
	DirectoryItem::setShowDirectoryMessages( !DirectoryItem::getShowDirectoryMessages() );
	KConfig *pConfig = kapp->config();
	pConfig->setGroup("MakeOutputView");
	pConfig->writeEntry("ShowDirNavigMsg", DirectoryItem::getShowDirectoryMessages());
	pConfig->sync();
	refill();
}

void MakeWidget::updateSettingsFromConfig()
{
	KConfig *pConfig = kapp->config();
	pConfig->setGroup("General Options");
	QFont outputFont = pConfig->readFontEntry("OutputViewFont");
	setFont(outputFont);
	pConfig->setGroup("MakeOutputView");
	m_bLineWrapping = pConfig->readBoolEntry("LineWrapping", true);
	m_compilerOutputLevel = (EOutputLevel) pConfig->readNumEntry("CompilerOutputLevel", (int) eShort);
	DirectoryItem::setShowDirectoryMessages( pConfig->readBoolEntry("ShowDirNavigMsg", false) );
}

bool MakeWidget::scanErrorForward( int parag )
{
	for ( int it = parag + 1;
	      it < (int)m_items.count();
	      ++it )
	{
		ErrorItem* item = dynamic_cast<ErrorItem*>( m_paragraphToItem[it] );
		if ( !item )
			continue;
		if( item->m_isWarning )
			continue;
		parag = it;
		document()->removeSelection(0);
		setSelection(parag, 0, parag+1, 0, 0);
		setCursorPosition(parag, 0);
		ensureCursorVisible();
		searchItem( it );
		return true;
	}
    return false;
}

bool MakeWidget::scanErrorBackward( int parag )
{
	for ( int it = parag - 1; it >= 0; --it)
	{
		ErrorItem* item = dynamic_cast<ErrorItem*>( m_paragraphToItem[it] );
		if ( !item )
			continue;
		if( item->m_isWarning )
			continue;
		parag = it;
		document()->removeSelection(0);
		setSelection(parag, 0, parag+1, 0, 0);
		setCursorPosition(parag, 0);
		ensureCursorVisible();
		searchItem( it );
		return true;
	}
	return false;
}

#include "makewidget.moc"
