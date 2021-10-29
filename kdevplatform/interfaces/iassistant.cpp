/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "iassistant.h"
#include "icore.h"

#include <QAction>
#include <QThread>

using namespace KDevelop;

Q_DECLARE_METATYPE(QExplicitlySharedDataPointer<IAssistantAction>)

//BEGIN IAssistant

void IAssistant::createActions()
{
}

QAction* IAssistantAction::toQAction(QObject* parent) const
{
    Q_ASSERT(QThread::currentThread() == ICore::self()->thread() && "Actions must be created in the application main thread"
                                                    "(implement createActions() to create your actions)");

    auto* ret = new QAction(icon(), description(), parent);
    ret->setToolTip(toolTip());

    //Add the data as a QExplicitlySharedDataPointer to the action, so this assistant stays alive at least as long as the QAction
    ret->setData(QVariant::fromValue(QExplicitlySharedDataPointer<IAssistantAction>(const_cast<IAssistantAction*>(this))));

    connect(ret, &QAction::triggered, this, &IAssistantAction::execute);
    return ret;
}

IAssistant::~IAssistant()
{
}

IAssistantAction::IAssistantAction()
    : QObject()
    , KSharedObject(*(QObject*)this)
{
}

IAssistantAction::~IAssistantAction()
{
}

QIcon IAssistantAction::icon() const
{
    return QIcon();
}

QString IAssistantAction::toolTip() const
{
    return QString();
}

//END IAssistantAction

//BEGIN AssistantLabelAction

AssistantLabelAction::AssistantLabelAction(const QString& description)
: m_description(description)
{

}

QString AssistantLabelAction::description() const
{
    return m_description;
}

void AssistantLabelAction::execute()
{
    // do nothing
}

QAction* AssistantLabelAction::toQAction(QObject* parent) const
{
    Q_UNUSED(parent);
    return nullptr;
}

//END AssistantLabelAction

//BEGIN: IAssistant

IAssistant::IAssistant()
: KSharedObject(*(QObject*)this)
{
}

QIcon IAssistant::icon() const
{
    return QIcon();
}

QString IAssistant::title() const
{
    return QString();
}

void IAssistant::doHide()
{
    emit hide();
}

QList< IAssistantAction::Ptr > IAssistant::actions() const
{
    if ( m_actions.isEmpty() ) {
        const_cast<IAssistant*>(this)->createActions();
    }
    return m_actions;
}

void IAssistant::addAction(const IAssistantAction::Ptr& action)
{
    m_actions << action;
}

void IAssistant::clearActions()
{
    m_actions.clear();
}

//END IAssistant

