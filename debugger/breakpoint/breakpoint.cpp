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
#include "../../interfaces/icore.h"
#include "../../interfaces/idebugcontroller.h"
#include "../interfaces/idebugsession.h"
#include "../interfaces/ibreakpointcontroller.h"

using namespace KDevelop;

Breakpoint::Breakpoint(BreakpointModel *model, BreakpointKind kind)
: m_model(model), enabled_(true),
  deleted_(false), kind_(kind),
  m_line(-1),
  m_movingCursor(0), m_ignoreHits(0)
{
}

Breakpoint::Breakpoint(BreakpointModel *model, const KConfigGroup& config)
: m_model(model), deleted_(false), m_line(-1), m_movingCursor(0), m_ignoreHits(0)
{
    QString kindString = config.readEntry("kind", "");
    int i;
    for (i = 0; i < LastBreakpointKind; ++i) {
        if (string_kinds[i] == kindString)
        {
            kind_ = (BreakpointKind)i;
            break;
        }
    }
    //FIXME: maybe, should silently ignore this breakpoint.
    Q_ASSERT(i < LastBreakpointKind);
    enabled_ = config.readEntry("enabled", false);
    m_url = config.readEntry("url", KUrl());
    m_line = config.readEntry("line", -1);
    m_expression = config.readEntry("expression", QString());
    setCondition(config.readEntry("condition", ""));
    setIgnoreHits(config.readEntry("ignoreHits", 0));
    
}

BreakpointModel *Breakpoint::breakpointModel()
{
    return m_model;
}

bool Breakpoint::setData(int index, const QVariant& value)
{
    if (index == EnableColumn)
    {
        enabled_ = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;
    }

    if (index == LocationColumn || index == ConditionColumn)
    {
        QString s = value.toString();
        if (index == LocationColumn) {
            QRegExp rx("^(.+):([0-9]+)$");
            int idx = rx.indexIn(s);
            if (kind_ == CodeBreakpoint && idx != -1) {
                m_url = KUrl(rx.cap(1));
                m_line = rx.cap(2).toInt() - 1;
                m_expression.clear();
            } else {
                m_expression = s;
                m_url.clear();
                m_line = -1;
            }
        } else {
            m_condition = s;
        }
    }

    reportChange(static_cast<Column>(index));

    return true;
}

QVariant Breakpoint::data(int column, int role) const
{
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
        if (role == Qt::DecorationRole) {
            if (!errors().isEmpty()) {
                return KIcon("dialog-warning");
            }
            switch (state()) {
                case NotStartedState:
                    return QVariant();
                case DirtyState:
                    return KIcon("system-switch-user");
                case PendingState:
                    return KIcon("help-contents");
                case CleanState:
                    return KIcon("dialog-ok-apply");
            }
        } else if (role == Qt::ToolTipRole) {
            if (!errors().isEmpty()) {
                return i18nc("@info:tooltip", "error");
            }
            switch (state()) {
                case NotStartedState:
                    return "";
                case DirtyState:
                    return i18nc("@info:tooltip", "dirty");
                case PendingState:
                    return i18nc("@info:tooltip", "pending");
                case CleanState:
                    return i18nc("@info:tooltip", "clean");
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
        if ((column == LocationColumn && errors().contains(LocationColumn))
            || (column == ConditionColumn && errors().contains(ConditionColumn)))
        {
            /* FIXME: does this leak? Is this efficient? */
            return KIcon("dialog-warning");
        }
        return QVariant();
    }

    if (column == ConditionColumn && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        return m_condition;
    }

    if (column == LocationColumn) {
        if (role == LocationRole || role == Qt::EditRole || role == Qt::ToolTipRole || role == Qt::DisplayRole) {
            QString ret;
            if (kind_ == CodeBreakpoint && m_line != -1) {
                if (role == Qt::DisplayRole) {
                    ret = m_url.fileName();
                } else {
                    ret = m_url.pathOrUrl(KUrl::RemoveTrailingSlash);
                }
                ret += ':' + QString::number(m_line+1);
            } else {
                ret = m_expression;
            }
            //FIXME: there should be proper columns for function name and address.
            if (!address_.isEmpty() && role == Qt::DisplayRole) {
                ret = QString("%1 (%2)").arg(ret).arg(address_);
            }
            return ret;
        }
    }

    return QVariant();
}

void Breakpoint::setDeleted()
{
    deleted_ = true;
    BreakpointModel* m = breakpointModel();
    if (!m)
        return; // already removed

    if (m->breakpointIndex(this, 0).isValid()) {
        m->removeRow(m->breakpointIndex(this, 0).row());
    }
    m_model = 0; // invalidate
}

int Breakpoint::line() const {
    return m_line;
}
void Breakpoint::setLine(int line) {
    Q_ASSERT(kind_ == CodeBreakpoint);
    m_line = line;
    reportChange(LocationColumn);
}
void Breakpoint::setUrl(const KUrl& url) {
    Q_ASSERT(kind_ == CodeBreakpoint);
    m_url = url;
    reportChange(LocationColumn);
}
KUrl Breakpoint::url() const {
    return m_url;
}
void Breakpoint::setLocation(const KUrl& url, int line)
{
    Q_ASSERT(kind_ == CodeBreakpoint);
    m_url = url;
    m_line = line;
    reportChange(LocationColumn);
}

QString KDevelop::Breakpoint::location() {
    return data(LocationColumn, LocationRole).toString();
}


void Breakpoint::save(KConfigGroup& config)
{
    config.writeEntry("kind", string_kinds[kind_]);
    config.writeEntry("enabled", enabled_);
    config.writeEntry("url", m_url);
    config.writeEntry("line", m_line);
    config.writeEntry("expression", m_expression);
    config.writeEntry("condition", m_condition);
    config.writeEntry("ignoreHits", m_ignoreHits);
}

Breakpoint::BreakpointKind Breakpoint::kind() const
{
    return kind_;
}

void Breakpoint::setAddress(const QString& address)
{
    address_ = address;
    //reportChange();
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
        return -1;
    }
}

bool Breakpoint::deleted() const
{
    return deleted_;
}

bool Breakpoint::enabled() const
{
    return data(EnableColumn, Qt::CheckStateRole).toBool();
}

void KDevelop::Breakpoint::setMovingCursor(KTextEditor::MovingCursor* cursor) {
    m_movingCursor = cursor;
}
KTextEditor::MovingCursor* KDevelop::Breakpoint::movingCursor() const {
    return m_movingCursor;
}

void Breakpoint::setIgnoreHits(int c)
{
    if (m_ignoreHits != c) {
        m_ignoreHits = c;
        reportChange(IgnoreHitsColumn);
    }
}

int Breakpoint::ignoreHits() const
{
    return m_ignoreHits;
}


void Breakpoint::setCondition(const QString& c)
{
    m_condition = c;
    reportChange(ConditionColumn);
}

QString Breakpoint::condition() const
{
    return m_condition;
}

void Breakpoint::setExpression(const QString& e)
{
    m_expression = e;
    reportChange(LocationColumn);
}

QString Breakpoint::expression() const
{
    return m_expression;
}

Breakpoint::BreakpointState Breakpoint::state() const
{
    IDebugSession* session = ICore::self()->debugController()->currentSession();
    if (session) {
        return session->breakpointController()->breakpointState(this);
    } else {
        return NotStartedState;
    }
}

QSet<Breakpoint::Column> Breakpoint::errors() const
{
    IDebugSession* session = ICore::self()->debugController()->currentSession();
    if (session) {
        return session->breakpointController()->breakpointErrors(this);
    } else {
        return QSet<Breakpoint::Column>();
    }

}

QString Breakpoint::errorText() const
{
    IDebugSession* session = ICore::self()->debugController()->currentSession();
    if (session) {
        return session->breakpointController()->breakpointErrorText(this);
    } else {
        return QString();
    }
}

void KDevelop::Breakpoint::reportChange(Column c)
{
    if (!breakpointModel())
        return;

    breakpointModel()->reportChange(this, c);
}

const char *Breakpoint::string_kinds[LastBreakpointKind] = {
    "Code",
    "Write",
    "Read",
    "Access"
};
