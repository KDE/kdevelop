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
    [[nodiscard]] QString currentTemplateFileName() const override
    {
        return templateArchiveFileName(m_view.currentIndex());
    }

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

} // unnamed namespace

class TemplatePagePrivate
{
public:
    explicit TemplatePagePrivate(ITemplateProvider& templateProvider)
        : model{templateProvider.createTemplatesModel()}
    {
        model->refresh();
    }

    void currentIndexChanged(const QModelIndex& index);
    void extractTemplate();

    [[nodiscard]] TreeViewTemplatesViewHelper viewHelper();

    /**
     * Call this function when refreshing the model invalidates the current index.
     */
    void currentIndexInvalidated();

    const std::unique_ptr<TemplatesModel> model;
    Ui::TemplatePage ui;
};

TemplatePage::TemplatePage(ITemplateProvider& templateProvider, QWidget* parent)
    : QWidget(parent)
    , d_ptr{std::make_unique<TemplatePagePrivate>(templateProvider)}
{
    Q_D(TemplatePage);

    d->ui.setupUi(this);

    if (const auto knsConfigurationFile = templateProvider.knsConfigurationFile(); !knsConfigurationFile.isEmpty()) {
        auto* const getNewButton =
            new KNSWidgets::Button(i18nc("@action:button", "Get More Templates"), knsConfigurationFile, this);
        connect(getNewButton, &KNSWidgets::Button::dialogFinished, this,
                [d](const QList<KNSCore::Entry>& changedEntries) {
                    if (!d->viewHelper().handleNewStuffDialogFinished(changedEntries)) {
                        d->currentIndexInvalidated();
                    }
                });
        d->ui.buttonLayout->insertWidget(1, getNewButton);
    }

    connect(d->ui.loadButton, &QPushButton::clicked, this, [this] {
        Q_D(TemplatePage);
        if (!d->viewHelper().loadTemplatesFromFiles(this)) {
            d->currentIndexInvalidated();
        }
    });

    d->ui.extractButton->setEnabled(false);
    connect(d->ui.extractButton, &QPushButton::clicked, this, [d] {
        d->extractTemplate();
    });

    d->ui.treeView->setModel(d->model.get());
    d->ui.treeView->expandAll();
    connect(d->ui.treeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [d](const QModelIndex& current) {
                d->currentIndexChanged(current);
            });
}

TemplatePage::~TemplatePage() = default;

void TemplatePagePrivate::currentIndexChanged(const QModelIndex& index)
{
    const auto canExtractArchive = index.isValid() && QFileInfo::exists(templateArchiveFileName(index));
    ui.extractButton->setEnabled(canExtractArchive);
}

void TemplatePagePrivate::extractTemplate()
{
    const auto archiveName = templateArchiveFileName(ui.treeView->currentIndex());
    QFileInfo info(archiveName);
    if (!info.exists())
    {
        ui.extractButton->setEnabled(false);
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

TreeViewTemplatesViewHelper TemplatePagePrivate::viewHelper()
{
    return TreeViewTemplatesViewHelper(*model, *ui.treeView);
}

void TemplatePagePrivate::currentIndexInvalidated()
{
    Q_ASSERT(!ui.treeView->currentIndex().isValid());
    currentIndexChanged({});
}

#include "moc_templatepage.cpp"
