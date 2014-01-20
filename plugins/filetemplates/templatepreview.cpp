/*
 * This file is part of KDevelop
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "templatepreview.h"

#include <language/codegen/templaterenderer.h>
#include <language/codegen/codedescription.h>

#include <QDir>
#include <QVBoxLayout>
#include <QTextStream>

#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <KTextEditor/ConfigInterface>
#include <KTextEditor/Document>

#include <KLocalizedString>
#include <kmacroexpander.h>

using namespace KDevelop;

TemplatePreviewRenderer::TemplatePreviewRenderer()
{
    QVariantHash vars;
    vars["name"] = "Example";
    vars["license"] = "This file is licensed under the ExampleLicense 3.0";
    // TODO: More variables, preferably the ones from TemplateClassGenerator

    VariableDescriptionList publicMembers;
    VariableDescriptionList protectedMembers;
    VariableDescriptionList privateMembers;
    publicMembers    << VariableDescription("int",    "number");
    protectedMembers << VariableDescription("string", "name");
    privateMembers   << VariableDescription("float",  "variable");
    vars["members"] = CodeDescription::toVariantList(publicMembers + protectedMembers + privateMembers);
    vars["public_members"]    = CodeDescription::toVariantList(publicMembers);
    vars["protected_members"] = CodeDescription::toVariantList(protectedMembers);
    vars["private_members"]   = CodeDescription::toVariantList(privateMembers);

    FunctionDescriptionList publicFunctions;
    FunctionDescriptionList protectedFunctions;
    FunctionDescriptionList privateFunctions;

    FunctionDescription complexFunction("doBar", VariableDescriptionList(), VariableDescriptionList());
    complexFunction.arguments << VariableDescription("bool", "really");
    complexFunction.arguments << VariableDescription("int", "howMuch");
    complexFunction.returnArguments << VariableDescription("double", QString());

    publicFunctions << FunctionDescription("doFoo", VariableDescriptionList(), VariableDescriptionList());
    publicFunctions << complexFunction;
    protectedFunctions << FunctionDescription("onUpdate", VariableDescriptionList(), VariableDescriptionList());

    vars["functions"] = CodeDescription::toVariantList(publicFunctions + protectedFunctions + privateFunctions);
    vars["public_functions"]    = CodeDescription::toVariantList(publicFunctions);
    vars["protected_functions"] = CodeDescription::toVariantList(protectedFunctions);
    vars["private_functions"]   = CodeDescription::toVariantList(privateFunctions);

    addVariables(vars);
}

TemplatePreviewRenderer::~TemplatePreviewRenderer()
{

}

TemplatePreview::TemplatePreview(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    m_variables["APPNAME"] = "Example";
    m_variables["APPNAMELC"] = "example";
    m_variables["APPNAMEUC"] = "EXAMPLE";
    m_variables["APPNAMEID"] = "Example";

    m_variables["PROJECTDIR"] = QDir::homePath() + "/projects/ExampleProjectDir";
    m_variables["PROJECTDIRNAME"] = "ExampleProjectDir";
    m_variables["VERSIONCONTROLPLUGIN"] = "kdevgit";

    KTextEditor::Document* doc = KTextEditor::Editor::instance()->createDocument(this);
    m_preview.reset(doc);
    m_preview->setReadWrite(false);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    m_view = m_preview->createView(this);
    if (KTextEditor::ConfigInterface* config = qobject_cast<KTextEditor::ConfigInterface*>(m_view)) {
        config->setConfigValue("icon-bar", false);
        config->setConfigValue("folding-bar", false);
        config->setConfigValue("line-numbers", false);
        config->setConfigValue("dynamic-word-wrap", true);
    }
    layout->addWidget(m_view);
}

TemplatePreview::~TemplatePreview()
{

}

QString TemplatePreview::setText(const QString& text, bool isProject, TemplateRenderer::EmptyLinesPolicy policy)
{
    QString rendered;
    QString errorString;

    if (!text.isEmpty()) {
        if (isProject) {
            rendered = KMacroExpander::expandMacros(text, m_variables);
        } else {
            TemplatePreviewRenderer renderer;
            renderer.setEmptyLinesPolicy(policy);
            rendered = renderer.render(text);
            errorString = renderer.errorString();
        }
    }

    m_preview->setReadWrite(true);
    m_preview->setText(rendered);
    m_view->setCursorPosition(KTextEditor::Cursor(0, 0));
    m_preview->setReadWrite(false);

    return errorString;
}

KTextEditor::Document* TemplatePreview::document() const
{
    return m_preview.data();
}
