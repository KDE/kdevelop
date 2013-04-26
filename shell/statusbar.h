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

#ifndef KDEVPLATFORM_STATUSBAR_H
#define KDEVPLATFORM_STATUSBAR_H

#include <QtCore/QTime>
#include <QtCore/QMap>

#include <KDE/KStatusBar>

class QProgressBar;
class QSignalMapper;
class QTimer;
class QLabel;

namespace Sublime {
class View;
}

namespace KDevelop
{

class IStatus;
class IPlugin;
class ProgressItem;
class ProgressDialog;
class StatusbarProgressWidget;
class ProgressManager;

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

    void registerStatus(QObject* status);

    void updateMessage();

    void viewChanged(Sublime::View* view);

public Q_SLOTS:
    void showErrorMessage(const QString& message, int timeout);
    
private Q_SLOTS:
    void clearMessage( KDevelop::IStatus* );
    void showMessage( KDevelop::IStatus*, const QString & message, int timeout);
    void hideProgress( KDevelop::IStatus* );
    void showProgress( KDevelop::IStatus*, int minimum, int maximum, int value);
    void slotTimeout();
    void viewStatusChanged(Sublime::View* view);

    void pluginLoaded(KDevelop::IPlugin*);
    void removeError(QWidget*);

private:
    QTimer* errorTimeout(QWidget* error, int timeout);

private:
    struct Message {
        QString text;
        int timeout;
    };

    QMap<IStatus*, Message> m_messages;
    QTimer* m_timer;
    QTime m_time;
    Sublime::View* m_currentView;
    QSignalMapper* m_errorRemovalMapper;
    QMap<IStatus*,ProgressItem*> m_progressItems;
    StatusbarProgressWidget* m_progressWidget; // embedded in the statusbar, shows a single progressbar & button to expand the overlay widget
    ProgressDialog* m_progressDialog; // the actual overlay widget that contains multiple progressbars and status messages
    ProgressManager* m_progressController; // progress item model
};

}

#endif // KDEVPLATFORM_STATUSBAR_H
