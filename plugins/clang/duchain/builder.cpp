/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "builder.h"

#include "util/clangdebug.h"

#include "templatehelpers.h"
#include "cursorkindtraits.h"
#include "clangducontext.h"
#include "macrodefinition.h"
#include "types/classspecializationtype.h"
#include "util/clangutils.h"
#include "util/clangtypes.h"

#include <util/pushvalue.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/stringhelpers.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/problem.h>

#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/referencetype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/enumerationtype.h>
#include <language/duchain/types/enumeratortype.h>
#include <language/duchain/types/typealiastype.h>
#include <language/duchain/types/indexedtype.h>

#include <clang-c/Documentation.h>

#include <QVarLengthArray>

#include <algorithm>
#include <unordered_map>
#include <typeinfo>


/// Turn on for debugging the declaration building
#define IF_DEBUG(x)

using namespace KDevelop;

namespace {

#if CINDEX_VERSION_MINOR >= 100
// TODO: this is ugly, can we find a better alternative?
bool jsonTestRun()
{
    static bool runningTest = qEnvironmentVariableIsSet("KDEV_CLANG_JSON_TEST_RUN");
    return runningTest;
}
#endif

//BEGIN helpers
// HACK: current alias type template machinery is badly broken wrt spelling
// location, work around this by adjusting all references to point to child
// type alias node with proper location
// TODO: investigate upstream implementation of CXCursor_TypeAliasTemplateDecl
CXCursor findEmbeddedTypeAlias(CXCursor aliasTemplate)
{
    auto result = clang_getNullCursor();
    clang_visitChildren(aliasTemplate, [] (CXCursor cursor, CXCursor, CXClientData data) {
        if (clang_getCursorKind(cursor) == CXCursor_TypeAliasDecl) {
            auto res = reinterpret_cast<CXCursor*>(data);
            *res = cursor;
            return CXChildVisit_Break;
        }
        return CXChildVisit_Continue;
    }, &result);
    return result;
}

/**
 * Find the cursor that cursor @p cursor references
 *
 * First tries to get the referenced cursor via clang_getCursorReferenced,
 * and if that fails, tries to get them via clang_getOverloadedDecl
 * (which returns the referenced cursor for CXCursor_OverloadedDeclRef, for example)
 *
 * @return Valid cursor on success, else null cursor
 */
CXCursor referencedCursor(CXCursor cursor)
{
    auto referenced = clang_getCursorReferenced(cursor);
    // HACK: see notes at getEmbeddedTypeAlias()
    if (clang_getCursorKind(referenced) == CXCursor_TypeAliasTemplateDecl) {
        return findEmbeddedTypeAlias(referenced);
    }

    if (!clang_equalCursors(cursor, referenced)) {
        return referenced;
    }

    // get the first result for now
    referenced = clang_getOverloadedDecl(cursor, 0);
    if (!clang_Cursor_isNull(referenced)) {
        return referenced;
    }
    return clang_getNullCursor();
}

Identifier makeId(CXCursor cursor)
{
    if (CursorKindTraits::isClassTemplate(cursor.kind)) {
        // TODO: how to handle functions here? We don't want to add the "real" function arguments here
        //       and there does not seem to be an API to get the template arguments for non-specializations easily
        // NOTE: using the QString overload of the Identifier ctor here, so that the template name gets parsed
        return Identifier(ClangString(clang_getCursorDisplayName(cursor)).toString());
    }

    const ClangString spelling(clang_getCursorSpelling(cursor));
    if (!spelling.isEmpty() && spelling.c_str()[0] == '[') {
        // skip unexposed DecompositionDecl, we want to get hold of the BindingsDecl inside instead
        return Identifier();
    }

    auto name = spelling.toIndexed();
    if (name.isEmpty() && CursorKindTraits::isClass(cursor.kind)) {
        // try to use the type name for typedef'ed anon structs etc. as a fallback
        auto type = ClangString(clang_getTypeSpelling(clang_getCursorType(cursor))).toString();
        // but don't associate a super long name for anon structs without a typedef
        if (!type.startsWith(QLatin1String("(anonymous "))) {
            name = IndexedString(type);
        }
    }

    return Identifier(name);
}

#if CINDEX_VERSION_MINOR >= 100 // FIXME https://bugs.llvm.org/show_bug.cgi?id=35333
QByteArray makeComment(CXComment comment)
{
    if (Q_UNLIKELY(jsonTestRun())) {
        auto kind = clang_Comment_getKind(comment);
        if (kind == CXComment_Text)
            return ClangString(clang_TextComment_getText(comment)).toByteArray();

        QByteArray text;
        int numChildren = clang_Comment_getNumChildren(comment);
        for (int i = 0; i < numChildren; ++i)
            text += makeComment(clang_Comment_getChild(comment, i));
        return text;
    }

    return ClangString(clang_FullComment_getAsHTML(comment)).toByteArray();
}
#endif

AbstractType* createDelayedType(CXType type)
{
    auto t = new DelayedType;

    QString typeName = ClangString(clang_getTypeSpelling(type)).toString();
    typeName.remove(QStringLiteral("const "));
    typeName.remove(QStringLiteral("volatile "));

    t->setIdentifier(IndexedTypeIdentifier(typeName));
    return t;
}

void contextImportDecl(DUContext* context, const DeclarationPointer& decl)
{
    auto top = context->topContext();
    if (auto import = decl->logicalInternalContext(top)) {
        context->addImportedParentContext(import);
        context->topContext()->updateImportsCache();
    }
}

//END helpers

CXChildVisitResult visitCursor(CXCursor cursor, CXCursor parent, CXClientData data);

//BEGIN IdType
template<CXCursorKind CK, class Enable = void>
struct IdType;

template<CXCursorKind CK>
struct IdType<CK, typename std::enable_if<CursorKindTraits::isClass(CK)>::type>
{
    using Type = StructureType;
};

template<CXCursorKind CK>
struct IdType<CK, typename std::enable_if<CK == CXCursor_TypedefDecl>::type>
{
    using Type = TypeAliasType;
};

template<CXCursorKind CK>
struct IdType<CK, typename std::enable_if<CK == CXCursor_TypeAliasDecl>::type>
{
    using Type = TypeAliasType;
};

template<CXCursorKind CK>
struct IdType<CK, typename std::enable_if<CK == CXCursor_EnumDecl>::type>
{
    using Type = EnumerationType;
};

template<CXCursorKind CK>
struct IdType<CK, typename std::enable_if<CK == CXCursor_EnumConstantDecl>::type>
{
    using Type = EnumeratorType;
};
//END IdType

//BEGIN DeclType
template<CXCursorKind CK, bool isDefinition, bool isClassMember, class Enable = void>
struct DeclType;

template<CXCursorKind CK, bool isDefinition, bool isInClass>
struct DeclType<CK, isDefinition, isInClass,
    typename std::enable_if<CursorKindTraits::isKDevDeclaration(CK, isInClass)>::type>
{
    using Type = Declaration;
};

template<CXCursorKind CK, bool isDefinition, bool isInClass>
struct DeclType<CK, isDefinition, isInClass,
    typename std::enable_if<CK == CXCursor_MacroDefinition>::type>
{
    using Type = MacroDefinition;
};

template<CXCursorKind CK, bool isDefinition, bool isInClass>
struct DeclType<CK, isDefinition, isInClass,
    typename std::enable_if<CursorKindTraits::isKDevForwardDeclaration(CK, isDefinition)>::type>
{
    using Type = ForwardDeclaration;
};

template<CXCursorKind CK, bool isDefinition, bool isInClass>
struct DeclType<CK, isDefinition, isInClass,
    typename std::enable_if<CursorKindTraits::isKDevClassDeclaration(CK, isDefinition)>::type>
{
    using Type = ClassDeclaration;
};

template<CXCursorKind CK, bool isDefinition, bool isInClass>
struct DeclType<CK, isDefinition, isInClass,
    typename std::enable_if<CursorKindTraits::isKDevClassFunctionDeclaration(CK, isInClass)>::type>
{
    using Type = ClassFunctionDeclaration;
};

template<CXCursorKind CK, bool isDefinition, bool isInClass>
struct DeclType<CK, isDefinition, isInClass,
    typename std::enable_if<CursorKindTraits::isKDevFunctionDeclaration(CK, isDefinition, isInClass)>::type>
{
    using Type = FunctionDeclaration;
};

template<CXCursorKind CK, bool isDefinition, bool isInClass>
struct DeclType<CK, isDefinition, isInClass,
    typename std::enable_if<CursorKindTraits::isKDevFunctionDefinition(CK, isDefinition, isInClass)>::type>
{
    using Type = FunctionDefinition;
};

template<CXCursorKind CK, bool isDefinition, bool isInClass>
struct DeclType<CK, isDefinition, isInClass,
    typename std::enable_if<CursorKindTraits::isKDevNamespaceAliasDeclaration(CK, isDefinition)>::type>
{
    using Type = NamespaceAliasDeclaration;
};

template<CXCursorKind CK, bool isDefinition, bool isInClass>
struct DeclType<CK, isDefinition, isInClass,
    typename std::enable_if<CursorKindTraits::isKDevClassMemberDeclaration(CK, isInClass)>::type>
{
    using Type = ClassMemberDeclaration;
};
//END DeclType

//BEGIN CurrentContext
struct CurrentContext
{
    CurrentContext(DUContext* context, const QSet<DUContext*>& keepAliveContexts)
        : context(context)
        , keepAliveContexts(keepAliveContexts)
    {
        DUChainReadLocker lock;
        previousChildContexts = context->childContexts();
        previousChildDeclarations = context->localDeclarations();
    }

    ~CurrentContext()
    {
        DUChainWriteLocker lock;
        for (auto* childContext : std::as_const(previousChildContexts)) {
            if (!keepAliveContexts.contains(childContext)) {
                delete childContext;
            }
        }
        qDeleteAll(previousChildDeclarations);
        if (resortChildContexts) {
            context->resortChildContexts();
        }
        if (resortLocalDeclarations) {
            context->resortLocalDeclarations();
        }
    }

    DUContext* context;
    // when updating, this contains child contexts of the current parent context
    QVector<DUContext*> previousChildContexts;
    // when updating, this contains contexts that must not be deleted
    QSet<DUContext*> keepAliveContexts;
    // when updating, this contains child declarations of the current parent context
    QVector<Declaration*> previousChildDeclarations;

    bool resortChildContexts = false;
    bool resortLocalDeclarations = false;
};
//END CurrentContext

//BEGIN Visitor
struct Visitor
{
    explicit Visitor(CXTranslationUnit tu, CXFile file,
                     const IncludeFileContexts& includes, const bool update);

    AbstractType *makeType(CXType type, CXCursor parent);
    AbstractType::Ptr makeAbsType(CXType type, CXCursor parent)
    {
        return AbstractType::Ptr(makeType(type, parent));
    }

//BEGIN dispatch*
    template<CXCursorKind CK,
      Decision IsInClass = CursorKindTraits::isInClass(CK),
      EnableIf<IsInClass == Decision::Maybe> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor, CXCursor parent);

    template<CXCursorKind CK,
        Decision IsInClass = CursorKindTraits::isInClass(CK),
        Decision IsDefinition = CursorKindTraits::isDefinition(CK),
        EnableIf<IsDefinition == Decision::Maybe && IsInClass != Decision::Maybe> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor, CXCursor parent);

    template<CXCursorKind CK,
        Decision IsInClass = CursorKindTraits::isInClass(CK),
        Decision IsDefinition = CursorKindTraits::isDefinition(CK),
        EnableIf<IsInClass != Decision::Maybe && IsDefinition != Decision::Maybe> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor, CXCursor parent);

    CXChildVisitResult dispatchTypeAliasTemplate(CXCursor cursor, CXCursor parent)
    {
        return CursorKindTraits::isClass(clang_getCursorKind(parent)) ? buildTypeAliasTemplateDecl<true>(cursor)
            : buildTypeAliasTemplateDecl<false>(cursor);
    }

    template<CXTypeKind TK>
    AbstractType *dispatchType(CXType type, CXCursor cursor)
    {
        IF_DEBUG(clangDebug() << "TK:" << type.kind;)

        auto kdevType = createType<TK>(type, cursor);
        if (kdevType) {
            setTypeModifiers<TK>(type, kdevType);
            setTypeSize(type, kdevType);
        }
        return kdevType;
    }
//BEGIN dispatch*

//BEGIN build*
    template<CXCursorKind CK, class DeclType, bool hasContext>
    CXChildVisitResult buildDeclaration(CXCursor cursor);

    template<bool IsInClass>
    CXChildVisitResult buildTypeAliasTemplateDecl(CXCursor cursor);

    CXChildVisitResult buildUse(CXCursor cursor);
    CXChildVisitResult buildMacroExpansion(CXCursor cursor);

    template<CXCursorKind CK>
    CXChildVisitResult buildCompoundStatement(CXCursor cursor);
    CXChildVisitResult buildCXXBaseSpecifier(CXCursor cursor);
    CXChildVisitResult buildParmDecl(CXCursor cursor);

//END build*

//BEGIN create*
    template<CXCursorKind CK, class DeclType>
    DeclType* createDeclarationCommon(CXCursor cursor, const Identifier& id)
    {
        auto range = ClangHelpers::cursorSpellingNameRange(cursor, id);

        if (id.isEmpty()) {
            // This is either an anonymous function parameter e.g.: void f(int);
            // Or anonymous struct/class/union e.g.: struct {} anonymous;
            // Set empty range for it
            range.end = range.start;
        }

        // check if cursor is inside a macro expansion (refer to rangeInRevisionForUse)
        auto clangRange = clang_Cursor_getSpellingNameRange(cursor, 0, 0);
        unsigned int expansionLocOffset;
        const auto clangRangeStart = clang_getRangeStart(clangRange);
        clang_getExpansionLocation(clangRangeStart, nullptr, nullptr, nullptr, &expansionLocOffset);
        if (m_macroExpansionLocations.contains(expansionLocOffset)) {
            unsigned int fileLocOffset;
            clang_getFileLocation(clangRangeStart, nullptr, nullptr, nullptr, &fileLocOffset);
            // Set empty ranges for declarations inside macro expansion
            if (fileLocOffset == expansionLocOffset) {
                range.end = range.start;
            }
        }

        if (m_update) {
            const IndexedIdentifier indexedId(id);
            DUChainWriteLocker lock;
            auto it = m_parentContext->previousChildDeclarations.begin();
            while (it != m_parentContext->previousChildDeclarations.end()) {
                auto decl = dynamic_cast<DeclType*>(*it);
                if (decl && decl->indexedIdentifier() == indexedId) {
                    decl->setRange(range);
                    m_parentContext->resortLocalDeclarations = true;
                    setDeclData<CK>(cursor, decl);
                    m_cursorToDeclarationCache[cursor] = decl;
                    m_parentContext->previousChildDeclarations.erase(it);
                    return decl;
                }
                ++it;
            }
        }
        auto decl = new DeclType(range, nullptr);
        decl->setIdentifier(id);
#if CINDEX_VERSION_MINOR >= 32
        decl->setExplicitlyTyped(clang_getCursorType(cursor).kind != CXType_Auto);
#endif
        m_cursorToDeclarationCache[cursor] = decl;
        setDeclData<CK>(cursor, decl);
        {
            DUChainWriteLocker lock;
            decl->setContext(m_parentContext->context);
        }
        return decl;
    }

    template<CXCursorKind CK, class DeclType>
    Declaration* createDeclaration(CXCursor cursor, const Identifier& id, DUContext *context)
    {
        auto decl = createDeclarationCommon<CK, DeclType>(cursor, id);
        auto type = createType<CK>(cursor);
        if (type) {
            setTypeSize(clang_getCursorType(cursor), type);
        }

        DUChainWriteLocker lock;
        if (context)
            decl->setInternalContext(context);
        setDeclType<CK>(decl, type);
        setDeclInCtxtData<CK>(cursor, decl);
        return decl;
    }

    template<CXCursorKind CK, DUContext::ContextType Type>
    DUContext* createContext(CXCursor cursor, const QualifiedIdentifier& scopeId = {})
    {
        // wtf: why is the DUContext API requesting a QID when it needs a plain Id?!
        // see: testNamespace
        auto range = ClangRange(clang_getCursorExtent(cursor)).toRangeInRevision();
        DUChainWriteLocker lock;
        if (m_update) {
            const IndexedQualifiedIdentifier indexedScopeId(scopeId);
            auto it = m_parentContext->previousChildContexts.begin();
            while (it != m_parentContext->previousChildContexts.end()) {
                auto ctx = *it;
                if (ctx->type() == Type && ctx->indexedLocalScopeIdentifier() == indexedScopeId) {
                    ctx->setRange(range);
                    m_parentContext->resortChildContexts = true;
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
        if (Type == DUContext::Other || Type == DUContext::Function)
            context->setInSymbolTable(false);

        if (CK == CXCursor_CXXMethod) {
            CXCursor semParent = clang_getCursorSemanticParent(cursor);
            // only import the semantic parent if it differs from the lexical parent
            if (!clang_Cursor_isNull(semParent) && !clang_equalCursors(semParent, clang_getCursorLexicalParent(cursor))) {
                auto semParentDecl = findDeclaration(semParent);
                if (semParentDecl) {
                    contextImportDecl(context, semParentDecl);
                }
            }
        }
        return context;
    }

    template<CXTypeKind TK, EnableIf<CursorKindTraits::integralType(TK) != IntegralType::TypeNotIntegralType> = dummy>
    AbstractType *createType(CXType, CXCursor)
    {
        // TODO: would be nice to instantiate a ConstantIntegralType here and set a value if possible
        // but unfortunately libclang doesn't offer API to that
        // also see https://marc.info/?l=cfe-commits&m=131609142917881&w=2
        return new IntegralType(CursorKindTraits::integralType(TK));
    }

    template <CXTypeKind TK, EnableIf<CursorKindTraits::isOpenCLType(TK)> = dummy>
    AbstractType* createType(CXType, CXCursor)
    {
        return new StructureType();
    }

#if CINDEX_VERSION_MINOR >= 60
    template <CXTypeKind TK, EnableIf<TK == CXType_Atomic> = dummy>
    AbstractType* createType(CXType type, CXCursor parent)
    {
        // Decompose the atomic type.
        return makeType(clang_Type_getValueType(type), parent);
    }
#endif

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

    template<CXTypeKind TK, EnableIf<TK == CXType_FunctionProto || TK == CXType_FunctionNoProto> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        auto func = new FunctionType;
        func->setReturnType(makeAbsType(clang_getResultType(type), parent));
        const int numArgs = clang_getNumArgTypes(type);
        for (int i = 0; i < numArgs; ++i) {
            func->addArgument(makeAbsType(clang_getArgType(type, i), parent));
        }

        if (clang_isFunctionTypeVariadic(type)) {
            auto type = new DelayedType;
            static const auto id = IndexedTypeIdentifier(QStringLiteral("..."));
            type->setIdentifier(id);
            type->setKind(DelayedType::Unresolved);
            func->addArgument(AbstractType::Ptr(type));
        }

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

        if (clang_Type_getNumTemplateArguments(type) != -1) {
            return createClassTemplateSpecializationType(type, decl);
        }

        auto t = new StructureType;
        if (decl) {
            t->setDeclaration(decl.data());
        } else {    // fallback, at least give the spelling to the user
            t->setDeclarationId(DeclarationId(IndexedQualifiedIdentifier(QualifiedIdentifier(ClangString(clang_getTypeSpelling(type)).toString()))));
        }
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

    template<CXTypeKind TK, EnableIf<CursorKindTraits::delayedTypeName(TK) != nullptr> = dummy>
    AbstractType *createType(CXType, CXCursor /*parent*/)
    {
        auto t = new DelayedType;
        static const IndexedTypeIdentifier id(CursorKindTraits::delayedTypeName(TK));
        t->setIdentifier(id);
        return t;
    }

    template <CXTypeKind TK, EnableIf<TK == CXType_Vector || TK == CXType_ExtVector || TK == CXType_Complex> = dummy>
    AbstractType* createType(CXType type, CXCursor /*parent*/)
    {
        return createDelayedType(type);
    }

    template<CXTypeKind TK, EnableIf<TK == CXType_Unexposed> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        auto numTA = clang_Type_getNumTemplateArguments(type);
        // TODO: We should really expose more types to libclang!
        if (numTA != -1 && ClangString(clang_getTypeSpelling(type)).toString().contains(QLatin1Char('<'))) {
            return createClassTemplateSpecializationType(type);
        }

        // Maybe it's the ElaboratedType. E.g.: "struct Type foo();" or "NS::Type foo();" or "void foo(enum Enum e);" e.t.c.
        auto oldType = type;

        type = clang_getCanonicalType(type);
        bool isElaboratedType = type.kind != CXType_FunctionProto && type.kind != CXType_FunctionNoProto && type.kind != CXType_Unexposed && type.kind != CXType_Invalid && type.kind != CXType_Record;

        return !isElaboratedType ? createDelayedType(oldType) : makeType(type, parent);
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

    template <CXCursorKind CK, EnableIf<CK == CXCursor_FunctionDecl> = dummy>
    AbstractType* createType(CXCursor cursor)
    {
        auto clangType = clang_getCursorType(cursor);

#if CINDEX_VERSION_MINOR < 31
        if (clangType.kind == CXType_Unexposed) {
            // Clang sometimes can return CXType_Unexposed for CXType_FunctionProto kind. E.g. if it's AttributedType.
            return dispatchType<CXType_FunctionProto>(clangType, cursor);
        }
#endif

        return makeType(clangType, cursor);
    }

    template<CXCursorKind CK, EnableIf<CK == CXCursor_LabelStmt> = dummy>
    AbstractType *createType(CXCursor)
    {
        auto t = new DelayedType;
        static const IndexedTypeIdentifier id(QStringLiteral("Label"));
        t->setIdentifier(id);
        return t;
    }

    template<CXCursorKind CK, EnableIf<!CursorKindTraits::isIdentifiedType(CK) && CK != CXCursor_FunctionDecl && CK != CXCursor_LabelStmt> = dummy>
    AbstractType *createType(CXCursor cursor)
    {
        auto clangType = clang_getCursorType(cursor);
        return makeType(clangType, cursor);
    }

#if CINDEX_VERSION_MINOR >= 32
    template<CXTypeKind TK, EnableIf<TK == CXType_Auto> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        auto deducedType = clang_getCanonicalType(type);
        bool isDeduced = deducedType.kind != CXType_Invalid && deducedType.kind != CXType_Auto;

        return !isDeduced ? createDelayedType(type) : makeType(deducedType, parent);
    }
#endif

#if CINDEX_VERSION_MINOR >= 34
    template<CXTypeKind TK, EnableIf<TK == CXType_Elaborated> = dummy>
    AbstractType *createType(CXType type, CXCursor parent)
    {
        auto underyingType = clang_Type_getNamedType(type);
        return makeType(underyingType, parent);
    }
#endif

    /// @param declaration an optional declaration that will be associated with created type
    AbstractType* createClassTemplateSpecializationType(CXType type, const DeclarationPointer& declaration = {})
    {
        auto numTA = clang_Type_getNumTemplateArguments(type);
        Q_ASSERT(numTA != -1);

        auto typeDecl = clang_getTypeDeclaration(type);

        if (!declaration && typeDecl.kind == CXCursor_NoDeclFound) {
            // clang_getTypeDeclaration doesn't handle all types, fall back to delayed type...
            return createDelayedType(type);
        }

        const QString tStr = ClangString(clang_getTypeSpelling(type)).toString();
        QVarLengthArray<QStringView, 8> typesStr;
        ParamIterator iter(u"<>", tStr);

        while (iter) {
            typesStr.push_back(*iter);
            ++iter;
        }

        auto cst = new ClassSpecializationType;

        for (int i = 0; i < numTA; i++) {
            auto argumentType = clang_Type_getTemplateArgumentAsType(type, i);
            AbstractType::Ptr currentType;
            if (argumentType.kind == CXType_Invalid) {
                if(i >= typesStr.size()){
                    currentType = createDelayedType(argumentType);
                } else {
                    auto t = new DelayedType;
                    t->setIdentifier(IndexedTypeIdentifier(typesStr[i]));
                    currentType = t;
                }
            } else {
                currentType = makeType(argumentType, typeDecl);
            }

            if (currentType) {
                cst->addParameter(currentType->indexed());
            }
        }

        auto decl = declaration ? declaration : findDeclaration(typeDecl);

        DUChainReadLocker lock;
        if (decl) {
            cst->setDeclaration(decl.data());
        } else { // fallback, at least give the spelling to the user
            Identifier id(tStr);
            id.clearTemplateIdentifiers();
            cst->setDeclarationId(DeclarationId(IndexedQualifiedIdentifier(QualifiedIdentifier(id))));
        }

        return cst;
    }

//END create*

//BEGIN setDeclData
    template<CXCursorKind CK>
    void setDeclData(CXCursor cursor, Declaration *decl, bool setComment = true) const;

    template<CXCursorKind CK>
    void setDeclData(CXCursor cursor, MacroDefinition* decl) const;

    template<CXCursorKind CK>
    void setDeclData(CXCursor cursor, ClassMemberDeclaration *decl) const;

    template<CXCursorKind CK, EnableIf<CursorKindTraits::isClassTemplate(CK)> = dummy>
    void setDeclData(CXCursor cursor, ClassDeclaration* decl) const;

    template<CXCursorKind CK, EnableIf<!CursorKindTraits::isClassTemplate(CK)> = dummy>
    void setDeclData(CXCursor cursor, ClassDeclaration* decl) const;

    template<CXCursorKind CK>
    void setDeclData(CXCursor cursor, AbstractFunctionDeclaration* decl) const;

    template<CXCursorKind CK>
    void setDeclData(CXCursor cursor, ClassFunctionDeclaration* decl) const;

    template<CXCursorKind CK>
    void setDeclData(CXCursor cursor, FunctionDeclaration *decl, bool setComment = true) const;

    template<CXCursorKind CK>
    void setDeclData(CXCursor cursor, FunctionDefinition *decl) const;

    template<CXCursorKind CK>
    void setDeclData(CXCursor cursor, NamespaceAliasDeclaration *decl) const;

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
            if (clang_CXXMethod_isConst(cursor)) {
                type->setModifiers(type->modifiers() | AbstractType::ConstModifier);
                decl->setAbstractType(type);
            }
        }
    }

    template<CXCursorKind CK>
    void setDeclInCtxtData(CXCursor cursor, FunctionDefinition *def)
    {
        setDeclInCtxtData<CK>(cursor, static_cast<FunctionDeclaration*>(def));

        const CXCursor canon = clang_getCanonicalCursor(cursor);
        if (auto decl = findDeclaration(canon)) {
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
    void setTypeSize(CXType type, AbstractType* kdevType) const;

    const CXFile m_file;
    const IncludeFileContexts &m_includes;

    DeclarationPointer findDeclaration(CXCursor cursor) const;
    void setIdTypeDecl(CXCursor typeCursor, IdentifiedType* idType) const;

    std::unordered_map<DUContext*, std::vector<CXCursor>> m_uses;
    /// At these location offsets (cf. @ref clang_getExpansionLocation) we encountered macro expansions
    QSet<unsigned int> m_macroExpansionLocations;
    mutable QHash<CXCursor, DeclarationPointer> m_cursorToDeclarationCache;
    CurrentContext *m_parentContext;

    const bool m_update;
};

//BEGIN setTypeModifiers
template<CXTypeKind TK>
void Visitor::setTypeModifiers(CXType type, AbstractType* kdevType) const
{
    quint64 modifiers = 0;
    if (clang_isConstQualifiedType(type)) {
        modifiers |= AbstractType::ConstModifier;
    }
    if (clang_isVolatileQualifiedType(type)) {
        modifiers |= AbstractType::VolatileModifier;
    }
#if CINDEX_VERSION_MINOR >= 60
    if (TK == CXType_Atomic) {
        modifiers |= AbstractType::AtomicModifier;
    }
#endif
    if (TK == CXType_Short || TK == CXType_UShort) {
        modifiers |= AbstractType::ShortModifier;
    }
    if (TK == CXType_Long || TK == CXType_LongDouble || TK == CXType_ULong) {
        modifiers |= AbstractType::LongModifier;
    }
    if (TK == CXType_LongLong || TK == CXType_ULongLong) {
        modifiers |= AbstractType::LongLongModifier;
    }
    if (TK == CXType_SChar) {
        modifiers |= AbstractType::SignedModifier;
    }
    if (TK == CXType_UChar || TK == CXType_UInt || TK == CXType_UShort
        || TK == CXType_UInt128 || TK == CXType_ULong || TK == CXType_ULongLong)
    {
        modifiers |= AbstractType::UnsignedModifier;
    }
    kdevType->setModifiers(modifiers);
}
//END setTypeModifiers

void Visitor::setTypeSize(CXType type, AbstractType* kdevType) const
{
    if (CINDEX_VERSION_MINOR < 59) {
        // clang_Type_getSizeOf is unstable, see https://bugs.kde.org/show_bug.cgi?id=431391
        return;
    }


    if (kdevType->whichType() == AbstractType::TypeFunction)
        return;

    type = clang_getCanonicalType(type);
    if (type.kind == CXType_Elaborated)
        return;

    auto sizeOf = clang_Type_getSizeOf(type);
    if (sizeOf >= 0) {
        kdevType->setSizeOf(sizeOf);

        // clang_Type_getAlignOf sometimes crashes, so better guard
        // it and only call it when we got a size
        auto alignOf = clang_Type_getAlignOf(type);
        if (alignOf >= 0)
            kdevType->setAlignOf(alignOf);
    }
}

//BEGIN dispatchCursor

template<CXCursorKind CK, Decision IsInClass,
         EnableIf<IsInClass == Decision::Maybe>>
CXChildVisitResult Visitor::dispatchCursor(CXCursor cursor, CXCursor parent)
{
    const bool decision = CursorKindTraits::isClass(clang_getCursorKind(parent));
    return decision ?
        dispatchCursor<CK, Decision::True, CursorKindTraits::isDefinition(CK)>(cursor, parent) :
        dispatchCursor<CK, Decision::False, CursorKindTraits::isDefinition(CK)>(cursor, parent);
}

template<CXCursorKind CK, Decision IsInClass, Decision IsDefinition,
         EnableIf<IsDefinition == Decision::Maybe && IsInClass != Decision::Maybe>>
CXChildVisitResult Visitor::dispatchCursor(CXCursor cursor, CXCursor parent)
{
    IF_DEBUG(clangDebug() << "IsInClass:" << IsInClass << "- isDefinition:" << IsDefinition;)

    const bool isDefinition = clang_isCursorDefinition(cursor);
    return isDefinition ?
        dispatchCursor<CK, IsInClass, Decision::True>(cursor, parent) :
        dispatchCursor<CK, IsInClass, Decision::False>(cursor, parent);
}

template<CXCursorKind CK, Decision IsInClass, Decision IsDefinition,
         EnableIf<IsInClass != Decision::Maybe && IsDefinition != Decision::Maybe>>
CXChildVisitResult Visitor::dispatchCursor(CXCursor cursor, CXCursor parent)
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
    // always build a context for class templates and functions, otherwise we "leak"
    // the function/template parameter declarations into the surrounding context,
    // which can lead to interesting bugs, like https://bugs.kde.org/show_bug.cgi?id=368067
    constexpr bool hasContext = isDefinition || CursorKindTraits::isFunction(CK) || CursorKindTraits::isClassTemplate(CK);

    return buildDeclaration<CK, typename DeclType<CK, isDefinition, isClassMember>::Type, hasContext>(cursor);
}

//END dispatchCursor

//BEGIN setDeclData
template<CXCursorKind CK>
void Visitor::setDeclData(CXCursor cursor, Declaration *decl, bool setComment) const
{
    if (setComment)
#if CINDEX_VERSION_MINOR < 100 // FIXME https://bugs.llvm.org/show_bug.cgi?id=35333
        decl->setComment(KDevelop::formatComment(ClangString(clang_Cursor_getRawCommentText(cursor)).toByteArray()));
#else
        decl->setComment(makeComment(clang_Cursor_getParsedComment(cursor)));
#endif
    if (CursorKindTraits::isAliasType(CK)) {
        decl->setIsTypeAlias(true);
    }
    if (CK == CXCursor_Namespace)
        decl->setKind(Declaration::Namespace);
    if (CK == CXCursor_EnumDecl || CK == CXCursor_EnumConstantDecl || CursorKindTraits::isClass(CK) || CursorKindTraits::isAliasType(CK))
        decl->setKind(Declaration::Type);

    int isAlwaysDeprecated;
    clang_getCursorPlatformAvailability(cursor, &isAlwaysDeprecated, nullptr, nullptr, nullptr, nullptr, 0);
    decl->setDeprecated(isAlwaysDeprecated);
}

/// @return the position in @p contents where the macro identifier ends.
static int skipMacroIdentifier(QStringView contents)
{
    const auto isPartOfIdentifier = [](QChar c) {
        return c.isLetterOrNumber() || c == QLatin1Char('_');
    };

    int posAfterMacroId = 0;
    while (posAfterMacroId < contents.size()) {
        posAfterMacroId = std::find_if_not(contents.cbegin() + posAfterMacroId, contents.cend(), isPartOfIdentifier)
            - contents.cbegin();

        // Escaped newline characters can separate parts of a macro identifier or a macro identifier and '(' in a
        // function-like macro. And the escape character can be separated from the '\n' it escapes by any number of
        // whitespace characters other than '\n'. Furthermore, the same escaped-newline pattern can precede the macro
        // identifier. Simply skip such escape and whitespace characters as displaying them in a tooltip is not useful.
        if (posAfterMacroId == contents.size() || contents[posAfterMacroId] != QLatin1Char{'\\'}) {
            break; // no escape character => the macro identifier ends here
        }
        ++posAfterMacroId;

        bool foundNewLineCharacter = false;
        while (posAfterMacroId < contents.size() && contents[posAfterMacroId].isSpace()) {
            if (contents[posAfterMacroId++] == QLatin1Char{'\n'}) {
                foundNewLineCharacter = true;
                break;
            }
        }
        if (!foundNewLineCharacter) {
            // The escape character does not escape a newline character. The code probably does not compile. Go back to
            // the previous character to prevent the calling code from wrongly considering this macro function-like.
            --posAfterMacroId;
            break;
        }
    }

    return posAfterMacroId;
}

template<CXCursorKind CK>
void Visitor::setDeclData(CXCursor cursor, MacroDefinition* decl) const
{
    setDeclData<CK>(cursor, static_cast<Declaration*>(decl));

    if (m_update) {
        decl->clearParameters();
    }

    auto unit = clang_Cursor_getTranslationUnit(cursor);
    auto range = clang_getCursorExtent(cursor);

    // TODO: Quite lacking API in libclang here.
    // No way to find out if this macro is function-like or not
    // cf. https://clang.llvm.org/doxygen/classclang_1_1MacroInfo.html
    // And no way to get the actual definition text range
    // Should be quite easy to expose that in libclang, though
    // Let' still get some basic support for this and parse on our own, it's not that difficult

    // Macro definition strings get '\n' replaced with "<br/>", then are rendered as HTML, which ignores whitespace.
    // Newline characters are escaped in macro definition C++ code. ClangUtils::getRawContents() preserves the escape
    // characters: its return value contains "\\\n". ClangUtils::getRawContents() removes whitespace, including the
    // escaped newline characters, at the end of the definition string.
    // Trim definition string views before passing them to MacroDefinition::setDefinition() to facilitate testing.
    // This trimming never strips newline characters in practice (shouldn't have been a problem even if it did).
    const auto setDefinition = [decl](QStringView definition) {
        decl->setDefinition(IndexedString{definition.trimmed()});
    };

    const QString rawContentsString = ClangUtils::getRawContents(unit, range);
    // Use a QStringView contents, because it works as fast as or faster than a QString in the code below.
    const QStringView contents(rawContentsString);

    const int posAfterMacroId = skipMacroIdentifier(contents);

    if (posAfterMacroId == contents.size() || contents[posAfterMacroId] != QLatin1Char{'('}) {
        // '(', a space, a tab or '/' (a comment) usually follows a macro identifier.
        // Compilers consider a macro function-like only if '(' immediately follows its identifier.
        decl->setFunctionLike(false);
        setDefinition(contents.sliced(posAfterMacroId));
        return;
    }

    decl->setFunctionLike(true);

    // extract macro function parameters
    ParamIterator paramIt(u"()", contents, posAfterMacroId);
    while (paramIt) {
        decl->addParameter(IndexedString(*paramIt));
        ++paramIt;
    }

    const auto paramEndPosition = paramIt.position();
    if (paramEndPosition > 0 && contents[paramEndPosition - 1] == QLatin1Char{')'}) {
        setDefinition(contents.sliced(paramEndPosition));
    } else {
        // unlikely: invalid macro definition, insert the complete #define statement
        const QString definition = QLatin1String("#define ") + contents;
        setDefinition(definition);
    }
}

template<CXCursorKind CK>
void Visitor::setDeclData(CXCursor cursor, ClassMemberDeclaration *decl) const
{
    setDeclData<CK>(cursor, static_cast<Declaration*>(decl));
    //A CXCursor_VarDecl in a class is static (otherwise it'd be a CXCursor_FieldDecl)
    if (CK == CXCursor_VarDecl)
        decl->setStatic(true);
    decl->setAccessPolicy(CursorKindTraits::kdevAccessPolicy(clang_getCXXAccessSpecifier(cursor)));

#if CINDEX_VERSION_MINOR >= 32
    decl->setMutable(clang_CXXField_isMutable(cursor));
#endif

#if CINDEX_VERSION_MINOR >= 30
    auto offset = clang_Cursor_getOffsetOfField(cursor);
    if (offset >= 0) {
        decl->setBitOffsetOf(offset);
    }
#endif

#if CINDEX_VERSION_MINOR >= 16
    if (clang_Cursor_isBitField(cursor)) {
        const auto bitWidth = clang_getFieldDeclBitWidth(cursor);
        decl->setBitWidth(bitWidth == -1 ? ClassMemberDeclaration::ValueDependentBitWidth : bitWidth);
    } else {
        decl->setBitWidth(ClassMemberDeclaration::NotABitField);
    }
#endif

    if (clang_isCursorDefinition(cursor)) {
        decl->setDeclarationIsDefinition(true);
    }
}

template<CXCursorKind CK, EnableIf<CursorKindTraits::isClassTemplate(CK)>>
void Visitor::setDeclData(CXCursor cursor, ClassDeclaration* decl) const
{
    CXCursorKind kind = clang_getTemplateCursorKind(cursor);
    switch (kind) {
        case CXCursor_UnionDecl: setDeclData<CXCursor_UnionDecl>(cursor, decl); break;
        case CXCursor_StructDecl: setDeclData<CXCursor_StructDecl>(cursor, decl); break;
        case CXCursor_ClassDecl: setDeclData<CXCursor_ClassDecl>(cursor, decl); break;
        default: Q_ASSERT(false); break;
    }
}

template<CXCursorKind CK, EnableIf<!CursorKindTraits::isClassTemplate(CK)>>
void Visitor::setDeclData(CXCursor cursor, ClassDeclaration* decl) const
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
void Visitor::setDeclData(CXCursor cursor, AbstractFunctionDeclaration* decl) const
{
    if (m_update) {
        decl->clearDefaultParameters();
    }
    // No setDeclData<CK>(...) here: AbstractFunctionDeclaration is an interface
    // TODO: Can we get the default arguments directly from Clang?
    // also see http://clang-developers.42468.n3.nabble.com/Finding-default-value-for-function-argument-with-clang-c-API-td4036919.html
    const QVector<QString> defaultArgs = ClangUtils::getDefaultArguments(cursor, ClangUtils::MinimumSize);
    for (const QString& defaultArg : defaultArgs) {
        decl->addDefaultParameter(IndexedString(defaultArg));
    }
}

template<CXCursorKind CK>
void Visitor::setDeclData(CXCursor cursor, ClassFunctionDeclaration* decl) const
{
    setDeclData<CK>(cursor, static_cast<AbstractFunctionDeclaration*>(decl));
    setDeclData<CK>(cursor, static_cast<ClassMemberDeclaration*>(decl));
    decl->setIsAbstract(clang_CXXMethod_isPureVirtual(cursor));
    decl->setStatic(clang_CXXMethod_isStatic(cursor));
    decl->setVirtual(clang_CXXMethod_isVirtual(cursor));

    // TODO: Set flags in one go? (needs new API in kdevplatform)
    const auto attributes = ClangUtils::specialAttributes(cursor);
    decl->setIsSignal(attributes & FunctionSignalFlag);
    decl->setIsSlot(attributes & FunctionSlotFlag);
    decl->setIsFinal(attributes & FinalFunctionFlag);
}

template<CXCursorKind CK>
void Visitor::setDeclData(CXCursor cursor, FunctionDeclaration *decl, bool setComment) const
{
    setDeclData<CK>(cursor, static_cast<AbstractFunctionDeclaration*>(decl));
    setDeclData<CK>(cursor, static_cast<Declaration*>(decl), setComment);
}

template<CXCursorKind CK>
void Visitor::setDeclData(CXCursor cursor, FunctionDefinition *decl) const
{
    bool setComment = clang_equalCursors(clang_getCanonicalCursor(cursor), cursor);
    setDeclData<CK>(cursor, static_cast<FunctionDeclaration*>(decl), setComment);
}

template<CXCursorKind CK>
void Visitor::setDeclData(CXCursor cursor, NamespaceAliasDeclaration *decl) const
{
    setDeclData<CK>(cursor, static_cast<Declaration*>(decl));
    clang_visitChildren(cursor, [] (CXCursor cursor, CXCursor parent, CXClientData data) -> CXChildVisitResult {
        if (clang_getCursorKind(cursor) == CXCursor_NamespaceRef) {
            const auto id = QualifiedIdentifier(ClangString(clang_getCursorSpelling(cursor)).toString());
            reinterpret_cast<NamespaceAliasDeclaration*>(data)->setImportIdentifier(id);
            return CXChildVisit_Break;
        } else {
            return visitCursor(cursor, parent, data);
        }
    }, decl);
}
//END setDeclData

//BEGIN build*
template<CXCursorKind CK, class DeclType, bool hasContext>
CXChildVisitResult Visitor::buildDeclaration(CXCursor cursor)
{
    auto id = makeId(cursor);
    if (CK == CXCursor_UnexposedDecl && id.isEmpty()) {
        // skip unexposed declarations that have no identifier set
        // this is useful to skip e.g. friend declarations
        return CXChildVisit_Recurse;
    }
    IF_DEBUG(clangDebug() << "id:" << id << "- CK:" << CK << "- DeclType:" << typeid(DeclType).name() << "- hasContext:" << hasContext;)

    // Code path for class declarations that may be defined "out-of-line", e.g.
    // "SomeNameSpace::SomeClass {};"
    QScopedPointer<CurrentContext> helperContext;
    if (CursorKindTraits::isClass(CK) || CursorKindTraits::isFunction(CK)) {
        const auto lexicalParent = clang_getCursorLexicalParent(cursor);
        const auto semanticParent = clang_getCursorSemanticParent(cursor);
        const bool isOutOfLine = !clang_equalCursors(lexicalParent, semanticParent);
        if (isOutOfLine) {
            const QString scope = ClangUtils::getScope(cursor);
            auto context = createContext<CK, DUContext::Helper>(cursor, QualifiedIdentifier(scope));
            helperContext.reset(new CurrentContext(context, m_parentContext->keepAliveContexts));
        }
    }

    // if helperContext is null, this is a no-op
    PushValue<CurrentContext*> pushCurrent(m_parentContext, helperContext.isNull() ? m_parentContext : helperContext.data());

    if (hasContext) {
        auto context = createContext<CK, CursorKindTraits::contextType(CK)>(cursor, QualifiedIdentifier(id));
        createDeclaration<CK, DeclType>(cursor, id, context);
        CurrentContext newParent(context, m_parentContext->keepAliveContexts);
        PushValue<CurrentContext*> pushCurrent(m_parentContext, &newParent);
        clang_visitChildren(cursor, &visitCursor, this);
        return CXChildVisit_Continue;
    }
    createDeclaration<CK, DeclType>(cursor, id, nullptr);
    return CXChildVisit_Recurse;
}

CXChildVisitResult Visitor::buildParmDecl(CXCursor cursor)
{
    return buildDeclaration<CXCursor_ParmDecl, typename DeclType<CXCursor_ParmDecl, false, false>::Type, false>(cursor);
}

CXChildVisitResult Visitor::buildUse(CXCursor cursor)
{
    m_uses[m_parentContext->context].push_back(cursor);
    return cursor.kind == CXCursor_DeclRefExpr || cursor.kind == CXCursor_MemberRefExpr ?
        CXChildVisit_Recurse : CXChildVisit_Continue;
}

CXChildVisitResult Visitor::buildMacroExpansion(CXCursor cursor)
{
    buildUse(cursor);

    // cache that we encountered a macro expansion at this location
    unsigned int offset;
    clang_getSpellingLocation(clang_getCursorLocation(cursor), nullptr, nullptr, nullptr, &offset);
    m_macroExpansionLocations << offset;

    return CXChildVisit_Recurse;
}

template<CXCursorKind CK>
CXChildVisitResult Visitor::buildCompoundStatement(CXCursor cursor)
{
    if (CK == CXCursor_LambdaExpr || m_parentContext->context->type() == DUContext::Function)
    {
        auto context = createContext<CK, CK == CXCursor_LambdaExpr ? DUContext::Function : DUContext::Other>(cursor);
        CurrentContext newParent(context, m_parentContext->keepAliveContexts);
        PushValue<CurrentContext*> pushCurrent(m_parentContext, &newParent);
        clang_visitChildren(cursor, &visitCursor, this);
        return CXChildVisit_Continue;
    }
    return CXChildVisit_Recurse;
}

CXChildVisitResult Visitor::buildCXXBaseSpecifier(CXCursor cursor)
{
    auto currentContext = m_parentContext->context;

    bool virtualInherited = clang_isVirtualBase(cursor);
    Declaration::AccessPolicy access = CursorKindTraits::kdevAccessPolicy(clang_getCXXAccessSpecifier(cursor));

    auto classDeclCursor = clang_getCursorReferenced(cursor);
    auto decl = findDeclaration(classDeclCursor);
    if (!decl) {
        // this happens for templates with template-dependent base classes e.g. - dunno whether we can/should do more here
        clangDebug() << "failed to find declaration for base specifier:" << clang_getCursorDisplayName(cursor);
        return CXChildVisit_Recurse;
    }

    DUChainWriteLocker lock;
    contextImportDecl(currentContext, decl);
    auto classDecl = dynamic_cast<ClassDeclaration*>(currentContext->owner());
    Q_ASSERT(classDecl);

    classDecl->addBaseClass({decl->indexedType(), access, virtualInherited});
    return CXChildVisit_Recurse;
}

template<bool IsInClass>
CXChildVisitResult Visitor::buildTypeAliasTemplateDecl(CXCursor cursor)
{
    auto aliasDecl = findEmbeddedTypeAlias(cursor);
    // NOTE: using aliasDecl here averts having to add a workaround to makeId()
    auto id = makeId(aliasDecl);
    // create template context to prevent leaking child template params
    auto context = createContext<CXCursor_TypeAliasTemplateDecl, DUContext::Template>(cursor, QualifiedIdentifier(id));
    using DeclType = typename DeclType<CXCursor_TypeAliasDecl, false, IsInClass>::Type;
    createDeclaration<CXCursor_TypeAliasDecl, DeclType>(aliasDecl, id, context);
    CurrentContext newParent(context, m_parentContext->keepAliveContexts);
    PushValue<CurrentContext*> pushCurrent(m_parentContext, &newParent);
    clang_visitChildren(cursor, [] (CXCursor cursor, CXCursor parent, CXClientData data) {
        // NOTE: immediately recurse into embedded alias decl
        return clang_getCursorKind(cursor) == CXCursor_TypeAliasDecl ?
            CXChildVisit_Recurse : visitCursor(cursor, parent, data);
    }, this);
    return CXChildVisit_Continue;
}
//END build*

DeclarationPointer Visitor::findDeclaration(CXCursor cursor) const
{
    const auto it = m_cursorToDeclarationCache.constFind(cursor);
    if (it != m_cursorToDeclarationCache.constEnd()) {
        return *it;
    }

    // fallback, and cache result
    auto decl = ClangHelpers::findDeclaration(cursor, m_includes);

    m_cursorToDeclarationCache.insert(cursor, decl);
    return decl;
}

void Visitor::setIdTypeDecl(CXCursor typeCursor, IdentifiedType* idType) const
{
    DeclarationPointer decl = findDeclaration(typeCursor);
    DUChainReadLocker lock;
    if (decl) {
        idType->setDeclaration(decl.data());
    }
}

AbstractType *Visitor::makeType(CXType type, CXCursor parent)
{
#define UseKind(TypeKind) case TypeKind: return dispatchType<TypeKind>(type, parent)
    switch (type.kind) {
    UseKind(CXType_Void);
    UseKind(CXType_Bool);
    UseKind(CXType_Short);
    UseKind(CXType_UShort);
    UseKind(CXType_Int);
    UseKind(CXType_UInt);
    UseKind(CXType_Long);
    UseKind(CXType_ULong);
    UseKind(CXType_LongLong);
    UseKind(CXType_ULongLong);
    UseKind(CXType_Half);
    UseKind(CXType_Float);
    UseKind(CXType_LongDouble);
    UseKind(CXType_Double);
    UseKind(CXType_Char_U);
    UseKind(CXType_Char_S);
    UseKind(CXType_UChar);
    UseKind(CXType_SChar);
    UseKind(CXType_Char16);
    UseKind(CXType_Char32);
    UseKind(CXType_Pointer);
    UseKind(CXType_BlockPointer);
    UseKind(CXType_MemberPointer);
    UseKind(CXType_ObjCObjectPointer);
    UseKind(CXType_ConstantArray);
    UseKind(CXType_VariableArray);
    UseKind(CXType_IncompleteArray);
    UseKind(CXType_DependentSizedArray);
    UseKind(CXType_LValueReference);
    UseKind(CXType_RValueReference);
    UseKind(CXType_FunctionNoProto);
    UseKind(CXType_FunctionProto);
    UseKind(CXType_Record);
    UseKind(CXType_Enum);
    UseKind(CXType_Typedef);
    UseKind(CXType_Int128);
    UseKind(CXType_UInt128);
    UseKind(CXType_Vector);
    UseKind(CXType_ExtVector);
    UseKind(CXType_Unexposed);
    UseKind(CXType_WChar);
    UseKind(CXType_ObjCInterface);
    UseKind(CXType_ObjCId);
    UseKind(CXType_ObjCClass);
    UseKind(CXType_ObjCSel);
    UseKind(CXType_NullPtr);
#if CINDEX_VERSION_MINOR >= 32
    UseKind(CXType_Auto);
#endif
#if CINDEX_VERSION_MINOR >= 34
    UseKind(CXType_Elaborated);
#endif
#if CINDEX_VERSION_MINOR >= 38
    UseKind(CXType_Float128);
#endif
#if CINDEX_VERSION_MINOR >= 60
    UseKind(CXType_Atomic);
#endif
    UseKind(CXType_Complex);
    UseKind(CXType_OCLImage1dRO);
    UseKind(CXType_OCLImage1dArrayRO);
    UseKind(CXType_OCLImage1dBufferRO);
    UseKind(CXType_OCLImage2dRO);
    UseKind(CXType_OCLImage2dArrayRO);
    UseKind(CXType_OCLImage2dDepthRO);
    UseKind(CXType_OCLImage2dArrayDepthRO);
    UseKind(CXType_OCLImage2dMSAARO);
    UseKind(CXType_OCLImage2dArrayMSAARO);
    UseKind(CXType_OCLImage2dMSAADepthRO);
    UseKind(CXType_OCLImage2dArrayMSAADepthRO);
    UseKind(CXType_OCLImage3dRO);
    UseKind(CXType_OCLImage1dWO);
    UseKind(CXType_OCLImage1dArrayWO);
    UseKind(CXType_OCLImage1dBufferWO);
    UseKind(CXType_OCLImage2dWO);
    UseKind(CXType_OCLImage2dArrayWO);
    UseKind(CXType_OCLImage2dDepthWO);
    UseKind(CXType_OCLImage2dArrayDepthWO);
    UseKind(CXType_OCLImage2dMSAAWO);
    UseKind(CXType_OCLImage2dArrayMSAAWO);
    UseKind(CXType_OCLImage2dMSAADepthWO);
    UseKind(CXType_OCLImage2dArrayMSAADepthWO);
    UseKind(CXType_OCLImage3dWO);
    UseKind(CXType_OCLImage1dRW);
    UseKind(CXType_OCLImage1dArrayRW);
    UseKind(CXType_OCLImage1dBufferRW);
    UseKind(CXType_OCLImage2dRW);
    UseKind(CXType_OCLImage2dArrayRW);
    UseKind(CXType_OCLImage2dDepthRW);
    UseKind(CXType_OCLImage2dArrayDepthRW);
    UseKind(CXType_OCLImage2dMSAARW);
    UseKind(CXType_OCLImage2dArrayMSAARW);
    UseKind(CXType_OCLImage2dMSAADepthRW);
    UseKind(CXType_OCLImage2dArrayMSAADepthRW);
    UseKind(CXType_OCLImage3dRW);
    UseKind(CXType_OCLSampler);
    UseKind(CXType_OCLEvent);
    UseKind(CXType_OCLQueue);
    UseKind(CXType_OCLReserveID);
    case CXType_Invalid:
        return nullptr;
    default:
        qCWarning(KDEV_CLANG) << "Unhandled type:" << type.kind << clang_getTypeSpelling(type);
        return nullptr;
    }
#undef UseKind
}

RangeInRevision rangeInRevisionForUse(CXCursor cursor, CXCursorKind referencedCursorKind, CXSourceRange useRange, const QSet<unsigned int>& macroExpansionLocations)
{
    auto range = ClangRange(useRange).toRangeInRevision();

    //TODO: Fix in clang, happens for operator<<, operator<, probably more
    if (clang_Range_isNull(useRange)) {
        useRange = clang_getCursorExtent(cursor);
        range = ClangRange(useRange).toRangeInRevision();
    }

    if (referencedCursorKind == CXCursor_ConversionFunction) {
        range.end = range.start;
        range.start.column--;
    }

    // For uses inside macro expansions, `useRange` is the whole macro use
    // from the macro identifier up to the closing parenthesis. In order not to interfere with
    // code navigation at the macro use location, we make it an empty range at its beginning.
    // also see JSON test 'macros.cpp'
    if (clang_getCursorKind(cursor) != CXCursor_MacroExpansion) {
        unsigned int expansionLocOffset;
        const auto useRangeStart = clang_getRangeStart(useRange);
        clang_getExpansionLocation(useRangeStart, nullptr, nullptr, nullptr, &expansionLocOffset);
        if (macroExpansionLocations.contains(expansionLocOffset)) {
            unsigned int fileLocOffset;
            clang_getFileLocation(useRangeStart, nullptr, nullptr, nullptr, &fileLocOffset);
            if (fileLocOffset == expansionLocOffset) {
                range.end = range.start;
            }
        }
    } else {
        // Workaround for wrong use range returned by clang for macro expansions
        const auto contents = ClangUtils::getRawContents(clang_Cursor_getTranslationUnit(cursor), useRange);
        const int firstOpeningParen = contents.indexOf(QLatin1Char('('));
        if (firstOpeningParen != -1) {
            range.end.column = range.start.column + firstOpeningParen;
            range.end.line = range.start.line;
        }
    }

    return range;
}

Visitor::Visitor(CXTranslationUnit tu, CXFile file,
                 const IncludeFileContexts& includes, const bool update)
    : m_file(file)
    , m_includes(includes)
    , m_parentContext(nullptr)
    , m_update(update)
{
    CXCursor tuCursor = clang_getTranslationUnitCursor(tu);
    auto top = includes[file];

    // when updating, this contains child contexts that should be kept alive
    // even when they are not part of the AST anymore
    // this is required for some assistants, such as the signature assistant
    QSet<DUContext*> keepAliveContexts;
    {
        DUChainReadLocker lock;
        const auto problems = top->problems();
        for (const auto& problem : problems) {
            const auto& desc = problem->description();
            if (desc.startsWith(QLatin1String("Return type of out-of-line definition of '"))
                && desc.endsWith(QLatin1String("' differs from that in the declaration"))) {
                auto ctx = top->findContextAt(problem->range().start);
                // keep the context and its parents alive
                // this also keeps declarations in this context alive
                while (ctx) {
                    keepAliveContexts << ctx;
                    ctx = ctx->parentContext();
                }
            }
        }
    }

    CurrentContext parent(top, keepAliveContexts);
    m_parentContext = &parent;
    clang_visitChildren(tuCursor, &visitCursor, this);

    if (m_update) {
        DUChainWriteLocker lock;
        top->deleteUsesRecursively();
    }
    for (const auto &contextUses : m_uses) {
        for (const auto &cursor : contextUses.second) {
            auto referenced = referencedCursor(cursor);
            if (clang_Cursor_isNull(referenced)) {
                continue;
            }
            // first, try the canonical referenced cursor
            // this is important to get the correct function declaration e.g.
            auto canonicalReferenced = clang_getCanonicalCursor(referenced);
            auto used = findDeclaration(canonicalReferenced);

            if (!used) {
                // if the above failed, try the non-canonicalized version as a fallback
                // this is required for friend declarations that occur before
                // the real declaration. there, the canonical cursor points to
                // the friend declaration which is not what we are looking for
                used = findDeclaration(referenced);
            }

            if (!used) { // as a last resort, try to resolve the forward declaration
                DUChainReadLocker lock;
                DeclarationPointer decl = ClangHelpers::findForwardDeclaration(clang_getCursorType(referenced), contextUses.first, referenced);
                used = decl;
                if (!used) {
                    continue;
                }
            }

#if CINDEX_VERSION_MINOR >= 29
            if (clang_Cursor_getNumTemplateArguments(referenced) >= 0) {
                // Ideally, we don't need this, but for function templates clang_getCanonicalCursor returns a function definition
                // See also the testUsesCreatedForDeclarations test
                DUChainReadLocker lock;
                used = DUChainUtils::declarationForDefinition(used.data());
            }
#endif

            const auto useRange = clang_getCursorReferenceNameRange(cursor, 0, 0);
            const auto range = rangeInRevisionForUse(cursor, referenced.kind, useRange, m_macroExpansionLocations);

            DUChainWriteLocker lock;
            auto usedIndex = top->indexForUsedDeclaration(used.data());
            contextUses.first->createUse(usedIndex, range);
        }
    }
}

//END Visitor

CXChildVisitResult visitCursor(CXCursor cursor, CXCursor parent, CXClientData data)
{
    auto *visitor = static_cast<Visitor*>(data);

    const auto kind = clang_getCursorKind(cursor);

    auto location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getFileLocation(location, &file, nullptr, nullptr, nullptr);
    if (!ClangUtils::isFileEqual(file, visitor->m_file)) {
        // don't skip MemberRefExpr with invalid location, see also:
        // http://lists.cs.uiuc.edu/pipermail/cfe-dev/2015-May/043114.html
        const auto invalidMemberRefExpr = !file && kind == CXCursor_MemberRefExpr;
        // also don't skip unexposed declarations, which may e.g. be an `extern "C"` directive
        const auto unexposedDecl = file && kind == CXCursor_UnexposedDecl;
        // newer clang properly reports LinkageSpec instead of UnexposedDecl
        const auto linkageSpec = file && kind == CXCursor_LinkageSpec;
        if (!invalidMemberRefExpr && !unexposedDecl && !linkageSpec) {
            return CXChildVisit_Continue;
        }
    }

#define UseCursorKind(CursorKind, ...) case CursorKind: return visitor->dispatchCursor<CursorKind>(__VA_ARGS__);
    switch (kind)
    {
    UseCursorKind(CXCursor_UnexposedDecl, cursor, parent);
    UseCursorKind(CXCursor_StructDecl, cursor, parent);
    UseCursorKind(CXCursor_UnionDecl, cursor, parent);
    UseCursorKind(CXCursor_ClassDecl, cursor, parent);
    UseCursorKind(CXCursor_EnumDecl, cursor, parent);
    UseCursorKind(CXCursor_FieldDecl, cursor, parent);
    UseCursorKind(CXCursor_EnumConstantDecl, cursor, parent);
    UseCursorKind(CXCursor_FunctionDecl, cursor, parent);
    UseCursorKind(CXCursor_VarDecl, cursor, parent);
    UseCursorKind(CXCursor_TypeAliasDecl, cursor, parent);
    UseCursorKind(CXCursor_TypedefDecl, cursor, parent);
    UseCursorKind(CXCursor_CXXMethod, cursor, parent);
    UseCursorKind(CXCursor_Namespace, cursor, parent);
    UseCursorKind(CXCursor_NamespaceAlias, cursor, parent);
    UseCursorKind(CXCursor_Constructor, cursor, parent);
    UseCursorKind(CXCursor_Destructor, cursor, parent);
    UseCursorKind(CXCursor_ConversionFunction, cursor, parent);
    UseCursorKind(CXCursor_TemplateTypeParameter, cursor, parent);
    UseCursorKind(CXCursor_NonTypeTemplateParameter, cursor, parent);
    UseCursorKind(CXCursor_TemplateTemplateParameter, cursor, parent);
    UseCursorKind(CXCursor_FunctionTemplate, cursor, parent);
    UseCursorKind(CXCursor_ClassTemplate, cursor, parent);
    UseCursorKind(CXCursor_ClassTemplatePartialSpecialization, cursor, parent);
    UseCursorKind(CXCursor_ObjCInterfaceDecl, cursor, parent);
    UseCursorKind(CXCursor_ObjCCategoryDecl, cursor, parent);
    UseCursorKind(CXCursor_ObjCProtocolDecl, cursor, parent);
    UseCursorKind(CXCursor_ObjCPropertyDecl, cursor, parent);
    UseCursorKind(CXCursor_ObjCIvarDecl, cursor, parent);
    UseCursorKind(CXCursor_ObjCInstanceMethodDecl, cursor, parent);
    UseCursorKind(CXCursor_ObjCClassMethodDecl, cursor, parent);
    UseCursorKind(CXCursor_ObjCImplementationDecl, cursor, parent);
    UseCursorKind(CXCursor_ObjCCategoryImplDecl, cursor, parent);
    UseCursorKind(CXCursor_MacroDefinition, cursor, parent);
    UseCursorKind(CXCursor_LabelStmt, cursor, parent);
    case CXCursor_TypeRef:
    case CXCursor_TemplateRef:
    case CXCursor_NamespaceRef:
    case CXCursor_MemberRef:
    case CXCursor_LabelRef:
    case CXCursor_OverloadedDeclRef:
    case CXCursor_VariableRef:
    case CXCursor_DeclRefExpr:
    case CXCursor_MemberRefExpr:
    case CXCursor_ObjCClassRef:
        return visitor->buildUse(cursor);
    case CXCursor_MacroExpansion:
        return visitor->buildMacroExpansion(cursor);
    case CXCursor_CompoundStmt:
        return visitor->buildCompoundStatement<CXCursor_CompoundStmt>(cursor);
    case CXCursor_LambdaExpr:
        return visitor->buildCompoundStatement<CXCursor_LambdaExpr>(cursor);
    case CXCursor_CXXBaseSpecifier:
        return visitor->buildCXXBaseSpecifier(cursor);
    case CXCursor_ParmDecl:
        return visitor->buildParmDecl(cursor);
    // TODO: fix upstream and then just adapt this to UseCursorKind()
    case CXCursor_TypeAliasTemplateDecl:
        return visitor->dispatchTypeAliasTemplate(cursor, parent);
    default:
        return CXChildVisit_Recurse;
    }
}

}

namespace Builder {

void visit(CXTranslationUnit tu, CXFile file, const IncludeFileContexts& includes, const bool update)
{
    Visitor visitor(tu, file, includes, update);
}

}
