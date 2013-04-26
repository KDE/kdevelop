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

#ifndef KDEVPLATFORM_IASSISTANT_H
#define KDEVPLATFORM_IASSISTANT_H

#include <QtCore/QPointer>
#include <QtGui/QIcon>
#include <KDE/KSharedPtr>
#include "interfacesexport.h"
#include <util/ksharedobject.h>

class KAction;

namespace KDevelop {

///Represents a single assistant action.
///Subclass it to create your own actions.
class KDEVPLATFORMINTERFACES_EXPORT IAssistantAction : public QObject, public KSharedObject
{
    Q_OBJECT
public:
    IAssistantAction();

    typedef KSharedPtr<IAssistantAction> Ptr;

    virtual ~IAssistantAction();

    ///Creates a KAction that represents this exact assistant action.
    ///The caller owns the action, and is responsible for deleting it.
    virtual KAction* toKAction() const;

    ///Should return a short description of the action.
    ///It may contain simple HTML formatting.
    ///Must be very short, so it nicely fits into the assistant popups.
    virtual QString description() const = 0;
    ///May return additional tooltip hover information.
    ///The default implementation returns an empty string.
    virtual QString toolTip() const;
    ///May return an icon for this action.
    ///The default implementation returns an invalid icon, which means that no icon is shown.
    virtual QIcon icon() const;

public Q_SLOTS:
    /**
     * Execute this action.
     *
     * NOTE: Implementations should properly emit executed(this) after being executed.
     */
    virtual void execute() = 0;

Q_SIGNALS:
    /**
     * Gets emitted when this action was executed.
     */
    void executed(IAssistantAction* action);
};

/**
 * A fake action that only shows a label.
 */
class KDEVPLATFORMINTERFACES_EXPORT AssistantLabelAction : public IAssistantAction
{
    Q_OBJECT
public:
    /**
     * @p description The label to show.
     */
    AssistantLabelAction(const QString& description);
    /**
     * @return the label contents.
     */
    virtual QString description() const;
    /**
     * The label cannot be executed.
     */
    virtual void execute();
    /**
     * No action is returned.
     */
    virtual KAction* toKAction() const;

private:
    QString m_description;
};

///Represents a single assistant popup.
///Subclass it to create your own assistants.
class KDEVPLATFORMINTERFACES_EXPORT IAssistant : public QObject, public KSharedObject
{
    Q_OBJECT
public:
    IAssistant();
    virtual ~IAssistant();

    typedef KSharedPtr<IAssistant> Ptr;

    ///Returns the stored list of actions, or can be overridden to return an own set.
    virtual QList<IAssistantAction::Ptr> actions() const;

    ///Adds the given action to the list of actions.
    ///Does not emit actionsChanged(), you have to do that when you're ready.
    virtual void addAction(const IAssistantAction::Ptr& action);

    ///Clears the stored list of actions.
    ///Does not emit actionsChanged(), you have to do that when you're ready.
    virtual void clearActions();

    ///May return an icon for this assistant
    virtual QIcon icon() const;

    ///May return the title of this assistant
    ///The text may be html formatted. If it can be confused with HTML,
    ///use Qt::escape(..).
    virtual QString title() const;
public Q_SLOTS:
    ///Emits hide(), which causes this assistant to be hidden
    virtual void doHide();
Q_SIGNALS:
    ///Can be emitted by the assistant when it should be hidden
    void hide();
    ///Can be emitted by the assistant when its actions have changed and should be re-read
    void actionsChanged();
private:
    QList<IAssistantAction::Ptr> m_actions;
};

}

#endif // KDEVPLATFORM_IASSISTANT_H
