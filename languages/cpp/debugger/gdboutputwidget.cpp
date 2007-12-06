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

#include <kcombobox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>

#include <QLabel>
#include <QLayout>
#include <q3textedit.h>
#include <QToolButton>
#include <QToolTip>
#include <QApplication>
#include <QClipboard>
#include <qdom.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QFocusEvent>
#include <Q3PopupMenu>
#include <Q3VBoxLayout>
#include <kvbox.h>
#include <khistorycombobox.h>
#include <KIcon>

#include "gdbglobal.h"
#include "debuggerplugin.h"

namespace GDBDebugger
{

/***************************************************************************/

GDBOutputWidget::GDBOutputWidget(CppDebuggerPlugin* plugin, GDBController* controller, QWidget *parent) :
    QWidget(parent),
    m_controller(controller),
    m_userGDBCmdEditor(0),
    m_Interrupt(0),
    m_gdbView(0),
    showInternalCommands_(false),
    maxLines_(5000)
{
    setWindowIcon(KIcon("inline_image"));
    setWindowTitle(i18n("GDB Output"));
    setToolTip(i18n("<b>GDB output</b><p>"
                    "Shows all gdb commands being executed. "
                    "You can also issue any other gdb command while debugging."));

    m_gdbView = new OutputText(this);
    m_gdbView->setTextFormat(Qt::LogText);

    Q3BoxLayout *userGDBCmdEntry = new Q3HBoxLayout();
    m_userGDBCmdEditor = new KHistoryComboBox (this);

    QLabel *label = new QLabel(i18n("&GDB cmd:"), this);
    label->setBuddy(m_userGDBCmdEditor);
    userGDBCmdEntry->addWidget(label);

    userGDBCmdEntry->addWidget(m_userGDBCmdEditor);
    userGDBCmdEntry->setStretchFactor(m_userGDBCmdEditor, 1);

    m_Interrupt = new QToolButton( this );
    m_Interrupt->setIcon ( KIcon ( "media-playback-pause" ) );
    userGDBCmdEntry->addWidget(m_Interrupt);
    m_Interrupt->setToolTip( i18n ( "Pause execution of the app to enter gdb commands" ) );

    Q3VBoxLayout *topLayout = new Q3VBoxLayout(this, 2);
    topLayout->addWidget(m_gdbView, 10);
    topLayout->addLayout(userGDBCmdEntry);

    slotStateChanged(s_none, s_dbgNotStarted);

    connect( m_userGDBCmdEditor, SIGNAL(returnPressed()), SLOT(slotGDBCmd()) );
    connect( m_Interrupt,        SIGNAL(clicked()),       SIGNAL(breakInto()));

    connect( &updateTimer_, SIGNAL(timeout()),
             this,  SLOT(flushPending()));

    connect( this,       SIGNAL(userGDBCmd(const QString &)),
             controller, SLOT(slotUserGDBCmd(const QString&)));
    connect( this,       SIGNAL(breakInto()),
             controller, SLOT(slotPauseApp()));

    connect( controller, SIGNAL(gdbInternalCommandStdout(const QString&)),
             this,       SLOT(slotInternalCommandStdout(const QString&)) );
    connect( controller, SIGNAL(gdbUserCommandStdout(const QString&)),
             this,       SLOT(slotUserCommandStdout(const QString&)) );

    connect( controller, SIGNAL(stateChanged(DBGStateFlags, DBGStateFlags)),
             this,       SLOT(slotStateChanged(DBGStateFlags, DBGStateFlags)));

    connect(plugin, SIGNAL(reset()), this, SLOT(clear()));
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

void GDBOutputWidget::slotInternalCommandStdout(const QString& line)
{
    newStdoutLine(line, true);
}

void GDBOutputWidget::slotUserCommandStdout(const QString& line)
{
    newStdoutLine(line, false);
}

namespace {
    QString colorify(QString text, const QString& color)
    {
        // Make sure the newline is at the end of the newly-added
        // string. This is so that we can always correctly remove
        // newline inside 'flushPending'.
        if (!text.endsWith("\n"))
          text.append('\n');

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
    allCommandsRaw_.append(line);
    trimList(allCommands_, maxLines_);
    trimList(allCommandsRaw_, maxLines_);

    if (!internal)
    {
        userCommands_.append(s);
        userCommandsRaw_.append(line);
        trimList(userCommands_, maxLines_);
        trimList(userCommandsRaw_, maxLines_);
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

void GDBOutputWidget::trimList(QStringList& l, int max_size)
{
    int length = l.count();
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
    QString colored = colorify(html_escape(line), "red");
    // Errors are shown inside user commands too.
    allCommands_.append(colored);
    trimList(allCommands_, maxLines_);
    userCommands_.append(colored);
    trimList(userCommands_, maxLines_);

    allCommandsRaw_.append(line);
    trimList(allCommandsRaw_, maxLines_);
    userCommandsRaw_.append(line);
    trimList(userCommandsRaw_, maxLines_);

    showLine(colored);
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
    m_userGDBCmdEditor->setFocus();
}

/***************************************************************************/

void GDBOutputWidget::slotStateChanged(DBGStateFlags oldStatus, DBGStateFlags newStatus)
{
    Q_UNUSED(oldStatus)
    if (newStatus & s_dbgNotStarted)
    {
        m_Interrupt->setEnabled(false);
        m_userGDBCmdEditor->setEnabled(false);
        return;
    }
    else
    {
        m_Interrupt->setEnabled(true);
    }

    if (newStatus & s_dbgBusy)
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


Q3PopupMenu* OutputText::createPopupMenu(const QPoint&)
{
    Q3PopupMenu* popup = new Q3PopupMenu(this);

    QAction* action = popup->addAction(i18n("Show Internal Commands"),
                               this,
                               SLOT(toggleShowInternalCommands()));

    action->setCheckable(true);
    action->setChecked(parent_->showInternalCommands_);
    action->setWhatsThis(i18n(
            "Controls if commands issued internally by KDevelop "
            "will be shown or not.<br>"
            "This option will affect only future commands, it won't "
            "add or remove already issued commands from the view."));

    popup->addAction(i18n("Copy All"),
                      this,
                      SLOT(copyAll()));

    return popup;
}

void OutputText::copyAll()
{
    /* See comments for allCommandRaw_ for explanations of
       this complex logic, as opposed to calling text(). */
    QStringList& raw = parent_->showInternalCommands_ ?
        parent_->allCommandsRaw_ : parent_->userCommandsRaw_;
    QString text;
    for (int i = 0; i < raw.size(); ++i)
        text += raw[i];

    // Make sure the text is pastable both with Ctrl-C and with
    // middle click.
    QApplication::clipboard()->setText(text, QClipboard::Clipboard);
    QApplication::clipboard()->setText(text, QClipboard::Selection);
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

