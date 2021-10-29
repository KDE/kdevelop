/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "templatepreviewtoolview.h"

#include "ui_templatepreviewtoolview.h"

#include "filetemplatesplugin.h"
#include "templatepreview.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <language/codegen/templaterenderer.h>

#include <KTextEditor/Document>
#include <KLocalizedString>

using namespace KDevelop;

TemplatePreviewToolView::TemplatePreviewToolView(FileTemplatesPlugin* plugin, QWidget* parent)
: QWidget(parent)
, ui(new Ui::TemplatePreviewToolView)
, m_original(nullptr)
, m_plugin(plugin)
{
    ui->setupUi(this);
    setWindowIcon(QIcon::fromTheme(QStringLiteral("document-preview"), windowIcon()));

    ui->messageWidget->hide();
    ui->emptyLinesPolicyComboBox->setCurrentIndex(1);

    IDocumentController* dc = ICore::self()->documentController();
    if (dc->activeDocument())
    {
        m_original = dc->activeDocument()->textDocument();
    }

    if (m_original)
    {
        documentActivated(dc->activeDocument());
    }

    connect(ui->projectRadioButton, &QRadioButton::toggled,
            this, &TemplatePreviewToolView::selectedRendererChanged);
    connect(ui->emptyLinesPolicyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TemplatePreviewToolView::selectedRendererChanged);
    selectedRendererChanged();

    connect(dc, &IDocumentController::documentActivated,
            this, &TemplatePreviewToolView::documentActivated);
    connect(dc, &IDocumentController::documentClosed,
            this, &TemplatePreviewToolView::documentClosed);
}

TemplatePreviewToolView::~TemplatePreviewToolView()
{
    delete ui;
}

void TemplatePreviewToolView::documentActivated(KDevelop::IDocument* document)
{
    documentChanged(document->textDocument());
}

void TemplatePreviewToolView::documentChanged(KTextEditor::Document* document)
{
    if (!isVisible()) {
        return;
    }


    if (m_original) {
        disconnect(m_original, &KTextEditor::Document::textChanged,
                   this, &TemplatePreviewToolView::documentChanged);
    }
    m_original = document;

    FileTemplatesPlugin::TemplateType type = FileTemplatesPlugin::NoTemplate;
    if (m_original) {
        connect(m_original, &KTextEditor::Document::textChanged,
                this, &TemplatePreviewToolView::documentChanged);
        type = m_plugin->determineTemplateType(document->url());
    }

    switch (type) {
        case FileTemplatesPlugin::NoTemplate:
            ui->messageWidget->setMessageType(KMessageWidget::Information);
            if (m_original) {
                ui->messageWidget->setText(xi18n("The active text document is not a <application>KDevelop</application> template"));
            } else {
                ui->messageWidget->setText(i18n("No active text document."));
            }
            ui->messageWidget->animatedShow();
            ui->preview->setText(QString());
            break;

        case FileTemplatesPlugin::FileTemplate:
            ui->classRadioButton->setChecked(true);
            sourceTextChanged(m_original->text());
            break;

        case FileTemplatesPlugin::ProjectTemplate:
            ui->projectRadioButton->setChecked(true);
            sourceTextChanged(m_original->text());
            break;
    }
}

void TemplatePreviewToolView::showEvent(QShowEvent*)
{
    IDocument* doc = ICore::self()->documentController()->activeDocument();
    documentChanged(doc ? doc->textDocument() : nullptr);
}

void TemplatePreviewToolView::documentClosed(IDocument* document)
{
    m_original = nullptr;

    if (document && document->textDocument() == m_original) {
        documentChanged(nullptr);
    }
}

void TemplatePreviewToolView::sourceTextChanged(const QString& text)
{
    QString errorString = ui->preview->setText(text, ui->projectRadioButton->isChecked(), m_policy);
    if (!errorString.isEmpty()) {
        ui->messageWidget->setMessageType(KMessageWidget::Error);
        ui->messageWidget->setText(errorString);
        ui->messageWidget->animatedShow();
    } else {
        ui->messageWidget->animatedHide();
    }

    if (m_original) {
        ui->preview->document()->setMode(m_original->mode());
    }
}

void TemplatePreviewToolView::selectedRendererChanged()
{
    if (ui->classRadioButton->isChecked())
    {
        TemplateRenderer::EmptyLinesPolicy policy = TemplateRenderer::KeepEmptyLines;
        switch (ui->emptyLinesPolicyComboBox->currentIndex())
        {
            case 0:
                policy = TemplateRenderer::KeepEmptyLines;
                break;

            case 1:
                policy = TemplateRenderer::TrimEmptyLines;
                break;

            case 2:
                policy = TemplateRenderer::RemoveEmptyLines;
                break;
        }
        m_policy = policy;
    }
    documentChanged(m_original);
}

