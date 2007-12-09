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

FrameItem::FrameItem(VariableCollection* parent, int frameNo, int threadNo)
    : AbstractVariableItem(parent),
      frameNo_(frameNo),
      threadNo_(threadNo),
      m_serial(-1),
      currentFrameBase((quint64)-1),
      currentFrameCodeAddress((quint64)-1)
{
}

GDBDebugger::FrameItem::~ FrameItem()
{
}

void FrameItem::refresh()
{
    if (isDirty())
    {
        setDirty(false);
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

        // rodda: actually, we want te fetch values separately, it might be quicker if there
        // are excessive amounts of variables.

        ++m_serial;

        controller()->addCommand(
            new GDBCommand(StackListArguments, QString("0 %1 %2")
                        .arg(threadNo_)
                        .arg(frameNo_),
                        this,
                        &FrameItem::argumentsReady));


        controller()->addCommand(
            new GDBCommand(StackListLocals, 0,
                        this,
                        &FrameItem::localsReady));
    }
}

// **************************************************************************

bool FrameItem::matchDetails(int frameNo, int threadNo)
{
    return frameNo == frameNo_ && threadNo == threadNo_;
}

void FrameItem::argumentsReady(const GDBMI::ResultRecord& r)
{
    if (r["stack-args"].size() == 0)
        return;

    const GDBMI::Value& args = r["stack-args"][0]["args"];

    fetch_time.start();

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

    controller()->addCommand(new CliCommand(NonMI, "info frame",
                                           this,
                                           &FrameItem::frameIdReady));
}

void FrameItem::frameIdReady(const QStringList& lines)
{
    //kDebug(9012) << "localAddresses: " << lines[1];

    QString frame_info;
    for(int i = 1; i < lines.size(); ++i)
        frame_info += lines[i];

    kDebug(9012) << "frame info: " << frame_info;
    frame_info.replace('\n', "");

    static QRegExp frame_base_rx("frame at 0x([0-9a-fA-F]*)");
    static QRegExp frame_code_rx("saved [a-zA-Z0-9]* 0x([0-9a-fA-F]*)");

    int i = frame_base_rx.indexIn(frame_info);
    int i2 = frame_code_rx.indexIn(frame_info);

    bool frameIdChanged = false;

   // TODO collapse non-this frames

    if (i != -1 && i2 != -1)
    {
        quint64 new_frame_base =
            frame_base_rx.cap(1).toULongLong(0, 16);
        quint64 new_code_address =
            frame_code_rx.cap(1).toULongLong(0, 16);
        kDebug(9012) << "Frame base = " << QString::number(new_frame_base, 16)
                      << " code = " << QString::number(new_code_address, 16);
        kDebug(9012) << "Previous frame " <<
            QString::number(currentFrameBase, 16)
                      << " code = " << QString::number(
                          currentFrameCodeAddress, 16);

        frameIdChanged = (new_frame_base != currentFrameBase ||
                          new_code_address != currentFrameCodeAddress);

        currentFrameBase = new_frame_base;
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
        else
            static_cast<VariableItem*>(child)->recreateLocallyMaybe();
    }

    foreach (AbstractVariableItem* child, children())
    {
        if (VariableItem* var = qobject_cast<VariableItem*>(child)) {
            var->clearHighlight();
            // For watched expressions, we don't have an easy way
            // to check if their meaning is still the same, so
            // unconditionally recreate them.
            var->recreate();
        }
    }

    // Note: can't use --all-values in this command, because gdb will
    // die if there's any uninitialized variable. Ouch!
    controller()->addCommand(new GDBCommand(VarUpdate,
                                "*",
                                this,
                                &FrameItem::handleVarUpdate));

    controller()->addCommand(new SentinelCommand(
                                this,
                                &FrameItem::variablesFetchDone));
}

void FrameItem::checkVariable(const QString & variable)
{
    VariableItem* var = 0;

    // See if we've got VariableItem for this one already.
    foreach (AbstractVariableItem* child, children())
    {
        if (VariableItem* vi = qobject_cast<VariableItem*>(child)) {
            if (vi->variableName() == variable) {
                var = vi;
                break;
            }
        }
    }

    if (!var)
    {
        var = new VariableItem(this);
        var->setExpression(variable);
        addChild(var);
    }

    var->clearHighlight();
    var->setLastSeen(m_serial);
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
        if (item)
            item->setDirty();
    }
}

void FrameItem::variablesFetchDone()
{
    // During parsing of fetched variable values, we might have issued
    // extra command to handle 'special values', like QString.
    // We don't want to enable updates just yet, because this will cause
    // flicker, so add a sentinel command just to enable updates.
    //
    // We need this intermediate hook because commands for special
    // representation are issues when responses to orginary fetch
    // values commands are received, so we can add sentinel command after
    // special representation fetch only when commands for ordinary
    // fetch are all executed.
    controller()->addCommand(new SentinelCommand(
                                this,
                                &FrameItem::fetchSpecialValuesDone));
}

QVariant FrameItem::data(int column, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            switch (column) {
                case ColumnName:
                    return i18n("Locals");
            }
    }

    return QVariant();
}

void FrameItem::fetchSpecialValuesDone()
{
    // FIXME: can currentFrame_ or currentThread_ change between
    // start of var fetch and call of 'variablesFetchDone'?
    kDebug(9012) << "Time to fetch variables: " << fetch_time.elapsed() <<
        "ms";
}

#include "frameitem.moc"
