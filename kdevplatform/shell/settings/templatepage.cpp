/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "templatepage.h"
#include "ui_templatepage.h"

#include <language/codegen/templatesmodel.h>
#include <language/codegen/templatesviewhelper.h>

#include <KArchive>
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

class TemplatePagePrivate final : public TemplatesViewHelper
{
public:
    explicit TemplatePagePrivate(TemplatePage* q, ITemplateProvider& templateProvider)
        : TemplatesViewHelper(templateProvider)
        , q_ptr{q}
    {
        Q_ASSERT(q);
    }

    void currentIndexChanged(const QModelIndex& index);
    void extractTemplate();

    Ui::TemplatePage ui;

protected:
    void refreshModel() override
    {
        TemplatesViewHelper::refreshModel();
        ui.treeView->expandAll();
    }

private:
    [[nodiscard]] QWidget* dialogParent() override
    {
        Q_Q(TemplatePage);
        return q;
    }

    [[nodiscard]] QString currentTemplateFileName() const override
    {
        return templateArchiveFileName(ui.treeView->currentIndex());
    }

    bool setCurrentTemplate(const QList<QModelIndex>& indexes) override
    {
        if (indexes.empty()) {
            return false;
        }
        ui.treeView->setCurrentIndex(indexes.constLast());
        return true;
    }

    void handleNoTemplateSelectedAfterRefreshingModel() override;

    TemplatePage* const q_ptr;
    Q_DECLARE_PUBLIC(TemplatePage)
};

TemplatePage::TemplatePage(ITemplateProvider& templateProvider, QWidget* parent)
    : QWidget(parent)
    , d_ptr{std::make_unique<TemplatePagePrivate>(this, templateProvider)}
{
    Q_D(TemplatePage);

    d->ui.setupUi(this);

    if (auto* const getMoreButton = d->createGetMoreTemplatesButton(templateProvider, this)) {
        d->ui.buttonLayout->insertWidget(0, getMoreButton);
    }
    auto* const loadButton = d->createLoadTemplateFromFileButton(this);
    d->ui.buttonLayout->insertWidget(1, loadButton);

    d->ui.extractButton->setEnabled(false);
    connect(d->ui.extractButton, &QPushButton::clicked, this, [d] {
        d->extractTemplate();
    });

    d->ui.treeView->setModel(&d->model());
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

void TemplatePagePrivate::handleNoTemplateSelectedAfterRefreshingModel()
{
    Q_ASSERT(!ui.treeView->currentIndex().isValid());
    currentIndexChanged({});
}

#include "moc_templatepage.cpp"
