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
#include <QLabel>
#include <klocalizedstring.h>
#include <KAction>
#include <QApplication>
#include <util/richtexttoolbutton.h>

const int SPACING_FROM_PARENT_BOTTOM = 5;

void AssistantPopup::updateActions() {
    QPalette palette = QApplication::palette();
    palette.setBrush(QPalette::Background, palette.toolTipBase());
    setPalette(palette);
    m_assistantActions = m_assistant->actions();
    bool haveTitle = false;
    if (!m_assistant->title().isEmpty()) {
        haveTitle = true;
        QLabel* title = new QLabel("<b>" + m_assistant->title() + ":<b>");
        title->setTextFormat(Qt::RichText);
        addWidget(title);
    }
    ///@todo Add some intelligent layouting to make sure the widget doesn't become too wide
    foreach(KDevelop::IAssistantAction::Ptr action, m_assistantActions)
    {
        if(haveTitle || action != m_assistantActions.first())
            addSeparator();
        addWidget(widgetForAction(action));
    }
    addSeparator();
    addWidget(widgetForAction(KDevelop::IAssistantAction::Ptr()));
    resize(sizeHint());
    move((parentWidget()->width() - width())/2, parentWidget()->height() - height() - SPACING_FROM_PARENT_BOTTOM);
}

AssistantPopup::AssistantPopup(QWidget* parent, KDevelop::IAssistant::Ptr assistant) : QToolBar(parent), m_assistant(assistant) {
    Q_ASSERT(assistant);
    setAutoFillBackground(true);
    updateActions();
}

QWidget* AssistantPopup::widgetForAction(KDevelop::IAssistantAction::Ptr action) {
    KDevelop::RichTextToolButton* button = new KDevelop::RichTextToolButton;
    KAction* realAction;
    QString buttonText;
    int index = m_assistantActions.indexOf(action);
    if(index == -1) {
        realAction = new KAction(button);
        buttonText = "<u>0</u> - " + i18n("Hide");
    }
    else {
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

void AssistantPopup::executeHideAction() {
    m_assistant->doHide();
}

KSharedPtr< KDevelop::IAssistant > AssistantPopup::assistant() const {
    return m_assistant;
}

#include "assistantpopup.moc"
