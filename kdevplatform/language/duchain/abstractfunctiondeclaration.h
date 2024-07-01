/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ABSTRACTFUNCTIONDECLARATION_H
#define KDEVPLATFORM_ABSTRACTFUNCTIONDECLARATION_H

#include "indexedducontext.h"

#include <language/languageexport.h>
#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

namespace KDevelop {
class DUContext;
class IndexedString;

class AbstractFunctionDeclarationData
{
public:
    AbstractFunctionDeclarationData() : m_isVirtual(false)
        , m_isInline(false)
        , m_isExplicit(false)
    {
    }
    IndexedDUContext m_functionContext;
    bool m_isVirtual : 1; ///@todo move into ClassFunctionDeclaration(Only valid for class-functions)
    bool m_isInline : 1;
    bool m_isExplicit : 1; ///@todo move into ClassFunctionDeclaration(Only valid for class-functions)
};

/**
 * Provides an interface to declarations which represent functions in a definition-use chain.
 * Don't inherit from this directly, use MergeAbstractFunctionDeclaration instead.
 */
class KDEVPLATFORMLANGUAGE_EXPORT AbstractFunctionDeclaration
{
public:
    virtual ~AbstractFunctionDeclaration();

    enum FunctionSpecifier {
        VirtualSpecifier  = 0x1 /**< indicates a virtual function */,
        InlineSpecifier   = 0x2 /**< indicates a inline function */,
        ExplicitSpecifier = 0x4 /**< indicates a explicit function */
    };
    Q_DECLARE_FLAGS(FunctionSpecifiers, FunctionSpecifier)

    void setFunctionSpecifiers(FunctionSpecifiers specifiers);

    bool isInline() const;
    void setInline(bool isInline);

    ///Only used for class-member function declarations(see ClassFunctionDeclaration)
    bool isVirtual() const;
    void setVirtual(bool isVirtual);

    ///Only used for class-member function declarations(see ClassFunctionDeclaration)
    bool isExplicit() const;
    void setExplicit(bool isExplicit);

    ///Return the DUContext::Function type ducontext (the function parameter context) of this function
    ///Same as internalContext if the function has no definition
    DUContext* internalFunctionContext() const;
    void setInternalFunctionContext(DUContext* context);

    /**
     * Returns the default-parameters that are set. The last default-parameter matches the last
     * argument of the function, but the returned vector will only contain default-values for those
     * arguments that have one, for performance-reasons.
     *
     * So the vector may be empty or smaller than the count of function-arguments.
     * */
    virtual const IndexedString* defaultParameters() const = 0;
    virtual unsigned int defaultParametersSize() const = 0;
    virtual void addDefaultParameter(const IndexedString& str) = 0;
    virtual void clearDefaultParameters()  = 0;
    ///Returns the default parameter assigned to the given argument number.
    ///This is a convenience-function.
    IndexedString defaultParameterForArgument(int index) const;

private:
    //Must be implemented by sub-classes to provide a pointer to the data
    virtual const AbstractFunctionDeclarationData* data() const = 0;
    virtual AbstractFunctionDeclarationData* dynamicData() = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractFunctionDeclaration::FunctionSpecifiers)

///Use this to merge AbstractFunctionDeclaration into the class hierarchy. Base must be the base-class
///in the hierarchy, and Data must be the Data class of the following Declaration, and must be based on AbstractFunctionDeclarationData
///and BaseData.
template <class Base, class _Data>
class MergeAbstractFunctionDeclaration
    : public Base
    , public AbstractFunctionDeclaration
{
public:
    template <class BaseData>
    explicit MergeAbstractFunctionDeclaration(BaseData& data) : Base(data)
    {
    }
    template <class BaseData, class Arg2>
    MergeAbstractFunctionDeclaration(BaseData& data, const Arg2& arg2) : Base(data, arg2)
    {
    }
    template <class BaseData, class Arg2, class Arg3>
    MergeAbstractFunctionDeclaration(BaseData& data, const Arg2& arg2, const Arg3& arg3) : Base(data, arg2, arg3)
    {
    }

private:
    const AbstractFunctionDeclarationData* data() const override
    {
        return static_cast<const _Data*>(Base::d_func());
    }
    AbstractFunctionDeclarationData* dynamicData() override
    {
        return static_cast<_Data*>(Base::d_func_dynamic());
    }
};

}

#endif // KDEVPLATFORM_ABSTRACTFUNCTIONDECLARATION_H
