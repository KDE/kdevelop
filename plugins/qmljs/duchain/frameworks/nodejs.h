/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef __FRAMEWORK_NODEJS_H__
#define __FRAMEWORK_NODEJS_H__

#include "../declarationbuilder.h"
#include "duchainexport.h"
#include <language/duchain/duchainpointer.h>
#include <util/path.h>

#include <QMutex>

namespace QmlJS {

/**
 * Singleton implementing support for the Node.js Javascript framework
 */
class KDEVQMLJSDUCHAIN_EXPORT NodeJS {
private:
    NodeJS();

public:
    static NodeJS& instance();

    /**
     * Initialize a QML/JS context so that it contains built-in Node.js declarations
     * that will allow the module to export symbols.
     *
     * @note The DUChain must be write-locked
     */
    void initialize(DeclarationBuilder* builder);

    /**
     * Declaration of the "exports" (or "module.exports") member of a Node.js
     * module.
     *
     * @param moduleName Name of the module to import ("http", "../module.js", etc)
     * @param url Url of the document that imports the module
     */
    KDevelop::DeclarationPointer moduleExports(const QString& moduleName,
                                               const KDevelop::IndexedString& url);

    /**
     * Declaration of a member of a module
     */
    KDevelop::DeclarationPointer moduleMember(const QString& moduleName,
                                              const QString& memberName,
                                              const KDevelop::IndexedString& url);

    /**
     * List of directories where Node.js modules visible from @p url may exist
     */
    KDevelop::Path::List moduleDirectories(const QString& url);

private:
    void createObject(const QString& name, int index, DeclarationBuilder* builder);
    QString moduleFileName(const QString& moduleName, const QString& url);
    QString fileOrDirectoryPath(const QString& baseName);

private:
    using CachedModuleFileNamesHash = QHash<QPair<QString, QString>, QString>;
    CachedModuleFileNamesHash m_cachedModuleFileNames;
    QMutex m_mutex;
};

}

#endif
