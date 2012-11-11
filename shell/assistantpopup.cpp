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

#include "assistantpopup.h"

#include <QVBoxLayout>
#include <QEvent>
#include <QLabel>
#include <QApplication>

#include <KLocalizedString>
#include <KAction>

#include <util/richtexttoolbutton.h>

using namespace KDevelop;

AssistantPopup::AssistantPopup(QWidget* parent, const IAssistant::Ptr& assistant)
: QToolBar()
, m_parentWidget(parent)
, m_assistant(assistant)
{
    setWindowFlags(Qt::ToolTip);

    Q_ASSERT(assistant);
    setAutoFillBackground(true);
    updateActions();

    connect(m_parentWidget, SIGNAL(destroyed(QObject*)),
        this, SLOT(deleteLater()));
    m_parentWidget->installEventFilter(this);
}

bool AssistantPopup::eventFilter(QObject* object, QEvent* event)
{
    if (object == m_parentWidget && event->type() == QEvent::Resize) {
        updatePosition();
    }
    return QObject::eventFilter(object, event);
}

IAssistant::Ptr AssistantPopup::assistant() const
{
    return m_assistant;
}

void AssistantPopup::executeHideAction()
{
    m_assistant->doHide();
}

void AssistantPopup::updatePosition()
{
    const QRect geometry = m_parentWidget->geometry();
    const QPoint topLeft = m_parentWidget->mapToGlobal(geometry.topLeft());
    // center horizontally and move below parent widget
    move( topLeft.x() + (geometry.width() - width()) / 2,
          topLeft.y() + geometry.height());
}

void AssistantPopup::updateActions()
{
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Background, palette.toolTipBase());
    palette.setBrush(QPalette::WindowText, palette.toolTipText());
    setPalette(palette);
    m_assistantActions = m_assistant->actions();
    bool haveTitle = false;
    if (!m_assistant->title().isEmpty()) {
        haveTitle = true;
        RichTextToolButton* title = new RichTextToolButton;
        title->setHtml("<b>" + m_assistant-> title() + ":</b>");
        title->setEnabled(false);
        addWidget(title);
    }
    ///@todo Add some intelligent layouting to make sure the widget doesn't become too wide
    foreach(IAssistantAction::Ptr action, m_assistantActions)
    {
        if(haveTitle || action != m_assistantActions.first())
            addSeparator();
        addWidget(widgetForAction(action));
    }
    addSeparator();
    addWidget(widgetForAction(IAssistantAction::Ptr()));
    resize(sizeHint());
    updatePosition();
}

QWidget* AssistantPopup::widgetForAction(const IAssistantAction::Ptr& action)
{
    KAction* realAction = action ? action->toKAction() : 0;
    RichTextToolButton* button = new RichTextToolButton;

    if (action && !realAction) {
        // non-executable "label" actions
        button->setHtml(action->description());
        button->setEnabled(false);
        return button;
    }

    QString buttonText;
    int index = m_assistantActions.indexOf(action);
    if (index == -1) {
        realAction = new KAction(button);
        buttonText = "<u>0</u> - " + i18n("Hide");
    } else {
        realAction = action->toKAction();
        buttonText = QString("<u>%1</u> - ").arg(index+1) + action->description();
    }
    realAction->setParent(button);
    connect(realAction, SIGNAL(triggered(bool)), SLOT(executeHideAction()));
    button->setDefaultAction(realAction);
    button->setText(QString("&%1").arg(index+1)); // Let the button care about the shortcut
    button->setHtml(buttonText);
    return button;
}

#include "assistantpopup.moc"
