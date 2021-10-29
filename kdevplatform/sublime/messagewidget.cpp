/*
    SPDX-FileCopyrightText: 2012 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "messagewidget.h"

// lib
#include "message.h"
// KF
#include <KMessageWidget>
// Qt
#include <QTimer>
#include <QToolTip>
#include <QVBoxLayout>

namespace Sublime {

constexpr int s_defaultAutoHideTime = 6 * 1000;

// the enums values do not necessarily match, hence translate case-by-case
KMessageWidget::MessageType kMessageWidgetMessageType(Message::MessageType messageType)
{
    return
        messageType == Message::Positive ?    KMessageWidget::Positive :
        messageType == Message::Information ? KMessageWidget::Information :
        messageType == Message::Warning ?     KMessageWidget::Warning :
        messageType == Message::Error ?       KMessageWidget::Error :
        /* else */                            KMessageWidget::Information;
}


MessageWidget::MessageWidget(QWidget* parent)
    : QWidget(parent)
    , m_autoHideTimer(new QTimer(this))
{
    auto* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);

    m_messageWidget = new KMessageWidget(this);
    m_messageWidget->setCloseButtonVisible(false);

    l->addWidget(m_messageWidget);
    setLayout(l);

    // tell the widget to always use the minimum size.
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    // by default, hide widgets
    m_messageWidget->hide();
    hide();

    // setup autoHide timer details
    m_autoHideTimer->setSingleShot(true);

    connect(m_messageWidget, &KMessageWidget::hideAnimationFinished,
            this, &MessageWidget::showNextMessage);
    connect(m_messageWidget, &KMessageWidget::linkHovered,
            this, &MessageWidget::linkHovered);
}

void MessageWidget::showNextMessage()
{
    // at this point, we should not have a currently shown message
    Q_ASSERT(!m_currentMessage);

    // if not message to show, just stop
    if (m_messageQueue.isEmpty()) {
        hide();
        return;
    }

    // track current message
    m_currentMessage = m_messageQueue[0];

    // set text etc.
    m_messageWidget->setText(m_currentMessage->text());
    m_messageWidget->setIcon(m_currentMessage->icon());

    // connect textChanged() and iconChanged(), so it's possible to change this on the fly
    connect(m_currentMessage, &Message::textChanged,
            m_messageWidget, &KMessageWidget::setText, Qt::UniqueConnection);
    connect(m_currentMessage, &Message::iconChanged,
            m_messageWidget, &KMessageWidget::setIcon, Qt::UniqueConnection);

    const KMessageWidget::MessageType widgetMessageType =
        kMessageWidgetMessageType(m_currentMessage->messageType());
    m_messageWidget->setMessageType(widgetMessageType);

    // remove all actions from the message widget
    const auto messageWidgetActions = m_messageWidget->actions();
    for (QAction* action : messageWidgetActions) {
        m_messageWidget->removeAction(action);
    }

    // add new actions to the message widget
    const auto m_currentMessageActions = m_currentMessage->actions();
    for (QAction* action : m_currentMessageActions) {
        m_messageWidget->addAction(action);
    }

    // set word wrap of the message
    setWordWrap(m_currentMessage);

    // setup auto-hide timer, and start if requested
    m_autoHideTime = m_currentMessage->autoHide();
    m_autoHideTimer->stop();
    if (m_autoHideTime >= 0) {
        connect(m_autoHideTimer, &QTimer::timeout,
                m_currentMessage, &Message::deleteLater, Qt::UniqueConnection);
        m_autoHideTimer->start(m_autoHideTime == 0 ? s_defaultAutoHideTime : m_autoHideTime);
    }

    // finally show
    show();
    // NOTE: use a singleShot timer to avoid resizing issues when showing the message widget the first time (bug #316666)
    QTimer::singleShot(0, m_messageWidget, &KMessageWidget::animatedShow);
}

void MessageWidget::setWordWrap(Message* message)
{
    // want word wrap anyway? -> ok
    if (message->wordWrap()) {
        m_messageWidget->setWordWrap(message->wordWrap());
        return;
    }

    // word wrap not wanted, that's ok if a parent widget does not exist
    if (!parentWidget()) {
        m_messageWidget->setWordWrap(false);
        return;
    }

    // word wrap not wanted -> enable word wrap if it breaks the layout otherwise
    int margin = 0;
    if (parentWidget()->layout()) {
        // get left/right margin of the layout, since we need to subtract these
        int leftMargin = 0, rightMargin = 0;
        parentWidget()->layout()->getContentsMargins(&leftMargin, nullptr, &rightMargin, nullptr);
        margin = leftMargin + rightMargin;
    }

    // if word wrap enabled, first disable it
    if (m_messageWidget->wordWrap()) {
        m_messageWidget->setWordWrap(false);
    }

    // make sure the widget's size is up-to-date in its hidden state
    m_messageWidget->ensurePolished();
    m_messageWidget->adjustSize();

    // finally enable word wrap, if there is not enough free horizontal space
    const int freeSpace = (parentWidget()->width() - margin) - m_messageWidget->width();
    if (freeSpace < 0) {
        //     qCDebug(LOG_KTE) << "force word wrap to avoid breaking the layout" << freeSpace;
        m_messageWidget->setWordWrap(true);
    }
}

void MessageWidget::postMessage(Message* message, const QVector<QSharedPointer<QAction>>& actions)
{
    Q_ASSERT(!m_messageHash.contains(message));
    m_messageHash.insert(message, actions);

    // insert message sorted after priority
    int i = 0;
    for (; i < m_messageQueue.count(); ++i) {
        if (message->priority() > m_messageQueue[i]->priority()) {
            break;
        }
    }

    // queue message
    m_messageQueue.insert(i, message);

    // catch if the message gets deleted
    connect(message, &Message::closed,
            this, &MessageWidget::messageDestroyed);

    if (i == 0 && !m_messageWidget->isHideAnimationRunning()) {
        // if message has higher priority than the one currently shown,
        // then hide the current one and then show the new one.
        if (m_currentMessage) {
            // autoHide timer may be running for currently shown message, therefore
            // simply disconnect autoHide timer to all timeout() receivers
            disconnect(m_autoHideTimer, SIGNAL(timeout()), nullptr, nullptr);
            m_autoHideTimer->stop();

            // if there is a current message, the message queue must contain 2 messages
            Q_ASSERT(m_messageQueue.size() > 1);
            Q_ASSERT(m_currentMessage == m_messageQueue[1]);

            // a bit unnice: disconnect textChanged() and iconChanged() signals of previously visible message
            disconnect(m_currentMessage, &Message::textChanged,
                       m_messageWidget, &KMessageWidget::setText);
            disconnect(m_currentMessage, &Message::iconChanged,
                       m_messageWidget, &KMessageWidget::setIcon);

            m_currentMessage = nullptr;
            m_messageWidget->animatedHide();
        } else {
            showNextMessage();
        }
    }
}

void MessageWidget::messageDestroyed(Message* message)
{
    // last moment when message is valid, since KTE::Message is already in
    // destructor we have to do the following:
    // 1. remove message from m_messageQueue, so we don't care about it anymore
    // 2. activate hide animation or show a new message()

    // remove widget from m_messageQueue
    int i = 0;
    for (; i < m_messageQueue.count(); ++i) {
        if (m_messageQueue[i] == message) {
            break;
        }
    }

    // the message must be in the list
    Q_ASSERT(i < m_messageQueue.count());

    // remove message
    m_messageQueue.removeAt(i);

    // remove message from hash -> release QActions
    Q_ASSERT(m_messageHash.contains(message));
    m_messageHash.remove(message);

    // if deleted message is the current message, launch hide animation
    if (message == m_currentMessage) {
        m_currentMessage = nullptr;
        m_messageWidget->animatedHide();
    }
}

void MessageWidget::startAutoHideTimer()
{
    // message does not want autohide, or timer already running
    if (!m_currentMessage                        // no message, nothing to do
        || m_autoHideTime < 0                    // message does not want auto-hide
        || m_autoHideTimer->isActive()           // auto-hide timer is already active
        || m_messageWidget->isHideAnimationRunning() // widget is in hide animation phase
        || m_messageWidget->isShowAnimationRunning() // widget is in show animation phase
    ) {
        return;
    }

    // safety checks: the message must still be valid
    Q_ASSERT(m_messageQueue.size());
    Q_ASSERT(m_currentMessage->autoHide() == m_autoHideTime);

    // start autoHide timer as requested
    m_autoHideTimer->start(m_autoHideTime == 0 ? s_defaultAutoHideTime : m_autoHideTime);
}

void MessageWidget::linkHovered(const QString &link)
{
    QToolTip::showText(QCursor::pos(), link, m_messageWidget);
}

QString MessageWidget::text() const
{
    return m_messageWidget->text();
}

}
