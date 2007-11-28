/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef BREAKPOINTCONTROLLER_H
#define BREAKPOINTCONTROLLER_H

#include <QList>

#include <KUrl>

#include <ktexteditor/markinterface.h>

namespace KDevelop { class IDocument; }

/**
* Describes a single breakpoint in the system*
* This is used so that we can track the breakpoints and move them appropriately
* as the user adds or removes lines of code before breakpoints.
*/

class BPItem
{
public:
    /**
    * default ctor - required from QValueList
    */
    BPItem() : m_fileName(""), m_lineNum(0)
    {}

    BPItem( const QString& fileName, const int lineNum)
            : m_fileName(fileName),
            m_lineNum(lineNum)
    {}

    int lineNum() const        {  return m_lineNum; }
    QString fileName() const    {  return m_fileName; }

    bool operator==( const BPItem& rhs ) const
    {
        return (m_fileName == rhs.m_fileName
                && m_lineNum == rhs.m_lineNum);
    }

private:
    QString m_fileName;
    int m_lineNum;
};


/**
* Handles signals from the editor that relate to breakpoints and the execution
* point of the debugger.
* We may change, add or remove breakpoints in this class.
*/
class BreakpointController : public QObject
{
    Q_OBJECT

public:
    BreakpointController(QObject* parent);
    ~BreakpointController();

    /**
    * Controls the breakpoint icon being displayed in the editor through the
    * markinterface
    *
    * @param fileName   The breakpoint is added or removed from this file
    * @param lineNum    ... at this line number
    * @param id         This is an internal id. which has a special number
    *                   that prevents us changing the mark icon. (why?)
    * @param enabled    The breakpoint could be enabled, disabled
    * @param pending    pending or active. Each state has a different icon.
    */
    void setBreakpoint(const QString &fileName, int lineNum,
                       int id, bool enabled, bool pending);

    /**
    * Displays an icon in the file at the line that the debugger has stoped
    * at.
    * @param url        The file the debugger has stopped at.
    * @param lineNum    The line number to display. Note: We may not know it.
    */
    void gotoExecutionPoint(const KUrl &url, int lineNum=-1);

    /**
    * Remove the executution point being displayed.
    */
    void clearExecutionPoint();

signals:

  /**
   * The user has toggled a breakpoint.
   */
  void toggledBreakpoint(const QString &fileName, int lineNum);

  /*
   * The user wants to edit the properties of a breakpoint.
   */
  void editedBreakpoint(const QString &fileName, int lineNum);
  
  /**
   * The user wants to enable/disable a breakpoint.
   */
  void toggledBreakpointEnabled(const QString &fileName, int lineNum);

private slots:

    /**
    * Whenever a new document is added this slot gets triggered and we then
    * look for a MarkInterfaceExtension part. When it is a
    * MarkInterfaceExtension part we set the various pixmaps of the
    * breakpoint icons.
    */
    void documentLoaded( KDevelop::IDocument* document );

    /**
    * Called by the TextEditor interface when the marks have changed position
    * because the user has added or removed source.
    * In here we figure out if we need to reset the breakpoints due to
    * these source changes.
    */
    void markChanged(KTextEditor::Document *document, KTextEditor::Mark mark, KTextEditor::MarkInterface::MarkChangeAction action);

private:
    static const QPixmap* inactiveBreakpointPixmap();
    static const QPixmap* activeBreakpointPixmap();
    static const QPixmap* reachedBreakpointPixmap();
    static const QPixmap* disabledBreakpointPixmap();
    static const QPixmap* executionPointPixmap();

    enum MarkType {
        Bookmark           = KTextEditor::MarkInterface::markType01,
        Breakpoint         = KTextEditor::MarkInterface::markType02,
        ActiveBreakpoint   = KTextEditor::MarkInterface::markType03,
        ReachedBreakpoint  = KTextEditor::MarkInterface::markType04,
        DisabledBreakpoint = KTextEditor::MarkInterface::markType05,
        ExecutionPoint     = KTextEditor::MarkInterface::markType06
    };

    QList<BPItem> BPList;
};

#endif
