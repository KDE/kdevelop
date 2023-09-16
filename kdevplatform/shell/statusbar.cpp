/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "statusbar.h"
#include "progresswidget/statusbarprogresswidget.h"
#include "progresswidget/progressmanager.h"
#include "progresswidget/progressdialog.h"

#include <QTimer>

#include <KColorScheme>
#include <KSqueezedTextLabel>
#include <KStatefulBrush>

#include <interfaces/istatus.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>

#include <sublime/view.h>

#include "plugincontroller.h"
#include "core.h"

namespace KDevelop
{

StatusBar::StatusBar(QWidget* parent)
    : QStatusBar(parent)
    , m_timer(new QTimer(this))
    , m_currentView(nullptr)
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

    addPermanentWidget(m_progressWidget, 0);
}

StatusBar::~StatusBar() = default;

void StatusBar::removeError(QWidget* w)
{
    removeWidget(w);
    w->deleteLater();
}

void StatusBar::viewChanged(Sublime::View* view)
{
    if (m_currentView)
        m_currentView->disconnect(this);

    m_currentView = view;

    if (view) {
        connect(view, &Sublime::View::statusChanged, this, &StatusBar::viewStatusChanged);
        QStatusBar::showMessage(view->viewStatus(), 0);

    }
}

void StatusBar::viewStatusChanged(Sublime::View* view)
{
    QStatusBar::showMessage(view->viewStatus(), 0);
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
    addWidget(error);
    timer->start(); // triggers removeError()
}

void StatusBar::slotTimeout()
{
    QMutableHashIterator<IStatus*, Message> it = m_messages;

    while (it.hasNext()) {
        it.next();
        if (it.value().timeout) {
            it.value().timeout -= m_timer->interval();
            if (it.value().timeout == 0)
                it.remove();
        }
    }

    updateMessage();
}

void StatusBar::updateMessage()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        m_timer->setInterval(m_time.elapsed());
        slotTimeout();
    }

    int timeout = 0;

    QStringList messages;
    messages.reserve(m_messages.size());
    for (const Message& m : qAsConst(m_messages)) {
        messages.append(m.text);

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
        m_time.start();
        m_timer->start(timeout);
    }
}

void StatusBar::clearMessage( IStatus* status )
{
    QTimer::singleShot(0, this, [this, status]() {
        const auto messageIt = m_messages.find(status);
        if (messageIt != m_messages.end()) {
            m_messages.erase(messageIt);
            updateMessage();
        }
    });
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
        } else {
            Message m;
            m.text = message;
            m.timeout = timeout;
            m_messages.insert(status, m);
            updateMessage();
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
            bool canBeCanceled = false;
            progressItemIt = m_progressItems.insert(status, m_progressController->createProgressItem(
                ProgressManager::createUniqueID(), status->statusName(), QString(), canBeCanceled));
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
