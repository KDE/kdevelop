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
#include <QToolButton>
#include <QLabel>
#include <klocalizedstring.h>

void AssistantPopup::updateActions() {
    setUpdatesEnabled(false);
    
    setFrameStyle(QFrame::Panel);
    
//     foreach(QObject* child, children())
//         delete child;

    QWidget* parent = qobject_cast<QWidget*>(this->parent());

    setGeometry(parent->width() / 6, parent->height() - 200, (parent->width() / 6) * 4, 200);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QString title = m_assistant->title();
    if(!title.isEmpty()) {
        QLabel* l = new QLabel(title);
        l->setWordWrap(true);
        layout->addWidget(l);
    }
    
    m_actions = m_assistant->actions();

    //Add widgets that need to be vertical
    foreach(KDevelop::IAssistantAction::Ptr action, m_actions)
        if(action->flags() & KDevelop::IAssistantAction::OwnLineFlag)
            layout->addWidget(widgetForAction(action));
    
    QHBoxLayout* hLayout = new QHBoxLayout;
    
    //Now add horizontally aligned actions
    foreach(KDevelop::IAssistantAction::Ptr action, m_actions)
        if(!(action->flags() & KDevelop::IAssistantAction::OwnLineFlag))
            hLayout->addWidget(widgetForAction(action));
    
    if(hLayout->count() == 0) {
        delete hLayout;
        layout->addWidget(widgetForAction(KDevelop::IAssistantAction::Ptr()));
    } else{
        hLayout->addWidget(widgetForAction(KDevelop::IAssistantAction::Ptr()));
        layout->addLayout(hLayout);
    }
    QSize hint = sizeHint();
    
    resize(hint);
    
    move((parent->width() - hint.width())/2, parent->height() - hint.height());
    
    setUpdatesEnabled(true);
}

AssistantPopup::AssistantPopup(QWidget* parent, KDevelop::IAssistant::Ptr assistant) : QFrame(parent), m_assistant(assistant) {
    Q_ASSERT(assistant);
    m_normalPalette = palette();
    
    setAutoFillBackground(true);

    updateActions();
}

void AssistantPopup::assistantActionsChanged() {
    updateActions();
}

QWidget* AssistantPopup::widgetForAction(KDevelop::IAssistantAction::Ptr action) {
    int index = m_actions.indexOf(action);
    QFrame* containerWidget = new QFrame;
    containerWidget->setAutoFillBackground(true);
    containerWidget->setPalette(m_normalPalette);
//     containerWidget->setFrameStyle(QFrame::Panel);
    QHBoxLayout* layout = new QHBoxLayout(containerWidget);
    
    QString desc;
    if(action)
        desc = action->description();
    else
        desc = i18n("Hide");
    
    QLabel* label = new QLabel(desc);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QToolButton* button = new QToolButton;
    button->setText(QString("%1").arg(index+1));

    if(index == -1) {
        kDebug() << "connecting with doHide";
        connect(button, SIGNAL(clicked(bool)), SLOT(executeHideAction()));
    }
    if(index == 0)
        connect(button, SIGNAL(clicked(bool)), SLOT(executeAction1()));
    if(index == 1)
        connect(button, SIGNAL(clicked(bool)), SLOT(executeAction2()));
    if(index == 2)
        connect(button, SIGNAL(clicked(bool)), SLOT(executeAction3()));
    if(index == 3)
        connect(button, SIGNAL(clicked(bool)), SLOT(executeAction4()));
    
    layout->addWidget(button);
    layout->addWidget(label);
    
    return containerWidget;
}

void AssistantPopup::executeHideAction() {
    KDevelop::IAssistant::Ptr assistant = m_assistant;
    assistant->doHide();
}

void AssistantPopup::executeAction(int number) {
    Q_ASSERT(m_assistant);
    //Copy the assistant, we may get destroyed during the execute() call
    KDevelop::IAssistant::Ptr assistant = m_assistant;
    
    QList<KDevelop::IAssistantAction::Ptr> actions = assistant->actions();
    if(actions.size() > number)
        actions[number]->execute();
    
    assistant->doHide();
}

void AssistantPopup::executeAction1() {
    executeAction(0);
}

void AssistantPopup::executeAction2() {
    executeAction(1);
}

void AssistantPopup::executeAction3() {
    executeAction(2);
}

void AssistantPopup::executeAction4() {
    executeAction(3);
}

KSharedPtr< KDevelop::IAssistant > AssistantPopup::assistant() const {
    return m_assistant;
}

#include "assistantpopup.moc"
