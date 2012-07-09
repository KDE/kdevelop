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
    QVBoxLayout* layout = new QVBoxLayout(this);
    m_browser = new KTextBrowser(this);
    layout->addWidget(m_browser);
    m_label = new QLabel(this);
    layout->addWidget(m_label);
    setLayout(layout);

    m_renderer = new TemplateRenderer;
    m_renderer->setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);

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
    m_browser->setText(rendered);
}
