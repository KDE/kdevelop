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
#ifndef __FRAMEWORK_NODEJS_H__
#define __FRAMEWORK_NODEJS_H__

#include "../declarationbuilder.h"
#include <language/duchain/duchainpointer.h>

namespace QmlJS {

/**
 * Singleton implementing support for the Node.js Javascript framework
 */
class NodeJS {
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
    KDevelop::DeclarationPointer moduleExports(const QString& moduleName, const QString& url);

private:
    void createObject(const QString& name, int index, DeclarationBuilder* builder);
    QString moduleFileName(const QString& moduleName, const QString& url);

private:
    typedef QHash<QPair<QString, QString>, QString> CachedModuleFileNamesHash;
    CachedModuleFileNamesHash m_cachedModuleFileNames;
};

}

#endif