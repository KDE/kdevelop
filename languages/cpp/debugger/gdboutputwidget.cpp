// *************************************************************************
//                          gdboutputwidget.cpp  -  description
//                             -------------------
//    begin                : 10th April 2003
//    copyright            : (C) 2003 by John Birch
//    email                : jbb@kdevelop.org
// **************************************************************************
//
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#include "gdboutputwidget.h"
#include "dbgcontroller.h"

#include <kcombobox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qdom.h>


namespace GDBDebugger
{

/***************************************************************************/

GDBOutputWidget::GDBOutputWidget( QWidget *parent, const char *name) :
    QWidget(parent, name),
    m_userGDBCmdEditor(0),
    m_Interrupt(0),
    m_gdbView(0),
    showInternalCommands_(false),
    maxLines_(5000)
{

    m_gdbView = new OutputText(this);
    m_gdbView->setTextFormat(QTextEdit::LogText);

    QBoxLayout *userGDBCmdEntry = new QHBoxLayout();
    m_userGDBCmdEditor = new KHistoryCombo (this, "gdb-user-cmd-editor");

    QLabel *label = new QLabel(i18n("&GDB cmd:"), this);
    label->setBuddy(m_userGDBCmdEditor);
    userGDBCmdEntry->addWidget(label);

    userGDBCmdEntry->addWidget(m_userGDBCmdEditor);
    userGDBCmdEntry->setStretchFactor(m_userGDBCmdEditor, 1);

    m_Interrupt = new QToolButton( this, "add breakpoint" );
    m_Interrupt->setSizePolicy ( QSizePolicy ( (QSizePolicy::SizeType)0,
                                         ( QSizePolicy::SizeType)0,
                                         0,
                                         0,
                                         m_Interrupt->sizePolicy().hasHeightForWidth())
                                         );
    m_Interrupt->setPixmap ( SmallIcon ( "player_pause" ) );
    userGDBCmdEntry->addWidget(m_Interrupt);
    QToolTip::add ( m_Interrupt, i18n ( "Pause execution of the app to enter gdb commands" ) );

    QVBoxLayout *topLayout = new QVBoxLayout(this, 2);
    topLayout->addWidget(m_gdbView, 10);
    topLayout->addLayout(userGDBCmdEntry);

    slotDbgStatus( "", s_dbgNotStarted);

    connect( m_userGDBCmdEditor, SIGNAL(returnPressed()), SLOT(slotGDBCmd()) );
    connect( m_Interrupt,        SIGNAL(clicked()),       SIGNAL(breakInto()));

    connect( m_gdbView, SIGNAL(showInternalCommandsChanged(bool)),
             this,      SLOT(slotShowInternalCommandsChanged(bool)));

    connect( &updateTimer_, SIGNAL(timeout()),
             this,  SLOT(flushPending()));
}

/***************************************************************************/

GDBOutputWidget::~GDBOutputWidget()
{
    delete m_gdbView;
    delete m_userGDBCmdEditor;
}

/***************************************************************************/

void GDBOutputWidget::clear()
{
    if (m_gdbView)
        m_gdbView->clear();

    userCommands_.clear();
    allCommands_.clear();
}

/***************************************************************************/

void GDBOutputWidget::slotInternalCommandStdout(const char* line)
{    
    newStdoutLine(line, true);
}

void GDBOutputWidget::slotUserCommandStdout(const char* line)
{
    newStdoutLine(line, false);
}

namespace {
    QString colorify(QString text, const QString& color)
    {
        // Make sure the newline is at the end of the newly-added
        // string. This is so that we can always correctly remove
        // newline inside 'flushPending'.
        Q_ASSERT(text.endsWith("\n"));
        if (text.endsWith("\n"))
        {
            text.remove(text.length()-1, 1);
        }
        text = "<font color=\"" + color +  "\">" + text + "</font>\n";
        return text;
    }
}


void GDBOutputWidget::newStdoutLine(const QString& line,
                                    bool internal)
{
    QString s = html_escape(line);
    if (s.startsWith("(gdb)"))
    {
        s = colorify(s, "blue");
    }

    allCommands_.append(s);
    trimList(allCommands_, maxLines_);

    if (!internal)
    {
        userCommands_.append(s);
        trimList(userCommands_, maxLines_);
    }

    if (!internal || showInternalCommands_)
        showLine(s);                 
}


void GDBOutputWidget::showLine(const QString& line)
{
    pendingOutput_ += line;

    // To improve performance, we update the view after some delay.
    if (!updateTimer_.isActive())
    {
        updateTimer_.start(100, true /* single shot */);
    }
}

void GDBOutputWidget::trimList(QStringList& l, unsigned max_size)
{
    unsigned int length = l.count();
    if (length > max_size)
    {
        for(int to_delete = length - max_size; to_delete; --to_delete)
        {
            l.erase(l.begin());
        }
    }
}

void GDBOutputWidget::setShowInternalCommands(bool show)
{
    if (show != showInternalCommands_)
    {
        showInternalCommands_ = show;
        
        // Set of strings to show changes, text edit still has old
        // set. Refresh.
        m_gdbView->clear();
        QStringList& newList = 
            showInternalCommands_ ? allCommands_ : userCommands_;

        QStringList::iterator i = newList.begin(), e = newList.end();
        for(; i != e; ++i)
        {
            // Note that color formatting is already applied to '*i'.
            showLine(*i);
        }
    }
}

/***************************************************************************/

void GDBOutputWidget::slotReceivedStderr(const char* line)
{
    // Errors are shown inside user commands too.
    allCommands_.append(line);
    trimList(allCommands_, maxLines_);
    userCommands_.append(line);
    trimList(userCommands_, maxLines_);

    showLine(colorify(html_escape(line), "red"));
}

/***************************************************************************/

void GDBOutputWidget::slotGDBCmd()
{
    QString GDBCmd(m_userGDBCmdEditor->currentText());
    if (!GDBCmd.isEmpty())
    {
        m_userGDBCmdEditor->addToHistory(GDBCmd);
        m_userGDBCmdEditor->clearEdit();
        emit userGDBCmd(GDBCmd);
    }
}

void GDBOutputWidget::flushPending()
{
    m_gdbView->setUpdatesEnabled(false);

    // QTextEdit adds newline after paragraph automatically.
    // So, remove trailing newline to avoid double newlines.
    if (pendingOutput_.endsWith("\n"))
        pendingOutput_.remove(pendingOutput_.length()-1, 1);
    Q_ASSERT(!pendingOutput_.endsWith("\n"));

    m_gdbView->append(pendingOutput_);
    pendingOutput_ = "";

    m_gdbView->scrollToBottom();
    m_gdbView->setUpdatesEnabled(true);
    m_gdbView->update();
}

/***************************************************************************/

void GDBOutputWidget::slotDbgStatus(const QString &, int statusFlag)
{
    if (statusFlag & s_dbgNotStarted)
    {
        m_Interrupt->setEnabled(false);
        m_userGDBCmdEditor->setEnabled(false);
        return;
    }
    else
    {
        m_Interrupt->setEnabled(true);
    }

    if (statusFlag & s_dbgBusy)
    {
        m_userGDBCmdEditor->setEnabled(false);
    }
    else
    {
        m_userGDBCmdEditor->setEnabled(true);
    }
}

/***************************************************************************/

void GDBOutputWidget::focusInEvent(QFocusEvent */*e*/)
{
    m_gdbView->scrollToBottom();
    m_userGDBCmdEditor->setFocus();
}

QString GDBOutputWidget::html_escape(const QString& s)
{
    QString r(s);
    r.replace("<", "&lt;");
    r.replace(">", "&gt;");
    return r;
}

void GDBOutputWidget::savePartialProjectSession(QDomElement* el)
{
    QDomDocument doc = el->ownerDocument();

    QDomElement showInternal = doc.createElement("showInternalCommands");
    showInternal.setAttribute("value", QString::number(showInternalCommands_));

    el->appendChild(showInternal);
}

void GDBOutputWidget::restorePartialProjectSession(const QDomElement* el)
{
    QDomElement showInternal = 
        el->namedItem("showInternalCommands").toElement();

    if (!showInternal.isNull())
    {
        showInternalCommands_ = showInternal.attribute("value", "0").toInt();
    }
}


//void OutputText::contextMenuEvent(QContextMenuEvent* e)
QPopupMenu* OutputText::createPopupMenu(const QPoint&)
{
    KPopupMenu* popup = new KPopupMenu;

    int id = popup->insertItem(i18n("Show Internal Commands"),
                               this,
                               SLOT(toggleShowInternalCommands()));

    popup->setItemChecked(id, parent_->showInternalCommands_);
    popup->setWhatsThis(
        id, 
        i18n(
            "Controls if commands issued internally by KDevelop "
            "will be shown or not.<br>"
            "This option will affect only future commands, it won't "
            "add or remove already issued commands from the view."));

    popup->insertItem(i18n("Copy All"),
                      this,
                      SLOT(copyAll()));


    return popup;
}

void OutputText::copyAll()
{
    // Make sure the text is pastable both with Ctrl-C and with
    // middle click.
    QApplication::clipboard()->setText(text(), QClipboard::Clipboard);
    QApplication::clipboard()->setText(text(), QClipboard::Selection);
}

void OutputText::toggleShowInternalCommands()
{
    parent_->setShowInternalCommands(!parent_->showInternalCommands_);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
}


#include "gdboutputwidget.moc"

