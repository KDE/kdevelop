/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2007 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KDEV_BREAKPOINTMODEL_H
#define KDEV_BREAKPOINTMODEL_H

#include <KDE/KTextEditor/MarkInterface>

#include "../util/treemodel.h"
#include "../util/treeitem.h"
#include "breakpoints.h"

class KUrl;

namespace KParts { class Part; }
namespace KTextEditor {
class Cursor;
}

namespace KDevelop
{
class IDocument;
class Breakpoints;
class Breakpoint;

class KDEVPLATFORMDEBUGGER_EXPORT BreakpointModel : public TreeModel
{
    Q_OBJECT

public:
    BreakpointModel(QObject* parent);
    virtual ~BreakpointModel() {}

    Breakpoints* breakpointsItem();

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    enum Columns {
        Enable,
        Type,
        Status,
        Location,
        Condition,
        IgnoreCount,
        Hits,
        Tracing,
        Last = Tracing
    };
    
    void toggleBreakpoint(const KUrl &url, const KTextEditor::Cursor& cursor);
    void toggleBreakpoint(const QString &filename, int lineNum);

protected:
    enum MarkType {
        BookmarkMark           = KTextEditor::MarkInterface::markType01,
        BreakpointMark         = KTextEditor::MarkInterface::markType02,
        ActiveBreakpointMark   = KTextEditor::MarkInterface::markType03,
        ReachedBreakpointMark  = KTextEditor::MarkInterface::markType04,
        DisabledBreakpointMark = KTextEditor::MarkInterface::markType05,
        ExecutionPointMark     = KTextEditor::MarkInterface::markType06
    };

Q_SIGNALS:
    /**
     * A breakpoint has been deleted by the user. The breakpoint object
     * still exists as is has eventualle be deleted from the debugger engine.
     */
    void breakpointDeleted(KDevelop::Breakpoint *breakpoint);

private Q_SLOTS:

    void slotPartAdded(KParts::Part* part);

    /**
    * Called by the TextEditor interface when the marks have changed position
    * because the user has added or removed source.
    * In here we figure out if we need to reset the breakpoints due to
    * these source changes.
    */
    void markChanged(KTextEditor::Document *document, KTextEditor::Mark mark, KTextEditor::MarkInterface::MarkChangeAction action);

    void textDocumentCreated(KDevelop::IDocument*);
    
protected:
    Breakpoints* universe_;


    static const QPixmap* inactiveBreakpointPixmap();
    static const QPixmap* activeBreakpointPixmap();
    static const QPixmap* reachedBreakpointPixmap();
    static const QPixmap* disabledBreakpointPixmap();
    static const QPixmap* executionPointPixmap();

private:
    friend class Breakpoint;
    void _breakpointDeleted(Breakpoint *breakpoint);

private Q_SLOTS:
    void updateMarks();
};


}

#endif
