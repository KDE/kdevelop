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
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KDEV_IBREAKPOINTCONTROLLER_H
#define KDEV_IBREAKPOINTCONTROLLER_H

#include <KTextEditor/MarkInterface>

#include "../util/treemodel.h"
#include "../util/treeitem.h"
#include "ibreakpoints.h"

class KUrl;

namespace KParts { class Part; }

namespace KDevelop
{
class IBreakpoints;

class KDEVPLATFORMDEBUGGER_EXPORT IBreakpointController : public TreeModel
{
    Q_OBJECT

public:
    IBreakpointController(QObject* parent, IBreakpoints* universe);
    virtual ~IBreakpointController() {}

    IBreakpoints* breakpointsItem();

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
    
protected:
    enum MarkType {
        BookmarkMark           = KTextEditor::MarkInterface::markType01,
        BreakpointMark         = KTextEditor::MarkInterface::markType02,
        ActiveBreakpointMark   = KTextEditor::MarkInterface::markType03,
        ReachedBreakpointMark  = KTextEditor::MarkInterface::markType04,
        DisabledBreakpointMark = KTextEditor::MarkInterface::markType05,
        ExecutionPointMark     = KTextEditor::MarkInterface::markType06
    };

signals:

    /**
    * The user has toggled a breakpoint.
    */
    void toggledBreakpoint(const QString &fileName, int lineNum);

    /**
    * The user wants to edit the properties of a breakpoint.
    */
    void editedBreakpoint(const QString &fileName, int lineNum);

    /**
    * The user wants to enable/disable a breakpoint.
    */
    void toggledBreakpointEnabled(const QString &fileName, int lineNum);

private slots:

    void slotPartAdded(KParts::Part* part);

    /**
    * Called by the TextEditor interface when the marks have changed position
    * because the user has added or removed source.
    * In here we figure out if we need to reset the breakpoints due to
    * these source changes.
    */
    void markChanged(KTextEditor::Document *document, KTextEditor::Mark mark, KTextEditor::MarkInterface::MarkChangeAction action);

protected:
    IBreakpoints* universe_;

    static const QPixmap* inactiveBreakpointPixmap();
    static const QPixmap* activeBreakpointPixmap();
    static const QPixmap* reachedBreakpointPixmap();
    static const QPixmap* disabledBreakpointPixmap();
    static const QPixmap* executionPointPixmap();
};


}

#endif
