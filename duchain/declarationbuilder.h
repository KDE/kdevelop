#include "clangtypes.h"

#include <language/duchain/identifier.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

namespace DeclarationBuilder {

    using namespace KDevelop;

    inline QByteArray buildComment(CXComment comment)
    {
        auto kind = clang_Comment_getKind(comment);
        if (kind == CXComment_Text)
            return QByteArray(ClangString(clang_TextComment_getText(comment)));

        QByteArray text;
        int numChildren = clang_Comment_getNumChildren(comment);
        for (int i = 0; i < numChildren; ++i)
            text += buildComment(clang_Comment_getChild(comment, i));
        return text;
    }

    template<class T> T *createDeclarationCommon(CXCursor cursor, Identifier id)
    {
        auto range = ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toRangeInRevision();
        auto comment = buildComment(clang_Cursor_getParsedComment(cursor));
        auto decl = new T(range, nullptr);
        decl->setComment(comment);
        decl->setIdentifier(id);
        return decl;
    }

    template<class T> T *createDecl(CXCursor cursor, Identifier id, DUContext *parentContext)
    {
        auto decl = createDeclarationCommon<T>(cursor, id);
        DUChainWriteLocker lock;
        decl->setContext(parentContext);
        return decl;
    }

    template<class T> T *createCtxtDecl(CXCursor cursor, Identifier id, DUContext *internalContext, DUContext *parentContext)
    {
        auto decl = createDeclarationCommon<T>(cursor, id);
        DUChainWriteLocker lock;
        decl->setContext(parentContext);
        decl->setInternalContext(internalContext);
        return decl;
    }

    template<CXCursorKind> Declaration *build(CXCursor, Identifier, DUContext*) { Q_ASSERT(false); return nullptr; }
    #define AddDeclarationBuilder(CK, DT) \
    template<> Declaration *build<CK>(CXCursor c, Identifier i, DUContext *p)\
    { return createDecl<DT>(c, i, p); }

    template<CXCursorKind> Declaration *build(CXCursor, Identifier, DUContext*, DUContext *) { Q_ASSERT(false); return nullptr; };
    #define AddCtxtDeclBuilder(CK, DT)\
    template<> Declaration *build<CK>(CXCursor c, Identifier id, DUContext *ic, DUContext *p)\
    { return createCtxtDecl<DT>(c, id, ic, p); }

    #define AddBothBuilders(CursorKind, DeclType)\
    AddDeclarationBuilder(CursorKind, DeclType)\
    AddCtxtDeclBuilder(CursorKind, DeclType)

    AddDeclarationBuilder(CXCursor_UnexposedDecl, Declaration);
    AddBothBuilders(CXCursor_StructDecl, Declaration);
    AddBothBuilders(CXCursor_UnionDecl, Declaration);
    AddBothBuilders(CXCursor_ClassDecl, Declaration);
    AddBothBuilders(CXCursor_EnumDecl, Declaration);
    AddDeclarationBuilder(CXCursor_FieldDecl, Declaration);
    AddDeclarationBuilder(CXCursor_EnumConstantDecl, Declaration);
    AddBothBuilders(CXCursor_FunctionDecl, Declaration);
    AddDeclarationBuilder(CXCursor_VarDecl, Declaration);
    AddDeclarationBuilder(CXCursor_ParmDecl, Declaration);
    AddDeclarationBuilder(CXCursor_TypedefDecl, Declaration);
    AddBothBuilders(CXCursor_CXXMethod, Declaration);
    AddCtxtDeclBuilder(CXCursor_Namespace, Declaration);
    AddBothBuilders(CXCursor_Constructor, Declaration);
    AddBothBuilders(CXCursor_Destructor, Declaration);
    AddBothBuilders(CXCursor_ConversionFunction, Declaration);
    AddDeclarationBuilder(CXCursor_TemplateTypeParameter, Declaration);
    AddDeclarationBuilder(CXCursor_NonTypeTemplateParameter, Declaration);
    AddDeclarationBuilder(CXCursor_TemplateTemplateParameter, Declaration);
    AddBothBuilders(CXCursor_FunctionTemplate, Declaration);
    AddBothBuilders(CXCursor_ClassTemplate, Declaration);
    AddBothBuilders(CXCursor_ClassTemplatePartialSpecialization, Declaration);
    AddDeclarationBuilder(CXCursor_NamespaceAlias, Declaration);
    AddDeclarationBuilder(CXCursor_UsingDirective, Declaration);
    AddDeclarationBuilder(CXCursor_UsingDeclaration, Declaration);
    AddDeclarationBuilder(CXCursor_TypeAliasDecl, Declaration);
}