/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#include "templateselectionpage.h"

#include "templateclassassistant.h"
#include "templatepreview.h"

#include <language/codegen/templatesmodel.h>
#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/templaterenderer.h>
#include <language/interfaces/icreateclasshelper.h>
#include <language/interfaces/ilanguagesupport.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>

#include "ui_templateselection.h"

#include <KNS3/DownloadDialog>
#include <KNS3/KNewStuffButton>
#include <KLocalizedString>
#include <KComponentData>
#include <KFileDialog>
#include <KTempDir>
#include <KTextEditor/Document>

using namespace KDevelop;

const char* LastUsedTemplateEntry = "LastUsedTemplate";
const char* FileTemplatesGroup = "SourceFileTemplates";

class KDevelop::TemplateSelectionPagePrivate
{
public:
    TemplateSelectionPagePrivate(TemplateSelectionPage* page_)
    : page(page_)
    {}

    TemplateSelectionPage* page;
    Ui::TemplateSelection* ui;
    QString selectedTemplate;
    TemplateClassAssistant* assistant;
    TemplatesModel* model;

    void currentTemplateChanged(const QModelIndex& index);
    void getMoreClicked();
    void loadFileClicked();
    void previewTemplate(const QString& templateFile);
};

void TemplateSelectionPagePrivate::currentTemplateChanged(const QModelIndex& index)
{
    // delete preview tabs
    if (!index.isValid() || index.child(0, 0).isValid())
    {
        // invalid or has child
        assistant->setValid(assistant->currentPage(), false);
        ui->previewLabel->setVisible(false);
        ui->tabWidget->setVisible(false);
    } else {
        selectedTemplate = model->data(index, TemplatesModel::DescriptionFileRole).toString();
        assistant->setValid(assistant->currentPage(), true);
        previewTemplate(selectedTemplate);
        ui->previewLabel->setVisible(true);
        ui->tabWidget->setVisible(true);
        ui->previewLabel->setText(i18nc("%1: template comment", "<b>Preview:</b> %1",
                                        index.data(TemplatesModel::CommentRole).toString()));
    }
}

void TemplateSelectionPagePrivate::previewTemplate(const QString& file)
{
    SourceFileTemplate fileTemplate(file);
    if (!fileTemplate.isValid() || fileTemplate.outputFiles().isEmpty()) {
        return;
    }

    TemplatePreviewRenderer renderer;
    renderer.setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);

    KTempDir dir;
    KUrl base(dir.name());
    QHash<QString, KUrl> fileUrls;
    foreach(const SourceFileTemplate::OutputFile& out, fileTemplate.outputFiles()) {
        KUrl url(base);
        url.addPath(renderer.render(out.outputName));
        fileUrls.insert(out.identifier, url);
    }
    DocumentChangeSet changes = renderer.renderFileTemplate(fileTemplate, base, fileUrls);
    changes.setActivationPolicy(DocumentChangeSet::DoNotActivate);
    changes.setUpdateHandling(DocumentChangeSet::NoUpdate);
    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    if (!result) {
        return;
    }

    int idx = 0;
    foreach(const SourceFileTemplate::OutputFile& out, fileTemplate.outputFiles()) {
        TemplatePreview* preview = 0;
        if (ui->tabWidget->count() > idx) {
            // reuse existing tab
            preview = qobject_cast<TemplatePreview*>(ui->tabWidget->widget(idx));
            ui->tabWidget->setTabText(idx, out.label);
            Q_ASSERT(preview);
        } else {
            // create new tabs on demand
            preview = new TemplatePreview(page);
            ui->tabWidget->addTab(preview, out.label);
        }
        preview->document()->openUrl(fileUrls.value(out.identifier));
        ++idx;
    }
    // remove superflous tabs from last time
    while (ui->tabWidget->count() > fileUrls.size()) {
        delete ui->tabWidget->widget(fileUrls.size());
    }
    return;
}

void TemplateSelectionPagePrivate::getMoreClicked()
{
    model->refresh();
}

void TemplateSelectionPagePrivate::loadFileClicked()
{
    QString filter = "application/x-desktop application/x-bzip-compressed-tar application/zip";
    QString fileName = KFileDialog::getOpenFileName(KUrl("kfiledialog:///kdevclasstemplate"), filter, page);

    if (!fileName.isEmpty())
    {
        QString destination = model->loadTemplateFile(fileName);
        QModelIndexList indexes = model->templateIndexes(destination);
        int n = indexes.size();
        if (n > 1)
        {
            ui->view->setCurrentIndex(indexes[1]);
        }
    }
}

void TemplateSelectionPage::saveConfig()
{
    KSharedConfig::Ptr config;
    if (IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl()))
    {
        config = project->projectConfiguration();
    }
    else
    {
        config = ICore::self()->activeSession()->config();
    }

    KConfigGroup group(config, FileTemplatesGroup);
    group.writeEntry(LastUsedTemplateEntry, d->selectedTemplate);
    group.sync();
}

TemplateSelectionPage::TemplateSelectionPage(TemplateClassAssistant* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, d(new TemplateSelectionPagePrivate(this))
{
    d->assistant = parent;

    d->ui = new Ui::TemplateSelection;
    d->ui->setupUi(this);

    d->model = new TemplatesModel("kdevfiletemplates", this);
    d->model->refresh();

    d->ui->view->setLevels(3);
    d->ui->view->setHeaderLabels(QStringList() << i18n("Language") << i18n("Framework") << i18n("Template"));
    d->ui->view->setModel(d->model);

    connect(d->ui->view, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
            SLOT(currentTemplateChanged(QModelIndex)));

    QModelIndex templateIndex = d->model->index(0, 0);

    while (templateIndex.child(0, 0).isValid())
    {
        templateIndex = templateIndex.child(0, 0);
    }

    KSharedConfig::Ptr config;
    if (IProject* project = ICore::self()->projectController()->findProjectForUrl(d->assistant->baseUrl()))
    {
        config = project->projectConfiguration();
    }
    else
    {
        config = ICore::self()->activeSession()->config();
    }

    KConfigGroup group(config, FileTemplatesGroup);
    QString lastTemplate = group.readEntry(LastUsedTemplateEntry);

    QModelIndexList indexes = d->model->match(d->model->index(0, 0), TemplatesModel::DescriptionFileRole, lastTemplate, 1, Qt::MatchRecursive);

    if (!indexes.isEmpty())
    {
        templateIndex = indexes.first();
    }

    d->ui->view->setCurrentIndex(templateIndex);

    KNS3::Button* getMoreButton = new KNS3::Button(i18n("Get More Templates..."), "kdevfiletemplates.knsrc", d->ui->view);
    connect (getMoreButton, SIGNAL(dialogFinished(KNS3::Entry::List)), SLOT(getMoreClicked()));
    d->ui->view->addWidget(0, getMoreButton);

    KPushButton* loadButton = new KPushButton(KIcon("application-x-archive"), i18n("Load Template From File"), d->ui->view);
    connect (loadButton, SIGNAL(clicked(bool)), SLOT(loadFileClicked()));
    d->ui->view->addWidget(0, loadButton);

    d->ui->view->setContentsMargins(0, 0, 0, 0);
}

TemplateSelectionPage::~TemplateSelectionPage()
{
    delete d->ui;
    delete d;
}

QSize TemplateSelectionPage::minimumSizeHint() const
{
    return QSize(400, 600);
}

QString TemplateSelectionPage::selectedTemplate() const
{
    return d->selectedTemplate;
}

#include "templateselectionpage.moc"
