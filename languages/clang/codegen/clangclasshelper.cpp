/*
 * KDevelop C++ Language Support
 *
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 * Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "clangclasshelper.h"

#include "util/clangdebug.h"
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/codedescription.h>

#include <QTemporaryFile>
#include <QDir>

using namespace KDevelop;

ClangClassHelper::ClangClassHelper()
{
}

ClangClassHelper::~ClangClassHelper() = default;

TemplateClassGenerator* ClangClassHelper::createGenerator(const QUrl& baseUrl)
{
    return new ClangTemplateNewClass(baseUrl);
}

QList<DeclarationPointer> ClangClassHelper::defaultMethods(const QString& name) const
{
    // TODO: this is the oldcpp approach, perhaps clang provides this directly?
    // TODO: default destructor misses info about virtualness, possible needs ICreateClassHelper change?

    QTemporaryFile file(QDir::tempPath() + QLatin1String("/class_") + name + QLatin1String("_XXXXXX.cpp"));
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
    ReferencedTopDUContext context(DUChain::self()->waitForUpdate(IndexedString(file.fileName()),
                                                                  TopDUContext::AllDeclarationsAndContexts));
    QList<DeclarationPointer> methods;
    {
        DUChainReadLocker lock;

        if (context && context->childContexts().size() == 1) {
            for (auto* declaration : context->childContexts().first()->localDeclarations()) {
                methods << DeclarationPointer(declaration);
            }
        }
    }

    return methods;
}

ClangTemplateNewClass::ClangTemplateNewClass(const QUrl& url)
    : TemplateClassGenerator(url)
{
}

ClangTemplateNewClass::~ClangTemplateNewClass() = default;

namespace {
template<typename Map>
void addVariables(QVariantHash* variables, QLatin1String suffix, const Map& map)
{
    for (auto it = map.begin(), end = map.end(); it != end; ++it) {
        variables->insert(it.key() + suffix, CodeDescription::toVariantList(it.value()));
    }
}
}

QVariantHash ClangTemplateNewClass::extraVariables() const
{
    QVariantHash variables;

    const QString publicAccess = QStringLiteral("public");

    QHash<QString, VariableDescriptionList> variableDescriptions;
    QHash<QString, FunctionDescriptionList> functionDescriptions;
    QHash<QString, FunctionDescriptionList> slotDescriptions;
    FunctionDescriptionList signalDescriptions;

    const auto desc = description();
    for (const auto& function : desc.methods) {
        const QString& access = function.access.isEmpty() ? publicAccess : function.access;

        if (function.isSignal) {
            signalDescriptions << function;
        } else if (function.isSlot) {
            slotDescriptions[access] << function;
        } else {
            functionDescriptions[access] << function;
        }
    }

    for (const auto& variable : desc.members) {
        const QString& access = variable.access.isEmpty() ? publicAccess : variable.access;

        variableDescriptions[access] << variable;
    }

    ::addVariables(&variables, QLatin1String("_members"), variableDescriptions);
    ::addVariables(&variables, QLatin1String("_functions"), functionDescriptions);
    ::addVariables(&variables, QLatin1String("_slots"), slotDescriptions);

    variables[QStringLiteral("signals")] = CodeDescription::toVariantList(signalDescriptions);
    variables[QStringLiteral("needs_qobject_macro")] = !slotDescriptions.isEmpty() || !signalDescriptions.isEmpty();

    // TODO: port to KDev-clang
#if 0
    QStringList includedFiles;
    DUChainReadLocker locker(DUChain::lock());

    QUrl sourceUrl;
    QHash<QString, QUrl> urls = fileUrls();
    if (!urls.isEmpty()) {
        sourceUrl = urls.constBegin().value();
    }
    else {
        // includeDirectiveFromUrl() expects a header URL
        sourceUrl = baseUrl();
        sourceUrl.setPath(sourceUrl.path() + '/' + QLatin1String(".h"));
    }

    for (const auto& base : directBaseClasses()) {
        if (!base) {
            continue;
        }

        clangDebug() << "Looking for includes for class" << base->identifier().toString();
        QExplicitlySharedDataPointer<Cpp::MissingIncludeCompletionItem> item = Cpp::includeDirectiveFromUrl(sourceUrl, IndexedDeclaration(base.data()));
        if(item)
        {
            clangDebug() << "Found one in" << item->m_canonicalPath;
            includedFiles << item->m_addedInclude;
        }
    }
    variables[QStringLiteral("included_files")] = includedFiles;
#endif

    return variables;
}


DocumentChangeSet ClangTemplateNewClass::generate()
{
    addVariables(extraVariables());
    return TemplateClassGenerator::generate();
}
