/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "statusbar.h"
#include "progresswidget/statusbarprogresswidget.h"
#include "progresswidget/progressmanager.h"
#include "progresswidget/progressdialog.h"

#include <QDebug>
#include <QTimer>

#include <KColorScheme>
#include <KSqueezedTextLabel>
#include <KStatefulBrush>

#include <interfaces/istatus.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>

#include "plugincontroller.h"
#include "core.h"

#include <utility>

namespace KDevelop
{
QDebug operator<<(QDebug debug, const StatusBar::Message& s)
{
    const QDebugStateSaver saver(debug);
    debug.nospace() << '{' << s.text << ", " << s.timeout << '}';
    return debug;
}

StatusBar::StatusBar(QWidget* parent)
    : QStatusBar(parent)
    , m_timer(new QTimer(this))
{
#ifdef Q_OS_MAC
    /* At time of writing this is only required for OSX and only has effect on OSX. 
       ifdef for robustness to future platform dependent theme/widget changes
       https://phabricator.kde.org/D656 
    */
    setStyleSheet(QStringLiteral("QStatusBar{background:transparent;}"));
#endif

    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &StatusBar::slotTimeout);
    connect(Core::self()->pluginController(), &IPluginController::pluginLoaded, this, &StatusBar::pluginLoaded);
    const QList<IPlugin*> plugins = Core::self()->pluginControllerInternal()->allPluginsForExtension(QStringLiteral("IStatus"));

    for (IPlugin* plugin : plugins) {
        registerStatus(plugin);
    }

    registerStatus(Core::self()->languageController()->backgroundParser());

    m_progressController = Core::self()->progressController();
    m_progressDialog = new ProgressDialog(this, parent); // construct this first, then progressWidget
    m_progressDialog->setVisible(false);
    m_progressWidget = new StatusbarProgressWidget(m_progressDialog, this);
    // Fix the progress widget's width to leave the rest of the horizontal space to status messages.
    m_progressWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    addPermanentWidget(m_progressWidget, 0);
}

StatusBar::~StatusBar() = default;

void StatusBar::removeError(QWidget* w)
{
    removeWidget(w);
    w->deleteLater();
}

void StatusBar::pluginLoaded(IPlugin* plugin)
{
    if (qobject_cast<IStatus*>(plugin))
        registerStatus(plugin);
}

void StatusBar::registerStatus(QObject* status)
{
    Q_ASSERT(qobject_cast<IStatus*>(status));
    // can't convert this to new signal slot syntax, IStatus is not a QObject
    connect(status, SIGNAL(clearMessage(KDevelop::IStatus*)),
            SLOT(clearMessage(KDevelop::IStatus*)));
    connect(status, SIGNAL(showMessage(KDevelop::IStatus*,QString,int)),
            SLOT(showMessage(KDevelop::IStatus*,QString,int)));
    connect(status, SIGNAL(hideProgress(KDevelop::IStatus*)),
            SLOT(hideProgress(KDevelop::IStatus*)));
    connect(status, SIGNAL(showProgress(KDevelop::IStatus*,int,int,int)),
            SLOT(showProgress(KDevelop::IStatus*,int,int,int)));
    connect(status, SIGNAL(showErrorMessage(QString,int)),
            SLOT(showErrorMessage(QString,int)));
}

QWidget* errorMessage(QWidget* parent, const QString& text)
{
    auto* label = new KSqueezedTextLabel(parent);
    KStatefulBrush red(KColorScheme::Window, KColorScheme::NegativeText);
    QPalette pal = label->palette();
    pal.setBrush(QPalette::WindowText, red.brush(label->palette()));
    label->setPalette(pal);
    label->setAlignment(Qt::AlignRight);
    label->setText(text);
    label->setToolTip(text);
    return label;
}

QTimer* StatusBar::errorTimeout(QWidget* error, int timeout)
{
    auto* timer = new QTimer(error);
    timer->setSingleShot(true);
    timer->setInterval(1000*timeout);
    connect(timer, &QTimer::timeout, this, [this, error](){ removeError(error); });
    return timer;
}

void StatusBar::showErrorMessage(const QString& message, int timeout)
{
    QWidget* error = errorMessage(this, message);
    QTimer* timer = errorTimeout(error, timeout);
    insertPermanentWidget(0, error);
    timer->start(); // triggers removeError()
}

void StatusBar::subtractFromEachMessageTimeout(int subtrahend, const IStatus* exceptThisMessage)
{
    Q_ASSERT(subtrahend > 0);

    QMutableHashIterator<IStatus*, Message> it = m_messages;

    while (it.hasNext()) {
        it.next();
        if (it.key() != exceptThisMessage && it.value().timeout > 0) {
            it.value().timeout -= subtrahend;
            if (it.value().timeout <= 0)
                it.remove();
        }
    }
}

void StatusBar::slotTimeout()
{
    // In theory, the actual elapsed time should be subtracted from each message's timeout. However,
    // our coarse timer can time out several milliseconds earlier, in which case the elapsed time would be
    // slightly less than m_timer->interval(), which is equal to the minimum positive status message timeout.
    // So subtract the timer interval instead in order to avoid the redundant, pedantic timer restarting
    // for several milliseconds remaining in the minimum message timeout.
    subtractFromEachMessageTimeout(m_timer->interval());

    updateMessage();
}

void StatusBar::updateMessage(const IStatus* justInsertedMessage)
{
    if (m_timer->isActive()) {
        m_timer->stop();
        if (const auto subtrahend = m_time.elapsed()) {
            subtractFromEachMessageTimeout(subtrahend, justInsertedMessage);
        }
    }

    int timeout = 0;

    QStringList messages;
    messages.reserve(m_messages.size());
    for (const Message& m : std::as_const(m_messages)) {
        if (m.timeout < 0) {
            continue;
        }
        messages.append(m.text);

        if (m.timeout == 0) {
            continue;
        }
        if (timeout)
            timeout = qMin(timeout, m.timeout);
        else
            timeout = m.timeout;
    }

    if (!messages.isEmpty())
        QStatusBar::showMessage(messages.join(QLatin1String("; ")));
    else
        QStatusBar::clearMessage();

    if (timeout) {
        Q_ASSERT(timeout > 0);
        m_time.start();
        m_timer->start(timeout);
    }
}

void StatusBar::clearMessage( IStatus* status )
{
    QTimer::singleShot(0, this, [this, status]() {
        takeMessage(status);
    });
}

QString StatusBar::takeMessage(IStatus* status)
{
    auto message = m_messages.take(status);
    // Messages with empty text are never inserted into m_messages. Therefore, message.text is empty here
    // only if the message was default-constructed due to missing key equal to @p status in m_messages.
    if (!message.text.isEmpty() && message.timeout >= 0) {
        updateMessage();
    }
    return std::move(message.text);
}

void StatusBar::showMessage( IStatus* status, const QString & message, int timeout)
{
    QPointer<QObject> context = dynamic_cast<QObject*>(status);
    QTimer::singleShot(0, this, [this, context, status, message, timeout]() {
        if (!context)
            return;
        const auto progressItemIt = m_progressItems.constFind(status);
        if (progressItemIt != m_progressItems.constEnd()) {
            ProgressItem* i = *progressItemIt;
            i->setStatus(message);
        } else if (!message.isEmpty()) { // do not display a semicolon and empty space in the aggregate status message
            Message m;
            m.text = message;
            m.timeout = timeout;
            m_messages.insert(status, m);
            if (timeout >= 0) {
                updateMessage(status);
            }
        }
    });
}

void StatusBar::hideProgress( IStatus* status )
{
    QTimer::singleShot(0, this, [this, status]() {
        const auto progressItemIt = m_progressItems.find(status);
        if (progressItemIt != m_progressItems.end()) {
            (*progressItemIt)->setComplete();
            m_progressItems.erase(progressItemIt);
        }
    });
}

void StatusBar::showProgress( IStatus* status, int minimum, int maximum, int value)
{
    QPointer<QObject> context = dynamic_cast<QObject*>(status);
    QTimer::singleShot(0, this, [this, context, status, minimum, maximum, value]() {
        if (!context)
            return;
        auto progressItemIt = m_progressItems.find(status);
        if (progressItemIt == m_progressItems.end()) {
            // Transfer an existing status message (if any) to the new progress item where it belongs.
            const auto statusMessage = takeMessage(status);
            bool canBeCanceled = false;
            progressItemIt = m_progressItems.insert(status, m_progressController->createProgressItem(
                ProgressManager::createUniqueID(), status->statusName(), statusMessage, canBeCanceled));
        }

        ProgressItem* i = *progressItemIt;
        m_progressWidget->raise();
        m_progressDialog->raise();
        if( minimum == 0 && maximum == 0 ) {
            i->setUsesBusyIndicator( true );
        } else {
            i->setUsesBusyIndicator( false );
            i->setProgress( 100*value/maximum );
        }
    });
}

}

#include "moc_statusbar.cpp"
