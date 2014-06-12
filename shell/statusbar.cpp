/* This file is part of the KDE project
   Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#include "statusbar.h"
#include "progresswidget/statusbarprogresswidget.h"
#include "progresswidget/progressmanager.h"
#include "progresswidget/progressdialog.h"

#include <QtCore/QTimer>
#include <QProgressBar>
#include <QLabel>
#include <QtCore/QSignalMapper>

#include <KDE/KColorScheme>
#include <KDE/KDebug>
#include <KSqueezedTextLabel>

#include <interfaces/istatus.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>

#include <sublime/view.h>

#include "plugincontroller.h"
#include "core.h"

namespace KDevelop
{

StatusBar::StatusBar(QWidget* parent)
    : KStatusBar(parent)
    , m_timer(new QTimer(this))
    , m_currentView(0)
    , m_errorRemovalMapper(new QSignalMapper(this))
{
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(slotTimeout()));
    connect(Core::self()->pluginController(), SIGNAL(pluginLoaded(KDevelop::IPlugin*)), SLOT(pluginLoaded(KDevelop::IPlugin*)));

    foreach (IPlugin* plugin, Core::self()->pluginControllerInternal()->allPluginsForExtension("IStatus"))
        registerStatus(plugin);

    registerStatus(Core::self()->languageController()->backgroundParser());

    connect(m_errorRemovalMapper, SIGNAL(mapped(QWidget*)), SLOT(removeError(QWidget*)));

    m_progressController = Core::self()->progressController();
    m_progressDialog = new ProgressDialog(this, parent); // construct this first, then progressWidget
    m_progressDialog->setVisible(false);
    m_progressWidget = new StatusbarProgressWidget(m_progressDialog, this);

    addPermanentWidget(m_progressWidget, 0);
}

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
        connect(view, SIGNAL(statusChanged(Sublime::View*)), this, SLOT(viewStatusChanged(Sublime::View*)));
        changeItem(view->viewStatus(), 0);

    }
}

void StatusBar::viewStatusChanged(Sublime::View* view)
{
    changeItem(view->viewStatus(), 0);
}

void StatusBar::pluginLoaded(IPlugin* plugin)
{
    if (qobject_cast<IStatus*>(plugin))
        registerStatus(plugin);
}

void StatusBar::registerStatus(QObject* status)
{
    Q_ASSERT(qobject_cast<IStatus*>(status));
    connect(status, SIGNAL(clearMessage(KDevelop::IStatus*)),
            SLOT(clearMessage(KDevelop::IStatus*)),
            Qt::QueuedConnection);
    connect(status, SIGNAL(showMessage(KDevelop::IStatus*,QString,int)),
            SLOT(showMessage(KDevelop::IStatus*,QString,int)),
            Qt::QueuedConnection);
    connect(status, SIGNAL(hideProgress(KDevelop::IStatus*)),
            SLOT(hideProgress(KDevelop::IStatus*)),
            Qt::QueuedConnection);
    connect(status, SIGNAL(showProgress(KDevelop::IStatus*,int,int,int)),
            SLOT(showProgress(KDevelop::IStatus*,int,int,int)),
            Qt::QueuedConnection);

    // Don't try to connect when the status object doesn't provide an error message signal (ie. avoid warning)
    if (status->metaObject()->indexOfSignal(SIGNAL(showErrorMessage(QString,int))) != -1) {
        connect(status, SIGNAL(showErrorMessage(QString,int)),
                SLOT(showErrorMessage(QString,int)), Qt::QueuedConnection);
    }
}

QWidget* errorMessage(QWidget* parent, const QString& text)
{
    KSqueezedTextLabel* label = new KSqueezedTextLabel(parent);
    KStatefulBrush red(KColorScheme::Window, KColorScheme::NegativeText);
    QPalette pal = label->palette();
    pal.setBrush(QPalette::WindowText, red.brush(label));
    label->setPalette(pal);
    label->setAlignment(Qt::AlignRight);
    label->setText(text);
    label->setToolTip(text);
    return label;
}

QTimer* StatusBar::errorTimeout(QWidget* error, int timeout)
{
    QTimer* timer = new QTimer(error);
    timer->setSingleShot(true);
    timer->setInterval(1000*timeout);
    m_errorRemovalMapper->setMapping(timer, error);
    connect(timer, SIGNAL(timeout()), m_errorRemovalMapper, SLOT(map()));
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
    QMutableMapIterator<IStatus*, Message> it = m_messages;

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

    QString ret;
    int timeout = 0;

    foreach (const Message& m, m_messages) {
        if (!ret.isEmpty())
            ret += "; ";

        ret += m.text;

        if (timeout)
            timeout = qMin(timeout, m.timeout);
        else
            timeout = m.timeout;
    }

    if (!ret.isEmpty())
        KStatusBar::showMessage(ret);
    else
        KStatusBar::clearMessage();

    if (timeout) {
        m_time.start();
        m_timer->start(timeout);
    }
}

void StatusBar::clearMessage( IStatus* status )
{
    if (m_messages.contains(status)) {
        m_messages.remove(status);
        updateMessage();
    }
}

void StatusBar::showMessage( IStatus* status, const QString & message, int timeout)
{
    if ( m_progressItems.contains(status) ) {
        ProgressItem* i = m_progressItems[status];
        i->setStatus(message);
    } else {
        Message m;
        m.text = message;
        m.timeout = timeout;
        m_messages.insert(status, m);
        updateMessage();
    }
}

void StatusBar::hideProgress( IStatus* status )
{
   if (m_progressItems.contains(status)) {
        m_progressItems[status]->setComplete();
        m_progressItems.remove(status);
   }

}

void StatusBar::showProgress( IStatus* status, int minimum, int maximum, int value)
{
    if (!m_progressItems.contains(status)) {
        bool canBeCanceled = false;
        m_progressItems[status] = m_progressController->createProgressItem(
            ProgressManager::getUniqueID(), status->statusName(), QString(), canBeCanceled);;
    }
    
    ProgressItem* i = m_progressItems[status];
    m_progressWidget->raise();
    m_progressDialog->raise();
    if( minimum == 0 && maximum == 0 ) {
        i->setUsesBusyIndicator( true );
    } else {
        i->setUsesBusyIndicator( false );
        i->setProgress( 100*value/maximum );
    }
}

}

#include "statusbar.moc"
