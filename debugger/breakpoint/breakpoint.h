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

#ifndef KDEV_BREAKPOINT_H
#define KDEV_BREAKPOINT_H

#include <QtCore/QSet>
#include "../util/treeitem.h"

class KConfigGroup;

namespace KDevelop
{

class KDEVPLATFORMDEBUGGER_EXPORT Breakpoint : public TreeItem
{
public:
    enum BreakpointKind {
        CodeBreakpoint = 0,
        WriteBreakpoint,
        ReadBreakpoint,
        AccessBreakpoint,
        LastBreakpointKind
    };

    Breakpoint(TreeModel *model, TreeItem *parent, BreakpointKind kind);
    Breakpoint(TreeModel *model, TreeItem *parent, const KConfigGroup& config);

    /** This constructor creates a "please enter location" item, that will
       turn into real breakpoint when user types something.  */
    Breakpoint(TreeModel *model, TreeItem *parent);

    int id() const { return id_; }
    void setId(int id);
    void fetchMoreChildren() {}

    /** Mark this breakpoint as no longer inserted, due to GDB
       no longer running.  */
    void markOut();

    void setColumn(int index, const QVariant& value);
    void setDeleted();

    int hitCount() const { return hitCount_; }

    QVariant data(int column, int role) const;

    bool pending() const { return pending_; }
    bool dirty() const { return !dirty_.empty(); }
    
    void save(KConfigGroup& config);

    static const int EnableColumn = 0;
    static const int StateColumn = 1;
    static const int TypeColumn = 2;
    static const int LocationColumn = 3;
    static const int ConditionColumn = 4;
    void sendMaybe() {
        //TODO NIKO
    }

    void setLocation(const QString& location);
    QString location();
    BreakpointKind kind() const;

    void setAddress(const QString& address);
    QString address();

    void setPending(bool pending);
    bool pending();

    void setHitCount(int hitCount);
    bool hitCount();

    bool pleaseEnterLocation();

    bool deleted();
protected:
    friend class Breakpoints;

    int id_;
    bool enabled_;
    QSet<int> dirty_;
    QSet<int> errors_;
    bool deleted_;
    int hitCount_;
    BreakpointKind kind_;
    /* The GDB 'pending' flag.  */
    bool pending_;
    /* For watchpoints, the address it is set at.  */
    QString address_;
    bool pleaseEnterLocation_;

    static const char *string_kinds[LastBreakpointKind];
};

}
#endif
