/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "templatepage.h"
#include "ui_templatepage.h"

#include <QFileDialog>

#include <interfaces/itemplateprovider.h>
#include <language/codegen/templatesmodel.h>

#include <KArchive>
#include <KNSCore/Entry>
#include <KNSWidgets/Button>
#include <KZip>
#include <KTar>

#include <util/scopeddialog.h>

TemplatePage::TemplatePage (KDevelop::ITemplateProvider* provider, QWidget* parent) : QWidget (parent),
m_provider(provider)
{
    ui = new Ui::TemplatePage;
    ui->setupUi(this);

    if (!m_provider->knsConfigurationFile().isEmpty()) {
        auto* getNewButton = new KNSWidgets::Button(i18nc("@action:button", "Get More Templates"),
                                                    m_provider->knsConfigurationFile(), this);
        connect(getNewButton, &KNSWidgets::Button::dialogFinished, this, &TemplatePage::handleNewStuffDialogFinished);
        ui->buttonLayout->insertWidget(1, getNewButton);
    }

    ui->loadButton->setVisible(!m_provider->supportedMimeTypes().isEmpty());
    connect(ui->loadButton, &QPushButton::clicked,
            this, &TemplatePage::loadFromFile);

    ui->extractButton->setEnabled(false);
    connect(ui->extractButton, &QPushButton::clicked,
            this, &TemplatePage::extractTemplate);

    provider->reload();

    ui->treeView->setModel(provider->templatesModel());
    ui->treeView->expandAll();
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &TemplatePage::currentIndexChanged);
}

TemplatePage::~TemplatePage()
{
    delete ui;
}

void TemplatePage::loadFromFile()
{
    KDevelop::ScopedDialog<QFileDialog> fileDialog(this);
    fileDialog->setMimeTypeFilters(m_provider->supportedMimeTypes());
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    if (!fileDialog->exec()) {
        return;
    }

    const auto& selectedFiles = fileDialog->selectedFiles();
    for (const auto& file : selectedFiles) {
        m_provider->loadTemplate(file);
    }

    m_provider->reload();
}

void TemplatePage::handleNewStuffDialogFinished(const QList<KNSCore::Entry>& changedEntries)
{
    if (!changedEntries.isEmpty()) {
        m_provider->reload();
    }
}

void TemplatePage::currentIndexChanged(const QModelIndex& index)
{
    QString archive = ui->treeView->model()->data(index, KDevelop::TemplatesModel::ArchiveFileRole).toString();
    ui->extractButton->setEnabled(QFileInfo::exists(archive));
}

void TemplatePage::extractTemplate()
{
    QModelIndex index = ui->treeView->currentIndex();
    QString archiveName= ui->treeView->model()->data(index, KDevelop::TemplatesModel::ArchiveFileRole).toString();

    QFileInfo info(archiveName);
    if (!info.exists())
    {
        ui->extractButton->setEnabled(false);
        return;
    }

    QScopedPointer<KArchive> archive;
    if (info.suffix() == QLatin1String("zip"))
    {
        archive.reset(new KZip(archiveName));
    }
    else
    {
        archive.reset(new KTar(archiveName));
    }

    archive->open(QIODevice::ReadOnly);

    const QString destination = QFileDialog::getExistingDirectory() + QLatin1Char('/') + info.baseName();
    archive->directory()->copyTo(destination);
}

#include "moc_templatepage.cpp"
