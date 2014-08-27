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

#include "contextbuilder.h"
#include <cmakemanager.h>

using namespace KDevelop;

KDevelop::DUContext* ContextBuilder::contextFromNode(CMakeContentIterator* node)
{
    return 0;
}

KDevelop::RangeInRevision ContextBuilder::editorFindRange(CMakeContentIterator* fromNode, CMakeContentIterator* toNode)
{
    return RangeInRevision();
}

KDevelop::QualifiedIdentifier ContextBuilder::identifierForNode(CMakeFunctionDesc* node)
{
    return QualifiedIdentifier(node->name);
}

void ContextBuilder::setContextOnNode(CMakeContentIterator* node, KDevelop::DUContext* context)
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

