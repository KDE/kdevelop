/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "iassistant.h"
#include "icore.h"

#include <KAction>
#include <QXmlStreamReader>
#include <QTextEdit>
#include <QThread>

using namespace KDevelop;

Q_DECLARE_METATYPE(KSharedPtr<IAssistantAction>)

static QString removeHtmlFromString(QString string)
{
    QXmlStreamReader xml("<root>" + string + "</root>");
    QString textString;
    while ( !xml.atEnd() ) {
        if ( xml.readNext() == QXmlStreamReader::Characters ) {
            textString += xml.text();
        }
    }
    return textString;
}

//BEGIN IAssistant

void IAssistant::createActions()
{
}

KAction* IAssistantAction::toKAction() const
{
    Q_ASSERT(QThread::currentThread() == ICore::self()->thread() && "Actions must be created in the application main thread"
                                                    "(implement createActions() to create your actions)");

    KAction* ret = new KAction(KIcon(icon()), removeHtmlFromString(description()), 0);
    ret->setToolTip(toolTip());

    //Add the data as a KSharedPtr to the action, so this assistant stays alive at least as long as the KAction
    ret->setData(QVariant::fromValue(KSharedPtr<IAssistantAction>(const_cast<IAssistantAction*>(this))));

    connect(ret, SIGNAL(triggered(bool)), SLOT(execute()));
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

KAction* AssistantLabelAction::toKAction() const
{
    return 0;
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

#include "iassistant.moc"
