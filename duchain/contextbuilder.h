#include "clangtypes.h"

#include <language/duchain/duchainlock.h>

extern CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData d);

namespace ContextBuilder {

    using namespace KDevelop;

    template<DUContext::ContextType type>
    DUContext *createContextCommon(CXCursor cursor, DUContext *parentContext)
    {
        auto range = ClangRange{clang_getCursorExtent(cursor)};
        auto context = new DUContext(range.toRangeInRevision(), parentContext);
        DUChainWriteLocker lock; //TODO: (..type..) constructor for DUContext?
        context->setType(type);
        clang_visitChildren(cursor, &::visit, context);
        return context;
    }

    template<DUContext::ContextType type>
    DUContext *createContextCommon(CXCursor cursor, Identifier id, DUContext *parentContext)
    {
        auto range = ClangRange{clang_getCursorExtent(cursor)};
        auto context = new DUContext(range.toRangeInRevision(), parentContext);
        DUChainWriteLocker lock; //TODO: (..type, id..) constructor for DUContext?
        context->setType(type);
        context->setLocalScopeIdentifier(parentContext->localScopeIdentifier() + id);
        clang_visitChildren(cursor, &::visit, context);
        return context;
    }

    template<CXCursorKind kind> DUContext *build(CXCursor, DUContext *) { Q_ASSERT(false); return nullptr; }
    #define AddContextBuilder(CursorKind, ContextType)\
    template<> DUContext *build<CursorKind>(CXCursor cursor, DUContext *parentContext)\
    { return createContextCommon<ContextType>(cursor, parentContext); }

    template<CXCursorKind kind> DUContext *build(CXCursor, Identifier, DUContext*) { Q_ASSERT(false); return nullptr; }
    #define AddIdContextBuilder(CursorKind, ContextType)\
    template<> DUContext *build<CursorKind>(CXCursor cursor, Identifier id, DUContext *parentContext)\
    { return createContextCommon<ContextType>(cursor, id, parentContext); }

    AddIdContextBuilder(CXCursor_StructDecl, DUContext::Class);
    AddIdContextBuilder(CXCursor_UnionDecl, DUContext::Class);
    AddIdContextBuilder(CXCursor_ClassDecl, DUContext::Class);
    AddIdContextBuilder(CXCursor_EnumDecl, DUContext::Enum);
    AddContextBuilder(CXCursor_FunctionDecl, DUContext::Other);
    AddContextBuilder(CXCursor_CXXMethod, DUContext::Other);
    AddIdContextBuilder(CXCursor_Namespace, DUContext::Namespace);
    AddContextBuilder(CXCursor_Constructor, DUContext::Other);
    AddContextBuilder(CXCursor_Destructor, DUContext::Other);
    AddContextBuilder(CXCursor_ConversionFunction, DUContext::Other);
    AddContextBuilder(CXCursor_FunctionTemplate, DUContext::Other);
    AddIdContextBuilder(CXCursor_ClassTemplate, DUContext::Class);
    AddIdContextBuilder(CXCursor_ClassTemplatePartialSpecialization, DUContext::Class);

}
