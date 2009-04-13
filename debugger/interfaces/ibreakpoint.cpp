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
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "ibreakpoint.h"
#include "ibreakpoints.h"
#include <KLocale>
#include <KIcon>
#include <KConfigGroup>

using namespace KDevelop;

IBreakpoint::IBreakpoint(TreeModel *model, TreeItem *parent, kind_t kind)
: TreeItem(model, parent), id_(-1), enabled_(true), 
  deleted_(false), hitCount_(0), kind_(kind),
  pending_(false), pleaseEnterLocation_(false)
{
    setData(QVector<QString>() << "" << "" << "" << "" << "");
}

IBreakpoint::IBreakpoint(TreeModel *model, TreeItem *parent,
                             const KConfigGroup& config)
: TreeItem(model, parent), id_(-1), enabled_(true),
  deleted_(false), hitCount_(0),
  pending_(false), pleaseEnterLocation_(false)
{
    QString kindString = config.readEntry("kind", "");
    int i;
    for (i = 0; i < last_breakpoint_kind; ++i)
        if (string_kinds[i] == kindString)
        {
            kind_ = (kind_t)i;
            break;
        }
    /* FIXME: maybe, should silently ignore this breakpoint.  */
    Q_ASSERT(i < last_breakpoint_kind);
    enabled_ = config.readEntry("enabled", false);

    QString location = config.readEntry("location", "");
    QString condition = config.readEntry("condition", "");

    dirty_.insert(location_column);

    setData(QVector<QString>() << "" << "" << "" << location << condition);
}

IBreakpoint::IBreakpoint(TreeModel *model, TreeItem *parent)
: TreeItem(model, parent), id_(-1), enabled_(true), 
  deleted_(false), hitCount_(0), 
  kind_(code_breakpoint), pending_(false), pleaseEnterLocation_(true)
{   
    setData(QVector<QString>() << "" << "" << "" << "" << "");
}

void IBreakpoint::setColumn(int index, const QVariant& value)
{
    if (index == enable_column)
    {
        enabled_ = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;
    }

    /* Helper breakpoint becomes a real breakpoint only if user types
       some real location.  */
    if (pleaseEnterLocation_ && value.toString().isEmpty())
        return;

    if (index == location_column || index == condition_column)
    {
        itemData[index] = value;
        if (pleaseEnterLocation_)
        {
            pleaseEnterLocation_ = false;
            static_cast<IBreakpoints*>(parentItem)->createHelperBreakpoint();
        }
    }

    dirty_.insert(index);
    errors_.remove(index);
    reportChange();
    sendMaybe();
}

void IBreakpoint::markOut()
{
    id_ = -1;
    dirty_.insert(location_column);
    dirty_.insert(condition_column);
}

QVariant IBreakpoint::data(int column, int role) const
{
    if (pleaseEnterLocation_)
    {
        if (column != location_column)
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

    if (column == enable_column)
    {
        if (role == Qt::CheckStateRole)
            return enabled_ ? Qt::Checked : Qt::Unchecked;
        else if (role == Qt::DisplayRole)
            return "";
        else
            return QVariant();
    }

    if (column == state_column)
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

    if (column == type_column && role == Qt::DisplayRole)
    {
        return string_kinds[kind_];
    }

    if (role == Qt::DecorationRole)
    {
        if ((column == location_column && errors_.contains(location_column))
            || (column == condition_column && errors_.contains(condition_column)))
        {
            /* FIXME: does this leak? Is this efficient? */
            return KIcon("dialog-warning");
        }
        return QVariant();
    }

    if (column == location_column && role == Qt::DisplayRole
        && !address_.isEmpty())
        return QString("%1 (%2)").arg(itemData[location_column].toString())
            .arg(address_);

    return TreeItem::data(column, role);
}

void IBreakpoint::setDeleted()
{
    deleted_ = true;
}

void IBreakpoint::setLocation(const QString& location)
{
    itemData[location_column] = location;
    dirty_.insert(location_column);
    reportChange();
    sendMaybe();
}

void IBreakpoint::save(KConfigGroup& config)
{
    config.writeEntry("kind", string_kinds[kind_]);
    config.writeEntry("enabled", enabled_);
    config.writeEntry("location", itemData[location_column]);
    config.writeEntry("condition", itemData[condition_column]);
}

const int IBreakpoint::enable_column;
const int IBreakpoint::state_column;
const int IBreakpoint::type_column;
const int IBreakpoint::location_column;
const int IBreakpoint::condition_column;

const char *IBreakpoint::string_kinds[last_breakpoint_kind] = {
    "Code",
    "Write",
    "Read",
    "Access"
};
