/*
    SPDX-FileCopyrightText: 2012 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Forked from KTextEditor's KateMessageWidget at v5.66.0
// Renamed class, dropped fading-enabling KateAnimation proxy

#ifndef KDEVPLATFORM_SUBLIME_MESSAGEWIDGET_H
#define KDEVPLATFORM_SUBLIME_MESSAGEWIDGET_H

// Qt
#include <QHash>
#include <QPointer>
#include <QWidget>
#include <QVector>
#include <QList>

class KMessageWidget;

namespace Sublime {

class Message;

/**
 * This class implements a message widget based on KMessageWidget.
 * It is used to show messages through the KTextEditior::MessageInterface.
 */
class MessageWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor. By default, the widget is hidden.
     */
    explicit MessageWidget(QWidget* parent = nullptr);

public:
    /**
     * Post a new incoming message. Show either directly, or queue
     */
    void postMessage(Message* message, const QVector<QSharedPointer<QAction>>& actions);

    // for unit test
    QString text() const;

protected Q_SLOTS:
    /**
     * Show the next message in the queue.
     */
    void showNextMessage();

    /**
     * Helper that enables word wrap to avoid breaking the layout
     */
    void setWordWrap(Message* message);

    /**
     * catch when a message is deleted, then show next one, if applicable.
     */
    void messageDestroyed(Message* message);
    /**
     * Start autoHide timer if requested
     */
    void startAutoHideTimer();
    /**
     * User hovers on a link in the message widget.
     */
    void linkHovered(const QString& link);

private:
    // sorted list of pending messages
    QList<Message*> m_messageQueue;
    // pointer to current Message
    QPointer<Message> m_currentMessage;
    // shared pointers to QActions as guard
    QHash<Message*, QVector<QSharedPointer<QAction>>> m_messageHash;
    // the message widget, showing the actual contents
    KMessageWidget* m_messageWidget;

private: // some state variables
    // autoHide only once user interaction took place
    QTimer* m_autoHideTimer;
    // flag: save message's autohide time
    int m_autoHideTime = -1;
};

}

#endif
