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

#ifndef KDEVPLATFORM_BREAKPOINT_H
#define KDEVPLATFORM_BREAKPOINT_H

#include <QtCore/QSet>
#include <KDE/KUrl>
#include "../util/treeitem.h"

class KConfigGroup;
namespace KTextEditor {
class MovingCursor;
}
namespace KDevelop
{
class BreakpointModel;

class KDEVPLATFORMDEBUGGER_EXPORT Breakpoint
{
public:
    enum BreakpointKind {
        CodeBreakpoint = 0,
        WriteBreakpoint,
        ReadBreakpoint,
        AccessBreakpoint,
        LastBreakpointKind
    };
    enum BreakpointState {
        NotStartedState,
        DirtyState,
        PendingState,
        CleanState
    };

    Breakpoint(BreakpointModel *model, BreakpointKind kind);
    Breakpoint(BreakpointModel *model, const KConfigGroup& config);

    ///Note: EnableColumn has 3, not 2(true and false) values: Qt::Unchecked, Qt:PartiallyChecked and Qt::Checked
    bool setData(int index, const QVariant& value);
    void setDeleted();

    QVariant data(int column, int role) const;

    void save(KConfigGroup& config);

    enum Column {
        EnableColumn,
        StateColumn,
        TypeColumn,
        LocationColumn,
        ConditionColumn,
        HitCountColumn,
        IgnoreHitsColumn
    };

    void setUrl(const KUrl &url);
    KUrl url() const;
    
    void setLine(int line);
    int line() const;
    
    void setLocation(const KUrl& url, int line);
    QString location();

    BreakpointKind kind() const;

    void setAddress(const QString& address);
    QString address() const;

    int hitCount() const;

    bool deleted() const;
    
    bool enabled() const;
    
    void setMovingCursor(KTextEditor::MovingCursor *cursor);
    KTextEditor::MovingCursor *movingCursor() const;

    void setIgnoreHits(int c);
    int ignoreHits() const;

    void setCondition(const QString &c);
    QString condition() const;

    void setExpression(const QString &c);
    QString expression() const;

    BreakpointState state() const;
    QString errorText() const;
    QSet<Column> errors() const;

protected:
    friend class IBreakpointController;
    
    BreakpointModel *breakpointModel();

    BreakpointModel *m_model;
    bool enabled_;
    bool deleted_;
    BreakpointKind kind_;
    /* For watchpoints, the address it is set at.  */
    QString address_;
    KUrl m_url;
    int m_line;
    QString m_condition;
    KTextEditor::MovingCursor *m_movingCursor;
    int m_ignoreHits;
    QString m_expression;

    static const char *string_kinds[LastBreakpointKind];

    void reportChange(Column c);
};

}
#endif
