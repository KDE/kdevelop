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

#include "templatepreview.h"
#include "ui_templatepreview.h"
#include <language/codegen/templaterenderer.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <KTextBrowser>

#include <QLabel>
#include <QVBoxLayout>
#include <grantlee/engine.h>
#include <KTextEditor/Document>

using namespace KDevelop;

TemplatePreview::TemplatePreview (QWidget* parent, Qt::WindowFlags f) : QWidget (parent, f)
{
    ui = new Ui::TemplatePreview;
    ui->setupUi(this);

    m_renderer = new TemplateRenderer;

    connect (ui->emptyLinesPolicyComboBox, SIGNAL(currentIndexChanged(int)), SLOT(policyIndexChanged(int)));
    policyIndexChanged(ui->emptyLinesPolicyComboBox->currentIndex());

    QVariantHash vars;
    vars["name"] = "Example";
    vars["license"] = "This file is licensed under the ExampleLicense 3.0";
    // TODO: More variables, preferably the ones from TemplateClassGenerator
    m_renderer->addVariables(vars);

    connect (ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), SLOT(documentActivated(KDevelop::IDocument*)));
    connect (ICore::self()->documentController(), SIGNAL(documentContentChanged(KDevelop::IDocument*)), SLOT(documentChanged(KDevelop::IDocument*)));
}

TemplatePreview::~TemplatePreview()
{
    delete m_renderer;
}

void TemplatePreview::documentActivated (KDevelop::IDocument* document)
{
    m_currentDocument = document;
    documentChanged(document);
}

void TemplatePreview::documentChanged (IDocument* document)
{
    if (document != m_currentDocument)
    {
        return;
    }

    QString text = document->textDocument()->text();
    sourceTextChanged(text);
}

void TemplatePreview::sourceTextChanged(const QString& text)
{
    QString rendered = m_renderer->render(text);
    ui->browser->setText(rendered);
}

void TemplatePreview::policyIndexChanged (int index)
{
    TemplateRenderer::EmptyLinesPolicy policy;
    switch (index)
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
    m_renderer->setEmptyLinesPolicy(policy);
    documentChanged(m_currentDocument);
}
