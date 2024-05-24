/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_STATUSBAR_H
#define KDEVPLATFORM_STATUSBAR_H

#include <QElapsedTimer>
#include <QHash>
#include <QStatusBar>

class QDebug;
class QTimer;

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
class StatusBar : public QStatusBar
{
    Q_OBJECT

public:
    /**
     * Constructs a status bar.
     */
    explicit StatusBar(QWidget* parent);
    ~StatusBar() override;

    void registerStatus(QObject* status);

    void updateMessage();

public Q_SLOTS:
    void showErrorMessage(const QString& message, int timeout);
    
private Q_SLOTS:
    void clearMessage( KDevelop::IStatus* );
    void showMessage( KDevelop::IStatus*, const QString & message, int timeout);
    void hideProgress( KDevelop::IStatus* );
    void showProgress( KDevelop::IStatus*, int minimum, int maximum, int value);
    void slotTimeout();

    void pluginLoaded(KDevelop::IPlugin*);
    void removeError(QWidget*);

private:
    QTimer* errorTimeout(QWidget* error, int timeout);

private:
    struct Message {
        QString text;
        int timeout;
    };
    friend QDebug operator<<(QDebug debug, const Message& m);

    QHash<IStatus*, Message> m_messages;
    QTimer* const m_timer;
    QElapsedTimer m_time;
    QHash<IStatus*,ProgressItem*> m_progressItems;
    StatusbarProgressWidget* m_progressWidget; // embedded in the statusbar, shows a single progressbar & button to expand the overlay widget
    ProgressDialog* m_progressDialog; // the actual overlay widget that contains multiple progressbars and status messages
    ProgressManager* m_progressController; // progress item model
};

}

#endif // KDEVPLATFORM_STATUSBAR_H
