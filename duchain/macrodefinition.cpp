/*
 * Copyright 2014  Kevin Funk <kfunk@kde.org>
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
 */

#include "macrodefinition.h"

#include <language/duchain/appendedlist.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/duchainregister.h>

using namespace KDevelop;

DEFINE_LIST_MEMBER_HASH(MacroDefinitionData, parameters, IndexedString)

class MacroDefinitionData : public DeclarationData
{
public:
    MacroDefinitionData()
        : isFunctionLike(false)
    {
        initializeAppendedLists();
    }
    MacroDefinitionData(const MacroDefinitionData& rhs)
        : DeclarationData(rhs)
        , isFunctionLike(rhs.isFunctionLike)
    {
        initializeAppendedLists();
        copyListsFrom(rhs);
    }
    ~MacroDefinitionData()
    {
        freeAppendedLists();
    }

    IndexedString definition;
    bool isFunctionLike : 1;

    START_APPENDED_LISTS_BASE(MacroDefinitionData, DeclarationData);
    APPENDED_LIST_FIRST(MacroDefinitionData, IndexedString, parameters);
    END_APPENDED_LISTS(MacroDefinitionData, parameters);
};

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(MacroDefinition);
}

MacroDefinition::MacroDefinition(const MacroDefinition& rhs)
    : Declaration(*new MacroDefinitionData(*rhs.d_func()))
{
}

MacroDefinition::MacroDefinition(MacroDefinitionData& data)
    : Declaration(data)
{
}

MacroDefinition::MacroDefinition(const RangeInRevision& range, DUContext* context )
  : Declaration(*new MacroDefinitionData, range)
{
    d_func_dynamic()->setClassId(this);
    if(context)
        setContext(context);
}

MacroDefinition::~MacroDefinition()
{
}

IndexedString MacroDefinition::definition() const
{
    return d_func()->definition;
}

void MacroDefinition::setDefinition(const IndexedString& definition)
{
    d_func_dynamic()->definition = definition;
}

bool MacroDefinition::isFunctionLike() const
{
    return d_func()->isFunctionLike;
}

void MacroDefinition::setFunctionLike(bool isFunctionLike)
{
    d_func_dynamic()->isFunctionLike = isFunctionLike;
}

const IndexedString* MacroDefinition::parameters() const
{
    return d_func()->parameters();
}

unsigned int MacroDefinition::parametersSize() const
{
    return d_func()->parametersSize();
}

void MacroDefinition::addParameter(const IndexedString& str)
{
    d_func_dynamic()->parametersList().append(str);
}

void MacroDefinition::clearParameters()
{
    d_func_dynamic()->parametersList().clear();
}
