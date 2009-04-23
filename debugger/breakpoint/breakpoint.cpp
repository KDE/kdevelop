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

#include <KLocale>
#include <KIcon>
#include <KConfigGroup>
#include <KDebug>

#include "breakpointmodel.h"
#include "breakpoints.h"

using namespace KDevelop;

Breakpoint::Breakpoint(BreakpointModel *model, TreeItem *parent, BreakpointKind kind)
: TreeItem(model, parent), id_(-1), enabled_(true), 
  deleted_(false), hitCount_(0), kind_(kind),
  pending_(false), pleaseEnterLocation_(false)
{
    setData(QVector<QVariant>() << QString() << QString() << QString() << QString() << QString());
}

Breakpoint::Breakpoint(BreakpointModel *model, TreeItem *parent,
                             const KConfigGroup& config)
: TreeItem(model, parent), id_(-1), enabled_(true),
  deleted_(false), hitCount_(0),
  pending_(false), pleaseEnterLocation_(false)
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

    dirty_.insert(LocationColumn);

    setData(QVector<QVariant>() << QString() << QString() << QString() << location << condition);
}

Breakpoint::Breakpoint(BreakpointModel *model, TreeItem *parent)
: TreeItem(model, parent), id_(-1), enabled_(true), 
  deleted_(false), hitCount_(0), 
  kind_(CodeBreakpoint), pending_(false), pleaseEnterLocation_(true)
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
        itemData[index] = value;
        if (pleaseEnterLocation_)
        {
            pleaseEnterLocation_ = false;
            static_cast<Breakpoints*>(parentItem)->createHelperBreakpoint();
        }
    }

    dirty_.insert(index);
    errors_.remove(index);

    reportChange(index);
}

void Breakpoint::markOut()
{
    id_ = -1;
    dirty_.insert(LocationColumn);
    dirty_.insert(ConditionColumn);
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
        if (role == Qt::DecorationRole)
        {
            if (dirty_.empty())
            {
                if (pending_)
                    return KIcon("help-contents");            
                return KIcon("dialog-apply");
            }
            else
                return KIcon("system-switch-user");
        }
        else if (role == Qt::DisplayRole)
            return "";
        else
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

    if (column == LocationColumn && role == Qt::DisplayRole
        && !address_.isEmpty())
        return QString("%1 (%2)").arg(itemData[LocationColumn].toString())
            .arg(address_);

    return TreeItem::data(column, role);
}

void Breakpoint::setDeleted()
{
    kDebug();
    deleted_ = true;
    breakpointModel()->emitBreakpointDeleted(this);
    removeSelf();
    //TODO actually delete the breakpoint after all debug engines have processed it
}

void Breakpoint::setLocation(const QString& location)
{
    setColumn(LocationColumn, location);
}

QString KDevelop::Breakpoint::location()
{
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

void Breakpoint::setId(int id)
{
    id_ = id;
}

void Breakpoint::setAddress(const QString& address)
{
    address_ = address;
    reportChange();
}

QString Breakpoint::address()
{
    return address_;
}

void Breakpoint::setPending(bool pending)
{
    pending_ = pending;
    reportChange();
}

bool Breakpoint::pending()
{
    return pending_;
}

void Breakpoint::setHitCount(int hitCount)
{
    hitCount_ = hitCount;
    reportChange();
}

bool Breakpoint::hitCount()
{
    return hitCount_;
}

bool Breakpoint::pleaseEnterLocation()
{
    return pleaseEnterLocation_;
}

bool Breakpoint::deleted()
{
    return deleted_;
}

bool Breakpoint::enabled()
{
    return data(EnableColumn, Qt::CheckStateRole).toBool();
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
