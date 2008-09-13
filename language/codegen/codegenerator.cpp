/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "codegenerator.h"

using namespace KDevelop;

class KDevelop::CodeGeneratorPrivate
{
};

CodeGenerator::CodeGenerator()
    : d(new CodeGeneratorPrivate)
{
}

CodeGenerator::~CodeGenerator()
{
    delete d;
}

EditorChangeSet* CodeGenerator::textEdits() const
{
    return 0;
}

void CodeGenerator::generateTextEdit(AstChangeSet* astChange)
{
}

void CodeGenerator::generateTextEdit(DUChainChangeSet* astChange)
{
}

#include "codegenerator.h"
