/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "templatepage.h"
#include "ui_templatepage.h"

#include <interfaces/itemplateprovider.h>
#include <language/codegen/templatesmodel.h>
#include <language/codegen/templatesviewhelper.h>

#include <KArchive>
#include <KNSWidgets/Button>
#include <KZip>
#include <KTar>

#include <QFileDialog>
#include <QList>

using namespace KDevelop;

namespace {
[[nodiscard]] QString templateArchiveFileName(const QModelIndex& index)
{
    return index.data(TemplatesModel::ArchiveFileRole).toString();
}
} // unnamed namespace

class TreeViewTemplatesViewHelper final : public TemplatesViewHelper
{
public:
    explicit TreeViewTemplatesViewHelper(TemplatesModel& model, QTreeView& view)
        : TemplatesViewHelper(model)
        , m_view{view}
    {
    }

protected:
    void refreshModel() override
    {
        TemplatesViewHelper::refreshModel();
        m_view.expandAll();
    }

private:
    bool setCurrentTemplate(const QList<QModelIndex>& indexes) override
    {
        if (indexes.empty()) {
            return false;
        }
        m_view.setCurrentIndex(indexes.constLast());
        return true;
    }

    QTreeView& m_view;
};

TemplatePage::TemplatePage(ITemplateProvider* provider, QWidget* parent)
    : QWidget(parent)
    , m_model{provider->createTemplatesModel()}
{
    m_model->refresh();

    ui = new Ui::TemplatePage;
    ui->setupUi(this);

    if (const auto knsConfigurationFile = provider->knsConfigurationFile(); !knsConfigurationFile.isEmpty()) {
        auto* const getNewButton =
            new KNSWidgets::Button(i18nc("@action:button", "Get More Templates"), knsConfigurationFile, this);
        connect(getNewButton, &KNSWidgets::Button::dialogFinished, this,
                [this](const QList<KNSCore::Entry>& changedEntries) {
                    if (!viewHelper().handleNewStuffDialogFinished(changedEntries)) {
                        currentIndexInvalidated();
                    }
                });
        ui->buttonLayout->insertWidget(1, getNewButton);
    }

    connect(ui->loadButton, &QPushButton::clicked, this, [this] {
        if (!viewHelper().loadTemplatesFromFiles(this)) {
            currentIndexInvalidated();
        }
    });

    ui->extractButton->setEnabled(false);
    connect(ui->extractButton, &QPushButton::clicked,
            this, &TemplatePage::extractTemplate);

    ui->treeView->setModel(m_model.get());
    ui->treeView->expandAll();
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &TemplatePage::currentIndexChanged);
}

TemplatePage::~TemplatePage()
{
    delete ui;
}

void TemplatePage::currentIndexChanged(const QModelIndex& index)
{
    const auto canExtractArchive = index.isValid() && QFileInfo::exists(templateArchiveFileName(index));
    ui->extractButton->setEnabled(canExtractArchive);
}

void TemplatePage::extractTemplate()
{
    const auto archiveName = templateArchiveFileName(ui->treeView->currentIndex());
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

TreeViewTemplatesViewHelper TemplatePage::viewHelper()
{
    return TreeViewTemplatesViewHelper(*m_model, *ui->treeView);
}

void TemplatePage::currentIndexInvalidated()
{
    Q_ASSERT(!ui->treeView->currentIndex().isValid());
    currentIndexChanged({});
}

#include "moc_templatepage.cpp"
