/*
    SPDX-FileCopyrightText: 2012-2013 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Forked from KTextEditor::Message at v5.66.0
// Dropped Document/View properties, made wordWrap true by default, dropped position & autoHideMode
// Should be investigated later to turn this and the messagewidget class
// into some reusable generic in-shell-message code, e.g. as KF module
// TODO: re-add autoHideMode once there is an idea how to determine user interaction with the shell

#ifndef KDEVPLATFORM_SUBLIME_MESSAGE_H
#define KDEVPLATFORM_SUBLIME_MESSAGE_H

#include "sublimeexport.h"
// Qt
#include <QAction>
#include <QIcon>
#include <QVector>
#include <QObject>

namespace Sublime
{

/**
 * @brief This class holds a Message to display in a message area.
 *
 * @section message_intro Introduction
 *
 * The Message class holds the data used to display interactive message widgets
 * in the shell. Use the MainWindow::postMessage() to post a message as follows:
 *
 * @code
 * // if you keep a pointer after calling postMessage(),
 * // always use a QPointer go guard your Message, 
 * QPointer<Sublime::Message> message =
 *     new Sublime::Message("text", Sublime::Message::Information);
 * message->addAction(...); // add your actions, if any...
 * window->postMessage(message);
 * @endcode
 *
 * A Message is deleted automatically if the Message gets closed, meaning that
 * you usually can forget the pointer. If you really need to delete a message
 * before the user processed it, always guard it with a QPointer!
 *
 * @section message_creation Message Creation and Deletion
 *
 * To create a new Message, use code like this:
 * @code
 * QPointer<Sublime::Message> message =
 *     new Sublime::Message("My information text", Message::Information);
 * // ...
 * @endcode
 *
 * Although discouraged in general, the text of the Message can be changed
 * on the fly when it is already visible with setText().
 *
 * Once you posted the Message through MainWindow::postMessage(), the
 * lifetime depends on the user interaction. The Message gets automatically
 * deleted if the user clicks a closing action in the message, or the set
 * timeout is reached.
 *
 * If you posted a message but want to remove it yourself again, just delete
 * the message. But beware of the following warning!
 *
 * @warning Always use QPointer\<Message\> to guard the message pointer from
 *          getting invalid, if you need to access the Message after you posted
 *          it.
 *
 * @section message_hiding Autohiding Messages
 *
 * Message%s can be shown for only a short amount of time by using the autohide
 * feature. With setAutoHide() a timeout in milliseconds can be set after which
 * the Message is automatically hidden.
 */
class KDEVPLATFORMSUBLIME_EXPORT Message : public QObject
{
    Q_OBJECT

    //
    // public data types
    //
public:
    /**
     * Message types used as visual indicator.
     * The message types match exactly the behavior of KMessageWidget::MessageType.
     * For simple notifications either use Positive or Information.
     */
    enum MessageType {
        Positive = 0, ///< positive information message
        Information,  ///< information message type
        Warning,      ///< warning message type
        Error         ///< error message type
    };

public:
    /**
     * Constructor for new messages.
     * @param type the message type, e.g. MessageType::Information
     * @param richtext text to be displayed
     */
    explicit Message(const QString& richtext, MessageType type = Message::Information);

    /**
     * Destructor.
     */
    ~Message() override;

public:
    /**
     * Returns the text set in the constructor.
     */
    QString text() const;

    /**
     * Returns the icon of this message.
     * If the message has no icon set, a null icon is returned.
     * @see setIcon()
     */
    QIcon icon() const;

    /**
     * Returns the message type set in the constructor.
     */
    MessageType messageType() const;

    /**
     * Adds an action to the message.
     *
     * By default (@p closeOnTrigger = true), the action closes the message
     * displayed. If @p closeOnTrigger is @e false, the message is stays open.
     *
     * The actions will be displayed in the order you added the actions.
     *
     * To connect to an action, use the following code:
     * @code
     * connect(action, &QAction::triggered, receiver, &ReceiverType::slotActionTriggered);
     * @endcode
     *
     * @param action action to be added
     * @param closeOnTrigger when triggered, the message widget is closed
     *
     * @warning The added actions are deleted automatically.
     *          So do @em not delete the added actions yourself.
     */
    void addAction(QAction* action, bool closeOnTrigger = true);

    /**
     * Accessor to all actions, mainly used in the internal implementation
     * to add the actions into the gui.
     * @see addAction()
     */
    QVector<QAction*> actions() const;

    /**
     * Set the auto hide time to @p delay milliseconds.
     * If @p delay < 0, auto hide is disabled.
     * If @p delay = 0, auto hide is enabled and set to a sane default
     * value of several seconds.
     *
     * By default, auto hide is disabled.
     *
     * @see autoHide()
     */
    void setAutoHide(int delay = 0);

    /**
     * Returns the auto hide time in milliseconds.
     * Please refer to setAutoHide() for an explanation of the return value.
     *
     * @see setAutoHide()
     */
    int autoHide() const;

    /**
     * Enabled word wrap according to @p wordWrap.
     * By default, auto wrap is enabled.
     *
     * Word wrap is enabled automatically, if the Message's width is larger than
     * the parent widget's width to avoid breaking the gui layout.
     *
     * @see wordWrap()
     */
    void setWordWrap(bool wordWrap);

    /**
     * Check, whether word wrap is enabled or not.
     *
     * @see setWordWrap()
     */
    bool wordWrap() const;

    /**
     * Set the priority of this message to @p priority.
     * Messages with higher priority are shown first.
     * The default priority is 0.
     *
     * @see priority()
     */
    void setPriority(int priority);

    /**
     * Returns the priority of the message.
     *
     * @see setPriority()
     */
    int priority() const;

public Q_SLOTS:
    /**
     * Sets the notification contents to @p richtext.
     * If the message was already sent through MainWindow::postMessage(),
     * the displayed text changes on the fly.
     * @note Change text on the fly with care, since changing the text may
     *       resize the notification widget, which may result in a distracting
     *       user experience.
     * @param richtext new notification text (rich text supported)
     * @see textChanged()
     */
    void setText(const QString& richtext);

    /**
     * Add an optional @p icon for this notification which will be shown next to
     * the message text. If the message was already sent through MainWindow::postMessage(),
     * the displayed icon changes on the fly.
     * @note Change the icon on the fly with care, since changing the text may
     *       resize the notification widget, which may result in a distracting
     *       user experience.
     * @param icon the icon to be displayed
     * @see iconChanged()
     */
    void setIcon(const QIcon& icon);

Q_SIGNALS:
    /**
     * This signal is emitted before the @p message is deleted. Afterwards, this
     * pointer is invalid.
     *
     * @param message the closed/processed message
     */
    void closed(Message* message);

    /**
     * This signal is emitted whenever setText() was called.
     *
     * @param text the new notification text (rich text supported)
     * @see setText()
     */
    void textChanged(const QString& text);

    /**
     * This signal is emitted whenever setIcon() was called.
     * @param icon the new notification icon
     * @see setIcon()
     */
    void iconChanged(const QIcon& icon);

private:
    const QScopedPointer<class MessagePrivate> d;
};

}

#endif
