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
#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kprocess.h>
#include <kregexp.h>
#include <kstatusbar.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"

#include "makeviewpart.h"


static const char *const error_xpm[] = {
"11 11 45 1",
" 	c None",
".	c #EC0000",
"+	c #F50000",
"@	c #E40000",
"#	c #DE0000",
"$	c #AE0000",
"%	c #FC0000",
"&	c #FF0000",
"*	c #FB0000",
"=	c #E70000",
"-	c #AF0000",
";	c #FF0101",
">	c #FF0202",
",	c #F00000",
"'	c #C90000",
")	c #890000",
"!	c #FF1717",
"~	c #FF2E2E",
"{	c #FF0606",
"]	c #F10000",
"^	c #CE0000",
"/	c #9A0000",
"(	c #530000",
"_	c #FF4545",
":	c #FF0808",
"<	c #FD0000",
"[	c #C70000",
"}	c #980000",
"|	c #570000",
"1	c #FE0202",
"2	c #F60000",
"3	c #DD0000",
"4	c #B90000",
"5	c #8A0000",
"6	c #5C0000",
"7	c #A30000",
"8	c #740000",
"9	c #4D0000",
"0	c #AB0000",
"a	c #850000",
"b	c #5D0000",
"c	c #480000",
"d	c #630000",
"e	c #4C0000",
"f	c #5F0000",
"   .+@#$   ",
"  %&&&*=-  ",
" %&;>&&,') ",
".&;!~{&]^/(",
"+&>~_:<.[}|",
"@&&{:123456",
"#*&&<2@[789",
"$=,].3[0abc",
" -'^[47ade ",
"  )/}58be  ",
"   (|f9c   "
};


static const char *const warning_xpm[] = {
"11 11 45 1",
" 	c None",
".	c #0000EC",
"+	c #0000F5",
"@	c #0000E4",
"#	c #0000DE",
"$	c #0000AE",
"%	c #0000FC",
"&	c #0000FF",
"*	c #0000FB",
"=	c #0000E7",
"-	c #0000AF",
";	c #0101FF",
">	c #0202FF",
",	c #0000F0",
"'	c #0000C9",
")	c #000089",
"!	c #1717FF",
"~	c #2E2EFF",
"{	c #0606FF",
"]	c #0000F1",
"^	c #0000CE",
"/	c #00009A",
"(	c #000053",
"_	c #4545FF",
":	c #0808FF",
"<	c #0000FD",
"[	c #0000C7",
"}	c #000098",
"|	c #000057",
"1	c #0202FE",
"2	c #0000F6",
"3	c #0000DD",
"4	c #0000B9",
"5	c #00008A",
"6	c #00005C",
"7	c #0000A3",
"8	c #000074",
"9	c #00004D",
"0	c #0000AB",
"a	c #000085",
"b	c #00005D",
"c	c #000048",
"d	c #000063",
"e	c #00004C",
"f	c #00005F",
"   .+@#$   ",
"  %&&&*=-  ",
" %&;>&&,') ",
".&;!~{&]^/(",
"+&>~_:<.[}|",
"@&&{:123456",
"#*&&<2@[789",
"$=,].3[0abc",
" -'^[47ade ",
"  )/}58be  ",
"   (|f9c   "
};


static const char *const message_xpm[] = {
"11 11 45 1",
" 	c None",
".	c #00EC00",
"+	c #00F500",
"@	c #00E400",
"#	c #00DE00",
"$	c #00AE00",
"%	c #00FC00",
"&	c #00FF00",
"*	c #00FB00",
"=	c #00E700",
"-	c #00AF00",
";	c #01FF01",
">	c #02FF02",
",	c #00F000",
"'	c #00C900",
")	c #008900",
"!	c #17FF17",
"~	c #2EFF2E",
"{	c #06FF06",
"]	c #00F100",
"^	c #00CE00",
"/	c #009A00",
"(	c #005300",
"_	c #45FF45",
":	c #08FF08",
"<	c #00FD00",
"[	c #00C700",
"}	c #009800",
"|	c #005700",
"1	c #02FE02",
"2	c #00F600",
"3	c #00DD00",
"4	c #00B900",
"5	c #008A00",
"6	c #005C00",
"7	c #00A300",
"8	c #007400",
"9	c #004D00",
"0	c #00AB00",
"a	c #008500",
"b	c #005D00",
"c	c #004800",
"d	c #006300",
"e	c #004C00",
"f	c #005F00",
"   .+@#$   ",
"  %&&&*=-  ",
" %&;>&&,') ",
".&;!~{&]^/(",
"+&>~_:<.[}|",
"@&&{:123456",
"#*&&<2@[789",
"$=,].3[0abc",
" -'^[47ade ",
"  )/}58be  ",
"   (|f9c   "
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
    
    connect(childproc, SIGNAL(receivedStdout(KProcess*,char*,int)),
            this, SLOT(slotReceivedOutput(KProcess*,char*,int)) );
    connect(childproc, SIGNAL(receivedStderr(KProcess*,char*,int)),
            this, SLOT(slotReceivedError(KProcess*,char*,int)) );
    connect(childproc, SIGNAL(processExited(KProcess*)),
            this, SLOT(slotProcessExited(KProcess*) )) ;

    items.setAutoDelete(true);
    parags = 0;
    moved = false;
    
    m_part = part;
}


MakeWidget::~MakeWidget()
{
    delete mimeSourceFactory();
    delete childproc;
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
        if ((*it)->parag > parag) {
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
        if ((*it)->parag < parag) {
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
    if (e->key() == Key_Return || e->key() == Key_Enter) {
        int parag, index;
        getCursorPosition(&parag, &index);
        searchItem(parag);
    } else
        QTextEdit::keyPressEvent(e);
}


void MakeWidget::searchItem(int parag)
{
    QListIterator<MakeItem> it(items);
    for (; it.current(); ++it) {
        if ((*it)->parag == parag) {
            m_part->partController()->editDocument((*it)->fileName, (*it)->lineNum);
            m_part->topLevel()->statusBar()->message( (*it)->text, 10000 );
            m_part->topLevel()->lowerView(this);
        }
        if ((*it)->parag >= parag)
            return;
    }
}


void MakeWidget::slotReceivedOutput(KProcess *, char *buffer, int buflen)
{
    // Flush stderr buffer
    if (!stderrbuf.isEmpty()) {
        insertLine1(stderrbuf, Normal);
        stderrbuf = "";
    }
    
    stdoutbuf += QString::fromLocal8Bit(buffer, buflen);
    int pos;
    while ( (pos = stdoutbuf.find('\n')) != -1) {
        QString line = stdoutbuf.left(pos);
        insertLine1(line, Normal);
        stdoutbuf.remove(0, pos+1);
    }
}


void MakeWidget::slotReceivedError(KProcess *, char *buffer, int buflen)
{
    // Flush stdout buffer
    if (!stdoutbuf.isEmpty()) {
        insertLine1(stdoutbuf, Diagnostic);
        stdoutbuf = "";
    }
    
    stderrbuf += QString::fromLocal8Bit(buffer, buflen);
    int pos;
    while ( (pos = stderrbuf.find('\n')) != -1) {
        QString line = stderrbuf.left(pos);
        insertLine1(line, Diagnostic);
        stderrbuf.remove(0, pos+1);
    }
}


void MakeWidget::slotProcessExited(KProcess *)
{
    QString s;
    Type t;
    
    if (childproc->normalExit()) {
        if (childproc->exitStatus()) {
            s = i18n("*** Exited with status: %1 ***").arg(childproc->exitStatus());
            t = Error;
        } else {
            s = i18n("*** Success ***");
            t = Diagnostic;
            emit m_part->commandFinished(currentCommand);
        }
    } else {
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
    else {
        commandList.clear();
        dirList.clear();
    }
}

// simple helper function - checks whether we entered a new directory
// (locale and Utf8 aware)
bool MakeWidget::matchEnterDir( const QString& line, QString& dir )
{
    // make outputs localised strings in Utf8 for entering/leaving directories...
    static const unsigned short fr_enter[] = {'E','n','t','r','e',' ','d','a','n','s',' ','l','e',' ','r',0xe9,'p','e','r','t','o','i','r','e'};
    static const unsigned short pl_enter[] = {'W','c','h','o','d','z',0x119,' ','k','a','t','a','l','o','g'};
    static const unsigned short ja_enter[] = {0x5165,0x308a,0x307e,0x3059,0x20,0x30c7,0x30a3,0x30ec,0x30af,0x30c8,0x30ea};
    static const unsigned short ko_enter[] = {0xb4e4,0xc5b4,0xac10};
    static const unsigned short ko_behind[] = {0x20,0xb514,0xb809,0xd1a0,0xb9ac};
    static const unsigned short pt_br_enter[] = {0x45,0x6e,0x74,0x72,0x61,0x6e,0x64,0x6f,0x20,0x6e,0x6f,0x20,0x64,0x69,0x72,0x65,0x74,0xf3,0x72,0x69,0x6f};
    static const unsigned short ru_enter[] = {0x412,0x445,0x43e,0x434,0x20,0x432,0x20,0x43a,0x430,0x442,0x430,0x43b,0x43e,0x433};

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

    if (dirChange.search(line) > -1 ) {
        QString msg = dirChange.cap(1);
        QString msgBehind = dirChange.cap(5);

        if ( msg == "Entering directory" ||   // English - default
             msg == "Wechsel in das Verzeichnis Verzeichnis" ||    // German - yes, this is badly translated
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
    static const unsigned short fr_leave[] = { 'Q','u','i','t','t','e',' ','l','e',' ','r',0xe9,'p','e','r','t','o','i','r','e' };
    static const unsigned short ja_leave[] = { 0x51fa,0x307e,0x3059,0x20,0x30c7,0x30a3,0x30ec,0x30af,0x30c8,0x30ea };
    static const unsigned short pt_br_leave[] = {'S','a','i','n','d','o',' ','d','o',' ','d','i','r','e','t',0xf3,'r','i','o'};
    static const unsigned short ru_leave[] = {0x412,0x44b,0x445,0x43e,0x434,0x20,0x438,0x437,0x20,0x43a,0x430,0x442,0x430,0x43b,0x43e,0x433};
    static const unsigned short ko_leave[] = {0xb098,0xac10};
    static const unsigned short ko_behind[] = {0x20,0xb514,0xb809,0xd1a0,0xb9ac};

    static const QString fr_l( (const QChar*)fr_leave, sizeof(fr_leave) / 2 );
    static const QString ja_l( (const QChar*)ja_leave, sizeof(ja_leave) / 2 );
    static const QString ko_l( (const QChar*)ko_leave, sizeof(ko_leave) / 2 );
    static const QString pt_br_l( (const QChar*)pt_br_leave, sizeof(pt_br_leave) / 2 );
    static const QString ru_l( (const QChar*)ru_leave, sizeof(ru_leave) / 2 );

    // we need a QRegExp because KRegExp is not Utf8 aware.
    // 0x00AB is LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    // 0X00BB is RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    static QRegExp dirChange(QString::fromLatin1(".*: (.+) (`|") + QChar(0x00BB) + QString::fromLatin1(")(.*)('|") + QChar(0x00AB) + QString::fromLatin1(")(.*)"));

    if (dirChange.search(line) > -1 ) {
        QString msg = dirChange.cap(1);
        QString msgBehind = dirChange.cap(5);

        if ( msg == "Leaving directory" || // en
             msg == "Verlassen des Verzeichnisses Verzeichnis" || // de
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
    if (matchEnterDir(line, eDir)) {
        QString *dir = new QString(eDir);
        dirstack.push(dir);
        kdDebug(9004) << "Entering dir: " << (*dir).ascii() << endl;
        insertLine2(line, Diagnostic);
        return;
    } else if (matchLeaveDir(line, eDir)) {
        kdDebug(9004) << "Leaving dir: " << eDir << endl;
        QString *dir = dirstack.pop();
        if (!dir) {
          kdWarning(9004) << "Left more directories than entered: " << eDir;
        } else if (dir->compare(eDir) != 0) {
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
    if (errorGccRx.match(line)) {
        hasmatch = true;
        fn = errorGccRx.group(errorGccFileGroup);
        row = QString(errorGccRx.group(errorGccRowGroup)).toInt()-1;
        text = QString(errorGccRx.group(errorGccTextGroup));
    } else if (errorFtnchekRx.match(line)) {
        kdDebug() << "Matching " << line << endl;
        hasmatch = true;
        fn = errorFtnchekRx.group(errorFtnchekFileGroup);
        row = QString(errorFtnchekRx.group(errorFtnchekRowGroup)).toInt()-1;
        text = QString(errorFtnchekRx.group(errorFrnchekTextGroup));
    } else if (errorJadeRx.match(line)) {
        hasmatch = true;
        fn = errorGccRx.group(errorJadeFileGroup);
        row = QString(errorJadeRx.group(errorJadeRowGroup)).toInt()-1;
        text = QString(errorJadeRx.group(errorJadeTextGroup));
    }
    
    if( hasmatch ) {
      // Add hacks for error strings you want excluded here
      if( text.find( QString::fromLatin1("(Each undeclared identifier is reported only once") ) >= 0
       || text.find( QString::fromLatin1("for each function it appears in.)") ) >= 0 )
       hasmatch = false;
    }
    
    if (hasmatch) {
        kdDebug(9004) << "Error in " << fn << " " << row << ": " << text << endl;
        if (dirstack.top())
            fn.prepend("/").prepend(*dirstack.top());
        kdDebug(9004) << "Path: " << fn << endl;
        items.append(new MakeItem(parags, fn, row, text));
        insertLine2(line, Error);
    } else {
        insertLine2(line, type);
    }
}


QString MakeWidget::getOutputColor( Type type )
{
    QString cName;
    int h, s, v;
    bool bright = false;

    // find out whether background is bright or dark
    paletteBackgroundColor().hsv( &h, &s, &v );
    if ( v > 127 )
        bright = true;

    switch ( type ) {
    case Error:
        cName = bright ? "darkRed" : "lightRed";
        break;
    case Diagnostic:
        cName = bright ? "black" : "white";
        break;
    default:
        cName = bright ? "darkBlue" : "lightBlue";
    }

    return cName;
}



void MakeWidget::insertLine2(const QString &line, Type type)
{
    ++parags;

    bool move = textCursor()->parag() == document()->lastParag() && textCursor()->atParagEnd();

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
    
    if (move) {
        moveCursor(MoveEnd, false);
    }

}

#include "makewidget.moc"
