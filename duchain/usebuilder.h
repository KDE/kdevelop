#include "clangtypes.h"

#include <language/duchain/duchainlock.h>

namespace UseBuilder {

    using namespace KDevelop;

    inline void createUseCommon(CXCursor cursor, DUContext *parentContext)
    {
        auto referenced = clang_getCursorReferenced(cursor);
        auto refLoc = clang_getCursorLocation(referenced);
        CXFile file;
        clang_getFileLocation(refLoc, &file, nullptr, nullptr, nullptr);
        auto url = IndexedString(ClangString(clang_getFileName(file)));
        auto refCursor = CursorInRevision(ClangLocation(refLoc));

        //TODO: handle uses of declarations in other topContexts
        DUChainWriteLocker lock;
        TopDUContext *top = parentContext->topContext();
        if (DUContext *local = top->findContextAt(refCursor)) {
            if (Declaration *used = local->findDeclarationAt(refCursor)) {
                auto usedIndex = top->indexForUsedDeclaration(used);
                auto useRange = ClangRange(clang_getCursorReferenceNameRange(cursor, CXNameRange_WantSinglePiece, 0));
                parentContext->createUse(usedIndex, useRange.toRangeInRevision());
            }
        }
    }

    template<CXCursorKind kind> CXChildVisitResult build(CXCursor, DUContext*) { Q_ASSERT(false); return nullptr; }
    #define AddUseBuilder(CursorKind, Ret)\
    template<> CXChildVisitResult build<CursorKind>(CXCursor cursor, DUContext *parentContext)\
    { createUseCommon(cursor, parentContext); return Ret; }

    AddUseBuilder(CXCursor_TypeRef, CXChildVisit_Continue)
    AddUseBuilder(CXCursor_CXXBaseSpecifier, CXChildVisit_Continue)
    AddUseBuilder(CXCursor_TemplateRef, CXChildVisit_Continue)
    AddUseBuilder(CXCursor_NamespaceRef, CXChildVisit_Continue)
    AddUseBuilder(CXCursor_MemberRef, CXChildVisit_Continue)
    AddUseBuilder(CXCursor_LabelRef, CXChildVisit_Continue)
    AddUseBuilder(CXCursor_OverloadedDeclRef, CXChildVisit_Continue)
    AddUseBuilder(CXCursor_VariableRef, CXChildVisit_Continue)
    AddUseBuilder(CXCursor_DeclRefExpr, CXChildVisit_Recurse)
    AddUseBuilder(CXCursor_MemberRefExpr, CXChildVisit_Recurse)
}
