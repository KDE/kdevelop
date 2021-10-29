/*
    SPDX-FileCopyrightText: 2014 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONTEXTBUILDER_H
#define CONTEXTBUILDER_H

#include <language/duchain/builders/abstractcontextbuilder.h>
#include <cmakelistsparser.h>

using CMakeContentIterator = QVectorIterator<CMakeFunctionDesc>;

class ContextBuilder : public KDevelop::AbstractContextBuilder<CMakeContentIterator, CMakeFunctionDesc>
{
public:
    KDevelop::DUContext* contextFromNode(CMakeContentIterator* node) override;
    KDevelop::RangeInRevision editorFindRange(CMakeContentIterator* fromNode, CMakeContentIterator* toNode) override;
    KDevelop::QualifiedIdentifier identifierForNode(CMakeFunctionDesc* node) override;
    void setContextOnNode(CMakeContentIterator* node, KDevelop::DUContext* context) override;
    KDevelop::TopDUContext* newTopContext(const KDevelop::RangeInRevision& range, KDevelop::ParsingEnvironmentFile* file = nullptr) override;
};

#endif // CONTEXTBUILDER_H
