/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_IASSISTANT_H
#define KDEVPLATFORM_IASSISTANT_H

#include <QIcon>
#include <QExplicitlySharedDataPointer>
#include "interfacesexport.h"
#include <util/ksharedobject.h>

class QAction;

namespace KDevelop {

///Represents a single assistant action.
///Subclass it to create your own actions.
class KDEVPLATFORMINTERFACES_EXPORT IAssistantAction : public QObject, public KSharedObject
{
    Q_OBJECT
public:
    IAssistantAction();

    using Ptr = QExplicitlySharedDataPointer<IAssistantAction>;

    ~IAssistantAction() override;

    ///Creates a QAction that represents this exact assistant action.
    ///The caller owns the action, and is responsible for deleting it.
    virtual QAction* toQAction(QObject* parent = nullptr) const;

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
    explicit AssistantLabelAction(const QString& description);
    /**
     * @return the label contents.
     */
    QString description() const override;
    /**
     * The label cannot be executed.
     */
    void execute() override;
    /**
     * No action is returned.
     */
    QAction* toQAction(QObject* parent = nullptr) const override;

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
    ~IAssistant() override;

    using Ptr = QExplicitlySharedDataPointer<IAssistant>;

    ///Returns the stored list of actions
    QList<IAssistantAction::Ptr> actions() const;

    ///Implement this and have it create the actions for your assistant.
    ///It will only be called if the assistant is displayed, which saves
    ///memory compared to creating the actions right away.
    ///Default implementation does nothing.
    virtual void createActions();

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
