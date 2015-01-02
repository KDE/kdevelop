/*
 *    This file is part of KDevelop
 *
 *    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef TUDUCHAIN_H
#define TUDUCHAIN_H

#include <duchain/clangduchainexport.h>
#include "templatehelpers.h"
#include "cursorkindtraits.h"
#include "clanghelpers.h"
#include "clangducontext.h"
#include "macrodefinition.h"
#include "util/clangdebug.h"
#include "util/clangutils.h"
#include "util/clangtypes.h"

#include <util/pushvalue.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/stringhelpers.h>

#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/referencetype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/enumerationtype.h>
#include <language/duchain/types/enumeratortype.h>
#include <language/duchain/types/typealiastype.h>

#include <unordered_map>
#include <typeinfo>

/// Turn on for debugging the declaration building
#define IF_DEBUG(x)

template<CXCursorKind CK, bool isDefinition, bool isClassMember, class Enable = void>
struct DeclType;
template<CXCursorKind CK, class Enable = void>
struct IdType;

class KDEVCLANGDUCHAIN_EXPORT TUDUChain
{
public:
    TUDUChain(CXTranslationUnit tu, CXFile file, const IncludeFileContexts& includes, const bool update);

private:
    static CXChildVisitResult visitCursor(CXCursor cursor, CXCursor parent, CXClientData data);

    // Uh oh, this isn't nice. Can we make this better?
    friend class TestFiles;
    static bool s_jsonTestRun;

    void setIdTypeDecl(CXCursor typeCursor, IdentifiedType *idType) const;
    void contextImportDecl(DUContext *context, const DeclarationPointer& decl) const;

    KDevelop::Identifier makeId(CXCursor cursor) const;
    QByteArray makeComment(CXComment comment) const;
    AbstractType *makeType(CXType type, CXCursor parent);
    AbstractType::Ptr makeAbsType(CXType type, CXCursor parent) { return AbstractType::Ptr(makeType(type, parent)); }
    AbstractType* createDelayedType(CXType type) const;

//BEGIN dispatch*
    template<CXCursorKind CK, EnableIf<CursorKindTraits::isUse(CK)> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor)
    {
        return buildUse<CK>(cursor);
    }

    template<CXCursorKind CK, EnableIf<CK == CXCursor_CompoundStmt> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor)
    {
        if (m_parentContext->context->type() == DUContext::Function)
        {
            auto context = createContext<CK, DUContext::Other>(cursor);
            CurrentContext newParent(context);
            PushValue<CurrentContext*> pushCurrent(m_parentContext, &newParent);
            clang_visitChildren(cursor, &visitCursor, this);
            return CXChildVisit_Continue;
        }
        return CXChildVisit_Recurse;
    }

    template<
      CXCursorKind CK,
      Decision IsInClass = CursorKindTraits::isInClass(CK),
      EnableIf<IsInClass == Decision::Maybe> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor, CXCursor parent)
    {
      const bool decision = CursorKindTraits::isClass(clang_getCursorKind(parent));
      return decision ?
        dispatchCursor<CK, Decision::True, CursorKindTraits::isDefinition(CK)>(cursor, parent) :
        dispatchCursor<CK, Decision::False, CursorKindTraits::isDefinition(CK)>(cursor, parent);
    }

    template<
        CXCursorKind CK,
        Decision IsInClass = CursorKindTraits::isInClass(CK),
        Decision IsDefinition = CursorKindTraits::isDefinition(CK),
        EnableIf<IsDefinition == Decision::Maybe && IsInClass != Decision::Maybe> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor, CXCursor parent)
    {
        IF_DEBUG(clangDebug() << "IsInClass:" << IsInClass << "- isDefinition:" << IsDefinition;)

        const bool isDefinition = clang_isCursorDefinition(cursor);
        return isDefinition ?
          dispatchCursor<CK, IsInClass, Decision::True>(cursor, parent) :
          dispatchCursor<CK, IsInClass, Decision::False>(cursor, parent);
    }

    template<
        CXCursorKind CK,
        Decision IsInClass = CursorKindTraits::isInClass(CK),
        Decision IsDefinition = CursorKindTraits::isDefinition(CK),
        EnableIf<IsInClass != Decision::Maybe && IsDefinition != Decision::Maybe> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor, CXCursor parent)
    {
        IF_DEBUG(clangDebug() << "IsInClass:" << IsInClass << "- isDefinition:" << IsDefinition;)

        // We may end up visiting the same cursor twice in some cases
        // see discussion on https://git.reviewboard.kde.org/r/119526/
        // TODO: Investigate why this is happening in libclang
        if ((CursorKindTraits::isClass(CK) || CK == CXCursor_EnumDecl) &&
                clang_getCursorKind(parent) == CXCursor_VarDecl) {
            return CXChildVisit_Continue;
        }

        constexpr bool isClassMember = IsInClass == Decision::True;
        constexpr bool isDefinition = IsDefinition == Decision::True;
        constexpr bool hasContext = CursorKindTraits::isFunction(CK) || (IsDefinition == Decision::True);
        return buildDeclaration<CK, typename DeclType<CK, isDefinition, isClassMember>::Type, hasContext>(cursor);
    }

    template<CXTypeKind TK>
    AbstractType *dispatchType(CXType type, CXCursor cursor)
    {
        IF_DEBUG(clangDebug() << "TK:" << type.kind;)

        auto kdevType = createType<TK>(type, cursor);
        setTypeModifiers<TK>(type, kdevType);
        return kdevType;
    }
//BEGIN dispatch*

//BEGIN build*
    template<CXCursorKind CK, class DeclType, bool hasContext>
    CXChildVisitResult buildDeclaration(CXCursor cursor)
    {
        auto id = makeId(cursor);
        IF_DEBUG(clangDebug() << "id:" << id << "- CK:" << CK << "- DeclType:" << typeid(DeclType).name() << "- hasContext:" << hasContext;)

        // Code path for class declarations that may be defined "out-of-line", e.g.
        // "SomeNameSpace::SomeClass {};"
        QScopedPointer<CurrentContext> helperContext;
        if (CursorKindTraits::isClass(CK)) {
            const auto lexicalParent = clang_getCursorLexicalParent(cursor);
            const auto semanticParent = clang_getCursorSemanticParent(cursor);
            const bool isOutOfLine = !clang_equalCursors(lexicalParent, semanticParent);
            if (isOutOfLine) {
                const QString scope = ClangUtils::getScope(cursor);
                auto context = createContext<CK, DUContext::Helper>(cursor, QualifiedIdentifier(scope));
                helperContext.reset(new CurrentContext(context));
            }
        }

        // if helperContext is null, this is a no-op
        PushValue<CurrentContext*> pushCurrent(m_parentContext, helperContext.isNull() ? m_parentContext : helperContext.data());

        if (hasContext) {
            auto context = createContext<CK, CursorKindTraits::contextType(CK)>(cursor, QualifiedIdentifier(id));
            createDeclaration<CK, DeclType>(cursor, id, context);
            CurrentContext newParent(context);
            PushValue<CurrentContext*> pushCurrent(m_parentContext, &newParent);
            clang_visitChildren(cursor, &visitCursor, this);
            return CXChildVisit_Continue;
        }
        createDeclaration<CK, DeclType>(cursor, id, nullptr);
        return CXChildVisit_Recurse;
    }

    template<CXCursorKind CK>
    CXChildVisitResult buildUse(CXCursor cursor)
    {
        m_uses[m_parentContext->context].push_back(cursor);
        return CK == CXCursor_DeclRefExpr || CK == CXCursor_MemberRefExpr ?
            CXChildVisit_Recurse : CXChildVisit_Continue;
    }
//END build*

//BEGIN create*
    template<CXCursorKind CK, class DeclType>
    DeclType* createDeclarationCommon(CXCursor cursor, const Identifier& id)
    {
        auto range = ClangHelpers::cursorSpellingNameRange(cursor, id);

        if (m_update) {
            const IndexedIdentifier indexedId(id);
            DUChainWriteLocker lock;
            auto it = m_parentContext->previousChildDeclarations.begin();
            while (it != m_parentContext->previousChildDeclarations.end()) {
                auto decl = dynamic_cast<DeclType*>(*it);
                if (decl && decl->indexedIdentifier() == indexedId) {
                    decl->setRange(range);
                    setDeclData<CK>(cursor, decl);
                    m_parentContext->previousChildDeclarations.erase(it);
                    return decl;
                }
                ++it;
            }
        }
        auto decl = new DeclType(range, nullptr);
        decl->setIdentifier(id);
        m_cursorToDeclarationCache[clang_hashCursor(cursor)] = decl;
        setDeclData<CK>(cursor, decl);
        return decl;
    }

    template<CXCursorKind CK, class DeclType>
    KDevelop::Declaration* createDeclaration(CXCursor cursor, const KDevelop::Identifier& id, KDevelop::DUContext *context)
    {
        auto decl = createDeclarationCommon<CK, DeclType>(cursor, id);
        auto type = createType<CK>(cursor);

        DUChainWriteLocker lock;
        decl->setContext(m_parentContext->context);
        if (context)
            decl->setInternalContext(context);
        setDeclType<CK>(decl, type);
        setDeclInCtxtData<CK>(cursor, decl);
        return decl;
    }

    template<CXCursorKind CK, KDevelop::DUContext::ContextType Type>
    KDevelop::DUContext* createContext(CXCursor cursor, const KDevelop::QualifiedIdentifier& scopeId = {})
    {
        // wtf: why is the DUContext API requesting a QID when it needs a plain Id?!
        // see: testNamespace
        auto range = ClangRange(clang_getCursorExtent(cursor)).toRangeInRevision();
        DUChainWriteLocker lock;
        if (m_update) {
            const KDevelop::IndexedQualifiedIdentifier indexedScopeId(scopeId);
            auto it = m_parentContext->previousChildContexts.begin();
            while (it != m_parentContext->previousChildContexts.end()) {
                auto ctx = *it;
                if (ctx->type() == Type && ctx->indexedLocalScopeIdentifier() == indexedScopeId) {
                    ctx->setRange(range);
                    m_parentContext->previousChildContexts.erase(it);
                    return ctx;
                }
                ++it;
            }
        }
        //TODO: (..type, id..) constructor for DUContext?
        auto context = new ClangNormalDUContext(range, m_parentContext->context);
        context->setType(Type);
        context->setLocalScopeIdentifier(scopeId);
        if (Type == KDevelop::DUContext::Other || Type == KDevelop::DUContext::Function)
            context->setInSymbolTable(false);

        if (CK == CXCursor_CXXMethod || CursorKindTraits::isClass(CK)) {
            CXCursor semParent = clang_getCursorSemanticParent(cursor);
            if (!clang_Cursor_isNull(semParent)) {
                auto semParentDecl = findDeclaration(semParent);
                if (semParentDecl) {
                    contextImportDecl(context, semParentDecl);
                }
            }
        }
        return context;
    }

    template<CXTypeKind TK, EnableIf<CursorKindTraits::integralType(TK) != -1> = dummy>
    AbstractType *createType(CXType, CXCursor)
    {
        // TODO: would be nice to instantiate a ConstantIntegralType here and set a value if possible
        // but unfortunately libclang doesn't offer API to that
        // also see http://marc.info/?l=cfe-commits&m=131609142917881&w=2
        return new IntegralType(CursorKindTraits::integralType(TK));
    }

    template<CXTypeKind TK, EnableIf<CursorKindTraits::isPointerType(TK)> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        auto ptr = new PointerType;
        ptr->setBaseType(makeAbsType(clang_getPointeeType(type), parent));
        return ptr;
    }

    template<CXTypeKind TK, EnableIf<CursorKindTraits::isArrayType(TK)> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        auto arr = new ArrayType;
        arr->setDimension((TK == CXType_IncompleteArray || TK == CXType_VariableArray || TK == CXType_DependentSizedArray) ? 0 : clang_getArraySize(type));
        arr->setElementType(makeAbsType(clang_getArrayElementType(type), parent));
        return arr;
    }

    template<CXTypeKind TK, EnableIf<TK == CXType_RValueReference || TK == CXType_LValueReference> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        auto ref = new ReferenceType;
        ref->setIsRValue(type.kind == CXType_RValueReference);
        ref->setBaseType(makeAbsType(clang_getPointeeType(type), parent));
        return ref;
    }

    template<CXTypeKind TK, EnableIf<TK == CXType_FunctionProto> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        auto func = new FunctionType;
        func->setReturnType(makeAbsType(clang_getResultType(type), parent));
        const int numArgs = clang_getNumArgTypes(type);
        for (int i = 0; i < numArgs; ++i) {
            func->addArgument(makeAbsType(clang_getArgType(type, i), parent));
        }
        /// TODO: variadic functions
        return func;
    }

    template<CXTypeKind TK, EnableIf<TK == CXType_Record || TK == CXType_ObjCInterface || TK == CXType_ObjCClass> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        DeclarationPointer decl = findDeclaration(clang_getTypeDeclaration(type));
        DUChainReadLocker lock;

        if (!decl) {
            // probably a forward-declared type
            decl = ClangHelpers::findForwardDeclaration(type, m_parentContext->context, parent);
        }

        auto t = new StructureType;
        t->setDeclaration(decl.data());
        return t;
    }

    template<CXTypeKind TK, EnableIf<TK == CXType_Enum> = dummy>
    AbstractType *createType(CXType type, CXCursor)
    {
        auto t = new EnumerationType;
        setIdTypeDecl(clang_getTypeDeclaration(type), t);
        return t;
    }

    template<CXTypeKind TK, EnableIf<TK == CXType_Typedef> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        auto t = new TypeAliasType;
        CXCursor location = clang_getTypeDeclaration(type);
        t->setType(makeAbsType(clang_getTypedefDeclUnderlyingType(location), parent));
        setIdTypeDecl(location, t);
        return t;
    }

    template<CXTypeKind TK, EnableIf<CursorKindTraits::delayedTypeName(TK) != 0> = dummy>
    AbstractType *createType(CXType, CXCursor parent)
    {
        auto t = new DelayedType;
        static const IndexedTypeIdentifier id(CursorKindTraits::delayedTypeName(TK));
        t->setIdentifier(id);
        return t;
    }

    template<CXTypeKind TK, EnableIf<TK == CXType_Vector || TK == CXType_Unexposed> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        ClangHelpers::findForwardDeclaration(type, m_parentContext->context, parent);
        return createDelayedType(type);
    }

    template<CXCursorKind CK, EnableIf<CursorKindTraits::isIdentifiedType(CK) && !CursorKindTraits::isAliasType(CK) && CK != CXCursor_EnumConstantDecl> = dummy>
    typename IdType<CK>::Type *createType(CXCursor)
    {
        return new typename IdType<CK>::Type;
    }

    template<CXCursorKind CK, EnableIf<CK == CXCursor_EnumConstantDecl> = dummy>
    EnumeratorType *createType(CXCursor cursor)
    {
        auto type = new EnumeratorType;
        type->setValue<quint64>(clang_getEnumConstantDeclUnsignedValue(cursor));
        return type;
    }

    template<CXCursorKind CK, EnableIf<CursorKindTraits::isAliasType(CK)> = dummy>
    TypeAliasType *createType(CXCursor cursor)
    {
        auto type = new TypeAliasType;
        type->setType(makeAbsType(clang_getTypedefDeclUnderlyingType(cursor), cursor));
        return type;
    }

    template<CXCursorKind CK, EnableIf<!CursorKindTraits::isIdentifiedType(CK)> = dummy>
    AbstractType *createType(CXCursor cursor)
    {
        auto clangType = clang_getCursorType(cursor);
        return makeType(clangType, cursor);
    }
//END create*

//BEGIN setDeclData
template<CXCursorKind CK>
void setDeclData(CXCursor cursor, Declaration *decl, bool setComment = true) const
{
    if (setComment)
        decl->setComment(makeComment(clang_Cursor_getParsedComment(cursor)));
    if (CursorKindTraits::isAliasType(CK)) {
        decl->setIsTypeAlias(true);
    }
    if (CK == CXCursor_Namespace)
        decl->setKind(Declaration::Namespace);
    if (CK == CXCursor_EnumDecl || CK == CXCursor_EnumConstantDecl || CursorKindTraits::isClass(CK))
        decl->setKind(Declaration::Type);

    int isAlwaysDeprecated;
    clang_getCursorPlatformAvailability(cursor, &isAlwaysDeprecated, nullptr, nullptr, nullptr, nullptr, 0);
    decl->setDeprecated(isAlwaysDeprecated);
}

template<CXCursorKind CK>
void setDeclData(CXCursor cursor, MacroDefinition* decl)
{
    setDeclData<CK>(cursor, static_cast<Declaration*>(decl));

    if (m_update) {
        decl->clearParameters();
    }

    auto unit = clang_Cursor_getTranslationUnit(cursor);
    auto range = clang_getCursorExtent(cursor);

    // TODO: Quite lacking API in libclang here.
    // No way to find out if this macro is function-like or not
    // cf. http://clang.llvm.org/doxygen/classclang_1_1MacroInfo.html
    // And no way to get the actual definition text range
    // Should be quite easy to expose that in libclang, though
    // Let' still get some basic support for this and parse on our own, it's not that difficult
    const QByteArray contents = ClangUtils::getRawContents(unit, range);
    const int firstOpeningParen = contents.indexOf('(');
    const int firstWhitespace = contents.indexOf(' ');
    const bool isFunctionLike = (firstOpeningParen != -1) && (firstOpeningParen < firstWhitespace);
    decl->setFunctionLike(isFunctionLike);

    // now extract the actual definition text
    int start = -1;
    if (isFunctionLike) {
        const int closingParen = KDevelop::findClose(contents, firstOpeningParen);
        if (closingParen != -1) {
            start = closingParen + 2; // + ')' + ' '

            // extract macro function parameters
            const QString parameters = QString::fromUtf8(contents.mid(firstOpeningParen, closingParen - firstOpeningParen + 1));
            ParamIterator paramIt("():", parameters, 0);
            while (paramIt) {
                decl->addParameter(IndexedString(*paramIt));
                ++paramIt;
            }
        }
    } else {
        start = firstWhitespace + 1; // + ' '
    }
    if (start == -1) {
        // unlikely: invalid macro definition, insert the complete #define statement
        decl->setDefinition(IndexedString("#define " + contents));
    } else if (start < contents.size()) {
        decl->setDefinition(IndexedString(contents.constData() + start));
    } // else: macro has no body => leave the definition text empty
}

template<CXCursorKind CK>
void setDeclData(CXCursor cursor, ClassMemberDeclaration *decl) const
{
    setDeclData<CK>(cursor, static_cast<Declaration*>(decl));
    //A CXCursor_VarDecl in a class is static (otherwise it'd be a CXCursor_FieldDecl)
    if (CK == CXCursor_VarDecl)
        decl->setStatic(true);
    decl->setAccessPolicy(CursorKindTraits::kdevAccessPolicy(clang_getCXXAccessSpecifier(cursor)));
}

template<CXCursorKind CK, EnableIf<CursorKindTraits::isClassTemplate(CK)> = dummy>
void setDeclData(CXCursor cursor, ClassDeclaration* decl) const
{
    CXCursorKind kind = clang_getTemplateCursorKind(cursor);
    switch (kind) {
        case CXCursor_UnionDecl: setDeclData<CXCursor_UnionDecl>(cursor, decl); break;
        case CXCursor_StructDecl: setDeclData<CXCursor_StructDecl>(cursor, decl); break;
        case CXCursor_ClassDecl: setDeclData<CXCursor_ClassDecl>(cursor, decl); break;
        default: Q_ASSERT(false); break;
    }
}

template<CXCursorKind CK, EnableIf<!CursorKindTraits::isClassTemplate(CK)> = dummy>
void setDeclData(CXCursor cursor, ClassDeclaration* decl) const
{
    if (m_update) {
        decl->clearBaseClasses();
    }
    setDeclData<CK>(cursor, static_cast<ClassMemberDeclaration*>(decl));
    if (CK == CXCursor_UnionDecl)
        decl->setClassType(ClassDeclarationData::Union);
    if (CK == CXCursor_StructDecl)
        decl->setClassType(ClassDeclarationData::Struct);
    if (clang_isCursorDefinition(cursor)) {
        decl->setDeclarationIsDefinition(true);
    }
}

template<CXCursorKind CK>
void setDeclData(CXCursor cursor, AbstractFunctionDeclaration* decl) const
{
    if (m_update) {
        decl->clearDefaultParameters();
    }
    // No setDeclData<CK>(...) here: AbstractFunctionDeclaration is an interface
    // TODO: Can we get the default arguments directly from Clang?
    // also see http://clang-developers.42468.n3.nabble.com/Finding-default-value-for-function-argument-with-clang-c-API-td4036919.html
    const QVector<QString> defaultArgs = ClangUtils::getDefaultArguments(cursor, ClangUtils::MinimumSize);
    foreach (const QString& defaultArg, defaultArgs) {
        decl->addDefaultParameter(IndexedString(defaultArg));
    }
}

template<CXCursorKind CK>
void setDeclData(CXCursor cursor, ClassFunctionDeclaration* decl) const
{
    setDeclData<CK>(cursor, static_cast<AbstractFunctionDeclaration*>(decl));
    setDeclData<CK>(cursor, static_cast<ClassMemberDeclaration*>(decl));
    decl->setAbstract(clang_CXXMethod_isPureVirtual(cursor));
    decl->setStatic(clang_CXXMethod_isStatic(cursor));
    decl->setVirtual(clang_CXXMethod_isVirtual(cursor));
}

template<CXCursorKind CK>
void setDeclData(CXCursor cursor, FunctionDeclaration *decl, bool setComment = true) const
{
    setDeclData<CK>(cursor, static_cast<AbstractFunctionDeclaration*>(decl));
    setDeclData<CK>(cursor, static_cast<Declaration*>(decl), setComment);
}

template<CXCursorKind CK>
void setDeclData(CXCursor cursor, FunctionDefinition *decl) const
{
    bool setComment = clang_equalCursors(clang_getCanonicalCursor(cursor), cursor);
    setDeclData<CK>(cursor, static_cast<FunctionDeclaration*>(decl), setComment);
}

//END setDeclData

//BEGIN setDeclInCtxtData
template<CXCursorKind CK>
void setDeclInCtxtData(CXCursor, Declaration*)
{
    //No-op
}
template<CXCursorKind CK>
void setDeclInCtxtData(CXCursor cursor, ClassFunctionDeclaration *decl)
{
    // HACK to retrieve function-constness
    // This looks like a bug in Clang -- In theory setTypeModifiers should take care of setting the const modifier
    // however, clang_isConstQualifiedType() for TK == CXType_FunctionProto always returns false
    // TODO: Debug further
    auto type = decl->abstractType();
    if (type) {
        if (ClangUtils::isConstMethod(cursor)) {
            type->setModifiers(type->modifiers() | AbstractType::ConstModifier);
            decl->setAbstractType(type);
        }
    }
}

template<CXCursorKind CK>
void setDeclInCtxtData(CXCursor cursor, FunctionDefinition *def)
{
    setDeclInCtxtData<CK>(cursor, static_cast<FunctionDeclaration*>(def));

    CXCursor canon = clang_getCanonicalCursor(cursor);
    if (clang_equalCursors(canon, cursor)) {
        def->setDeclarationIsDefinition(true);
    } else if (auto decl = findDeclaration(canon)) {
        def->setDeclaration(decl.data());
    }
}
//END setDeclInCtxtData

//BEGIN setDeclType
    template<CXCursorKind CK>
    void setDeclType(Declaration *decl, typename IdType<CK>::Type *type)
    {
        setDeclType<CK>(decl, static_cast<IdentifiedType*>(type));
        setDeclType<CK>(decl, static_cast<AbstractType*>(type));
    }

    template<CXCursorKind CK>
    void setDeclType(Declaration *decl, IdentifiedType *type)
    {
        type->setDeclaration(decl);
    }

    template<CXCursorKind CK>
    void setDeclType(Declaration *decl, AbstractType *type)
    {
        decl->setAbstractType(AbstractType::Ptr(type));
    }
//END setDeclType

template<CXTypeKind TK>
void setTypeModifiers(CXType type, AbstractType* kdevType) const;

private:
    struct CurrentContext
    {
        CurrentContext(KDevelop::DUContext* context)
            : context(context)
        {
            DUChainReadLocker lock;
            previousChildContexts = context->childContexts();
            previousChildDeclarations = context->localDeclarations();
        }
        ~CurrentContext()
        {
            DUChainWriteLocker lock;
            qDeleteAll(previousChildContexts);
            qDeleteAll(previousChildDeclarations);
        }

        KDevelop::DUContext* context;
        // when updatig, this contains child contexts of the current parent context
        QVector<KDevelop::DUContext*> previousChildContexts;
        // when updatig, this contains child declarations of the current parent context
        QVector<KDevelop::Declaration*> previousChildDeclarations;
    };
    friend CurrentContext;

    const CXFile m_file;
    const IncludeFileContexts &m_includes;

    DeclarationPointer findDeclaration(CXCursor cursor);

    std::unordered_map<DUContext*, std::vector<CXCursor>> m_uses;
    /// At these location offsets (cf. @ref clang_getExpansionLocation) we encountered macro expansions
    QSet<unsigned int> m_macroExpansionLocations;
    QHash<unsigned int, DeclarationPointer> m_cursorToDeclarationCache;
    CurrentContext *m_parentContext;

    const bool m_update;
};

template<>
CXChildVisitResult TUDUChain::buildUse<CXCursor_CXXBaseSpecifier>(CXCursor cursor);
template<>
CXChildVisitResult TUDUChain::buildUse<CXCursor_MacroExpansion>(CXCursor cursor);

#endif //TUDUCHAIN_H
