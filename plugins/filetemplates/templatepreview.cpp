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
    vars[QStringLiteral("name")] = "Example";
    vars[QStringLiteral("license")] = "This file is licensed under the ExampleLicense 3.0";
    // TODO: More variables, preferably the ones from TemplateClassGenerator

    VariableDescriptionList publicMembers;
    VariableDescriptionList protectedMembers;
    VariableDescriptionList privateMembers;
    publicMembers    << VariableDescription(QStringLiteral("int"),    QStringLiteral("number"));
    protectedMembers << VariableDescription(QStringLiteral("string"), QStringLiteral("name"));
    privateMembers   << VariableDescription(QStringLiteral("float"),  QStringLiteral("variable"));
    vars[QStringLiteral("members")] = CodeDescription::toVariantList(publicMembers + protectedMembers + privateMembers);
    vars[QStringLiteral("public_members")]    = CodeDescription::toVariantList(publicMembers);
    vars[QStringLiteral("protected_members")] = CodeDescription::toVariantList(protectedMembers);
    vars[QStringLiteral("private_members")]   = CodeDescription::toVariantList(privateMembers);

    FunctionDescriptionList publicFunctions;
    FunctionDescriptionList protectedFunctions;
    FunctionDescriptionList privateFunctions;

    FunctionDescription complexFunction(QStringLiteral("doBar"), VariableDescriptionList(), VariableDescriptionList());
    complexFunction.arguments << VariableDescription(QStringLiteral("bool"), QStringLiteral("really"));
    complexFunction.arguments << VariableDescription(QStringLiteral("int"), QStringLiteral("howMuch"));
    complexFunction.returnArguments << VariableDescription(QStringLiteral("double"), QString());

    publicFunctions << FunctionDescription(QStringLiteral("doFoo"), VariableDescriptionList(), VariableDescriptionList());
    publicFunctions << complexFunction;
    protectedFunctions << FunctionDescription(QStringLiteral("onUpdate"), VariableDescriptionList(), VariableDescriptionList());

    vars[QStringLiteral("functions")] = CodeDescription::toVariantList(publicFunctions + protectedFunctions + privateFunctions);
    vars[QStringLiteral("public_functions")]    = CodeDescription::toVariantList(publicFunctions);
    vars[QStringLiteral("protected_functions")] = CodeDescription::toVariantList(protectedFunctions);
    vars[QStringLiteral("private_functions")]   = CodeDescription::toVariantList(privateFunctions);

    vars[QStringLiteral("testCases")]  = QStringList {
        QStringLiteral("doFoo"),
        QStringLiteral("doBar"),
        QStringLiteral("doMore")
    };

    addVariables(vars);
}

TemplatePreviewRenderer::~TemplatePreviewRenderer()
{

}

TemplatePreview::TemplatePreview(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
    m_variables[QStringLiteral("APPNAME")] = QStringLiteral("Example");
    m_variables[QStringLiteral("APPNAMELC")] = QStringLiteral("example");
    m_variables[QStringLiteral("APPNAMEUC")] = QStringLiteral("EXAMPLE");
    m_variables[QStringLiteral("APPNAMEID")] = QStringLiteral("Example");

    m_variables[QStringLiteral("PROJECTDIR")] = QDir::homePath() + "/projects/ExampleProjectDir";
    m_variables[QStringLiteral("PROJECTDIRNAME")] = QStringLiteral("ExampleProjectDir");
    m_variables[QStringLiteral("VERSIONCONTROLPLUGIN")] = QStringLiteral("kdevgit");

    KTextEditor::Document* doc = KTextEditor::Editor::instance()->createDocument(this);
    m_preview.reset(doc);
    m_preview->setReadWrite(false);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    m_view = m_preview->createView(this);
    m_view->setStatusBarEnabled(false);
    if (KTextEditor::ConfigInterface* config = qobject_cast<KTextEditor::ConfigInterface*>(m_view)) {
        config->setConfigValue(QStringLiteral("icon-bar"), false);
        config->setConfigValue(QStringLiteral("folding-bar"), false);
        config->setConfigValue(QStringLiteral("line-numbers"), false);
        config->setConfigValue(QStringLiteral("dynamic-word-wrap"), true);
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
