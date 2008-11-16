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

#include "newbreakpoint.h"
#include "breakpoints.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"

#include "mi/gdbmi.h"

#include <KConfigGroup>
#include <KLocale>
#include <KIcon>

using namespace GDBMI;

namespace GDBDebugger
{

NewBreakpoint::NewBreakpoint(TreeModel *model, TreeItem *parent, 
                             GDBController* controller, kind_t kind)
: TreeItem(model, parent), id_(-1), enabled_(true), 
  controller_(controller), deleted_(false), hitCount_(0), kind_(kind),
  pending_(false), pleaseEnterLocation_(false)
{
    setData(QVector<QString>() << "" << "" << "" << "" << "");
}

NewBreakpoint::NewBreakpoint(TreeModel *model, TreeItem *parent, 
                             GDBController* controller,
                             const KConfigGroup& config)
: TreeItem(model, parent), id_(-1), enabled_(true), 
  controller_(controller), deleted_(false), hitCount_(0),
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

NewBreakpoint::NewBreakpoint(TreeModel *model, TreeItem *parent,
                             GDBController* controller)
: TreeItem(model, parent), id_(-1), enabled_(true), 
  controller_(controller), deleted_(false), hitCount_(0), 
  kind_(code_breakpoint), pending_(false), pleaseEnterLocation_(true)
{   
    setData(QVector<QString>() << "" << "" << "" << "" << "");
}

void NewBreakpoint::update(const GDBMI::Value &b)
{
    id_ = b["number"].toInt();
    
    QString type = b["type"].literal();
    const char *code_breakpoints[] = {
        "breakpoint", "hw breakpoint", "until", "finish"};

    if (b.hasField("original-location"))
    {
        if (address_.isEmpty())
        {
            /* If the address is not empty, it means that the breakpoint
               is set by KDevelop, not by the user, and that we want to
               show the original expression, not the address, in the table.
               TODO: this also means that if used added a watchpoint in gdb
               like "watch foo", then we'll show it in the breakpoint table
               just fine, but after KDevelop restart, we'll try to add the
               breakpoint using basically "watch *&(foo)".  I'm not sure if 
               that's a problem or not.  */
            itemData[location_column] = b["original-location"].literal();
        }
    }
    else
    {
        itemData[location_column] = "Your GDB is too old";
    }
    

    if (!dirty_.contains(condition_column)
        && !errors_.contains(condition_column))
    {
        if (b.hasField("cond"))
            itemData[condition_column] = b["cond"].literal();
    }

    if (b.hasField("addr") && b["addr"].literal() == "<PENDING>")
        pending_ = true;
    else 
        pending_ = false;
   
    hitCount_ = b["times"].toInt();
    reportChange();

#if 0        
    {bp_watchpoint, "watchpoint"},
    {bp_hardware_watchpoint, "hw watchpoint"},
    {bp_read_watchpoint, "read watchpoint"},
    {bp_access_watchpoint, "acc watchpoint"},
#endif

#if 0
    {
        bp->setHits(b["times"].toInt());
        if (b.hasField("ignore"))
            bp->setIgnoreCount(b["ignore"].toInt());
        else
            bp->setIgnoreCount(0);
        if (b.hasField("cond"))
            bp->setConditional(b["cond"].literal());
        else
            bp->setConditional(QString::null);
        
        // TODO: make the above functions do this instead
        bp->notifyModified();
    }
    else
    {
        // It's a breakpoint added outside, most probably
        // via gdb console. Add it now.
        QString type = b["type"].literal();

        if (type == "breakpoint" || type == "hw breakpoint")
        {
            if (b.hasField("fullname") && b.hasField("line"))
            {
                Breakpoint* bp = new FilePosBreakpoint(this,
                                                       b["fullname"].literal(),
                                                       b["line"].literal().toInt());

                bp->setActive(m_activeFlag, id);
                bp->setActionAdd(false);
                bp->setPending(false);

                addBreakpoint(bp);
            }
        }

    }
#endif        
}

void NewBreakpoint::setColumn(int index, const QVariant& value)
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
            static_cast<Breakpoints*>(parentItem)->createHelperBreakpoint();
        }
    }

    dirty_.insert(index);
    errors_.remove(index);
    reportChange();
    sendToGDBMaybe();
}

void NewBreakpoint::markOut()
{
    id_ = -1;
    dirty_.insert(location_column);
    dirty_.insert(condition_column);
}

QVariant NewBreakpoint::data(int column, int role) const
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

void NewBreakpoint::setDeleted()
{
    deleted_ = true;
}

int NewBreakpoint::hitCount() const
{
    return hitCount_;
}

void NewBreakpoint::sendToGDBMaybe()
{
    if (pleaseEnterLocation_)
        return;

    if (controller_->stateIsOn(s_dbgNotStarted))
    {
        if (deleted_)
        {
            /* If user wants to delete this breakpoint, and the
               debugger is not running, we can just immediately do it.  */
            removeSelf();
            return;
        }
        else
        {
            /* In all other cases, have to wait for debugger to start before
               messing with breakpoints.  */
            return;
        }
    }

    /** See what is dirty, and send the changes.  For simplicity, send
        changes one-by-one and call sendToGDB again in the completion
        handler.  
        FIXME: should handle and annotate the errors?
    */
    if (deleted_)
    {
        if (id_ == -1)
            removeSelf();
        else
            controller_->addCommand(
                new GDBCommand(BreakDelete, QString::number(id_),
                               this, &NewBreakpoint::handleDeleted));        
    }
    else if (dirty_.contains(location_column))
    {
        if (id_ != -1)
        {
            /* We already have GDB breakpoint for this, so we need to remove
               this one.  */
            controller_->addCommand(
                new GDBCommand(BreakDelete, QString::number(id_),
                               this, &NewBreakpoint::handleDeleted));           
        }
        else
        {
            if (kind_ == code_breakpoint)
                controller_->addCommand(
                    new GDBCommand(BreakInsert, 
                                   itemData[location_column].toString(),
                                   this, &NewBreakpoint::handleInserted, true));
            else
                controller_->addCommand(
                    new GDBCommand(
                        DataEvaluateExpression,
                        QString("&(%1)").arg(
                            itemData[location_column].toString()),
                        this,
                        &NewBreakpoint::handleAddressComputed, true));
        }
    }
    else if (dirty_.contains(enable_column))
    {
        controller_->addCommand(
            new GDBCommand(enabled_ ? BreakEnable : BreakDisable,  
                           QString::number(id_),
                           this, &NewBreakpoint::handleEnabledOrDisabled, 
                           true));
    }
    else if (dirty_.contains(condition_column))
    {
        controller_->addCommand(
            new GDBCommand(BreakCondition, 
                           QString::number(id_) + " " + 
                           itemData[condition_column].toString(),
                           this, &NewBreakpoint::handleConditionChanged, true));
    }
}

void NewBreakpoint::handleDeleted(const GDBMI::ResultRecord &v)
{
    Q_UNUSED(v);
    // FIXME: if deleting breakpoint for real, should commit suicide.
    if (deleted_)
    {
        removeSelf();
    }
    else
    {
        id_ = -1;
        sendToGDBMaybe();
    }
}

void NewBreakpoint::handleInserted(const GDBMI::ResultRecord &r)
{
    if (r.reason == "error")
    {
        errors_.insert(location_column);
        dirty_.remove(location_column);
        reportChange();        
        emit static_cast<Breakpoints*>(parentItem)
            ->error(this, r["msg"].literal(), location_column);
    }
    else
    {
        dirty_.remove(location_column);
        if (r.hasField("bkpt"))
            update(r["bkpt"]);
        else
        {
            // For watchpoint creation, GDB basically does not say
            // anything.  Just record id.
            id_ = r["wpt"]["number"].toInt();
        }
        reportChange();
        sendToGDBMaybe();
    }
}

void NewBreakpoint::handleEnabledOrDisabled(const GDBMI::ResultRecord &r)
{
    Q_UNUSED(r);
    // FIXME: handle error. Enable error most likely means the
    // breakpoint itself cannot be inserted in the target.
    dirty_.remove(enable_column);
    reportChange();
    sendToGDBMaybe();
}

void NewBreakpoint::handleConditionChanged(const GDBMI::ResultRecord &r)
{
    if (r.reason == "error")
    {
        errors_.insert(condition_column);
        dirty_.remove(condition_column);
        reportChange();
        emit static_cast<Breakpoints*>(parentItem)
            ->error(this, r["msg"].literal(), condition_column);
    }
    else
    {
        /* GDB does not print the breakpoint in response to -break-condition.
           Presumably, it means that the condition is always what we want.  */
        dirty_.remove(condition_column);
        reportChange();
        sendToGDBMaybe();
    }
}

void NewBreakpoint::handleAddressComputed(const GDBMI::ResultRecord &r)
{
    if (r.reason == "error")
    {
        errors_.insert(location_column);
        dirty_.remove(location_column);
        reportChange();
        emit static_cast<Breakpoints*>(parentItem)
            ->error(this, r["msg"].literal(), location_column);
    }
    else
    {
        address_ = r["value"].literal();

        QString opt;
        if (kind_ == read_breakpoint)
            opt = "-r ";
        else if (kind_ == access_breakpoint)
            opt = "-a ";

        controller_->addCommand(
            new GDBCommand(
                BreakWatch,
                opt + QString("*%1").arg(address_),
                this, &NewBreakpoint::handleInserted, true));
    }
}

void NewBreakpoint::setLocation(const QString& location)
{
    itemData[location_column] = location;
    dirty_.insert(location_column);
    reportChange();
    sendToGDBMaybe();
}

void NewBreakpoint::save(KConfigGroup& config)
{
    config.writeEntry("kind", string_kinds[kind_]);
    config.writeEntry("enabled", enabled_);
    config.writeEntry("location", itemData[location_column]);
    config.writeEntry("condition", itemData[condition_column]);
}

}
