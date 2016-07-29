/*
 * MI based debugger specific Variable
 *
 * Copyright 2009 Vladimir Prus <ghost@cs.msu.su>
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

#ifndef MIVARIABLE_H
#define MIVARIABLE_H

#include "mi/mi.h"

#include <debugger/variable/variablecollection.h>

#include <QMap>
#include <QPointer>


class CreateVarobjHandler;
class FetchMoreChildrenHandler;
namespace KDevMI {
class MIDebugSession;
class MIVariable : public KDevelop::Variable
{
public:
    MIVariable(MIDebugSession *session, KDevelop::TreeModel* model, KDevelop::TreeItem* parent,
               const QString& expression, const QString& display = "");

    ~MIVariable();

    /* FIXME: should eventually remove, so that existance of
        varobjs is fully encapsulalated inside GdbVariable.  */
    const QString& varobj() const;
    void handleUpdate(const MI::Value& var);

    /* Called when debugger dies.  Clears the association between varobj names
        and Variable instances.  */
    void markAsDead();

    bool canSetFormat() const override { return true; }

protected: // Variable overrides
    void attachMaybe(QObject *callback, const char *callbackMethod) override;
    void fetchMoreChildren() override;
    void formatChanged() override;

protected: // Internal
    friend class ::CreateVarobjHandler;
    friend class ::FetchMoreChildrenHandler;

    QString enquotedExpression() const;

    bool sessionIsAlive() const;

    void setVarobj(const QString& v);
    QString varobj_;

    QPointer<MIDebugSession> debugSession;

    // How many children should be fetched in one
    // increment.
    static const int fetchStep = 5;
};
} // end of KDevMI

#endif
