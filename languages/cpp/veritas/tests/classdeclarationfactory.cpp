/*
* KDevelop xUnit testing support
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "classdeclarationfactory.h"

// kdevplatform
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/duchainlock.h>

// kdevelop
#include <language/duchain/classdeclaration.h>
#include "../cppduchain/declarationbuilder.h"
#include "../cppduchain/environmentmanager.h"
#include "../cppduchain/usebuilder.h"
#include "../parser/parser.h"
#include "../parser/parsesession.h"
#include "../parser/rpp/preprocessor.h"
#include "../parser/rpp/pp-engine.h"

using Veritas::Test::DeclarationFactory;
using namespace KDevelop;
using namespace Cpp;

int DeclarationFactory::m_count = 0;

DeclarationFactory::DeclarationFactory()
{
    m_lock = new DUChainWriteLocker(DUChain::lock());
    m_lock->unlock();
}

DeclarationFactory::~DeclarationFactory()
{
    if (m_lock && m_lock->locked()) {
        m_lock->unlock();
        delete m_lock;
    }
}

namespace
{
ClassDeclaration* classDeclarationIn(DUContext* ctx)
{
    Q_ASSERT(ctx->localDeclarations().count() !=0 );
    Declaration* dcl = ctx->localDeclarations().first();
    Q_ASSERT(dcl);
    ClassDeclaration* clazz = dynamic_cast<ClassDeclaration*>(dcl);
    Q_ASSERT(clazz);
    return clazz;
}
}

ClassDeclaration* DeclarationFactory::classFromText(const QByteArray& text)
{
    if (m_lock->locked()) m_lock->unlock();
    TopDUContext* top = parseText(text);
    Q_ASSERT(top);
    m_tops << top;
    if (!m_lock->locked()) m_lock->lock();
    DUContext* context = top;
    
    if (context->localDeclarations().count() == 0) {
        // see if we'r in a namespace. if so extract the class from there
        QVector<DUContext*> children = context->childContexts();
        Q_ASSERT(!children.isEmpty());
        Q_ASSERT(children[0]->type() == DUContext::Namespace);
        context = children[0];
        if (context->localDeclarations().count() == 0) {
            // still no classdeclaration, try one more
            QVector<DUContext*> children = context->childContexts();
            Q_ASSERT(!children.isEmpty());
            Q_ASSERT(children[0]->type() == DUContext::Namespace);
            context = children[0];
        }
    }
    
    ClassDeclaration* clazz = classDeclarationIn(context);
    if (m_lock->locked()) m_lock->unlock();
    return clazz;
}

QList<Declaration*> DeclarationFactory::variablesFromText(const QByteArray& text)
{
    if (m_lock->locked()) m_lock->unlock();
    TopDUContext* top = parseText(text);
    Q_ASSERT(top);
    m_tops << top;
    if (!m_lock->locked()) m_lock->lock();
    Q_ASSERT(top->localDeclarations().count() !=0 );
    QList<Declaration*> ret;
    foreach(Declaration* dcl, top->localDeclarations()) {
        if (dcl && (dcl->kind() == Declaration::Instance) && (dcl->isDefinition())) {
            ret << dcl;
        }
    }
    if (m_lock->locked()) m_lock->unlock();
    return ret;
}

QList<Declaration*> dcls(DUContext* ctx)
{
    QList<Declaration*> ret;
    foreach(Declaration* dcl, ctx->localDeclarations()) {
        if ((dcl->kind() == Declaration::Instance) && (dcl->isDefinition())) {
            DelayedType::Ptr type = dcl->type<DelayedType>();
            if (!type || type->kind() != DelayedType::Unresolved) { 
                kDebug() << "Is a resolved type, or delayed.";
            } else {
                ret << dcl;
            }
        }
    }
    foreach(DUContext* cctx, ctx->childContexts()) {
        ret += dcls(cctx);
    }
    return ret;
}

QList<Declaration*> DeclarationFactory::unresolvedVariablesFromText(const QByteArray& text)
{
    if (m_lock->locked()) m_lock->unlock();
    TopDUContext* top = parseText(text);
    Q_ASSERT(top);
    m_tops << top;
    if (!m_lock->locked()) m_lock->lock();
    Q_ASSERT(top->localDeclarations().count() !=0 );
    QList<Declaration*> ret = dcls(top); 
    if (m_lock->locked()) m_lock->unlock();
    return ret;
}

void DeclarationFactory::release()
{
    foreach(TopDUContext* top, m_tops) {
        if (top) {
            TopDUContextPointer tp(top);
            DUChain::self()->removeDocumentChain(static_cast<TopDUContext*>(top));
            Q_ASSERT(!tp);
        }
    }
    m_lock->unlock();
}

TopDUContext* DeclarationFactory::parseText(const QByteArray& text)
{
    ParseSession* session = new ParseSession();
    rpp::Preprocessor preprocessor;
    rpp::pp pp(&preprocessor);
    session->setContentsAndGenerateLocationTable(
        pp.processFile("anonymous", text));

    Parser parser(&m_control);
    TranslationUnitAST* ast = parser.parse(session);
    ast->session = session;

    IndexedString url(QString("/internal/%1").arg(m_count));
    DeclarationBuilder definitionBuilder(session);
    Cpp::EnvironmentFilePointer file(new Cpp::EnvironmentFile(url,0));
    TopDUContext* top = definitionBuilder.buildDeclarations(file, ast);

    UseBuilder useBuilder(session);
    useBuilder.buildUses(ast);

    delete session;
    return top;
}
