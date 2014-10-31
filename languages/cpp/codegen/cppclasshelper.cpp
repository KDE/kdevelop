/*
 * KDevelop C++ Language Support
 *
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "cppclasshelper.h"
#include "../debug.h"
#include <codecompletion/missingincludeitem.h>
#include <language/duchain/declaration.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/codedescription.h>

#include <QTemporaryFile>

using namespace KDevelop;

CppClassHelper::CppClassHelper()
{

}

CppClassHelper::~CppClassHelper()
{

}

TemplateClassGenerator* CppClassHelper::createGenerator(const QUrl& baseUrl)
{
    return new CppTemplateNewClass(baseUrl);
}

QList<DeclarationPointer> CppClassHelper::defaultMethods(const QString& name) const
{
    QTemporaryFile file(QDir::tempPath() + QLatin1String("/class_XXXXXX") + QLatin1String(".cpp"));
    file.setAutoRemove(false);
    file.open();
    QTextStream stream(&file);
    stream << "class " << name << " {\n"
    << "  public:\n"
    // default ctor
    << "    " << name << "();\n"
    // copy ctor
    << "    " << name << "(const " << name << "& other);\n"
    // default dtor
    << "    ~" << name << "();\n"
    // assignment operator
    << "    " << name << "& operator=(const " << name << "& other);\n"
    // equality operator
    << "    bool operator==(const " << name << "& other) const;\n"
    << "};\n";
    file.close();
    ReferencedTopDUContext context(DUChain::self()->waitForUpdate( IndexedString(file.fileName()),
                                                                   TopDUContext::AllDeclarationsAndContexts ));
    DUChainReadLocker lock;

    QList<DeclarationPointer> methods;

    if (context && context->childContexts().size() == 1) {
        foreach (Declaration* declaration, context->childContexts().first()->localDeclarations())
        {
            methods << DeclarationPointer(declaration);
        }
    }

    file.remove();
    return methods;
}

CppTemplateNewClass::CppTemplateNewClass(const QUrl& url)
: TemplateClassGenerator(url)
{

}

CppTemplateNewClass::~CppTemplateNewClass()
{

}

QVariantHash CppTemplateNewClass::extraVariables()
{
    QVariantHash variables;

    QMap<QString, VariableDescriptionList> variableDescriptions;

    QMap<QString, FunctionDescriptionList> functionDescriptions;
    QMap<QString, FunctionDescriptionList> slotDescriptions;
    FunctionDescriptionList signalDescriptions;

    foreach (const FunctionDescription& function, description().methods)
    {
        QString access = function.access;
        if (access.isEmpty())
        {
            access = "public";
        }
        if (function.isSignal)
        {
            signalDescriptions << function;
        }
        else if (function.isSlot)
        {
            slotDescriptions[access] << function;
        }
        else
        {
            functionDescriptions[access] << function;
        }
    }

    foreach (const VariableDescription& variable, description().members)
    {
        QString access = variable.access;
        if (access.isEmpty())
        {
            access = "public";
        }
        variableDescriptions[access] << variable;
    }

    QMap<QString, VariableDescriptionList>::const_iterator vit, vend;
    vit = variableDescriptions.constBegin();
    vend = variableDescriptions.constEnd();
    for (; vit != vend; ++vit)
    {
        variables[vit.key() + "_members"] = CodeDescription::toVariantList(vit.value());
    }

    QMap<QString, FunctionDescriptionList>::const_iterator fit, fend;
    fit = functionDescriptions.constBegin();
    fend = functionDescriptions.constEnd();
    for (; fit != fend; ++fit)
    {
        variables[fit.key() + "_functions"] = CodeDescription::toVariantList(fit.value());
    }

    fit = slotDescriptions.constBegin();
    fend = slotDescriptions.constEnd();
    for (; fit != fend; ++fit)
    {
        variables[fit.key() + "_slots"] = CodeDescription::toVariantList(fit.value());
    }

    variables["signals"] = CodeDescription::toVariantList(signalDescriptions);
    variables["needs_qobject_macro"] = !slotDescriptions.isEmpty() || !signalDescriptions.isEmpty();

    QStringList includedFiles;
    DUChainReadLocker locker(DUChain::lock());

    QUrl sourceUrl;
    QHash<QString, QUrl> urls = fileUrls();
    if (!urls.isEmpty())
    {
        sourceUrl = urls.constBegin().value();
    }
    else
    {
        // includeDirectiveFromUrl() expects a header URL
        sourceUrl = baseUrl();
        sourceUrl.setPath(sourceUrl.path() + '/' + ".h");
    }

    foreach (const DeclarationPointer& base, directBaseClasses())
    {
        if (!base)
        {
            continue;
        }
        qCDebug(CPP) << "Looking for includes for class" << base->identifier().toString();
        QExplicitlySharedDataPointer<Cpp::MissingIncludeCompletionItem> item = Cpp::includeDirectiveFromUrl(sourceUrl, IndexedDeclaration(base.data()));
        if(item)
        {
            qCDebug(CPP) << "Found one in" << item->m_canonicalPath;
            includedFiles << item->m_addedInclude;
        }
    }
    variables["included_files"] = includedFiles;

    return variables;
}


DocumentChangeSet CppTemplateNewClass::generate()
{
  addVariables(extraVariables());
  return TemplateClassGenerator::generate();
}

void CppTemplateNewClass::addBaseClass(const QString& base)
{
  //strip access specifier
  QStringList splitBase = base.split(' ', QString::SkipEmptyParts);

  //if no access specifier is found use public by default
  if(splitBase.size() == 1)
  {
    splitBase.prepend("public");
  }

  //Call base function with the access specifier
  TemplateClassGenerator::addBaseClass(splitBase.join(" "));
}
