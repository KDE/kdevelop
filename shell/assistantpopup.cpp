/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2012 Milian Wolff <mail@milianw.de>

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
#include <QLabel>
#include <QDebug>

#include <KLocalizedString>
#include <KAction>
#include <KParts/MainWindow>

#include <util/richtexttoolbutton.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

const int SPACING_FROM_PARENT_BOTTOM = 5;

using namespace KDevelop;

AssistantPopup::AssistantPopup(QWidget* parent, const IAssistant::Ptr& assistant)
// main window as parent to use maximal space available in worst case
: QToolBar(ICore::self()->uiController()->activeMainWindow())
, m_assistant(assistant)
, m_contextWidget(parent)
{
    Q_ASSERT(assistant);
    setAutoFillBackground(true);
    setMovable(false);

    updateActions();
    updatePosition();

    connect(m_contextWidget, SIGNAL(destroyed(QObject*)),
        this, SLOT(deleteLater()));
    m_contextWidget->installEventFilter(this);
}

bool AssistantPopup::eventFilter(QObject* object, QEvent* event)
{
    Q_ASSERT(object == m_contextWidget);
    Q_UNUSED(object);
    if (event->type() == QEvent::Resize) {
        updatePosition();
    } else if (event->type() == QEvent::Hide) {
        executeHideAction();
    }
    return false;
}

void AssistantPopup::updatePosition()
{
    // our top left is the bottom left of the context widget
    const QPoint topLeft = parentWidget()->mapFromGlobal(m_contextWidget->mapToGlobal(m_contextWidget->geometry().bottomLeft()));
    // x: either center-aligned below context widget or right-aligned in parentWidget() i.e. main window if no space is left
    // y: just below context widget or just bottom of window
    move(qMin(topLeft.x() + (m_contextWidget->width() - width()) / 2, parentWidget()->width() - width()),
         qMin(topLeft.y(), parentWidget()->height() - height()));
}

IAssistant::Ptr AssistantPopup::assistant() const
{
    return m_assistant;
}

void AssistantPopup::executeHideAction()
{
    m_assistant->doHide();
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
    int mnemonic = 1;
    ///@todo Add some intelligent layouting to make sure the widget doesn't become too wide
    foreach(IAssistantAction::Ptr action, m_assistantActions)
    {
        if(haveTitle || action != m_assistantActions.first())
            addSeparator();
        addWidget(widgetForAction(action, mnemonic));
    }
    addSeparator();
    mnemonic = 0;
    addWidget(widgetForAction(IAssistantAction::Ptr(), mnemonic));
    resize(sizeHint());
}

QWidget* AssistantPopup::widgetForAction(const IAssistantAction::Ptr& action, int& mnemonic)
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
        buttonText = i18n("Hide");
    } else {
        realAction = action->toKAction();
        buttonText = action->description();
    }
    realAction->setParent(button);
    connect(realAction, SIGNAL(triggered(bool)), SLOT(executeHideAction()));
    button->setDefaultAction(realAction);
    button->setText(QString("&%1").arg(mnemonic)); // Let the button care about the shortcut
    button->setHtml(QString("<u>%1</u> - ").arg(mnemonic) + buttonText);
    mnemonic++;
    return button;
}

#include "assistantpopup.moc"
