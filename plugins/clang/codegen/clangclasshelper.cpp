/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "clangclasshelper.h"

#include "util/clangdebug.h"
#include "duchain/unknowndeclarationproblem.h"

#include <interfaces/iprojectcontroller.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/codedescription.h>
#include <custom-definesandincludes/idefinesandincludesmanager.h>
#include <project/projectmodel.h>
#include <util/path.h>

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
    // equality operators
    << "    bool operator==(const " << name << "& other) const;\n"
    << "    bool operator!=(const " << name << "& other) const;\n"
    << "};\n";
    file.close();
    ReferencedTopDUContext context(DUChain::self()->waitForUpdate(IndexedString(file.fileName()),
                                                                  TopDUContext::AllDeclarationsAndContexts));
    QList<DeclarationPointer> methods;
    {
        DUChainReadLocker lock;

        if (context && context->childContexts().size() == 1) {
            const auto localDeclarations = context->childContexts().first()->localDeclarations();
            methods.reserve(localDeclarations.size());
            for (auto* declaration : localDeclarations) {
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

QString includeArgumentForFile(const QString& includefile, const Path::List& includePaths,
                               const Path& source)
{
    const auto sourceFolder = source.parent();
    const Path canonicalFile(QFileInfo(includefile).canonicalFilePath());

    QString shortestDirective;
    bool isRelative = false;

    // we can include the file directly
    if (sourceFolder == canonicalFile.parent()) {
        shortestDirective = canonicalFile.lastPathSegment();
        isRelative = true;
    } else {
        // find the include directive with the shortest length
        for (const auto& includePath : includePaths) {
            QString relative = includePath.relativePath(canonicalFile);
            if (relative.startsWith(QLatin1String("./"))) {
                relative.remove(0, 2);
            }

            if (shortestDirective.isEmpty() || relative.length() < shortestDirective.length()) {
                shortestDirective = relative;
                isRelative = (includePath == sourceFolder);
            }
        }
    }

    // Item not found in include path?
    if (shortestDirective.isEmpty()) {
        return {};
    }

    if (isRelative) {
        return QLatin1Char('\"') + shortestDirective + QLatin1Char('\"');
    }
    return QLatin1Char('<') + shortestDirective + QLatin1Char('>');
}

QString includeDirectiveArgumentFromPath(const Path& file,
                                         const DeclarationPointer& declaration)
{
    const auto includeManager = IDefinesAndIncludesManager::manager();
    const auto filePath = file.toLocalFile();
    const auto projectModel = ICore::self()->projectController()->projectModel();
    auto item = projectModel->itemForPath(IndexedString(filePath));

    if (!item) {
        // try the folder where the file is placed and guess includes from there
        // prefer target over file
        const auto folderPath = IndexedString(file.parent().toLocalFile());
        clangDebug() << "File not known, guessing includes from items in folder:" << folderPath.str();

        // default to the folder, if no targets or files
        item = projectModel->itemForPath(folderPath);
        if (item) {
            const auto targetItems = item->targetList();
            bool itemChosen = false;
            // Prefer items defined inside a target with non-empty includes.
            for (const auto& targetItem : targetItems) {
                item = targetItem;
                if (!includeManager->includes(targetItem, IDefinesAndIncludesManager::ProjectSpecific).isEmpty()) {
                    clangDebug() << "Guessing includes from target" << targetItem->baseName();
                    itemChosen = true;
                    break;
                }
            }
            if (!itemChosen) {
                const auto fileItems = item->fileList();
                // Prefer items defined inside a target with non-empty includes.
                for (const auto& fileItem : fileItems) {
                    item = fileItem;
                    if (!includeManager->includes(fileItem, IDefinesAndIncludesManager::ProjectSpecific).isEmpty()) {
                        clangDebug() << "Guessing includes from file" << fileItem->baseName();
                        break;
                    }
                }
            }
        }
    }

    const auto includePaths = includeManager->includes(item);

    if (includePaths.isEmpty()) {
        clangDebug() << "Include path is empty";
        return {};
    }

    clangDebug() << "found include paths for" << file << ":" << includePaths;

    const auto includeFiles = UnknownDeclarationProblem::findMatchingIncludeFiles(QVector<Declaration*> {declaration.data()});
    if (includeFiles.isEmpty()) {
        // return early as the computation of the include paths is quite expensive
        return {};
    }

    // create include arguments for all candidates
    QStringList includeArguments;
    includeArguments.reserve(includeFiles.size());
    for (const auto& includeFile : includeFiles) {
        const auto includeArgument = includeArgumentForFile(includeFile, includePaths, file);
        if (includeArgument.isEmpty()) {
            clangDebug() << "unable to create include argument for" << includeFile << "in" << file.toLocalFile();
        }
        includeArguments << includeArgument;
    }

    if (includeArguments.isEmpty()) {
        return {};
    }

    std::sort(includeArguments.begin(), includeArguments.end(),
              [](const QString& lhs, const QString& rhs) {
        return lhs.length() < rhs.length();
    });

    return includeArguments.at(0);
}

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

    QStringList includedFiles;
    DUChainReadLocker locker(DUChain::lock());

    QUrl sourceUrl;
    const auto urls = fileUrls();
    if (!urls.isEmpty()) {
        sourceUrl = urls.constBegin().value();
    } else {
        // includeDirectiveArgumentFromPath() expects a path to the folder where includes are used from
        sourceUrl = baseUrl();
        sourceUrl.setPath(sourceUrl.path() + QLatin1String("/.h"));
    }
    const Path sourcePath(sourceUrl);

    const auto& directBaseClasses = this->directBaseClasses();
    for (const auto& baseClass : directBaseClasses) {
        if (!baseClass) {
            continue;
        }

        clangDebug() << "Looking for includes for class" << baseClass->identifier().toString();

        const QString includeDirective = includeDirectiveArgumentFromPath(sourcePath, baseClass);
        if (!includeDirective.isEmpty()) {
            includedFiles << includeDirective;
        }
    }
    variables[QStringLiteral("included_files")] = includedFiles;

    return variables;
}


DocumentChangeSet ClangTemplateNewClass::generate()
{
    addVariables(extraVariables());
    return TemplateClassGenerator::generate();
}
