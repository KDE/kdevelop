#include "declarationbuilder.h"

#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/types/constantintegraltype.h>

#include "parsesession.h"
#include "clangduchainhelpers.h"
#include <clang/AST/ASTContext.h>

using namespace KDevelop;

DeclarationBuilder::DeclarationBuilder(ParseSession* session)
: DeclarationBuilderBase()
{
    m_session = session;
}

KDevelop::ReferencedTopDUContext DeclarationBuilder::build(const KDevelop::IndexedString& url, AST* node, KDevelop::ReferencedTopDUContext updateContext)
{
    ///TODO: cleanup
    Q_ASSERT(m_session->url() == url);
    return DeclarationBuilderBase::build(url, node, updateContext);
}

bool DeclarationBuilder::TraverseDecl(clang::Decl* decl)
{
    //qWarning() << decl->getASTContext().getSourceManager().getFileLoc(decl->getLocation()).printToString(decl->getASTContext().getSourceManager()).c_str();
    //if (clang::NamedDecl *named = dynamic_cast<clang::NamedDecl*>(decl))
    //    qWarning() << "visiting decl named" << declQualifiedIdentifier(named).toString() << decl->getDeclKindName();
    //else
    //    qWarning() << "visiting unnamed decl of kind" << decl->getDeclKindName();
    //TODO: set isDefinition flag
    //if (m_session->sourceManager().getFileID(decl->getLocation()) != m_session->fileID())
    //    return true;

    if (clang::NamedDecl *named = dynamic_cast<clang::NamedDecl*>(decl))
    {
        qDebug() << "visiting named decl";
        NameAST ast(named, m_session->preprocessor());
        switch(decl->getKind())
        {
        case clang::Decl::CXXRecord:
        {
            m_declToDeclaration[decl] = openDeclaration<KDevelop::ClassDeclaration>(&ast, 0);
            break;
        }
        case clang::Decl::Function:
        {
            m_declToDeclaration[decl] = openDeclaration<KDevelop::FunctionDeclaration>(&ast, 0);
            break;
        }
        case clang::Decl::Var:
        default:
        {
            m_declToDeclaration[decl] = openDeclaration<KDevelop::Declaration>(&ast, 0);
            break;
        }
        }
    }
    return ContextBuilder::TraverseDecl(decl);
}

bool DeclarationBuilder::VisitDeclRefExpr(clang::DeclRefExpr* refExpr)
{
    DUChainWriteLocker lock;
    KDevelop::Declaration *declaration = m_declToDeclaration.value(refExpr->getDecl(), 0);
    Q_ASSERT(declaration); //Clang doesn't create refs for nonexistant decls... right?
    //FIXME: In a class context, perhaps the decl hasn't been visited yet... see what happens
    KDevelop::RangeInRevision range = declRefRange(refExpr);
    currentContext()->createUse(declaration->topContext()->indexForUsedDeclaration(declaration), range);
    return true;
}

bool DeclarationBuilder::VisitTypeLoc(clang::TypeLoc TL)
{
    DUChainWriteLocker lock;
    TypeBuilder::VisitTypeLoc(TL);
    qWarning() << "visiting type" << TL.getType().getAsString().c_str();
    if (TL.getType().isCanonical() && TL.getTypePtr()->isClassType())
    {
        KDevelop::Declaration *declaration = m_declToDeclaration.value(TL.getType()->getAsCXXRecordDecl(), 0);
        Q_ASSERT(declaration);

        StructureType::Ptr structType(new StructureType());
        structType->setDeclaration(declaration);
        currentDeclaration<Declaration>()->setAbstractType(structType.cast<AbstractType>());
        qDebug() << "assigned to" << currentDeclaration<Declaration>()->toString();
        
        KDevelop::RangeInRevision range = typeRange(TL, m_session->sourceManager());
        currentContext()->createUse(declaration->topContext()->indexForUsedDeclaration(declaration), range);
    }
    else if (TL.getType()->isScalarType())
    {
        //Move this out to another function
        IntegralType::Ptr integralType(new IntegralType());
        switch(TL.getType()->getScalarTypeKind())
        {
        case clang::Type::STK_CPointer:
        case clang::Type::STK_BlockPointer:
        case clang::Type::STK_ObjCObjectPointer:
        case clang::Type::STK_MemberPointer:
            break; //Non integrals
        case clang::Type::STK_Bool:
            integralType->setDataType(KDevelop::IntegralType::TypeBoolean);
            break;
        case clang::Type::STK_Integral: //get more detail
            integralType->setDataType(KDevelop::IntegralType::TypeInt);
            break;
        case clang::Type::STK_Floating: //float or double?
            integralType->setDataType(KDevelop::IntegralType::TypeFloat);
            break;
        case clang::Type::STK_IntegralComplex:
            //WTF?
        case clang::Type::STK_FloatingComplex:
            //WTF?
        break;
        }
        currentDeclaration<Declaration>()->setAbstractType(integralType.cast<AbstractType>());
    }
    return true;
}
