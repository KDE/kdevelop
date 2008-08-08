/*
 * KDevelop xUnit testing support
 *
 * Copyright 2008 Manuel Breugelmans
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
#include "languages/cpp/cppduchain/classdeclaration.h"
#include "languages/cpp/cppduchain/declarationbuilder.h"
#include "languages/cpp/cppduchain/environmentmanager.h"
#include "languages/cpp/cppduchain/usebuilder.h"
#include "languages/cpp/parser/parser.h"
#include "languages/cpp/parser/parsesession.h"
#include "languages/cpp/parser/rpp/preprocessor.h"
#include "languages/cpp/parser/rpp/pp-engine.h"

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

Cpp::ClassDeclaration* DeclarationFactory::classFromText(const QByteArray& text)
{
    if (m_lock->locked()) m_lock->unlock();
    TopDUContext* top = parseText(text);
    Q_ASSERT(top);
    m_tops << top;
    if (!m_lock->locked()) m_lock->lock();
    Q_ASSERT(top->localDeclarations().count() !=0 );
    Declaration* dcl = top->localDeclarations().first();
    Q_ASSERT(dcl);
    ClassDeclaration* clazz = dynamic_cast<ClassDeclaration*>(dcl);
    Q_ASSERT(clazz);
    if (m_lock->locked()) m_lock->unlock();
    return clazz;
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
        pp.processFile("anonymous", rpp::pp::Data, text));

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
