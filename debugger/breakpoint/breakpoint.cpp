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
  m_smartCursor(0), m_ignoreHits(0)
{
}

Breakpoint::Breakpoint(BreakpointModel *model, const KConfigGroup& config)
: m_model(model), enabled_(true),
  deleted_(false),
  m_line(-1),
  m_smartCursor(0), m_ignoreHits(0)
{
    Q_ASSERT(0);
    /* TODO NIKO
    QString kindString = config.readEntry("kind", "");
    int i;
    for (i = 0; i < LastBreakpointKind; ++i)
        if (string_kinds[i] == kindString)
        {
            kind_ = (BreakpointKind)i;
            break;
        }
    //FIXME: maybe, should silently ignore this breakpoint.
    Q_ASSERT(i < LastBreakpointKind);
    enabled_ = config.readEntry("enabled", false);

    QString location = config.readEntry("location", "");
    QString condition = config.readEntry("condition", "");

    setData(QVector<QVariant>() << QString() << QString() << QString() << location << condition);
    */
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
        if (index == LocationColumn) {
            QString s = value.toString();
            m_url = KUrl(s.left(s.lastIndexOf(':')));
            m_line = s.right(s.length() - s.lastIndexOf(':') - 1).toInt() - 1;
        } else {
            m_condition = value.toString();
        }
    }

    errors_.remove(index);

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
            switch (state()) {
                case DirtyState:
                    return KIcon("system-switch-user");
                case PendingState:
                    return KIcon("help-contents");
                case CleanState:
                    return KIcon("dialog-apply");
            }
        } else if (role == Qt::ToolTipRole) {
            switch (state()) {
                case DirtyState:
                    return i18n("dirty");
                case PendingState:
                    return i18n("pending");
                case CleanState:
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
    } else if (column == ConditionColumn && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        return m_condition;
    }

    return QVariant();
}

void Breakpoint::setDeleted()
{
    deleted_ = true;
    BreakpointModel* m = breakpointModel();
    if (m->breakpointIndex(this, 0).isValid()) {
        m->removeRow(m->breakpointIndex(this, 0).row());
    }
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
    config.writeEntry("url", m_url);
    config.writeEntry("line", m_line);
    config.writeEntry("condition", m_condition);
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
        return 0;
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

void KDevelop::Breakpoint::setSmartCursor(KTextEditor::SmartCursor* cursor) {
    m_smartCursor = cursor;
}
KTextEditor::SmartCursor* KDevelop::Breakpoint::smartCursor() const {
    return m_smartCursor;
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


void KDevelop::Breakpoint::setCondition(const QString& c)
{
    m_condition = c;
    reportChange(ConditionColumn);
}

QString KDevelop::Breakpoint::condition()
{
    return m_condition;
}

Breakpoint::BreakpointState Breakpoint::state() const
{
    IDebugSession* session = ICore::self()->debugController()->currentSession();
    if (session) {
        return session->breakpointController()->breakpointState(this);
    } else {
        return DirtyState;
    }
}


void KDevelop::Breakpoint::reportChange(Column c)
{
    breakpointModel()->reportChange(this, c);
}

const char *Breakpoint::string_kinds[LastBreakpointKind] = {
    "Code",
    "Write",
    "Read",
    "Access"
};
