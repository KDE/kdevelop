/*  SPDX-License-Identifier: LGPL-2.0-or-later

    Copyright (C) 2012 Dominik Haumann <dhaumann@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
