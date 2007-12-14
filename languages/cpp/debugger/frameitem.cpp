/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "frameitem.h"

#include <QApplication>

#include <KMessageBox>
#include <KLocale>
#include <KDebug>

#include "gdbcontroller.h"
#include "gdbcommand.h"
#include "variableitem.h"
#include "variablecollection.h"

using namespace GDBDebugger;
using namespace GDBMI;

FrameItem::FrameItem(VariableCollection* parent)
    : AbstractVariableItem(parent),
      m_serial(-1),
      currentFrameCodeAddress((quint64)-1)
{
}

FrameItem::~ FrameItem()
{
}

void FrameItem::refreshChildren()
{
    if (isChildrenDirty())
    {
        setChildrenDirty(false);
        // In GDB 6.4, the -stack-list-locals command is broken.
        // If there's any local reference variable which is not
        // initialized yet, for example because it's in the middle
        // of function, gdb will still print it and try to dereference
        // it. If the address in not accessible, the MI command will
        // exit with an error, and we won't be able to see *any*
        // locals. A patch is submitted:
        //    http://sourceware.org/ml/gdb-patches/2006-04/msg00069.html
        // but we need to work with 6.4, not with some future version. So,
        // we just -stack-list-locals to get just names of the locals,
        // but not their values.
        // We'll fetch values separately:

        // rodda: actually, we want to fetch values separately, it might be quicker if there
        // are excessive amounts of variables.

        ++m_serial;

        addCommandUnaltered(
            new GDBCommand(StackListArguments, QString("0 %1 %2")
                        .arg(frame())
                        .arg(frame()),
                        this,
                        &FrameItem::argumentsReady));


        addCommandUnaltered(
            new GDBCommand(StackListLocals, 0,
                        this,
                        &FrameItem::localsReady));
    }
}

// **************************************************************************

bool FrameItem::matchDetails(int frameNo, int threadNo)
{
    return frameNo == frame() && threadNo == thread();
}

void FrameItem::argumentsReady(const GDBMI::ResultRecord& r)
{
    if (r["stack-args"].size() == 0)
        return;

    const GDBMI::Value& args = r["stack-args"][0]["args"];

    for(int i = 0; i < args.size(); ++i)
    {
        checkVariable(args[i].literal());
    }
}

void FrameItem::localsReady(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& locals = r["locals"];

    for(int i = 0; i < locals.size(); ++i)
    {
        QString val = locals[i].literal();

        // Check ada internal variables like <R45b>, <L23R> ...
        bool is_ada_variable = (val[0] == '<' && val[val.length() - 1] == '>');

        if (!is_ada_variable)
        {
            checkVariable(val);
        }
    }

    GDBCommand* command = new GDBCommand(StackInfoFrame);
    command->setHandler(this, &FrameItem::frameIdReady);
    addCommand(command);
}

void FrameItem::frameIdReady(const GDBMI::ResultRecord& result)
{
    //kDebug(9012) << "localAddresses: " << lines[1];

    bool frameIdChanged = false;

   // TODO collapse non-this frames

    if (result.hasField("frame") || result["frame"].hasField("addr"))
    {
        const GDBMI::Value& frame = result["frame"];
        quint64 new_code_address = frame["addr"].literal().remove("0x").toULongLong(0, 16);

        m_function.clear();
        if (frame.hasField("func"))
            m_function = frame["func"].literal();

        if (frame.hasField("level"))
            setFrame(frame["level"].toInt());

        kDebug(9012) << "Frame address = " << QString::number(new_code_address, 16);
        kDebug(9012) << "Previous address = " << QString::number(currentFrameCodeAddress, 16);

        frameIdChanged = (new_code_address != currentFrameCodeAddress);

        currentFrameCodeAddress = new_code_address;
    }
    else
    {
        KMessageBox::information(
            qApp->activeWindow(),
            "<b>Can't get frame id</b>"
            "Could not found frame id from output of 'info frame'. "
            "Further debugging can be unreliable. ",
            i18n("Internal error"));
    }

    /*if (frameIdChanged)
    {
        // Remove all variables.
        // FIXME: probably, need to do this in all frames.
        foreach (AbstractVariableItem* child, children())
            deleteChild(child);
    }*/

    // Remove VariableItems that don't correspond to any local
    // variables any longer. Perform type/address updates
    // for others.
    foreach (AbstractVariableItem* child, children())
    {
        if (child->lastSeen() != m_serial)
            deleteChild(child);
        //else
            //static_cast<VariableItem*>(child)->recreateLocallyMaybe();
    }

    foreach (AbstractVariableItem* child, children())
    {
        if (VariableItem* var = qobject_cast<VariableItem*>(child)) {
            var->clearHighlight();
            // For watched expressions, we don't have an easy way
            // to check if their meaning is still the same, so
            // unconditionally recreate them.

            // TODO reimplement
            //var->recreate();
        }
    }

    // Note: can't use --all-values in this command, because gdb will
    // die if there's any uninitialized variable. Ouch!
    addCommand(new GDBCommand(VarUpdate,
                                "*",
                                this,
                                &FrameItem::handleVarUpdate));
}

void FrameItem::checkVariable(const QString & variable)
{
    VariableItem* var = 0;

    // See if we've got VariableItem for this one already.
    foreach (AbstractVariableItem* child, children())
    {
        if (VariableItem* vi = qobject_cast<VariableItem*>(child)) {
            if (vi->gdbExpression() == variable) {
                var = vi;
                break;
            }
        }
    }

    if (!var)
    {
        QString variableObjectName = VariableItem::nextVariableObjectName();
        m_temporaryExpressions.insert(variableObjectName, variable);
        addCommand(
            // Need to quote expression, otherwise gdb won't like
            // spaces inside it.
            new GDBCommand(VarCreate, QString("%1 * \"%2\"")
                           .arg(variableObjectName)
                           .arg(variable),
                           this,
                           &FrameItem::handleVariableObjectCreated,
                           true));
        return;
    }

    var->clearHighlight();
    var->setLastSeen(m_serial);
}

void FrameItem::handleVariableObjectCreated(const GDBMI::ResultRecord& r)
{
    QString type;
    if (r.hasField("type"))
        type = r["type"].literal();

    VariableItem* var = collection()->createVariableItem(type, this);
    var->setExpression(m_temporaryExpressions.take(r["name"].literal()));
    var->setVariableObject(r, VariableItem::natural, false);
    var->setLastSeen(m_serial);
    addChild(var);
}

void FrameItem::handleVarUpdate(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& changed = r["changelist"];

    for(int i = 0; i < changed.size(); ++i)
    {
        const GDBMI::Value& c = changed[i];

        QString name = c["name"].literal();
        if (c.hasField("in_scope") && c["in_scope"].literal() == "false")
            continue;

        AbstractVariableItem* item = collection()->itemForVariableObject(name);

        if (c.hasField("type_changed"))
            if (c["type_changed"].literal() == "true") {
                AbstractVariableItem* parent = item->abstractParent();
                Q_ASSERT(parent && parent == this);
                deleteChild(item);

                checkVariable(name);
                continue;
            }


        if (item)
            // Children and value dirty
            item->setDirty();
    }
}

QVariant FrameItem::data(int column, int role) const
{
    if (isChildrenDirty())
        const_cast<FrameItem*>(this)->refreshChildren();

    switch (role) {
        case Qt::DisplayRole:
            switch (column) {
                case ColumnName:
                    return i18n("Frame");
                case ColumnValue:
                    return m_function;
            }
    }

    return QVariant();
}

#include "frameitem.moc"
