/*
    SPDX-FileCopyrightText: 2014 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "contextbuilder.h"
#include <cmakemanager.h>

using namespace KDevelop;

KDevelop::DUContext* ContextBuilder::contextFromNode(CMakeContentIterator* /*node*/)
{
    return nullptr;
}

KDevelop::RangeInRevision ContextBuilder::editorFindRange(CMakeContentIterator* /*fromNode*/, CMakeContentIterator* /*toNode*/)
{
    return RangeInRevision();
}

KDevelop::QualifiedIdentifier ContextBuilder::identifierForNode(CMakeFunctionDesc* node)
{
    return QualifiedIdentifier(node->name);
}

void ContextBuilder::setContextOnNode(CMakeContentIterator* /*node*/, KDevelop::DUContext* /*context*/)
{

}

TopDUContext* ContextBuilder::newTopContext(const RangeInRevision& range, ParsingEnvironmentFile* file)
{
    if (!file) {
        file = new ParsingEnvironmentFile(document());
        file->setLanguage(CMakeManager::languageName());
    }

    return KDevelop::AbstractContextBuilder< CMakeContentIterator, CMakeFunctionDesc >::newTopContext(range, file);
}

