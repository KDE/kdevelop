/*
 * KDevelop xUnit test support
 *
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

#include "testswitch.h"

#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <QFileInfo>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

// kdevplatform includes
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/use.h>
#include <language/duchain/duchain.h>
#include <language/editor/simplecursor.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/persistentsymboltable.h>

// kdevelop includes
#include "../cppduchain/cppduchain.h"
#include "../cppduchain/environmentmanager.h"
#include "../cppduchain/cpppreprocessenvironment.h"

using Veritas::TestSwitch;
using namespace KDevelop;

namespace
{

IDocument* activeDocument()
{
    ICore* core = ICore::self();
    IDocumentController* dc = core->documentController();
    IDocument* doc = dc->activeDocument();
    if (!doc || !doc->textDocument() || !doc->textDocument()->activeView()) {
        return 0;
    } else {
        return doc;
    }
}

QStringList headerExtensions(QString("h,H,hh,hxx,hpp,tlh,h++").split(','));
bool isHeader(const KUrl& url)
{
    QFileInfo fi(url.path());
    return headerExtensions.contains(fi.suffix());
}

/*! check if ctx contains a qExec() call */
bool hasQExecInvocation(DUContext* ctx, TopDUContext* top)
{
    const Use* u = ctx->uses();
    Declaration* d;
    Identifier qExec("qExec");
    for(int i=0; i<ctx->usesCount(); i++, u++) {
        d = top->usedDeclarationForIndex(u->m_declarationIndex);
        if (!d || !d->isFunctionDeclaration()) continue;
        if (d->identifier() == qExec) return true;
    }
    return false;
}

/*! find a main() declaration */
DUContext* findMainContext(TopDUContext* ctx)
{
    Identifier main("main");
    QList<Declaration*> dcls = ctx->findDeclarations(main);
    Declaration* d = 0;
    foreach(Declaration* d2, dcls) {
        if (d2->isFunctionDeclaration() && d2->isDefinition()) {
            d = d2;
            break;
        } else {
        }
    }
    if(d)
      return d->internalContext();
    else
      return 0;
}

/*! return true if ctx contains a QTest main function (expanded QTEST_MAIN macro) */
bool hasQTestMainFunction(TopDUContext* ctx)
{
    DUContext* mainCtx = findMainContext(ctx);
    if (!mainCtx) return false; // no main() present
    return hasQExecInvocation(mainCtx, ctx);
}

Declaration* mostFrequentClass(const QMap<Declaration*, int>& classes)
{
    int maxCount = -1;
    Declaration* max = 0;
    QMapIterator<Declaration*, int> it(classes);
    while (it.hasNext()) {
        it.next();
        if (it.value() > maxCount) {
            maxCount = it.value();
            max = it.key();
        }
    }
    return max;
}

/*! Try to fetch the owner class declaration for a member function */
Declaration* isMemberFunctionDefinition(Declaration* decl)
{
    if (!decl->isFunctionDeclaration() || !decl->isDefinition()) {
        return 0; // not a member defintion
    }
    return Cpp::localClassFromCodeContext(decl->internalContext());
}

/*! Increment the class count for @param clazz */
void updateClassCount(Declaration* clazz, QMap<Declaration*,int>& classes)
{
    if (!classes.contains(clazz)) {
        classes[clazz] = 0;
    }
    classes[clazz] += 1;
    kDebug() << "Incremented member definition count for " << clazz->toString();
}

/*! Calculates the class with the highest number of member function
    implementations in context ctx */
Declaration* dominantClassInCpp(TopDUContext* ctx)
{
    kDebug() << "";
    QMap<Declaration*, int> classes;
    foreach(Declaration* d, ctx->localDeclarations()) {
        if (Declaration* const clazz = isMemberFunctionDefinition(d)) {
            updateClassCount(clazz, classes);
        }
    }
    return mostFrequentClass(classes);
}

/*! Increment the class count for the owner class of context @param ctx */
void updateClassMemberCountFor(DUContext* ctx, QMap<Declaration*,int>& classes)
{
    Declaration* clazz = ctx->owner();
    if (!classes.contains(clazz)) classes[clazz] = 0;
    int& count = classes[clazz];
    foreach(Declaration* d, ctx->localDeclarations()) {
        if (d->isFunctionDeclaration()) {
            count += 1;
        }
    }
}

/*! recursive functions which traverses ctx for classes and updates the
    member counts in the classes map */
void computeClassMemberCount(DUContext* parent, QMap<Declaration*,int>& classes)
{
    foreach(DUContext* child, parent->childContexts()){
        switch(child->type()) {
        case DUContext::Class: {
            updateClassMemberCountFor(child, classes); break;
        } case DUContext::Namespace: {
            computeClassMemberCount(child, classes); break;
        } default: {}
        }
    }
}

/*! Calculates the class with the highest number of member function
    declarations in context ctx */
Declaration* dominantClassInHeader(TopDUContext* ctx)
{
    kDebug() << "";
    QMap<Declaration*, int> classes;
    computeClassMemberCount(ctx, classes);
    kDebug() << "classes_" << classes;
    return mostFrequentClass(classes);
}

bool isTest(Declaration* clazz)
{
    QualifiedIdentifier qid = clazz->qualifiedIdentifier();
    return qid.toString().endsWith("Test"); //|| hasQTestMainFunction(docCtx);
}

QualifiedIdentifier getUnitUnderTestIdFor(Declaration* testClass)
{
    QString qidStr(testClass->qualifiedIdentifier().toString());
    qidStr.chop(4);            // truncate 'Test'
    QualifiedIdentifier uutQid(qidStr);
    return uutQid;
}

QualifiedIdentifier getTestIdFor(Declaration* uutClass)
{
    QString qidStr(uutClass->qualifiedIdentifier().toString());
    QualifiedIdentifier testQid(qidStr + "Test");
    return testQid;
}

KUrl findTargetLocation(QualifiedIdentifier target)
{
    kDebug() << "Querying persistent symbol table for " << target.toString();
    PersistentSymbolTable& pst = PersistentSymbolTable::self();
    const IndexedDeclaration* matches;
    uint count;
    pst.declarations(target, count, matches);
    KUrl url;
/*    if (count > 0) {
        url = KUrl(matches->declaration()->url().str());
    }*/
    for(uint i=0; i<count; ++i, ++matches) {
        Declaration* matched = matches->declaration();
        if (!matched || matched->url().isEmpty()) continue;
        kDebug() << "matched " << matched->toString() << " " << matched->url().str();
        if (!matched->isForwardDeclaration()) {
            url = KUrl(matches->declaration()->url().str());
        }
    }
    return url;
}

/*! Parse doc string @p comment for @p key and extract it's argument.
    Return default string if not found. */
QString extractArgumentFromDocs(const QString& key, const QString& comment)
{
    kDebug() << key;
    int i = comment.indexOf(key);
    if (i==-1) return QString();
    QString rest = comment.mid(i + 1 + QString(key).count()); // +1 is for space
    QStringList spl = rest.split(' ', QString::SkipEmptyParts);
    rest = spl[0];
    if (rest.contains("\n") || rest.contains("*")) {
        return QString();
    } else {
      return rest;
    }
}

KUrl resolveTargetUrl(QualifiedIdentifier& target)
{
    KUrl targetUrl = findTargetLocation(target);
    if (targetUrl.isEmpty() && target.count() > 1) {
        // try without namespaces
        QualifiedIdentifier qid(target.last().toString());
        targetUrl = findTargetLocation(qid);
    }
    if (targetUrl.isEmpty() && target.count() > 2) {
        // given foo::baz::Bar try foo::Bar
        QualifiedIdentifier qid(target.first().toString());
        qid += QualifiedIdentifier(target.last().toString());
        targetUrl = findTargetLocation(qid);
    }
    return targetUrl;
}

} // end anonymous namespace

TestSwitch::TestSwitch(QObject* parent)
  : QObject(parent), m_actionConnected(false), m_standardMacros(0), m_buzzySwitching(false)
{}

TestSwitch::~TestSwitch()
{}

void TestSwitch::connectAction(KActionCollection* col)
{
    if (m_actionConnected) return;
    KAction* a = col->addAction("switch_test_unitundertest");
    a->setText( i18n("&Switch Test/Unit Under Test") );
    a->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_R );
    connect(a, SIGNAL(triggered()),
            this, SLOT(swapTest_UnitUnderTest()));
    m_actionConnected = true;
}


void TestSwitch::setStandardMacros(Cpp::ReferenceCountedMacroSet* macros)
{
    m_standardMacros = macros;
}

TopDUContext* TestSwitch::documentContextFor(const KUrl& url)
{
    CppPreprocessEnvironment* env = new CppPreprocessEnvironment(0, Cpp::EnvironmentFilePointer());
    if (m_standardMacros) env->merge(*m_standardMacros);
    DUChain* chainStore = DUChain::self();
    TopDUContext* docCtx = chainStore->chainForDocument(url, env);
    delete env;
    if(docCtx && docCtx->parsingEnvironmentFile() && docCtx->parsingEnvironmentFile()->isProxyContext()) {
        if(!docCtx->importedParentContexts().isEmpty()) {
            docCtx = dynamic_cast<TopDUContext*>(docCtx->importedParentContexts().first().context(0));
        }
    }
    return docCtx;
}

#define STOP_IF(X, MSG) \
  if (X) { kDebug() << MSG; m_buzzySwitching = false; return; }

void TestSwitch::swapTest_UnitUnderTest()
{
    if (m_buzzySwitching) return;
    m_buzzySwitching = true;
    kDebug() << "Switching between test and unit under test";
    IDocument* doc = activeDocument();
    STOP_IF(!doc, "Can't switch, no active document.")

    DUChainReadLocker lock(DUChain::lock());
    TopDUContext* docCtx = documentContextFor(doc->url());
    STOP_IF(!docCtx, QString("Failed to get chain for %1").arg(doc->url().path()))

    Declaration* clazz = (isHeader(doc->url())) ?
        dominantClassInHeader(docCtx) :
        dominantClassInCpp(docCtx);
    STOP_IF(!clazz, "No dominant class found.")
    kDebug() << "Dominant Class >> " << clazz->toString();

    QString dox = clazz->comment(), docTarget;
    kDebug() << "DOX\n" << dox;
    if (dox.contains("@unitundertest")) {
        docTarget = extractArgumentFromDocs("@unitundertest", dox);
    } else if (dox.contains("@unittest")) {
        docTarget = extractArgumentFromDocs("@unittest", dox);
    }

    QualifiedIdentifier target; // the class to load
    if (!docTarget.isEmpty()) {
        target = QualifiedIdentifier(docTarget);
    } else { // resort to naming conventions
        target = (isTest(clazz)) ?
            getUnitUnderTestIdFor(clazz) :
            getTestIdFor(clazz);
    }
    KUrl targetUrl = resolveTargetUrl(target);
    lock.unlock();

    if (!targetUrl.isEmpty()) {
        kDebug() << "Found test/uut in " << targetUrl;
        IDocumentController* dc = ICore::self()->documentController();
        dc->openDocument(targetUrl);
    } else {
        kDebug() << "Failed to find switch candidate for " << doc->url();
    }
    m_buzzySwitching = false;
}

#include "testswitch.moc"
