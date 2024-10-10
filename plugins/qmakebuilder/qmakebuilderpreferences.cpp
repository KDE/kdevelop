/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakebuilderpreferences.h"

#include <QIcon>

#include <KIO/DeleteJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KJobWidgets>

#include "ui_qmakeconfig.h"
#include "qmakebuilddirchooser.h"
#include "qmakebuilddirchooserdialog.h"
#include "qmakeconfig.h"
#include <debug.h>

#include <interfaces/iproject.h>

QMakeBuilderPreferences::QMakeBuilderPreferences(KDevelop::IPlugin* plugin,
                                                 const KDevelop::ProjectConfigOptions& options, QWidget* parent)
    : KDevelop::ConfigPage(plugin, nullptr, parent)
    , m_project(options.project)
{
    m_prefsUi = new Ui::QMakeConfig;
    m_prefsUi->setupUi(this);

    m_chooserUi = new QMakeBuildDirChooser(m_project);
    auto groupBoxLayout = new QVBoxLayout(m_prefsUi->groupBox);
    groupBoxLayout->addWidget(m_chooserUi);

    m_chooserUi->kcfg_buildDir->setEnabled(false); // build directory MUST NOT be changed here
    connect(m_chooserUi, &QMakeBuildDirChooser::changed, this, &QMakeBuilderPreferences::changed);
    connect(m_chooserUi, &QMakeBuildDirChooser::changed, this, &QMakeBuilderPreferences::validate);

    connect(m_prefsUi->buildDirCombo, &QComboBox::currentTextChanged, this, &QMakeBuilderPreferences::loadOtherConfig);
    connect(m_prefsUi->buildDirCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &QMakeBuilderPreferences::changed);
    connect(m_prefsUi->addButton, &QAbstractButton::pressed, this, &QMakeBuilderPreferences::addBuildConfig);
    connect(m_prefsUi->removeButton, &QAbstractButton::pressed, this, &QMakeBuilderPreferences::removeBuildConfig);

    reset(); // load initial values
}

QMakeBuilderPreferences::~QMakeBuilderPreferences()
{
    // not a QObject !
    delete m_chooserUi;
}

void QMakeBuilderPreferences::reset()
{
    qCDebug(KDEV_QMAKEBUILDER) << "loading data";
    // refresh combobox
    KConfigGroup cg(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP());
    const QString buildPath = cg.readEntry(QMakeConfig::BUILD_FOLDER, QString());

    // update build list (this will trigger loadOtherConfig if signals are still connected)
    disconnect(m_prefsUi->buildDirCombo, &QComboBox::currentTextChanged, this, &QMakeBuilderPreferences::loadOtherConfig);
    m_prefsUi->buildDirCombo->clear();
    m_prefsUi->buildDirCombo->insertItems(0, cg.groupList());
    if (m_prefsUi->buildDirCombo->contains(buildPath)) {
        m_prefsUi->buildDirCombo->setCurrentItem(buildPath);
        m_chooserUi->loadConfig(buildPath);
    }
    qCDebug(KDEV_QMAKEBUILDER) << "Loaded" << cg.groupList() << buildPath;
    m_prefsUi->removeButton->setEnabled(m_prefsUi->buildDirCombo->count() > 1);
    connect(m_prefsUi->buildDirCombo, &QComboBox::currentTextChanged, this, &QMakeBuilderPreferences::loadOtherConfig);

    validate();
}

QString QMakeBuilderPreferences::name() const
{
    return i18nc("@title:tab", "QMake");
}

void QMakeBuilderPreferences::apply()
{
    qCDebug(KDEV_QMAKEBUILDER) << "Saving data";
    QString errormsg;

    if (m_chooserUi->validate(&errormsg)) {
        // data is valid: save, once in the build dir's data and also as current data
        m_chooserUi->saveConfig();
        KConfigGroup config(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP());
        m_chooserUi->saveConfig(config);
        config.writeEntry(QMakeConfig::BUILD_FOLDER, m_chooserUi->buildDir());
    } else {
        // invalid data: message box
        KMessageBox::error(nullptr, errormsg, QStringLiteral("Data is invalid!"));
        // FIXME dialog behaves like if save really happened (dialog closes if user click ok) even if changed signal is
        // emitted
    }
}

void QMakeBuilderPreferences::validate()
{
    m_chooserUi->validate();
}

void QMakeBuilderPreferences::loadOtherConfig(const QString& config)
{
    qCDebug(KDEV_QMAKEBUILDER) << "Loading config " << config;
    m_chooserUi->loadConfig(config);
    apply(); // since current config has changed, it must be saved immediately
}

void QMakeBuilderPreferences::addBuildConfig()
{
    qCDebug(KDEV_QMAKEBUILDER) << "Adding a new config.";
    // for more simplicity, just launch regular dialog
    auto dlg = new QMakeBuildDirChooserDialog(m_project);
    if (dlg->exec() == QDialog::Accepted) {
        m_prefsUi->buildDirCombo->setCurrentItem(dlg->buildDir(), true);
        m_prefsUi->removeButton->setEnabled(m_prefsUi->buildDirCombo->count() > 1);
        // TODO run qmake
    }
}

void QMakeBuilderPreferences::removeBuildConfig()
{
    qCDebug(KDEV_QMAKEBUILDER) << "Removing config" << m_prefsUi->buildDirCombo->currentText();
    QString removed = m_prefsUi->buildDirCombo->currentText();
    KConfigGroup cg(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP());

    m_prefsUi->buildDirCombo->removeItem(m_prefsUi->buildDirCombo->currentIndex());
    m_prefsUi->removeButton->setEnabled(m_prefsUi->buildDirCombo->count() > 1);
    cg.group(removed).deleteGroup(KConfigBase::Persistent);

    if (QDir(removed).exists()) {
        int ret = KMessageBox::warningTwoActions(
            this,
            i18n("The %1 directory is about to be removed in KDevelop's list.\n"
                 "Do you want KDevelop to delete it in the file system as well?",
                 removed),
            {}, KStandardGuiItem::del(),
            KGuiItem(i18nc("@action:button", "Do Not Delete"), QStringLiteral("dialog-cancel")));
        if (ret == KMessageBox::PrimaryAction) {
            auto deleteJob = KIO::del(QUrl::fromLocalFile(removed));
            KJobWidgets::setWindow(deleteJob, this);
            if (!deleteJob->exec())
                KMessageBox::error(this, i18n("Could not remove: %1.", removed));
        }
    }
}

#include "moc_qmakebuilderpreferences.cpp"
