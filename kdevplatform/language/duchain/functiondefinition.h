/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CLASSFUNCTIONDEFINITION_H
#define KDEVPLATFORM_CLASSFUNCTIONDEFINITION_H

#include "functiondeclaration.h"
#include <language/languageexport.h>

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT FunctionDefinitionData
    : public FunctionDeclarationData
{
public:

    //Holds the declaration id for this definition, if this is a definition with separate declaration
    DeclarationId m_declaration;
};
///A FunctionDefinition represents a function-definition that is assigned to a separate function-declaration.
///It allows mapping from definition to declaration and from declaration to definition.
class KDEVPLATFORMLANGUAGE_EXPORT FunctionDefinition
    : public FunctionDeclaration
{
public:
    FunctionDefinition(const RangeInRevision& range, DUContext* context);
    explicit FunctionDefinition(FunctionDefinitionData& data);
    ~FunctionDefinition() override;

    FunctionDefinition& operator=(const FunctionDefinition& rhs) = delete;

    /**
     * Find the declaration for this definition, if one exists.
     *
     * @param topContext the top-context from which to search
     * \returns the declaration matching this definition, otherwise null if no matching declaration has been found.
     * */
    Declaration* declaration(const TopDUContext* topContext = nullptr) const;

    ///Returns true if a Declaration has been assigned to this Definition
    bool hasDeclaration() const;

    ///Attaches this definition to the given declaration persistently.
    void setDeclaration(Declaration* declaration);

    enum {
        Identity = 21
    };

    /**
     * Find the definition for the given declaration, if one exists.
     *
     * \returns the definition matching this declaration, otherwise null if no matching definition has been found.
     * */
    static Declaration* definition(const Declaration* decl);

protected:
    FunctionDefinition (const FunctionDefinition& rhs);

private:
    Declaration* clonePrivate() const override;
    DUCHAIN_DECLARE_DATA(FunctionDefinition)
};
}

#endif
