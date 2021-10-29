/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DEBUGVISITOR_H
#define DEBUGVISITOR_H

#include "parsesession.h"
#include "clangprivateexport.h"

class KDEVCLANGPRIVATE_EXPORT DebugVisitor
{
public:
    explicit DebugVisitor(ParseSession* session);

    void visit(CXTranslationUnit unit, CXFile file);

private:
    ParseSession* m_session;
};

#endif // DEBUGVISITOR_H
