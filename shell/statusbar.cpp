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

#include <QTimer>
#include <QProgressBar>

#include <istatus.h>

#include "view.h"

#include "plugincontroller.h"
#include "ilanguagecontroller.h"
#include "backgroundparser.h"
#include "core.h"

namespace KDevelop
{

StatusBar::StatusBar(QWidget* parent)
    : KStatusBar(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(slotTimeout()));
    connect(Core::self()->pluginController(), SIGNAL(pluginLoaded(KDevelop::IPlugin*)), SLOT(pluginLoaded(KDevelop::IPlugin*)));

    foreach (IPlugin* plugin, Core::self()->pluginControllerInternal()->allPluginsForExtension("IStatus", QStringList()))
        registerStatusPlugin(plugin);

    registerStatusPlugin(Core::self()->languageController()->backgroundParser());
    
    insertPermanentItem(i18n("No View Selected"), 0);
}

void StatusBar::viewChanged(Sublime::View* view)
{
    disconnect(0, 0, this, SLOT(viewStatusChanged(Sublime::View*)));
    if (view) {
        connect(view, SIGNAL(viewStatusChanged(Sublime::View*)), this, SLOT(viewStatusChanged(Sublime::View*)));
        changeItem(view->viewStatus(), 0);
    } else {
        changeItem(i18n("No View Selected"), 0);
    }
}

void StatusBar::viewStatusChanged(Sublime::View* view)
{
    changeItem(view->viewStatus(), 0);
}

void StatusBar::pluginLoaded(IPlugin* plugin)
{
    if (qobject_cast<IStatus*>(plugin))
        registerStatusPlugin(plugin);
}

void StatusBar::registerStatusPlugin(QObject* status)
{
    connect(status, SIGNAL(clearMessage()), SLOT(clearMessage()));
    connect(status, SIGNAL(showMessage(const QString&, int)), SLOT(showMessage(const QString&, int)));
    connect(status, SIGNAL(hideProgress()), SLOT(hideProgress()));
    connect(status, SIGNAL(showProgress(int, int, int)), SLOT(showProgress(int, int, int)));
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

void StatusBar::clearMessage()
{
    IStatus* status = qobject_cast<IStatus*>(sender());

    if (m_messages.contains(status)) {
        m_messages.remove(status);
        updateMessage();
    }
}

void StatusBar::showMessage(const QString & message, int timeout)
{
    IStatus* status = qobject_cast<IStatus*>(sender());

    Message m;
    m.text = message;
    m.timeout = timeout;

    m_messages.insert(status, m);

    updateMessage();
}

void StatusBar::hideProgress()
{
    IStatus* status = qobject_cast<IStatus*>(sender());

    if (m_progressBars.contains(status)) {
        delete m_progressBars[status];
        m_progressBars.remove(status);
    }
}

void StatusBar::showProgress(int minimum, int maximum, int value)
{
    IStatus* status = qobject_cast<IStatus*>(sender());

    QProgressBar* bar;

    if (m_progressBars.contains(status)) {
        bar = m_progressBars[status];
        if (bar->minimum() != minimum)
            bar->setMinimum(minimum);
        if (bar->maximum() != maximum)
            bar->setMinimum(maximum);
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
