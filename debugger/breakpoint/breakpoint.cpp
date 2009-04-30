/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2006, 2008 Vladimir Prus <ghost@cs.msu.su>
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

#include "breakpoint.h"

#include <KDE/KLocale>
#include <KDE/KIcon>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>

#include "breakpointmodel.h"
#include "breakpoints.h"
#include "../../interfaces/icore.h"
#include "../../interfaces/idebugcontroller.h"
#include "../interfaces/idebugsession.h"
#include "../interfaces/ibreakpointcontroller.h"

using namespace KDevelop;

Breakpoint::Breakpoint(BreakpointModel *model, TreeItem *parent, BreakpointKind kind)
: TreeItem(model, parent), enabled_(true), 
  deleted_(false), kind_(kind),
  pleaseEnterLocation_(false), m_line(-1),
  m_smartCursor(0)
{
    setData(QVector<QVariant>() << QString() << QString() << QString() << QString() << QString());
}

Breakpoint::Breakpoint(BreakpointModel *model, TreeItem *parent,
                             const KConfigGroup& config)
: TreeItem(model, parent), enabled_(true),
  deleted_(false),
  pleaseEnterLocation_(false), m_line(-1),
  m_smartCursor(0)
{
    QString kindString = config.readEntry("kind", "");
    int i;
    for (i = 0; i < LastBreakpointKind; ++i)
        if (string_kinds[i] == kindString)
        {
            kind_ = (BreakpointKind)i;
            break;
        }
    /* FIXME: maybe, should silently ignore this breakpoint.  */
    Q_ASSERT(i < LastBreakpointKind);
    enabled_ = config.readEntry("enabled", false);

    QString location = config.readEntry("location", "");
    QString condition = config.readEntry("condition", "");

    setData(QVector<QVariant>() << QString() << QString() << QString() << location << condition);
}

Breakpoint::Breakpoint(BreakpointModel *model, TreeItem *parent)
: TreeItem(model, parent), enabled_(true), 
  deleted_(false),
  kind_(CodeBreakpoint), pleaseEnterLocation_(true), m_line(-1),
  m_smartCursor(0)
{   
    setData(QVector<QVariant>() << QString() << QString() << QString() << QString() << QString());
}

BreakpointModel *Breakpoint::breakpointModel()
{
    return static_cast<BreakpointModel*>(model_);
}

void Breakpoint::setColumn(int index, const QVariant& value)
{
    if (index == EnableColumn)
    {
        enabled_ = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;
    }

    /* Helper breakpoint becomes a real breakpoint only if user types
       some real location.  */
    if (pleaseEnterLocation_ && value.toString().isEmpty())
        return;

    if (index == LocationColumn || index == ConditionColumn)
    {
        if (index == LocationColumn) {
            QString s = value.toString();
            m_url = KUrl(s.left(s.lastIndexOf(':')));
            m_line = s.right(s.length() - s.lastIndexOf(':') - 1).toInt() - 1;
        } else {
            itemData[index] = value;
        }
        if (pleaseEnterLocation_) {
            pleaseEnterLocation_ = false;
            static_cast<Breakpoints*>(parentItem)->createHelperBreakpoint();
        }
    }
    
    errors_.remove(index);

    reportChange(index);
}

QVariant Breakpoint::data(int column, int role) const
{
    if (pleaseEnterLocation_)
    {
        if (column != LocationColumn)
        {
            if (role == Qt::DisplayRole)
                return QString();
            else
                return QVariant();
        }
        
        if (role == Qt::DisplayRole)
            return i18n("Double-click to create new code breakpoint");
        if (role == Qt::ForegroundRole)
            // FIXME: returning hardcoded gray is bad,
            // but we don't have access to any widget, or pallette
            // thereof, at this point.
            return QColor(128, 128, 128);
        if (role == Qt::EditRole)
            return QString();
    }

    if (column == EnableColumn)
    {
        if (role == Qt::CheckStateRole)
            return enabled_ ? Qt::Checked : Qt::Unchecked;
        else if (role == Qt::DisplayRole)
            return "";
        else
            return QVariant();
    }

    if (column == StateColumn)
    {
        IDebugSession* session = ICore::self()->debugController()->currentSession();
        IBreakpointController::BreakpointState state;
        if (session) {
            state = session->breakpointController()->breakpointState(this);
        } else {
            state = IBreakpointController::DirtyState;
        }
        if (role == Qt::DecorationRole) {
            switch (state) {
                case IBreakpointController::DirtyState:
                    return KIcon("system-switch-user");
                case IBreakpointController::PendingState:
                    return KIcon("help-contents");
                case IBreakpointController::CleanState:
                    return KIcon("dialog-apply");
            }
        } else if (role == Qt::ToolTipRole) {
            switch (state) {
                case IBreakpointController::DirtyState:
                    return i18n("dirty");
                case IBreakpointController::PendingState:
                    return i18n("pending");
                case IBreakpointController::CleanState:
                    return i18n("clean");
            }
        } else if (role == Qt::DisplayRole) {
            return "";
        }
        return QVariant();
    }

    if (column == TypeColumn && role == Qt::DisplayRole)
    {
        return string_kinds[kind_];
    }

    if (role == Qt::DecorationRole)
    {
        if ((column == LocationColumn && errors_.contains(LocationColumn))
            || (column == ConditionColumn && errors_.contains(ConditionColumn)))
        {
            /* FIXME: does this leak? Is this efficient? */
            return KIcon("dialog-warning");
        }
        return QVariant();
    }

    if (column == LocationColumn && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        QString ret = m_url.toLocalFile(KUrl::RemoveTrailingSlash);
        ret += ':' + QString::number(m_line+1);
        if (!address_.isEmpty() && role == Qt::DisplayRole) {
            ret = QString("%1 (%2)").arg(ret).arg(address_);
        }
        return ret;
    }

    return TreeItem::data(column, role);
}

void Breakpoint::setDeleted()
{
    kDebug();
    deleted_ = true;
    breakpointModel()->_breakpointDeleted(this);
    removeSelf();
    //TODO actually delete the breakpoint after all debug engines have processed it
}

int Breakpoint::line() const {
    return m_line;
}
void Breakpoint::setLine(int line) {
    m_line = line;
    reportChange(LocationColumn);
}
void Breakpoint::setUrl(const KUrl& url) {
    m_url = url;
    reportChange(LocationColumn);
}
KUrl Breakpoint::url() const {
    return m_url;
}
void Breakpoint::setLocation(const KUrl& url, int line)
{
    m_url = url;
    m_line = line;
    reportChange(LocationColumn);
}

QString KDevelop::Breakpoint::location() {
    return data(LocationColumn, Qt::DisplayRole).toString();
}


void Breakpoint::save(KConfigGroup& config)
{
    config.writeEntry("kind", string_kinds[kind_]);
    config.writeEntry("enabled", enabled_);
    config.writeEntry("location", itemData[LocationColumn]);
    config.writeEntry("condition", itemData[ConditionColumn]);
}

Breakpoint::BreakpointKind Breakpoint::kind() const
{
    return kind_;
}

void Breakpoint::setAddress(const QString& address)
{
    address_ = address;
    reportChange();
}

QString Breakpoint::address() const
{
    return address_;
}

int Breakpoint::hitCount() const
{
    IDebugSession* session = ICore::self()->debugController()->currentSession();
    if (session) {
        return session->breakpointController()->breakpointHitCount(this);
    } else {
        return 0;
    }
}

bool Breakpoint::pleaseEnterLocation() const
{
    return pleaseEnterLocation_;
}

bool Breakpoint::deleted() const
{
    return deleted_;
}

bool Breakpoint::enabled() const
{
    return data(EnableColumn, Qt::CheckStateRole).toBool();
}

void KDevelop::Breakpoint::setSmartCursor(KTextEditor::SmartCursor* cursor) {
    m_smartCursor = cursor;
}
KTextEditor::SmartCursor* KDevelop::Breakpoint::smartCursor() const {
    return m_smartCursor;
}



const int Breakpoint::EnableColumn;
const int Breakpoint::StateColumn;
const int Breakpoint::TypeColumn;
const int Breakpoint::LocationColumn;
const int Breakpoint::ConditionColumn;

const char *Breakpoint::string_kinds[LastBreakpointKind] = {
    "Code",
    "Write",
    "Read",
    "Access"
};

#include "breakpoint.moc"
