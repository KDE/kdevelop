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

#ifndef KDEVSTATUSBAR_H
#define KDEVSTATUSBAR_H

#include <QTime>
#include <QMap>

#include <KStatusBar>

class QProgressBar;
class QTimer;

namespace Sublime {
class View;
}

namespace KDevelop
{

class IStatus;
class IPlugin;

/**
 * Status bar
 */
class StatusBar : public KStatusBar
{
    Q_OBJECT

public:
    /**
     * Constructs a status bar.
     */
    StatusBar(QWidget* parent);

    void registerStatusPlugin(QObject* status);

    void updateMessage();

    void viewChanged(Sublime::View* view);

private Q_SLOTS:
    void clearMessage();
    void showMessage(const QString & message, int timeout);
    void hideProgress();
    void showProgress(int minimum, int maximum, int value);
    void slotTimeout();
    void viewStatusChanged(Sublime::View* view);

    void pluginLoaded(KDevelop::IPlugin*);
    
private:
    struct Message {
        QString text;
        int timeout;
    };

    QMap<IStatus*, Message> m_messages;
    QMap<IStatus*, QProgressBar*> m_progressBars;
    QTimer* m_timer;
    QTime m_time;
};

}

#endif // KDEVSTATUSBAR_H
