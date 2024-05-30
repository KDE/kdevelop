/*
    SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WELCOMEPAGEVIEW_H
#define WELCOMEPAGEVIEW_H

// Qt
#include <QWidget>
// Std
#include <memory>

class SessionListModel;

namespace Ui {
class WelcomePageView;
}

class QModelIndex;

class WelcomePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePageWidget(QWidget* parent = nullptr);

private Q_SLOTS:
    void onHomepageClicked();
    void onLearnAboutTeamClicked();
    void onJoinTeamClicked();
    void onHandbookClicked();

    void onNewProjectClicked();
    void onOpenProjectClicked();
    void onFetchProjectClicked();
    void onRecentProjectsClicked();

    void onWelcomeTextLinkClicked(const QString& link);

    void onSessionEntered(const QModelIndex& index);
    void onSessionsViewportEntered();

    void onSessionClicked(const QModelIndex& index);
    void onSessionListSizeChanged(int size);

private:
    QAction* mainWindowActionById(const QString& id) const;
    void triggerMainWindowActionById(const QString& id);
    void triggerMainWindowActionMenuById(const QString& id);

private:
    std::unique_ptr<Ui::WelcomePageView> const m_ui;
    SessionListModel* const m_sessionListModel;
};

#endif
