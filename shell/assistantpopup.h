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

#ifndef KDEVPLATFORM_ASSISTANTPOPUP_H
#define KDEVPLATFORM_ASSISTANTPOPUP_H

#include <QToolBar>
#include <interfaces/iassistant.h>
#include <ksharedptr.h>


class AssistantPopup : public QToolBar
{
    Q_OBJECT

public:
    typedef KSharedPtr<AssistantPopup> Ptr;
    /**
     * @p widget The widget below which the assistant should be shown.
     * The current main window will be used as parent widget for the popup.
     * This is to make use of the maximal space available and prevent any lines
     * in e.g. the editor to be hidden by the popup.
     */
    AssistantPopup(QWidget* widget, const KDevelop::IAssistant::Ptr& assistant);
    KDevelop::IAssistant::Ptr assistant() const;

public slots:
    void executeHideAction();

private slots:
    void updatePosition();

private:
    virtual bool eventFilter(QObject* object, QEvent* event);
    void updateActions();
    QWidget* widgetForAction(const KDevelop::IAssistantAction::Ptr& action, int& mnemonic);
    KDevelop::IAssistant::Ptr m_assistant;
    QList<KDevelop::IAssistantAction::Ptr> m_assistantActions;
    QWidget* m_contextWidget;
};

#endif // KDEVPLATFORM_ASSISTANTPOPUP_H
