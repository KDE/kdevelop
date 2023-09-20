/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <ghproviderwidget.h>

#include <QLabel>
#include <QListView>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/vcslocation.h>

#include <ghaccount.h>
#include <ghdialog.h>
#include <ghresource.h>
#include <ghlineedit.h>
#include <ghprovidermodel.h>

#include <KLocalizedString>
#include <KMessageBox>

using namespace KDevelop;
namespace gh
{

ProviderWidget::ProviderWidget(QWidget *parent)
    : IProjectProviderWidget(parent)
{
    auto* widgetLayout = new QVBoxLayout;
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(widgetLayout);

    m_projects = new QListView(this);
    connect(m_projects, &QListView::clicked, this, &ProviderWidget::projectIndexChanged);

    m_waiting = new QLabel(i18n("Waiting for response"), this);
    m_waiting->setAlignment(Qt::AlignCenter);
    m_waiting->hide();

    auto *model = new ProviderModel(this);
    m_projects->setModel(model);
    m_projects->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resource = new Resource(this, model);
    m_account = new Account(m_resource);
    connect(m_resource, &Resource::reposUpdated, m_waiting, &QLabel::hide);

    auto *topLayout = new QHBoxLayout();
    m_edit = new LineEdit(this);
    m_edit->setPlaceholderText(i18nc("@info:placeholder", "Search..."));
    m_edit->setToolTip(i18nc("@info:tooltip", "You can press the Return key if you do not want to wait."));
    connect(m_edit, &LineEdit::returnPressed, this, &ProviderWidget::searchRepo);
    topLayout->addWidget(m_edit);

    m_combo = new QComboBox(this);
    m_combo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProviderWidget::searchRepo);
    fillCombo();
    topLayout->addWidget(m_combo);

    auto* settings = new QPushButton(QIcon::fromTheme(QStringLiteral("configure")), QString(), this);
    settings->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    settings->setToolTip(i18nc("@info:tooltip", "Configure your GitHub account"));
    connect(settings, &QPushButton::clicked, this, &ProviderWidget::showSettings);
    topLayout->addWidget(settings);

    layout()->addItem(topLayout);
    layout()->addWidget(m_waiting);
    layout()->addWidget(m_projects);
}

KDevelop::VcsJob * ProviderWidget::createWorkingCopy(const QUrl &dest)
{
    QModelIndex pos = m_projects->currentIndex();
    if (!pos.isValid())
        return nullptr;

    auto plugin = ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IBasicVersionControl"), QStringLiteral("kdevgit"));
    if (!plugin) {
        KMessageBox::error(nullptr, i18n("The Git plugin could not be loaded which is required to import a GitHub project."), i18nc("@title:window", "GitHub Provider Error"));
        return nullptr;
    }

    QString url = pos.data(ProviderModel::VcsLocationRole).toString();
    if (m_account->validAccount())
      url = QLatin1String("https://") + m_account->token() + QLatin1Char('@') + QStringView(url).mid(8);
    QUrl real = QUrl(url);
    VcsLocation loc(real);

    auto vc = plugin->extension<IBasicVersionControl>();
    Q_ASSERT(vc);
    return vc->createWorkingCopy(loc, dest);
}

void ProviderWidget::fillCombo()
{
    m_combo->clear();
    m_combo->addItem(i18nc("@item:inlistbox", "User"), 1);
    m_combo->addItem(i18nc("@item:inlistbox", "Organization"), 3);
    if (m_account->validAccount()) {
        m_combo->addItem(m_account->name(), 0);
        m_combo->setCurrentIndex(2);
    }
    const QStringList &orgs = m_account->orgs();
    for (const QString& org : orgs)
        m_combo->addItem(org, 2);
}

bool ProviderWidget::isCorrect() const
{
    return m_projects->currentIndex().isValid();
}

void ProviderWidget::projectIndexChanged(const QModelIndex &currentIndex)
{
    if (currentIndex.isValid()) {
        QString name = currentIndex.data().toString();
        emit changed(name);
    }
}

void ProviderWidget::showSettings()
{
    auto *dialog = new Dialog(this, m_account);
    connect(dialog, &Dialog::shouldUpdate, this, &ProviderWidget::fillCombo);
    dialog->show();
}

void ProviderWidget::searchRepo()
{
    bool enabled = true;
    QString uri, text = m_edit->text();
    int idx = m_combo->itemData(m_combo->currentIndex()).toInt();

    switch (idx) {
    case 0: /* Looking for this user's repo */
        uri = QStringLiteral("/user/repos");
        enabled = false;
        break;
    case 1: /* Looking for some user's repo */
        if (text == m_account->name())
            uri = QStringLiteral("/user/repos");
        else
            uri = QStringLiteral("/users/%1/repos").arg(text);
        break;
    case 2: /* Known organization */
        text = m_combo->currentText();
        enabled = false;
        [[fallthrough]];
    default:/* Looking for some organization's repo. */
        uri = QStringLiteral("/orgs/%1/repos").arg(text);
        break;
    }
    m_edit->setEnabled(enabled);
    m_waiting->show();
    m_resource->searchRepos(uri, m_account->token());
}

} // End of namespace gh

#include "moc_ghproviderwidget.cpp"
