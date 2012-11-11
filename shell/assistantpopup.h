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

#ifndef ASSISTANTPOPUP_H
#define ASSISTANTPOPUP_H

#include <QToolBar>
#include <interfaces/iassistant.h>
#include <ksharedptr.h>


class AssistantPopup : public QToolBar
{
    Q_OBJECT

public:
    typedef KSharedPtr<AssistantPopup> Ptr;

    /**
     * @p parent The widget below which the assistant should be shown.
     */
    AssistantPopup(QWidget* parent, const KDevelop::IAssistant::Ptr& assistant);
    KDevelop::IAssistant::Ptr assistant() const;

    virtual bool eventFilter(QObject* object, QEvent* event);

public slots:
    void executeHideAction();

private:
    void updateActions();
    QWidget* widgetForAction(const KDevelop::IAssistantAction::Ptr& action);
    void updatePosition();

    QWidget* m_parentWidget;
    KDevelop::IAssistant::Ptr m_assistant;
    QList<KDevelop::IAssistantAction::Ptr> m_assistantActions;
};

#endif // ASSISTANTPOPUP_H
