/*
    SPDX-FileCopyrightText: 2014 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "usebuilder.h"
#include <cmakeutils.h>
#include <QGlobalStatic>
#include <language/duchain/declaration.h>

static QSet<QString> initCommands()
{
    QStringList ids = CMake::executeProcess(QStringLiteral("cmake"), QStringList(QStringLiteral("--help-command-list"))).split(QLatin1Char('\n'));
    if (ids.isEmpty()) {
        return {};
    }
    const auto secondIt = ids.constBegin() + 1;
    return QSet<QString>(secondIt, ids.constEnd());
}

// TODO: maybe share this again with codecompletionmodel and documentation
Q_GLOBAL_STATIC_WITH_ARGS(QSet<QString>, s_commands, (initCommands()))

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

void UseBuilder::newUse(const KDevelop::RangeInRevision& sr, const KDevelop::DeclarationPointer& d)
{
    m_ctx->createUse(m_ctx->indexForUsedDeclaration(d.data()), sr, 0);
}
