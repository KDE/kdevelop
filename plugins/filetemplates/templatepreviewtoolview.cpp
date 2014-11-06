/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

#include <QLabel>
#include <QVBoxLayout>

using namespace KDevelop;

TemplatePreviewToolView::TemplatePreviewToolView(FileTemplatesPlugin* plugin, QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, ui(new Ui::TemplatePreviewToolView)
, m_original(0)
, m_plugin(plugin)
{
    ui->setupUi(this);
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
    connect(ui->emptyLinesPolicyComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
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
    if (!isVisible()) {
        return;
    }

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
    documentChanged(doc ? doc->textDocument() : 0);
}

void TemplatePreviewToolView::documentClosed(IDocument* document)
{
    if (!isVisible()) {
        return;
    }

    if (document && document->textDocument() == m_original) {
        documentChanged(0);
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

