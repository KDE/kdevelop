/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "templatepreview.h"

#include <language/codegen/templaterenderer.h>
#include <language/codegen/codedescription.h>

#include <QDir>
#include <QVBoxLayout>

#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <KTextEditor/Document>

#include <KLocalizedString>
#include <KMacroExpander>

using namespace KDevelop;

TemplatePreviewRenderer::TemplatePreviewRenderer()
{
    QVariantHash vars;
    vars[QStringLiteral("name")] = QStringLiteral("Example");
    vars[QStringLiteral("license")] = QStringLiteral("This file is licensed under the ExampleLicense 3.0");
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

TemplatePreview::TemplatePreview(QWidget* parent)
    : QWidget(parent)
{
    m_variables[QStringLiteral("APPNAME")] = QStringLiteral("Example");
    m_variables[QStringLiteral("APPNAMELC")] = QStringLiteral("example");
    m_variables[QStringLiteral("APPNAMEUC")] = QStringLiteral("EXAMPLE");
    m_variables[QStringLiteral("APPNAMEID")] = QStringLiteral("Example");

    m_variables[QStringLiteral("PROJECTDIR")] = QDir::homePath() + QLatin1String("/projects/ExampleProjectDir");
    m_variables[QStringLiteral("PROJECTDIRNAME")] = QStringLiteral("ExampleProjectDir");
    m_variables[QStringLiteral("VERSIONCONTROLPLUGIN")] = QStringLiteral("kdevgit");

    KTextEditor::Document* doc = KTextEditor::Editor::instance()->createDocument(this);
    m_preview.reset(doc);
    m_preview->setReadWrite(false);

    auto* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    m_view = m_preview->createView(this);
    m_view->setStatusBarEnabled(false);
    m_view->setConfigValue(QStringLiteral("icon-bar"), false);
    m_view->setConfigValue(QStringLiteral("folding-bar"), false);
    m_view->setConfigValue(QStringLiteral("line-numbers"), false);
    m_view->setConfigValue(QStringLiteral("dynamic-word-wrap"), true);
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

#include "moc_templatepreview.cpp"
