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

#include <QtCore/QTimer>
#include <QtGui/QProgressBar>
#include <QtGui/QLabel>
#include <QtCore/QSignalMapper>

#include <KDE/KColorScheme>
#include <KDE/KDebug>

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

    foreach (IPlugin* plugin, Core::self()->pluginControllerInternal()->allPluginsForExtension("IStatus", QStringList()))
        registerStatus(plugin);

    registerStatus(Core::self()->languageController()->backgroundParser());

    connect(m_errorRemovalMapper, SIGNAL(mapped(QWidget*)), SLOT(removeError(QWidget*)));
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
    connect(status, SIGNAL(clearMessage(KDevelop::IStatus*)), SLOT(clearMessage(KDevelop::IStatus*)));
    connect(status, SIGNAL(showMessage(KDevelop::IStatus*, const QString&, int)), SLOT(showMessage(KDevelop::IStatus*, const QString&, int)));
    connect(status, SIGNAL(hideProgress(KDevelop::IStatus*)), SLOT(hideProgress(KDevelop::IStatus*)));
    connect(status, SIGNAL(showProgress(KDevelop::IStatus*,int, int, int)), SLOT(showProgress(KDevelop::IStatus*, int, int, int)));

    // Don't try to connect when the status object doesn't provide an error message signal (ie. avoid warning)
    if (status->metaObject()->indexOfSignal(SIGNAL(showErrorMessage(const QString&, int))) != -1)
        connect(status, SIGNAL(showErrorMessage(const QString&, int)), SLOT(showErrorMessage(const QString&, int)));
}

QWidget* errorMessage(QWidget* parent, const QString& text)
{
    QLabel* label = new QLabel(parent);
    KStatefulBrush red(KColorScheme::Window, KColorScheme::NegativeText);
    QPalette pal = label->palette();
    pal.setBrush(QPalette::WindowText, red.brush(label));
    label->setPalette(pal);
    label->setAlignment(Qt::AlignRight);
    label->setText(text);
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
    Message m;
    m.text = message;
    m.timeout = timeout;

    m_messages.insert(status, m);

    updateMessage();
}

void StatusBar::hideProgress( IStatus* status )
{
    if (m_progressBars.contains(status)) {
        delete m_progressBars[status];
        m_progressBars.remove(status);
    }
}

void StatusBar::showProgress( IStatus* status, int minimum, int maximum, int value)
{
    QProgressBar* bar;

    if (m_progressBars.contains(status)) {
        bar = m_progressBars[status];
        if (bar->minimum() != minimum)
            bar->setMinimum(minimum);
        if (bar->maximum() != maximum)
            bar->setMaximum(maximum);
        if (bar->value() != value)
            bar->setValue(value);

    } else {
        bar = new QProgressBar(this);
        bar->setRange(minimum, maximum);
        bar->setValue(value);
        bar->setMaximumWidth(300);
        bar->setMaximumHeight((height()*2)/3);
        m_progressBars.insert(status, bar);

        addPermanentWidget(bar);
    }
}

}

#include "statusbar.moc"
