/*
    SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "welcomepageview.h"

// plugin
#include "ui_welcomepageview.h"
#include "sessionlistmodel.h"
#include "debug.h"
// KDevPlatform
#include <shell/core.h>
#include <shell/uicontroller.h>
#include <shell/sessioncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <sublime/area.h>
#include <sublime/mainwindow.h>
// KF
#include <KAboutData>
#include <KXMLGUIFactory>
#include <KActionCollection>
#include <KIconLoader>
// Qt
#include <QSortFilterProxyModel>
#include <QDesktopServices>

WelcomePageWidget::WelcomePageWidget(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::WelcomePageView())
    , m_sessionListModel(new SessionListModel(this))
{
    m_ui->setupUi(this);
    m_ui->appIconLabel->setPixmap(
        QIcon::fromTheme(QStringLiteral("kdevelop")).pixmap(KIconLoader::global()->currentSize(KIconLoader::Desktop)));

    m_ui->pageFrame->setBackgroundRole(QPalette::Base);

    auto* sessionsModelSortProxyModel = new QSortFilterProxyModel(this);
    sessionsModelSortProxyModel->setSourceModel(m_sessionListModel);
    sessionsModelSortProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    sessionsModelSortProxyModel->sort(0);

    m_ui->sessionsListView->setModel(sessionsModelSortProxyModel);

#ifdef Q_OS_WIN
    m_ui->welcomeText->setText(
        m_ui->welcomeText->text()
        + i18n("<br/>\n"
               "<h3>Note for Windows users</h3>\n"
               "<p>Note that KDevelop does NOT ship a C/C++ compiler on Windows!</p>\n"
               "<p>You need to install either GCC via MinGW or install a recent version of the Microsoft Visual Studio "
               "IDE and make sure the environment is setup correctly <i>before</i> starting KDevelop.</p>\n"
               "<p>If you need further assistance, please check out the <a "
               "href=\"https://userbase.kde.org/KDevelop4/Manual/WindowsSetup\">KDevelop under Windows "
               "instructions.</a></p>"));
#endif
    connect(m_ui->welcomeText, &QLabel::linkActivated, this, &WelcomePageWidget::onWelcomeTextLinkClicked);

    onSessionListSizeChanged(m_sessionListModel->size());
    connect(m_sessionListModel, &SessionListModel::sizeChanged, this, &WelcomePageWidget::onSessionListSizeChanged);

    connect(m_ui->sessionsListView, &QAbstractItemView::clicked, this, &WelcomePageWidget::onSessionClicked);

    // track mouse entering indexes and the viewport to control the mouse cursor shape over items
    connect(m_ui->sessionsListView, &QAbstractItemView::entered, this, &WelcomePageWidget::onSessionEntered);
    connect(m_ui->sessionsListView, &QAbstractItemView::viewportEntered, this,
            &WelcomePageWidget::onSessionsViewportEntered);

    connect(m_ui->homepageLinkLabel, &KUrlLabel::leftClickedUrl, this, &WelcomePageWidget::onHomepageClicked);
    connect(m_ui->learnAboutLinkLabel, &KUrlLabel::leftClickedUrl, this, &WelcomePageWidget::onLearnAboutTeamClicked);
    connect(m_ui->joinTeamLinkLabel, &KUrlLabel::leftClickedUrl, this, &WelcomePageWidget::onJoinTeamClicked);
    connect(m_ui->handbookLinkLabel, &KUrlLabel::leftClickedUrl, this, &WelcomePageWidget::onHandbookClicked);

    connect(m_ui->newProjectButton, &QPushButton::clicked, this, &WelcomePageWidget::onNewProjectClicked);
    connect(m_ui->openProjectButton, &QPushButton::clicked, this, &WelcomePageWidget::onOpenProjectClicked);
    connect(m_ui->fetchProjectButton, &QPushButton::clicked, this, &WelcomePageWidget::onFetchProjectClicked);
    connect(m_ui->recentProjectsButton, &QPushButton::clicked, this, &WelcomePageWidget::onRecentProjectsClicked);
}

QAction* WelcomePageWidget::mainWindowActionById(const QString& id) const
{
    const auto clients = KDevelop::Core::self()->uiController()->activeMainWindow()->guiFactory()->clients();
    for (auto* const client : clients) {
        auto* const action = client->actionCollection()->action(id);
        if (action) {
            return action;
        }
    }
    return nullptr;
}

void WelcomePageWidget::triggerMainWindowActionById(const QString& id)
{
    auto* const action = mainWindowActionById(id);
    if (!action) {
        qCWarning(PLUGIN_WELCOMEPAGE) << "Action not found in mainwindow:" << id;
        return;
    }

    action->trigger();
}

void WelcomePageWidget::triggerMainWindowActionMenuById(const QString& id)
{
    auto* const action = mainWindowActionById(id);
    if (!action) {
        qCWarning(PLUGIN_WELCOMEPAGE) << "Action not found in mainwindow:" << id;
        return;
    }
    auto* const menu = action->menu();
    if (!menu) {
        qCWarning(PLUGIN_WELCOMEPAGE) << "Action has no menu:" << id;
        return;
    }

    menu->popup(QCursor::pos());
}

void WelcomePageWidget::onWelcomeTextLinkClicked(const QString& link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void WelcomePageWidget::onHomepageClicked()
{
    QDesktopServices::openUrl(QUrl(KAboutData::applicationData().homepage()));
}

void WelcomePageWidget::onLearnAboutTeamClicked()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://userbase.kde.org/KDevelop")));
}

void WelcomePageWidget::onJoinTeamClicked()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://kdevelop.org/contribute-kdevelop")));
}

void WelcomePageWidget::onHandbookClicked()
{
    triggerMainWindowActionById(QStringLiteral("help_contents"));
}

void WelcomePageWidget::onNewProjectClicked()
{
    triggerMainWindowActionById(QStringLiteral("project_new"));
}

void WelcomePageWidget::onOpenProjectClicked()
{
    triggerMainWindowActionById(QStringLiteral("project_open"));
}

void WelcomePageWidget::onFetchProjectClicked()
{
    triggerMainWindowActionById(QStringLiteral("project_fetch"));
}

void WelcomePageWidget::onRecentProjectsClicked()
{
    triggerMainWindowActionMenuById(QStringLiteral("project_open_recent"));
}

void WelcomePageWidget::onSessionEntered(const QModelIndex& index)
{
    if (index.isValid()) {
        m_ui->sessionsListView->setCursor(Qt::PointingHandCursor);
    }
}

void WelcomePageWidget::onSessionsViewportEntered()
{
    m_ui->sessionsListView->unsetCursor();
}

void WelcomePageWidget::onSessionClicked(const QModelIndex& index)
{
    const auto id = index.data(SessionListModel::SessionIdRole).toString();
    if (id.isEmpty()) {
        return;
    }
    KDevelop::Core::self()->sessionController()->loadSession(id);
}

void WelcomePageWidget::onSessionListSizeChanged(int size)
{
    // we always have at least one active session
    const bool showWelcomeText = (size < 2);
    m_ui->sessionsOrWelcomeWidget->setCurrentIndex(showWelcomeText ? 1 : 0);
}

#include "moc_welcomepageview.cpp"
