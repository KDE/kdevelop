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

#ifndef CONTEXTBUILDER_H
#define CONTEXTBUILDER_H

#include <language/duchain/builders/abstractcontextbuilder.h>
#include <cmakelistsparser.h>

typedef QListIterator<CMakeFunctionDesc> CMakeContentIterator;

class ContextBuilder : public KDevelop::AbstractContextBuilder<CMakeContentIterator, CMakeFunctionDesc>
{
public:
    virtual KDevelop::DUContext* contextFromNode(CMakeContentIterator* node);
    virtual KDevelop::RangeInRevision editorFindRange(CMakeContentIterator* fromNode, CMakeContentIterator* toNode);
    virtual KDevelop::QualifiedIdentifier identifierForNode(CMakeFunctionDesc* node);
    virtual void setContextOnNode(CMakeContentIterator* node, KDevelop::DUContext* context);
    virtual KDevelop::TopDUContext* newTopContext(const KDevelop::RangeInRevision& range, KDevelop::ParsingEnvironmentFile* file = 0);
};

#endif // CONTEXTBUILDER_H
