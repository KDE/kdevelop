/* KDevelop CMake Support
 *
 * Copyright 2014 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "usebuilder.h"
#include <cmakeutils.h>
#include <QGlobalStatic>
#include <language/duchain/declaration.h>

static QStringList initCommands()
{
    QStringList ids = CMake::executeProcess("cmake", QStringList("--help-command-list")).split('\n');
    ids.removeFirst();
    return ids;
}

// TODO: maybe share this again with codecompletionmodel and documentation
Q_GLOBAL_STATIC_WITH_ARGS(QSet<QString>, s_commands, (initCommands().toSet()))

UseBuilder::UseBuilder(const KDevelop::ReferencedTopDUContext& ctx)
    : m_ctx(ctx)
{
}

void UseBuilder::startVisiting(CMakeContentIterator* node)
{
    for(; node->hasNext(); ) {
        const CMakeFunctionDesc& func = node->next();

        QString fname = func.name.toLower();
        if (!s_commands->contains(fname)) {
            KDevelop::DUChainWriteLocker lock;

            KDevelop::Identifier nameid(fname);
            QList<KDevelop::Declaration*> declarations = m_ctx->findDeclarations(nameid, func.range().start);
            if (!declarations.isEmpty()) {
                newUse(
                    func.nameRange(),
                    KDevelop::DeclarationPointer(declarations.first())
                );
            }
        }
    }
}

void UseBuilder::newUse(const KDevelop::RangeInRevision& sr, KDevelop::DeclarationPointer d)
{
    m_ctx->createUse(m_ctx->indexForUsedDeclaration(d.data()), sr, 0);
}
