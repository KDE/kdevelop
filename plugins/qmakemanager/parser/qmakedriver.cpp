/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakedriver.h"

#include <QTextStream>
#include <QFile>

#include <debug.h>
#include "qmakeparser.h"
#include "qmakeast.h"

#include "qmakedebugvisitor.h"
#include "buildastvisitor.h"

#include "ast.h"

#define ifDebug(x)

namespace QMake {

Driver::Driver()
{
}

bool Driver::readFile(const QString& filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(KDEV_QMAKE) << "Couldn't open project file:" << filename;
        return false;
    }
    QTextStream s(&f);
    m_content = s.readAll();
    return true;
}
void Driver::setContent(const QString& content)
{
    m_content = content;
}
void Driver::setDebug(bool debug)
{
    m_debug = debug;
}
bool Driver::parse(ProjectAST** qmast)
{
    KDevPG::TokenStream tokenStream;
    KDevPG::MemoryPool memory_pool;

    Parser qmakeparser;
    qmakeparser.setTokenStream(&tokenStream);
    qmakeparser.setMemoryPool(&memory_pool);
    qmakeparser.setDebug(m_debug);

    qmakeparser.tokenize(m_content);
    ProjectAst* ast = nullptr;
    bool matched = qmakeparser.parseProject(&ast);
    if (matched) {
        ifDebug(qCDebug(KDEV_QMAKE) << "Successfully parsed";) if (m_debug)
        {
            DebugVisitor d(&qmakeparser);
            d.visitProject(ast);
        }
        if (*qmast)
            delete *qmast;
        *qmast = new ProjectAST();
        BuildASTVisitor d(&qmakeparser, *qmast);
        d.visitProject(ast);
        ifDebug(qCDebug(KDEV_QMAKE) << "Found" << (*qmast)->statements.count() << "Statements";)
    } else {
        ast = nullptr;
        qCDebug(KDEV_QMAKE) << "Couldn't parse content";
    }
    return matched;
}
}
