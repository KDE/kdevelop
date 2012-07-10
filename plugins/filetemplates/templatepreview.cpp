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
#include <QFileInfo>
#include <QDir>

#include <KTextEditor/Document>
#include <KTextEditor/EditorChooser>
#include <KTextEditor/View>
#include <KTemporaryFile>
#include <kmacroexpander.h>

using namespace KDevelop;

TemplatePreview::TemplatePreview (QWidget* parent, Qt::WindowFlags f) : QWidget (parent, f)
{
    ui = new Ui::TemplatePreview;
    ui->setupUi(this);

    m_renderer = new TemplateRenderer;

    IDocumentController* dc = ICore::self()->documentController();
    m_currentDocument = dc->activeDocument();

    KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
    m_document = editor->createDocument(this);
    ui->verticalLayout->insertWidget(0, m_document->createView(this));
    documentChanged(m_currentDocument);

    connect (ui->projectRadioButton, SIGNAL(toggled(bool)), SLOT(selectedRendererChanged()));
    connect (ui->emptyLinesPolicyComboBox, SIGNAL(currentIndexChanged(int)), SLOT(selectedRendererChanged()));
    selectedRendererChanged();

    QVariantHash vars;
    vars["name"] = "Example";
    vars["license"] = "This file is licensed under the ExampleLicense 3.0";
    // TODO: More variables, preferably the ones from TemplateClassGenerator
    m_renderer->addVariables(vars);

    m_variables["APPNAME"] = "Example";
    m_variables["APPNAMELC"] = "example";
    m_variables["APPNAMEUC"] = "EXAMPLE";
    m_variables["APPNAMEID"] = "Example";

    m_variables["PROJECTDIR"] = QDir::homePath() + "/projects/ExampleProjectDir";
    m_variables["PROJECTDIRNAME"] = "ExampleProjectDir";
    m_variables["VERSIONCONTROLPLUGIN"] = "kdevgit";

    connect (dc, SIGNAL(documentActivated(KDevelop::IDocument*)), SLOT(documentActivated(KDevelop::IDocument*)));
    connect (dc, SIGNAL(documentClosed(KDevelop::IDocument*)), SLOT(documentClosed(KDevelop::IDocument*)));
    connect (dc, SIGNAL(documentContentChanged(KDevelop::IDocument*)), SLOT(documentChanged(KDevelop::IDocument*)));
}

TemplatePreview::~TemplatePreview()
{
    delete m_renderer;
}

void TemplatePreview::documentActivated (KDevelop::IDocument* document)
{
    Q_ASSERT(document);
    kDebug() << document->url();
    
    delete m_tmpFile;
    m_tmpFile = new KTemporaryFile;
    QFileInfo info(document->url().toLocalFile());
    m_tmpFile->setSuffix('.' + info.suffix());

    m_tmpFile->open();
    kDebug() << m_tmpFile->fileName();
    m_document->openUrl(KUrl(m_tmpFile->fileName()));

    m_currentDocument = document;
    documentChanged(document);
}

void TemplatePreview::documentChanged (IDocument* document)
{
    if (document != m_currentDocument || !document)
    {
        return;
    }

    QString text = document->textDocument()->text();
    sourceTextChanged(text);
}

void TemplatePreview::documentClosed (IDocument* document)
{
    if (document == m_currentDocument)
    {
        m_currentDocument = 0;
    }
    sourceTextChanged(QString());
}

void TemplatePreview::sourceTextChanged(const QString& text)
{
    m_document->setReadWrite(true);
    if (text.isEmpty())
    {
        m_document->setText(i18n("No active document"));
    }
    else
    {
        bool project = ui->projectRadioButton->isChecked();
        QString rendered = project ? KMacroExpander::expandMacros(text, m_variables) : m_renderer->render(text);
        if (rendered.simplified() == text.simplified())
        {
            // If the difference in only in whitespace, this is probably not a suitable template
            if (project)
            {
                m_document->setText(i18n("The active document is not a <application>KDevelop</application> project template"));
            }
            else
            {
                m_document->setText(i18n("The active document is not a <application>KDevelop</application> class template"));
            }
        }
        else
        {
            m_document->setText(rendered);
        }
    }
    m_document->save();
    m_document->setReadWrite(false);
}

void TemplatePreview::selectedRendererChanged()
{
    if (ui->classRadioButton->isChecked())
    {
        TemplateRenderer::EmptyLinesPolicy policy;
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
        m_renderer->setEmptyLinesPolicy(policy);
    }
    documentChanged(m_currentDocument);
}
