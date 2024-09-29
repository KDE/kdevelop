/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "usescollector.h"
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/indexedtype.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <backgroundparser/parsejob.h>
#include <backgroundparser/backgroundparser.h>
#include "../classmemberdeclaration.h"
#include "../abstractfunctiondeclaration.h"
#include "../functiondefinition.h"
#include <debug.h>
#include <interfaces/iuicontroller.h>
#include <codegen/coderepresentation.h>
#include <sublime/message.h>
#include <util/algorithm.h>

#include <KLocalizedString>

using namespace KDevelop;

///@todo make this language-neutral
static Identifier destructorForName(const Identifier& name)
{
    QString str = name.identifier().str();
    if (str.startsWith(QLatin1Char('~')))
        return Identifier(str);
    return Identifier(QLatin1Char('~') + str);
}

///@todo Only collect uses within currently loaded projects

template <class ImportanceChecker>
void collectImporters(ImportanceChecker& checker, ParsingEnvironmentFile* current,
                      QSet<ParsingEnvironmentFile*>& visited, QSet<ParsingEnvironmentFile*>& collected)
{
    //Ignore proxy-contexts while collecting. Those build a parallel and much more complicated structure.
    if (current->isProxyContext())
        return;

    if (!Algorithm::insert(visited, current).inserted)
        return;

    if (checker(current))
        collected.insert(current);

    const auto importers = current->importers();
    for (const ParsingEnvironmentFilePointer& importer : importers) {
        if (importer.data())
            collectImporters(checker, importer.data(), visited, collected);
        else
            qCDebug(LANGUAGE) << "missing environment-file, strange";
    }
}

///The returned set does not include the file itself
///@param visited should be empty on each call, used to prevent endless recursion
void allImportedFiles(ParsingEnvironmentFilePointer file, QSet<IndexedString>& set,
                      QSet<ParsingEnvironmentFilePointer>& visited)
{
    const auto imports = file->imports();
    for (const ParsingEnvironmentFilePointer& import : imports) {
        if (!import) {
            qCDebug(LANGUAGE) << "warning: missing import";
            continue;
        }
        if (Algorithm::insert(visited, import).inserted) {
            set.insert(import->url());
            allImportedFiles(import, set, visited);
        }
    }
}

void UsesCollector::setCollectConstructors(bool process)
{
    m_collectConstructors = process;
}

void UsesCollector::setProcessDeclarations(bool process)
{
    m_processDeclarations = process;
}

void UsesCollector::setCollectOverloads(bool collect)
{
    m_collectOverloads = collect;
}

void UsesCollector::setCollectDefinitions(bool collect)
{
    m_collectDefinitions = collect;
}

QList<IndexedDeclaration> UsesCollector::declarations()
{
    return m_declarations;
}

bool UsesCollector::isReady() const
{
    return m_waitForUpdate.size() == m_updateReady.size();
}

bool UsesCollector::shouldRespectFile(const IndexedString& document)
{
    return ( bool )ICore::self()->projectController()->findProjectForUrl(document.toUrl()) ||
           ( bool )ICore::self()->documentController()->documentForUrl(document.toUrl());
}

struct ImportanceChecker
{
    explicit ImportanceChecker(UsesCollector& collector) : m_collector(collector)
    {
    }
    bool operator ()(ParsingEnvironmentFile* file)
    {
        return m_collector.shouldRespectFile(file->url());
    }
    UsesCollector& m_collector;
};

void UsesCollector::startCollecting()
{
    DUChainReadLocker lock(DUChain::lock());

    if (Declaration* decl = m_declaration.data()) {
        if (m_collectDefinitions) {
            if (auto* def = dynamic_cast<FunctionDefinition*>(decl)) {
                //Jump from definition to declaration
                Declaration* declaration = def->declaration();
                if (declaration)
                    decl = declaration;
            }
        }

        ///Collect all overloads into "decls"
        QList<Declaration*> decls;

        if (m_collectOverloads && decl->context()->owner() && decl->context()->type() == DUContext::Class) {
            //First find the overridden base, and then all overriders of that base.
            while (Declaration* overridden = DUChainUtils::overridden(decl))
                decl = overridden;
            uint maxAllowedSteps = 10000;
            decls += DUChainUtils::overriders(decl->context()->owner(), decl, maxAllowedSteps);
            if (maxAllowedSteps == 10000) {
                ///@todo Fail!
            }
        }

        decls << decl;

        ///Collect all "parsed versions" or forward-declarations etc. here, into allDeclarations
        QSet<IndexedDeclaration> allDeclarations;

        for (Declaration* overload : std::as_const(decls)) {
            m_declarations = DUChainUtils::collectAllVersions(overload);
            for (const IndexedDeclaration& d : std::as_const(m_declarations)) {
                if (!d.data() || d.data()->id() != overload->id())
                    continue;
                allDeclarations.insert(d);

                if (m_collectConstructors && d.data() && d.data()->internalContext() &&
                    d.data()->internalContext()->type() == DUContext::Class) {
                    const QList<Declaration*> constructors = d.data()->internalContext()->findLocalDeclarations(
                        d.data()->identifier(), CursorInRevision::invalid(), nullptr,
                        AbstractType::Ptr(), DUContext::OnlyFunctions);
                    for (Declaration* constructor : constructors) {
                        auto* classFun = dynamic_cast<ClassFunctionDeclaration*>(constructor);
                        if (classFun && classFun->isConstructor())
                            allDeclarations.insert(IndexedDeclaration(constructor));
                    }

                    Identifier destructorId = destructorForName(d.data()->identifier());

                    const QList<Declaration*> destructors = d.data()->internalContext()->findLocalDeclarations(
                        destructorId, CursorInRevision::invalid(), nullptr,
                        AbstractType::Ptr(), DUContext::OnlyFunctions);
                    for (Declaration* destructor : destructors) {
                        auto* classFun = dynamic_cast<ClassFunctionDeclaration*>(destructor);
                        if (classFun && classFun->isDestructor())
                            allDeclarations.insert(IndexedDeclaration(destructor));
                    }
                }
            }
        }

        ///Collect definitions for declarations
        if (m_collectDefinitions) {
            for (const IndexedDeclaration d : std::as_const(allDeclarations)) {
                Declaration* definition = FunctionDefinition::definition(d.data());
                if (definition) {
                    qCDebug(LANGUAGE) << "adding definition";
                    allDeclarations.insert(IndexedDeclaration(definition));
                }
            }
        }

        m_declarations.clear();

        ///Step 4: Copy allDeclarations into m_declarations, build top-context list, etc.
        QList<ReferencedTopDUContext> candidateTopContexts;
        candidateTopContexts.reserve(allDeclarations.size());
        m_declarations.reserve(allDeclarations.size());
        for (const IndexedDeclaration d : std::as_const(allDeclarations)) {
            m_declarations << d;
            m_declarationTopContexts.insert(d.indexedTopContext());
            //We only collect declarations with the same type here..
            candidateTopContexts << d.indexedTopContext().data();
        }

        ImportanceChecker checker(*this);

        QSet<ParsingEnvironmentFile*> visited;
        QSet<ParsingEnvironmentFile*> collected;

        qCDebug(LANGUAGE) << "count of source candidate top-contexts:" << candidateTopContexts.size();

        ///We use ParsingEnvironmentFile to collect all the relevant importers, because loading those is very cheap, compared
        ///to loading a whole TopDUContext.
        if (decl->inSymbolTable()) {
            //The declaration can only be used from other contexts if it is in the symbol table
            for (const ReferencedTopDUContext& top : std::as_const(candidateTopContexts)) {
                if (top->parsingEnvironmentFile()) {
                    collectImporters(checker, top->parsingEnvironmentFile().data(), visited, collected);
                    //In C++, visibility is not handled strictly through the import-structure.
                    //It may happen that an object is visible because of an earlier include.
                    //We can not perfectly handle that, but we can at least handle it if the header includes
                    //the header that contains the declaration. That header may be parsed empty due to header-guards,
                    //but we still need to pick it up here.
                    const QList<ParsingEnvironmentFilePointer> allVersions = DUChain::self()->allEnvironmentFiles(
                        top->url());
                    for (const ParsingEnvironmentFilePointer& version : allVersions)
                        collectImporters(checker, version.data(), visited, collected);
                }
            }
        }
        KDevelop::ParsingEnvironmentFile* file = decl->topContext()->parsingEnvironmentFile().data();
        if (!file)
            return;

        if (checker(file))
            collected.insert(file);

        {
            QSet<ParsingEnvironmentFile*> filteredCollected;
            QMap<IndexedString, bool> grepCache;
            // Filter the collected files by performing a grep
            for (ParsingEnvironmentFile* file : std::as_const(collected)) {
                IndexedString url = file->url();
                QMap<IndexedString, bool>::iterator grepCacheIt = grepCache.find(url);
                if (grepCacheIt == grepCache.end()) {
                    CodeRepresentation::Ptr repr = KDevelop::createCodeRepresentation(url);
                    if (repr) {
                        QVector<KTextEditor::Range> found = repr->grep(decl->identifier().identifier().str());
                        grepCacheIt = grepCache.insert(url, !found.isEmpty());
                    }
                }
                if (grepCacheIt.value())
                    filteredCollected << file;
            }

            qCDebug(LANGUAGE) << "Collected contexts for full re-parse, before filtering: " << collected.size() <<
                " after filtering: " << filteredCollected.size();
            collected = filteredCollected;
        }

        ///We have all importers now. However since we can tell parse-jobs to also update all their importers, we only need to
        ///update the "root" top-contexts that open the whole set with their imports.
        QSet<IndexedString> rootFiles;
        QSet<IndexedString> allFiles;
        for (ParsingEnvironmentFile* importer : std::as_const(collected)) {
            QSet<IndexedString> allImports;
            QSet<ParsingEnvironmentFilePointer> visited;
            allImportedFiles(ParsingEnvironmentFilePointer(importer), allImports, visited);
            //Remove all files from the "root" set that are imported by this one
            ///@todo more intelligent
            rootFiles -= allImports;
            allFiles += allImports;
            allFiles.insert(importer->url());
            rootFiles.insert(importer->url());
        }

        emit maximumProgressSignal(rootFiles.size());
        maximumProgress(rootFiles.size());

        //If we used the AllDeclarationsContextsAndUsesRecursive flag here, we would compute way too much. This way we only
        //set the minimum-features selectively on the files we really require them on.
        for (ParsingEnvironmentFile* file : std::as_const(collected)) {
            m_staticFeaturesManipulated.insert(file->url());
        }

        m_staticFeaturesManipulated.insert(decl->url());

        const auto currentFeaturesManipulated = m_staticFeaturesManipulated;
        for (const IndexedString& file : currentFeaturesManipulated) {
            ParseJob::setStaticMinimumFeatures(file, TopDUContext::AllDeclarationsContextsAndUses);
        }

        m_waitForUpdate = rootFiles;

        for (const IndexedString& file : std::as_const(rootFiles)) {
            qCDebug(LANGUAGE) << "updating root file:" << file.str();
            DUChain::self()->updateContextForUrl(file, TopDUContext::AllDeclarationsContextsAndUses, this);
        }
    } else {
        emit maximumProgressSignal(0);
        maximumProgress(0);
    }
}

void UsesCollector::maximumProgress(uint max)
{
    Q_UNUSED(max);
}

UsesCollector::UsesCollector(IndexedDeclaration declaration) : m_declaration(declaration)
    , m_collectOverloads(true)
    , m_collectDefinitions(true)
    , m_collectConstructors(false)
    , m_processDeclarations(true)
{
}

UsesCollector::~UsesCollector()
{
    ICore::self()->languageController()->backgroundParser()->revertAllRequests(this);

    const auto currentFeaturesManipulated = m_staticFeaturesManipulated;
    for (const IndexedString& file : currentFeaturesManipulated) {
        ParseJob::unsetStaticMinimumFeatures(file, TopDUContext::AllDeclarationsContextsAndUses);
    }
}

void UsesCollector::progress(uint processed, uint total)
{
    Q_UNUSED(processed);
    Q_UNUSED(total);
}

void UsesCollector::updateReady(const KDevelop::IndexedString& url, KDevelop::ReferencedTopDUContext topContext)
{
    DUChainReadLocker lock(DUChain::lock());

    if (!topContext) {
        qCDebug(LANGUAGE) << "failed updating" << url.str();
    } else {
        if (topContext->parsingEnvironmentFile() && topContext->parsingEnvironmentFile()->isProxyContext()) {
            ///Use the attached content-context instead
            const auto importedParentContexts = topContext->importedParentContexts();
            for (const DUContext::Import& import : importedParentContexts) {
                if (import.context(nullptr) && import.context(nullptr)->topContext()->parsingEnvironmentFile() &&
                    !import.context(nullptr)->topContext()->parsingEnvironmentFile()->isProxyContext()) {
                    if ((import.context(nullptr)->topContext()->features() &
                         TopDUContext::AllDeclarationsContextsAndUses)) {
                        ReferencedTopDUContext newTop(import.context(nullptr)->topContext());
                        topContext = newTop;
                        break;
                    }
                }
            }

            if (topContext->parsingEnvironmentFile() && topContext->parsingEnvironmentFile()->isProxyContext()) {
                qCDebug(LANGUAGE) << "got bad proxy-context for" << url.str();
                topContext = nullptr;
            }
        }
    }

    if (m_waitForUpdate.contains(url) && Algorithm::insert(m_updateReady, url).inserted) {
        m_checked.clear();

        emit progressSignal(m_updateReady.size(), m_waitForUpdate.size());
        progress(m_updateReady.size(), m_waitForUpdate.size());
    }

    if (!topContext || !topContext->parsingEnvironmentFile()) {
        qCDebug(LANGUAGE) << "bad top-context";
        return;
    }

    if (!m_staticFeaturesManipulated.contains(url))
        return; //Not interesting

    if (!(topContext->features() & TopDUContext::AllDeclarationsContextsAndUses)) {
        ///@todo With simplified environment-matching, the same file may have been imported multiple times,
        ///while only one of  those was updated. We have to check here whether this file is just such an import,
        ///or whether we work on with it.
        ///@todo We will lose files that were edited right after their update here.
        qCWarning(LANGUAGE) << "WARNING: context" << topContext->url().str() << "does not have the required features!!";
        // TODO no i18n?
        const QString messageText = QLatin1String("Updating ") +
            ICore::self()->projectController()->prettyFileName(topContext->url().toUrl(), KDevelop::IProjectController::FormatPlain) + QLatin1String(" failed!");
        auto* message = new Sublime::Message(messageText, Sublime::Message::Warning);
        message->setAutoHide(0);
        ICore::self()->uiController()->postMessage(message);
        return;
    }

    if (topContext->parsingEnvironmentFile()->needsUpdate()) {
        qCWarning(LANGUAGE) << "WARNING: context" << topContext->url().str() << "is not up to date!";
        const auto prettyFileName = ICore::self()->projectController()->prettyFileName(topContext->url().toUrl(), KDevelop::IProjectController::FormatPlain);
        const auto messageText = i18n("%1 still needs an update!", prettyFileName);
        auto* message = new Sublime::Message(messageText, Sublime::Message::Warning);
        message->setAutoHide(0);
        ICore::self()->uiController()->postMessage(message);
//       return;
    }

    IndexedTopDUContext indexed(topContext.data());
    if (m_checked.contains(indexed))
        return;

    if (!topContext.data()) {
        qCDebug(LANGUAGE) << "updated top-context is zero:" << url.str();
        return;
    }

    m_checked.insert(indexed);

    if (m_declaration.data() && ((m_processDeclarations && m_declarationTopContexts.contains(indexed)) ||
                                 DUChainUtils::contextHasUse(topContext.data(), m_declaration.data()))) {
        if (Algorithm::insert(m_processed, topContext->url()).inserted) {
            lock.unlock();
            emit processUsesSignal(topContext);
            processUses(topContext);
            lock.lock();
        }
    } else {
        if (!m_declaration.data()) {
            qCDebug(LANGUAGE) << "declaration has become invalid";
        }
    }

    QList<KDevelop::ReferencedTopDUContext> imports;

    const auto importedParentContexts = topContext->importedParentContexts();
    for (const DUContext::Import& imported : importedParentContexts) {
        if (imported.context(nullptr) && imported.context(nullptr)->topContext())
            imports << KDevelop::ReferencedTopDUContext(imported.context(nullptr)->topContext());
    }

    for (const KDevelop::ReferencedTopDUContext& import : std::as_const(imports)) {
        IndexedString url = import->url();
        lock.unlock();
        updateReady(url, import);
        lock.lock();
    }
}

IndexedDeclaration UsesCollector::declaration() const
{
    return m_declaration;
}

#include "moc_usescollector.cpp"
