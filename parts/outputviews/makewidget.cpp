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

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "processlinemaker.h"

#include "makeviewpart.h"


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

class MakeItem
{
public:
    MakeItem(int pg, const QString fn, int ln, const QString tx)
            : parag(pg), fileName(fn), lineNum(ln), text(tx)
    {}
    int parag;
    QString fileName;
    int lineNum;
    QString text;
};


MakeWidget::MakeWidget(MakeViewPart *part)
: QTextEdit(0, "make widget")
 ,m_part(part)
 ,m_vertScrolling(false)
 ,m_horizScrolling(false)
 ,m_bCompiling(false)
 ,m_errorGccRx("([^: \t]+):([0-9]+):(.*)")
 ,m_errorGccFileGroup(1)
 ,m_errorGccRowGroup(2)
 ,m_errorGccTextGroup(3)
 ,m_errorFtnchekRx("\"(.*)\", line ([0-9]+):(.*)")
 ,m_errorFtnchekFileGroup(1)
 ,m_errorFtnchekRowGroup(2)
 ,m_errorFrnchekTextGroup(3)
 ,m_errorJadeRx("[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)")
 ,m_errorJadeFileGroup(1)
 ,m_errorJadeRowGroup(2)
 ,m_errorJadeTextGroup(3)
 ,m_compileFile1("(?:g\\+\\+|/bin/sh\\s.*libtool.*--mode=compile).*`.*`(.+)")
 ,m_compileFile2("(?:g\\+\\+|/bin/sh\\s.*libtool.*--mode=compile).* -c ([^\\s;]+)")
 ,m_compileFile3("(?:g\\+\\+|/bin/sh\\s.*libtool.*--mode=compile).* -c -.*")
 ,m_mocFile(".*/moc\\b.*\\s-o\\s([^\\s;]+)")
 ,m_linkFile("(?:g\\+\\+|/bin/sh\\s.*libtool.*--mode=link).* -o ([^\\s;]+)")
 ,m_installFile("(?:/usr/bin/install|/bin/sh\\s.*mkinstalldirs).*\\s([^\\s;]+)")
 ,m_fileNameGroup(1)
{
    updateSettingsFromConfig();


    if (m_bLineWrapping) {
	setWordWrap(WidgetWidth);
    }
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

    connect(childproc, SIGNAL(processExited(KProcess*)),
            this, SLOT(slotProcessExited(KProcess*) )) ;

    connect( verticalScrollBar(), SIGNAL(sliderPressed()),
             this, SLOT(verticScrollingOn()) );
    connect( verticalScrollBar(), SIGNAL(sliderReleased()),
             this, SLOT(verticScrollingOff()) );
    connect( horizontalScrollBar(), SIGNAL(sliderPressed()),
             this, SLOT(horizScrollingOn()) );
    connect( horizontalScrollBar(), SIGNAL(sliderReleased()),
             this, SLOT(horizScrollingOff()) );

    items.setAutoDelete(true);
    parags = 0;
    moved = false;

    updateColors();
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
    if (it == commandList.end())
        return;

    currentCommand = *it;
    commandList.remove(it);
    
    int i = currentCommand.findRev(" gmake");
    if (i == -1) { m_bCompiling = false; }
    else {
	QString s = currentCommand.right(currentCommand.length() - i);
	if (s.contains("configure ")        || s.contains(" Makefile.cvs") ||
	    s.contains(" clean")            || s.contains(" distclean") ||
	    s.contains(" package-messages") ||  s.contains(" install")) 
	{ m_bCompiling = false; }
	else { m_bCompiling = true; }	    
    }

    it =  dirList.begin();
    QString dir = *it;
    dirList.remove(it);

    clear(); // clear the widget
    items.clear();
    parags = 0;
    moved = false;
    
    m_veryShortOutput.clear();
    m_shortOutput.clear(); // clear the shadow string lists
    m_fullOutput.clear();

    insertLine2(currentCommand, Diagnostic);
    insertLine2(currentCommand, Diagnostic, &m_veryShortOutput);
    insertLine2(currentCommand, Diagnostic, &m_shortOutput);
    insertLine2(currentCommand, Diagnostic, &m_fullOutput);
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

    QListIterator<MakeItem> it(items);
    for (; it.current(); ++it)
        if ((*it)->parag > parag)
        {
            moved = true;
            parag = (*it)->parag;
            document()->removeSelection(0);
            setSelection(parag, 0, parag+1, 0, 0);
            setCursorPosition(parag, 0);
            ensureCursorVisible();
            m_part->partController()->editDocument((*it)->fileName, (*it)->lineNum);
            m_part->mainWindow()->statusBar()->message( (*it)->text, 10000 );
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

    QListIterator<MakeItem> it(items);
    for (it.toLast(); it.current(); --it)
        if ((*it)->parag < parag)
        {
            moved = true;
            parag = (*it)->parag;
            document()->removeSelection(0);
            setSelection(parag, 0, parag+1, 0, 0);
            setCursorPosition(parag, 0);
            ensureCursorVisible();
            m_part->partController()->editDocument((*it)->fileName, (*it)->lineNum);
            m_part->mainWindow()->statusBar()->message( (*it)->text, 10000 );
            m_part->mainWindow()->lowerView(this);
            return;
        }

    KNotifyClient::beep();
}


void MakeWidget::contentsMousePressEvent(QMouseEvent *e)
{
    QTextEdit::contentsMousePressEvent(e);
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
    QListIterator<MakeItem> it(items);
    for (; it.current(); ++it)
    {
        if ((*it)->parag == parag)
        {
            m_part->partController()->editDocument((*it)->fileName, (*it)->lineNum);
            m_part->mainWindow()->statusBar()->message( (*it)->text, 10000 );
            m_part->mainWindow()->lowerView(this);
        }
        if ((*it)->parag >= parag)
            return;
    }
}

void MakeWidget::insertStdoutLine(const QString& line)
{
    insertLine1(line, Diagnostic);
}

void MakeWidget::insertStderrLine(const QString& line)
{
    insertLine1(line, Normal);
}


void MakeWidget::slotProcessExited(KProcess *)
{
    QString s;
    Type t;

    if (childproc->normalExit())
    {
        if (childproc->exitStatus())
        {
            s = i18n("*** Exited with status: %1 ***").arg(childproc->exitStatus());
            t = Error;
            KNotifyClient::event( "ProcessError", i18n("The process has finished with errors"));
        }
        else
        {
            s = i18n("*** Success ***");
            t = Diagnostic;
            KNotifyClient::event( "ProcessSuccess", i18n("The process has finished successfully"));
            emit m_part->commandFinished(currentCommand);
        }
    }
    else
    {
        s = i18n("*** Compilation aborted ***");
        t = Error;
    }

    insertLine2(s, t);
    insertLine2(s, t, &m_veryShortOutput);
    insertLine2(s, t, &m_shortOutput);
    insertLine2(s, t, &m_fullOutput);

    m_part->mainWindow()->statusBar()->message(QString("%1: %2").arg(currentCommand).arg(s), 3000);
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

// simple helper function - checks whether we entered a new directory
// (locale and Utf8 aware)
bool MakeWidget::matchEnterDir( const QString& line, QString& dir )
{
    // make outputs localised strings in Utf8 for entering/leaving directories...
    static const unsigned short fr_enter[] =
        {'E','n','t','r','e',' ','d','a','n','s',' ','l','e',' ','r',0xe9,'p','e','r','t','o','i','r','e'
        };
    static const unsigned short pl_enter[] =
        {'W','c','h','o','d','z',0x119,' ','k','a','t','a','l','o','g'
        };
    static const unsigned short ja_enter[] =
        {
            0x5165,0x308a,0x307e,0x3059,0x20,0x30c7,0x30a3,0x30ec,0x30af,0x30c8,0x30ea
        };
    static const unsigned short ko_enter[] =
        {
            0xb4e4,0xc5b4,0xac10
        };
    static const unsigned short ko_behind[] =
        {
            0x20,0xb514,0xb809,0xd1a0,0xb9ac
        };
    static const unsigned short pt_br_enter[] =
        {
            0x45,0x6e,0x74,0x72,0x61,0x6e,0x64,0x6f,0x20,0x6e,0x6f,0x20,0x64,0x69,0x72,0x65,0x74,0xf3,0x72,0x69,0x6f
        };
    static const unsigned short ru_enter[] =
        {
            0x412,0x445,0x43e,0x434,0x20,0x432,0x20,0x43a,0x430,0x442,0x430,0x43b,0x43e,0x433
        };

    static const QString fr_e( (const QChar*)fr_enter, sizeof(fr_enter) / 2 );
    static const QString pl_e( (const QChar*)pl_enter, sizeof(pl_enter) / 2 );
    static const QString ja_e( (const QChar*)ja_enter, sizeof(ja_enter) / 2 );
    static const QString ko_e( (const QChar*)ko_enter, sizeof(ko_enter) / 2 );
    static const QString ko_b( (const QChar*)ko_behind, sizeof(ko_behind) / 2 );
    static const QString pt_br_e( (const QChar*)pt_br_enter, sizeof(pt_br_enter) / 2 );
    static const QString ru_e( (const QChar*)ru_enter, sizeof(ru_enter) / 2 );

    // we need a QRegExp because KRegExp is not Utf8 aware.
    // 0x00AB is LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    // 0X00BB is RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    static QRegExp dirChange(QString::fromLatin1(".*: (.+) (`|") + QChar(0x00BB) + QString::fromLatin1(")(.*)('|") + QChar(0x00AB) + QString::fromLatin1(")(.*)"));

    if (dirChange.search(line) > -1 )
    {
        QString msg = dirChange.cap(1);
        QString msgBehind = dirChange.cap(5);

        if ( msg == "Entering directory" ||   // English - default
                msg == "Wechsel in das Verzeichnis Verzeichnis" ||    // German - yes, this is badly translated
		msg == "Wechsel in das Verzeichnis" || // German without typo
                msg == "Cambiando a directorio" || // Spanish
                msg == fr_e || // French
                msg == ja_e || // Japanese
                ( msg == ko_e && msgBehind == ko_b ) || // Korean
                msg == "Binnengaan van directory" || // Dutch
                msg == pl_e || // Polish
                msg == pt_br_e || // Brazilian Portuguese
                msg == ru_e ) // Russian
        {
            dir = dirChange.cap(3);
            return true;
        }
    }

    return false;
}

// simple helper function - checks whether we left a directory
// (locale and Utf8 aware).
bool MakeWidget::matchLeaveDir( const QString& line, QString& dir )
{
    static const unsigned short fr_leave[] =
        { 'Q','u','i','t','t','e',' ','l','e',' ','r',0xe9,'p','e','r','t','o','i','r','e'
        };
    static const unsigned short ja_leave[] =
        {
            0x51fa,0x307e,0x3059,0x20,0x30c7,0x30a3,0x30ec,0x30af,0x30c8,0x30ea
        };
    static const unsigned short pt_br_leave[] =
        {'S','a','i','n','d','o',' ','d','o',' ','d','i','r','e','t',0xf3,'r','i','o'
        };
    static const unsigned short ru_leave[] =
        {
            0x412,0x44b,0x445,0x43e,0x434,0x20,0x438,0x437,0x20,0x43a,0x430,0x442,0x430,0x43b,0x43e,0x433
        };
    static const unsigned short ko_leave[] =
        {
            0xb098,0xac10
        };
    static const unsigned short ko_behind[] =
        {
            0x20,0xb514,0xb809,0xd1a0,0xb9ac
        };

    static const QString fr_l( (const QChar*)fr_leave, sizeof(fr_leave) / 2 );
    static const QString ja_l( (const QChar*)ja_leave, sizeof(ja_leave) / 2 );
    static const QString ko_l( (const QChar*)ko_leave, sizeof(ko_leave) / 2 );
    static const QString pt_br_l( (const QChar*)pt_br_leave, sizeof(pt_br_leave) / 2 );
    static const QString ru_l( (const QChar*)ru_leave, sizeof(ru_leave) / 2 );

    // we need a QRegExp because KRegExp is not Utf8 aware.
    // 0x00AB is LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    // 0X00BB is RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    static QRegExp dirChange(QString::fromLatin1(".*: (.+) (`|") + QChar(0x00BB) + QString::fromLatin1(")(.*)('|") + QChar(0x00AB) + QString::fromLatin1(")(.*)"));

    if (dirChange.search(line) > -1 )
    {
        QString msg = dirChange.cap(1);
        QString msgBehind = dirChange.cap(5);

        if ( msg == "Leaving directory" || // en
                msg == "Verlassen des Verzeichnisses Verzeichnis" || // de
		msg == "Verlassen des Verzeichnisses" ||
                msg == "Saliendo directorio" || // es
                msg == fr_l || // fr
                msg == ja_l || //ja
                ( msg == (const char*)ko_leave && msg == (const char*)ko_behind ) || //ko
                msg == "Verdwijnen uit directory" || //nl
                msg == "Opuszczam katalog" || //po
                msg == pt_br_l || //pt_BR
                msg == ru_l ) //ru
        {
            dir = dirChange.cap(3);
            return true;
        }
    }

    return false;
}


void MakeWidget::insertLine1(const QString &line, Type type)
{
    QString eDir;
    if (matchEnterDir(line, eDir))
    {
        QString *dir = new QString(eDir);
        dirstack.push(dir);
        kdDebug(9004) << "Entering dir: " << (*dir).ascii() << endl;
	if (m_bShowDirNavMsg) {
	    insertLine2(line, Diagnostic);
	    insertLine2(line, Diagnostic, &m_veryShortOutput);
	    insertLine2(line, Diagnostic, &m_shortOutput);
	    insertLine2(line, Diagnostic, &m_fullOutput);
	}
        return;
    }
    else if (matchLeaveDir(line, eDir))
    {
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
        delete dir;
	if (m_bShowDirNavMsg) {
	    insertLine2(line, Diagnostic);
	    insertLine2(line, Diagnostic, &m_veryShortOutput);
	    insertLine2(line, Diagnostic, &m_shortOutput);
	    insertLine2(line, Diagnostic, &m_fullOutput);
	}
        return;
    }

    QString fn;
    int row = -1;
    QString text;

    bool hasmatch = false;
    if (m_errorGccRx.search(line) != -1)
    {
        hasmatch = true;
        fn = m_errorGccRx.cap(m_errorGccFileGroup);
        row = QString(m_errorGccRx.cap(m_errorGccRowGroup)).toInt()-1;
        text = QString(m_errorGccRx.cap(m_errorGccTextGroup));
    }
    else if (m_errorFtnchekRx.search(line) != -1)
    {
        kdDebug() << "Matching " << line << endl;
        hasmatch = true;
        fn = m_errorFtnchekRx.cap(m_errorFtnchekFileGroup);
        row = QString(m_errorFtnchekRx.cap(m_errorFtnchekRowGroup)).toInt()-1;
        text = QString(m_errorFtnchekRx.cap(m_errorFrnchekTextGroup));
    }
    else if (m_errorJadeRx.search(line) != -1)
    {
        hasmatch = true;
        fn = m_errorGccRx.cap(m_errorJadeFileGroup);
        row = QString(m_errorJadeRx.cap(m_errorJadeRowGroup)).toInt()-1;
        text = QString(m_errorJadeRx.cap(m_errorJadeTextGroup));
    }

    if( hasmatch )
    {
        // Add hacks for error strings you want excluded here
        if( text.find( QString::fromLatin1("(Each undeclared identifier is reported only once") ) >= 0
                || text.find( QString::fromLatin1("for each function it appears in.)") ) >= 0 )
            hasmatch = false;
    }

    QString shortOutput;
    if (hasmatch)
    {
        kdDebug(9004) << "Error in " << fn << " " << row << ": " << text << endl;
        if (dirstack.top() && fn[0] != '/')
            fn.prepend("/").prepend(*dirstack.top());
        // consolidate /../
        QFileInfo info(fn);
        fn = info.absFilePath();
        kdDebug(9004) << "Path: " << fn << endl;
        items.append(new MakeItem(parags, fn, row, text));
	type = Error;
    }
    else if (m_bCompiling) {
	if (m_compileFile1.search(line) != -1) {
	    QString tool;
	    if (!line.startsWith("g++")) { tool = "(libtool)"; }
	    shortOutput = i18n("compiling <b>%1</b> %2").arg(m_compileFile1.cap(m_fileNameGroup)).arg(tool);
	}
	else if (m_compileFile3.search(line) != -1) {
	    int i = line.findRev(" ");
	    QString filename = line.right(line.length()-i);
	    QString tool;
	    if (!line.startsWith("g++")) { tool = " (libtool)"; }
	    shortOutput = i18n("compiling <b>%1</b> %2").arg(filename).arg(tool);
	}
	else if (m_compileFile2.search(line) != -1) {
	    QString tool;
	    if (!line.startsWith("g++")) { tool = " (libtool)"; }
	    shortOutput = i18n("compiling <b>%1</b> %2").arg(m_compileFile2.cap(m_fileNameGroup)).arg(tool);
	}
	else if (m_mocFile.search(line) != -1) {
	    shortOutput = i18n("generating <b>%1</b> (moc)").arg(m_mocFile.cap(m_fileNameGroup));
	}
	else if (m_linkFile.search(line) != -1) {
	    QString tool;
	    if (!line.startsWith("g++")) { tool = " (libtool)"; }
	    shortOutput = i18n("linking <b>%1</b> %2").arg(m_linkFile.cap(m_fileNameGroup)).arg(tool);
	}
	else if (m_installFile.search(line) != -1) {
	    shortOutput = i18n("installing <b>%1</b>").arg(m_installFile.cap(m_fileNameGroup));
	}
    }
    
    // append it to this textedit widget
    bool bShortOutputExprMatched = !shortOutput.isEmpty();
    bool bConfiguring = line.startsWith("checking "); // hack to consider an implicite configure call
    if (bShortOutputExprMatched && m_compilerOutputLevel != eFull) {
	insertLine2(shortOutput, StyledDiagnostic);
    }
    else if (bConfiguring || !(m_bCompiling && m_compilerOutputLevel == eVeryShort && type == Diagnostic)){
	insertLine2(line, type);
    }
    
    // also store the short and long version in stringlists to allow switching on the fly
    insertLine2(line, type, &m_fullOutput);
    QString ln = line; // because line is const
    if (bShortOutputExprMatched) {
	ln = shortOutput;
	type = StyledDiagnostic;
    }
    insertLine2(ln, type, &m_shortOutput);
    if (bConfiguring || !(m_bCompiling && type == Diagnostic)) 
    {
	insertLine2(ln, type, &m_veryShortOutput);
    }
}

void MakeWidget::paletteChange(const QPalette& /*oldPalette*/)
{
    kdDebug(9004) << "Palette Change" << endl;
    updateColors();
}

void MakeWidget::updateColors()
{
    int h,s,v;
    paletteBackgroundColor().hsv( &h, &s, &v );
    bool bright_bg = (v > 127);
    errorColor = bright_bg ? "maroon" : "red";
    diagnosticColor = bright_bg ? "black" : "white";
    normalColor = bright_bg ? "navy" : "blue";
}

QString MakeWidget::getOutputColor( Type type )
{

    switch ( type )
    {
    case Error:
        return errorColor;
    case Diagnostic:
    case StyledDiagnostic:
        return diagnosticColor;
    default:
        return normalColor;
    }
}



void MakeWidget::insertLine2(const QString &line, Type type, QStringList* pStringList)
{
    QString icon;
    if (type == Error)
        icon = "<img src=\"error\"></img><nobr> </nobr>";
    else if (type == Diagnostic || type == StyledDiagnostic)
        icon = "<img src=\"warning\"></img><nobr> </nobr>";
    else
        icon = "<img src=\"message\"></img><nobr> </nobr>";

    QString color = getOutputColor( type );
    QString eLine = QStyleSheet::escape( line );
    // Qt >= 3.1 doesn't need a <br>.
#if QT_VERSION < 0x040000
    static const QString br = QString::fromLatin1( qVersion() ).section( ".", 1, 1 ).toInt() > 0 ? "" : "<br>";
#else
    static const QString br;
#endif
    
    if (pStringList) {
	// just store it in a stringlist, if the user wants to switch between short and full output on the fly
	pStringList->append(QString("<code>%1<font color=\"%2\">%3</font></code>%4").arg(icon).arg(color).arg(type == StyledDiagnostic ? line : eLine).arg(br));
	return;
    }
    
    ++parags;

    int para, index;
    getCursorPosition( &para, &index );

    bool atEnd = para == paragraphs() - 1 && index == paragraphLength( para );

    int paraFrom, indexFrom, paraTo, indexTo;
    getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo, 0);

    append(QString("<code>%1<font color=\"%2\">%3</font></code>%4").arg(icon).arg(color).arg(type == StyledDiagnostic ? line : eLine).arg(br));

    setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);

    if (atEnd && !m_vertScrolling && !m_horizScrolling)
    {
        moveCursor(MoveEnd, false);
        moveCursor(MoveLineStart, false);//if linewrap is off we must avoid the jumping of the vertical scrollbar
    }

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
    pMenu->setItemChecked(id, m_bShowDirNavMsg);
    
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
    else {
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
    switch (level) {
    case eVeryShort:
	{
	    QStringList::Iterator it = m_veryShortOutput.begin();
	    while (it != m_veryShortOutput.end()) {
		append(*it); it++;
	    }
	}
	break;
    case eShort:
	{
	    QStringList::Iterator it = m_shortOutput.begin();
	    while (it != m_shortOutput.end()) {
		append(*it); it++;
	    }
	}
	break;
    case eFull:
	{
	    QStringList::Iterator it = m_fullOutput.begin();
	    while (it != m_fullOutput.end()) {
		append(*it); it++;
	    }
	}
	break;
    default:;
    }
}

void MakeWidget::toggleShowDirNavigMessages()
{
    m_bShowDirNavMsg = !m_bShowDirNavMsg;    
    KConfig *pConfig = kapp->config();
    pConfig->setGroup("MakeOutputView");
    pConfig->writeEntry("ShowDirNavigMsg", m_bShowDirNavMsg);
    pConfig->sync();
}

void MakeWidget::updateSettingsFromConfig()
{
    KConfig *pConfig = kapp->config();
    pConfig->setGroup("MakeOutputView");
    setFont(pConfig->readFontEntry("Messages Font"));
    m_bLineWrapping = pConfig->readBoolEntry("LineWrapping", true);
    m_compilerOutputLevel = (EOutputLevel) pConfig->readNumEntry("CompilerOutputLevel", (int) eVeryShort);
    m_bShowDirNavMsg = pConfig->readBoolEntry("ShowDirNavigMsg", false);
}

#include "makewidget.moc"
