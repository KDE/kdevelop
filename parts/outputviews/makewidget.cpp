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

#include <qapplication.h>
#include <qdir.h>
#include <qimage.h>
#include <qregexp.h>
#include <qstylesheet.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kprocess.h>
#include <kregexp.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kstatusbar.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
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
{
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

    connect(childproc, SIGNAL(processExited(KProcess*)),
            this, SLOT(slotProcessExited(KProcess*) )) ;

    items.setAutoDelete(true);
    parags = 0;
    moved = false;

    m_part = part;

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

    it =  dirList.begin();
    QString dir = *it;
    dirList.remove(it);

    clear();
    items.clear();
    parags = 0;
    moved = false;

    insertLine2(currentCommand, Diagnostic);
    childproc->clearArguments();
    *childproc << currentCommand;
    childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);

    dirstack.clear();
    dirstack.push(new QString(dir));

    m_part->topLevel()->raiseView(this);
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
            m_part->topLevel()->statusBar()->message( (*it)->text, 10000 );
            m_part->topLevel()->lowerView(this);
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
            m_part->topLevel()->statusBar()->message( (*it)->text, 10000 );
            m_part->topLevel()->lowerView(this);
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
            m_part->topLevel()->statusBar()->message( (*it)->text, 10000 );
            m_part->topLevel()->lowerView(this);
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

    m_part->topLevel()->statusBar()->message(i18n("%1: %2").arg(currentCommand).arg(s), 3000);
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
    // KRegExp has ERE syntax
    KRegExp errorGccRx("([^: \t]+):([0-9]+):(.*)");
    KRegExp errorFtnchekRx("\"(.*)\", line ([0-9]+):(.*)");
    KRegExp errorJadeRx("[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)");
    const int errorGccFileGroup = 1;
    const int errorGccRowGroup = 2;
    const int errorGccTextGroup = 3;
    const int errorFtnchekFileGroup = 1;
    const int errorFtnchekRowGroup = 2;
    const int errorFrnchekTextGroup = 3;
    const int errorJadeFileGroup = 1;
    const int errorJadeRowGroup = 2;
    const int errorJadeTextGroup = 3;

    QString eDir;
    if (matchEnterDir(line, eDir))
    {
        QString *dir = new QString(eDir);
        dirstack.push(dir);
        kdDebug(9004) << "Entering dir: " << (*dir).ascii() << endl;
        insertLine2(line, Diagnostic);
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
        insertLine2(line, Diagnostic);
        return;
    }

    QString fn;
    int row = -1;
    QString text;

    bool hasmatch = false;
    if (errorGccRx.match(line))
    {
        hasmatch = true;
        fn = errorGccRx.group(errorGccFileGroup);
        row = QString(errorGccRx.group(errorGccRowGroup)).toInt()-1;
        text = QString(errorGccRx.group(errorGccTextGroup));
    }
    else if (errorFtnchekRx.match(line))
    {
        kdDebug() << "Matching " << line << endl;
        hasmatch = true;
        fn = errorFtnchekRx.group(errorFtnchekFileGroup);
        row = QString(errorFtnchekRx.group(errorFtnchekRowGroup)).toInt()-1;
        text = QString(errorFtnchekRx.group(errorFrnchekTextGroup));
    }
    else if (errorJadeRx.match(line))
    {
        hasmatch = true;
        fn = errorGccRx.group(errorJadeFileGroup);
        row = QString(errorJadeRx.group(errorJadeRowGroup)).toInt()-1;
        text = QString(errorJadeRx.group(errorJadeTextGroup));
    }

    if( hasmatch )
    {
        // Add hacks for error strings you want excluded here
        if( text.find( QString::fromLatin1("(Each undeclared identifier is reported only once") ) >= 0
                || text.find( QString::fromLatin1("for each function it appears in.)") ) >= 0 )
            hasmatch = false;
    }

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
        insertLine2(line, Error);
    }
    else
    {
        insertLine2(line, type);
    }
}

void MakeWidget::paletteChange(const QPalette& oldPalette)
{
    kdDebug(9004) << "Palette Change" << endl;
    updateColors();
}

void MakeWidget::updateColors()
{
    int h,s,v;
    paletteBackgroundColor().hsv( &h, &s, &v );
    bool bright_bg = (v > 127);
    errorColor = bright_bg ? "darkRed" : "lightRed";
    diagnosticColor = bright_bg ? "black" : "white";
    normalColor = bright_bg ? "darkBlue" : "lightBlue";
}

QString MakeWidget::getOutputColor( Type type )
{

    switch ( type )
    {
    case Error:
        return errorColor;
    case Diagnostic:
        return diagnosticColor;
    default:
        return normalColor;
    }
}



void MakeWidget::insertLine2(const QString &line, Type type)
{
    ++parags;

    int para, index;
    getCursorPosition( &para, &index );

    bool move = para == paragraphs() - 1 && index == paragraphLength( para );

    int paraFrom, indexFrom, paraTo, indexTo;
    getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo, 0);

    QString icon;
    if (type == Error)
        icon = "<img src=\"error\"><nobr> </nobr>";
    else if (type == Diagnostic)
        icon = "<img src=\"warning\"><nobr> </nobr>";
    else
        icon = "<img src=\"message\"><nobr> </nobr>";

    QString color = getOutputColor( type );
    QString eLine = QStyleSheet::escape( line );
    append(QString("<code>%1<font color=\"%2\">%3</font></code><br>").arg(icon).arg(color).arg(eLine));
    setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);

    if (move)
    {
        moveCursor(MoveEnd, false);
    }

}

#include "makewidget.moc"
