/*
    SPDX-FileCopyrightText: 2009 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MIVARIABLE_H
#define MIVARIABLE_H

#include "mi/mi.h"

#include <debugger/variable/variablecollection.h>

#include <QPointer>


class CreateVarobjHandler;
class FetchMoreChildrenHandler;
class SetFormatHandler;
namespace KDevMI {
class MIDebugSession;
class MIVariable : public KDevelop::Variable
{
    Q_OBJECT


public:
    MIVariable(MIDebugSession *session, KDevelop::TreeModel* model, KDevelop::TreeItem* parent,
               const QString& expression, const QString& display = QString());

    ~MIVariable() override;

    /* FIXME: should eventually remove, so that existence of
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
    friend class ::SetFormatHandler;

    /**
     * Construct a MIVariable child directly from a MI value
     */
    MIVariable *createChild(const MI::Value &child);

    QString enquotedExpression() const;
    virtual QString formatValue(const QString &rawValue) const;

    /**
     * If a given tuple value has a field "value", format the value
     * of the field and assign the result to @c this->value().
     */
    void setValueToOptionalValueFieldOf(const MI::Value& tupleValue);

    bool sessionIsAlive() const;

    void setVarobj(const QString& v);

protected:
    QPointer<MIDebugSession> m_debugSession;

private:
    QString m_varobj;

    // How many children should be fetched in one
    // increment.
    static const int s_fetchStep = 5;
};
} // end of KDevMI

#endif
