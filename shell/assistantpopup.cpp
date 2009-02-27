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

AssistantPopup::AssistantPopup(QWidget* parent, KDevelop::IAssistant::Ptr assistant) : QFrame(parent), m_assistant(assistant) {
    setGeometry(parent->width() / 6, parent->height() - 200, (parent->width() / 6) * 4, 200);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QString title = assistant->title();
    if(!title.isEmpty())
        layout->addWidget(new QLabel(title));
    
    //Add widgets that need to be vertical
    foreach(KDevelop::IAssistantAction::Ptr action, assistant->actions())
        if(action->flags() & KDevelop::IAssistantAction::OwnLineFlag)
            layout->addWidget(widgetForAction(action));
    
    QHBoxLayout* hLayout = new QHBoxLayout;
    
    //Now add horizontally aligned actions
    foreach(KDevelop::IAssistantAction::Ptr action, assistant->actions())
        if(!(action->flags() & KDevelop::IAssistantAction::OwnLineFlag))
            hLayout->addWidget(widgetForAction(action));
    
    if(hLayout->count() == 0)
        delete hLayout;
    else
        layout->addLayout(hLayout);
    
    move(pos().x(), parent->height() - height());
}

QWidget* AssistantPopup::widgetForAction(KDevelop::IAssistantAction::Ptr action) {
    QLabel* ret = new QLabel(action->description());
    ret->setAlignment(Qt::AlignLeft);
    return ret;
}

void AssistantPopup::executeAction1() {
    QList<KDevelop::IAssistantAction::Ptr> actions = m_assistant->actions();
    if(actions.size() > 0)
        actions[0]->execute();
}

void AssistantPopup::executeAction2() {
    QList<KDevelop::IAssistantAction::Ptr> actions = m_assistant->actions();
    if(actions.size() > 1)
        actions[1]->execute();
}

void AssistantPopup::executeAction3() {
    QList<KDevelop::IAssistantAction::Ptr> actions = m_assistant->actions();
    if(actions.size() > 2)
        actions[2]->execute();
}

void AssistantPopup::executeAction4() {
    QList<KDevelop::IAssistantAction::Ptr> actions = m_assistant->actions();
    if(actions.size() > 3)
        actions[3]->execute();
}
