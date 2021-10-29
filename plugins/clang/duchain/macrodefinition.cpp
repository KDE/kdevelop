/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "macrodefinition.h"

#include <language/duchain/appendedlist.h>
#include <language/duchain/declarationdata.h>

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
        , definition(rhs.definition)
        , isFunctionLike(rhs.isFunctionLike)
    {
        initializeAppendedLists();
        copyListsFrom(rhs);
    }
    ~MacroDefinitionData()
    {
        freeAppendedLists();
    }

    MacroDefinitionData& operator=(const MacroDefinitionData& rhs) = delete;

    IndexedString definition;
    bool isFunctionLike : 1;

    START_APPENDED_LISTS_BASE(MacroDefinitionData, DeclarationData);
    APPENDED_LIST_FIRST(MacroDefinitionData, IndexedString, parameters);
    END_APPENDED_LISTS(MacroDefinitionData, parameters);
};

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
    setKind(Declaration::Macro);
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

QString MacroDefinition::toString() const
{
    return i18n("Macro %1", identifier().toString());
}

DUCHAIN_DEFINE_TYPE(MacroDefinition)
