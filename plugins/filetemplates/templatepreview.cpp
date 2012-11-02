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
#include <language/codegen/codedescription.h>
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

TemplatePreview::TemplatePreview(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, ui(new Ui::TemplatePreview)
, m_renderer(0)
, m_original(0)
, m_preview(0)
{
    ui->setupUi(this);
    ui->messageWidget->hide();

    m_renderer = new TemplateRenderer;

    IDocumentController* dc = ICore::self()->documentController();
    if (dc->activeDocument())
    {
        m_original = dc->activeDocument()->textDocument();
    }

    KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
    m_preview = editor->createDocument(this);
    ui->verticalLayout->insertWidget(1, m_preview->createView(this));
    if (m_original)
    {
        documentActivated(dc->activeDocument());
    }

    connect (ui->projectRadioButton, SIGNAL(toggled(bool)), SLOT(selectedRendererChanged()));
    connect (ui->emptyLinesPolicyComboBox, SIGNAL(currentIndexChanged(int)), SLOT(selectedRendererChanged()));
    selectedRendererChanged();

    QVariantHash vars;
    vars["name"] = "Example";
    vars["license"] = "This file is licensed under the ExampleLicense 3.0";
    // TODO: More variables, preferably the ones from TemplateClassGenerator

    VariableDescriptionList members;
    members << VariableDescription("int", "number");
    members << VariableDescription("string", "name");
    vars["members"] = CodeDescription::toVariantList(members);

    FunctionDescriptionList functions;
    functions << FunctionDescription("doSomething", VariableDescriptionList(), VariableDescriptionList());
    FunctionDescription complexFunction("doSomethingElse", VariableDescriptionList(), VariableDescriptionList());
    complexFunction.arguments << VariableDescription("bool", "really");
    complexFunction.arguments << VariableDescription("int", "howMuch");
    complexFunction.returnArguments << VariableDescription("double", QString());
    functions << complexFunction;
    vars["functions"] = CodeDescription::toVariantList(functions);

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
}

TemplatePreview::~TemplatePreview()
{
    delete ui;
    delete m_renderer;
}

void TemplatePreview::documentActivated (KDevelop::IDocument* document)
{
    Q_ASSERT(document);
    kDebug() << document->url();

    if (m_original)
    {
        disconnect (m_original, SIGNAL(textChanged(KTextEditor::Document*)), this, SLOT(documentChanged(KTextEditor::Document*)));
    }
    m_original = document->textDocument();
    connect (m_original, SIGNAL(textChanged(KTextEditor::Document*)), this, SLOT(documentChanged(KTextEditor::Document*)));
    documentChanged(m_original);
}

void TemplatePreview::documentChanged (KTextEditor::Document* document)
{
    if (isVisible() && document && document == m_original)
    {
        sourceTextChanged(m_original->text());
    }
}

void TemplatePreview::showEvent(QShowEvent* event)
{
    if (m_original)
    {
        sourceTextChanged(m_original->text());
    }
}

void TemplatePreview::documentClosed (IDocument* document)
{
    if (document && document->textDocument() == m_original)
    {
        if (m_original)
        {
            disconnect (m_original, SIGNAL(textChanged(KTextEditor::Document*)), this, SLOT(documentChanged(KTextEditor::Document*)));
        }
        m_original = 0;
    }
    sourceTextChanged(QString());
}

void TemplatePreview::sourceTextChanged(const QString& text)
{
    kDebug();
    m_preview->setReadWrite(true);
    if (text.isEmpty())
    {
        ui->messageWidget->setMessageType(KMessageWidget::Information);
        ui->messageWidget->setText(i18n("No active document"));
        ui->messageWidget->animatedShow();
    }
    else
    {
        bool project = ui->projectRadioButton->isChecked();
        QString rendered;
        QString errorString;
        bool error = false;
        if (project)
        {
            rendered = KMacroExpander::expandMacros(text, m_variables);
        }
        else
        {
            rendered = m_renderer->render(text);
            errorString = m_renderer->errorString();
            error = !errorString.isEmpty();
        }

        if (rendered.simplified() == text.simplified())
        {
            error = false;
            // If the difference in only in whitespace, this is probably not a suitable template
            if (project)
            {
                errorString = i18n("The active document is not a <application>KDevelop</application> project template");
            }
            else
            {
                errorString = i18n("The active document is not a <application>KDevelop</application> class template");
            }
            m_preview->clear();
        }
        else
        {
            m_preview->setText(rendered);
        }

        if (!errorString.isEmpty())
        {
            ui->messageWidget->setMessageType(error ? KMessageWidget::Error : KMessageWidget::Information);
            ui->messageWidget->setText(errorString);
            ui->messageWidget->animatedShow();
        }
        else
        {
            ui->messageWidget->animatedHide();
        }

    }
    if (m_original)
    {
        m_preview->setMode(m_original->mode());
    }
    m_preview->setReadWrite(false);
}

void TemplatePreview::selectedRendererChanged()
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
        m_renderer->setEmptyLinesPolicy(policy);
    }
    documentChanged(m_original);
}
