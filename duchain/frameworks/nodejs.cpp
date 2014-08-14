/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
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
 *
 */
#include "nodejs.h"
#include "../helper.h"
#include "../parsesession.h"

#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/integraltype.h>
#include <util/path.h>

#include <QFile>
#include <QDir>
#include <QStandardPaths>

using namespace KDevelop;

namespace QmlJS {

NodeJS::NodeJS()
{
}

NodeJS& NodeJS::instance()
{
    static NodeJS* i = nullptr;

    if (!i) {
        i = new NodeJS();
    }

    return *i;
}

void NodeJS::initialize(DeclarationBuilder* builder)
{
    QMutexLocker lock(&m_mutex);

    // Create "module", a structure that may contain "exports" if the module
    // refers to module.exports
    createObject(QLatin1String("module"), 1, builder);

    // Create "exports", that can also contain the exported symbols of the module
    createObject(QLatin1String("exports"), 2, builder);
}

void NodeJS::createObject(const QString& name, int index, DeclarationBuilder* builder)
{
    QualifiedIdentifier identifier(name);

    StructureType::Ptr type(new StructureType);
    Declaration* decl = builder->openDeclaration<Declaration>(identifier, RangeInRevision());

    type->setDeclaration(decl);
    decl->setAlwaysForceDirect(true);
    decl->setKind(Declaration::Type);   // Not exactly what the user would expect, but this ensures that QmlJS::getInternalContext does not recurse infinitely
    decl->setInternalContext(builder->openContext(
        (QmlJS::AST::Node*)nullptr + index,                // Index is used to disambiguate the contexts. "node" is never dereferenced and is only stored in a hash table
        RangeInRevision(),
        DUContext::Class,
        identifier
    ));

    builder->closeContext();
    builder->openType(type);
    builder->closeAndAssignType();
}

DeclarationPointer NodeJS::moduleExports(const QString& moduleName, const IndexedString& url)
{
    QString fileName = moduleFileName(moduleName, url.str());
    DeclarationPointer exports;

    if (fileName.isEmpty()) {
        return exports;
    }

    ReferencedTopDUContext topContext = ParseSession::contextOfFile(fileName, url, 0);
    DUChainReadLocker lock;

    if (topContext) {
        QualifiedIdentifier idModule(QLatin1String("module"));
        QualifiedIdentifier idExports(QLatin1String("exports"));

        // Try "module.exports". If this declaration exists, it contains the
        // module's exports
        exports = getDeclaration(idModule, topContext.data());

        if (exports && exports->internalContext()) {
            exports = getDeclaration(idExports, exports->internalContext(), false);
        }

        // Try "exports", that always exist, has a structure type, and contains
        // the exported symbols
        if (!exports) {
            exports = getDeclaration(idExports, topContext.data());
        }
    }

    return exports;
}

DeclarationPointer NodeJS::moduleMember(const QString& moduleName,
                                        const QString& memberName,
                                        const IndexedString& url)
{
    DeclarationPointer module = moduleExports(moduleName, url);
    DeclarationPointer member;

    if (module) {
        member = QmlJS::getDeclaration(
            QualifiedIdentifier(memberName),
            QmlJS::getInternalContext(module),
            false
        );
    }

    return member;
}

Path::List NodeJS::moduleDirectories(const QString& url)
{
    Path::List paths;

    // QML/JS ships several modules that exist only in binary form in Node.js
    QStringList dirs = QStandardPaths::locateAll(
        QStandardPaths::GenericDataLocation,
        QLatin1String("kdevqmljssupport/nodejsmodules"),
        QStandardPaths::LocateDirectory
    );

    for (auto dir : dirs) {
        paths.append(Path(dir));
    }

    // url/../node_modules, then url/../../node_modules, etc
    Path path(url);
    path.addPath(QLatin1String(".."));

    while (path.segments().size() > 1) {
        paths.append(path.cd(QLatin1String("node_modules")));
        path.addPath(QLatin1String(".."));
    }

    return paths;
}

QString NodeJS::moduleFileName(const QString& moduleName, const QString& url)
{
    QMutexLocker lock(&m_mutex);
    auto pair = qMakePair(moduleName, url);

    if (m_cachedModuleFileNames.contains(pair)) {
        return m_cachedModuleFileNames.value(pair);
    }

    QString& fileName = m_cachedModuleFileNames[pair];

    // Absolue and relative URLs
    if (moduleName.startsWith(QLatin1Char('/')) || moduleName.startsWith(QLatin1Char('.'))) {
        // NOTE: This is not portable to Windows, but the Node.js documentation
        // only talks about module names that start with /, ./ and ../ .
        fileName = fileOrDirectoryPath(Path(url).cd(QLatin1String("..")).cd(moduleName).toLocalFile());
        return fileName;
    }

    // Try all the paths that might contain modules
    for (auto path : moduleDirectories(url)) {
        fileName = fileOrDirectoryPath(path.cd(moduleName).toLocalFile());

        if (!fileName.isNull()) {
            break;
        }
    }

    return fileName;
}

QString NodeJS::fileOrDirectoryPath(const QString& baseName)
{
    if (QFile::exists(baseName)) {
        return baseName;
    } else if (QFile::exists(baseName + QLatin1String(".js"))) {
        return baseName + QLatin1String(".js");
    } else if (QFile::exists(baseName + QLatin1String("/index.js"))) {
        // TODO: package.json files currently not supported
        return baseName + QLatin1String("/index.js");
    }

    return QString();
}

}
