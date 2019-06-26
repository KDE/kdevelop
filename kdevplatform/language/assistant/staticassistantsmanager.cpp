/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "staticassistantsmanager.h"
#include <debug.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguagesupport.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>

#include <language/duchain/problem.h>
#include <language/editor/documentrange.h>

using namespace KDevelop;
using namespace KTextEditor;

class KDevelop::StaticAssistantsManagerPrivate
{
public:
    explicit StaticAssistantsManagerPrivate(StaticAssistantsManager* qq)
        : q(qq)
    { }

    void updateReady(const IndexedString& document, const KDevelop::ReferencedTopDUContext& topContext);
    void documentLoaded(KDevelop::IDocument*);
    void textInserted(KTextEditor::Document* document, const Cursor& cursor, const QString& text);
    void textRemoved(KTextEditor::Document* document, const Range& cursor, const QString& removedText);

    StaticAssistantsManager* q;

    QVector<StaticAssistant::Ptr> m_registeredAssistants;
};

StaticAssistantsManager::StaticAssistantsManager(QObject* parent)
    : QObject(parent)
    , d_ptr(new StaticAssistantsManagerPrivate(this))
{
    Q_D(StaticAssistantsManager);

    connect(KDevelop::ICore::self()->documentController(),
            &IDocumentController::documentLoaded,
            this, [this](IDocument* document) {
        Q_D(StaticAssistantsManager);
        d->documentLoaded(document);
    });
    const auto documents = ICore::self()->documentController()->openDocuments();
    for (IDocument* document : documents) {
        d->documentLoaded(document);
    }

    connect(DUChain::self(), &DUChain::updateReady,
            this, &StaticAssistantsManager::notifyAssistants);
}

StaticAssistantsManager::~StaticAssistantsManager()
{
}

void StaticAssistantsManager::registerAssistant(const StaticAssistant::Ptr& assistant)
{
    Q_D(StaticAssistantsManager);

    if (d->m_registeredAssistants.contains(assistant))
        return;

    d->m_registeredAssistants << assistant;
}

void StaticAssistantsManager::unregisterAssistant(const StaticAssistant::Ptr& assistant)
{
    Q_D(StaticAssistantsManager);

    d->m_registeredAssistants.removeOne(assistant);
}

QVector<StaticAssistant::Ptr> StaticAssistantsManager::registeredAssistants() const
{
    Q_D(const StaticAssistantsManager);

    return d->m_registeredAssistants;
}

void StaticAssistantsManagerPrivate::documentLoaded(IDocument* document)
{
    if (document->textDocument()) {
        auto doc = document->textDocument();
        QObject::connect(doc, &KTextEditor::Document::textInserted, q,
                         [&](KTextEditor::Document* doc, const Cursor& cursor, const QString& text) {
            textInserted(doc, cursor, text);
        });
        QObject::connect(doc, &KTextEditor::Document::textRemoved, q,
                         [&](KTextEditor::Document* doc, const Range& range, const QString& removedText) {
            textRemoved(doc, range, removedText);
        });
    }
}

void StaticAssistantsManagerPrivate::textInserted(Document* doc, const Cursor& cursor, const QString& text)
{
    auto changed = false;
    for (auto& assistant : qAsConst(m_registeredAssistants)) {
        auto range = Range(cursor, cursor + Cursor(0, text.size()));
        auto wasUseful = assistant->isUseful();
        assistant->textChanged(doc, range, {});
        if (wasUseful != assistant->isUseful()) {
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT q->problemsChanged(IndexedString(doc->url()));
    }
}

void StaticAssistantsManagerPrivate::textRemoved(Document* doc, const Range& range,
                                                 const QString& removedText)
{
    auto changed = false;
    for (auto& assistant : qAsConst(m_registeredAssistants)) {
        auto wasUseful = assistant->isUseful();
        assistant->textChanged(doc, range, removedText);
        if (wasUseful != assistant->isUseful()) {
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT q->problemsChanged(IndexedString(doc->url()));
    }
}

void StaticAssistantsManager::notifyAssistants(const IndexedString& url,
                                               const KDevelop::ReferencedTopDUContext& context)
{
    Q_D(StaticAssistantsManager);

    for (auto& assistant : qAsConst(d->m_registeredAssistants)) {
        assistant->updateReady(url, context);
    }
}

QVector<KDevelop::Problem::Ptr> KDevelop::StaticAssistantsManager::problemsForContext(
    const KDevelop::ReferencedTopDUContext& top) const
{
    Q_D(const StaticAssistantsManager);

    View* view = ICore::self()->documentController()->activeTextDocumentView();
    if (!view || !top || IndexedString(view->document()->url()) != top->url()) {
        return {};
    }

    auto doc = top->url();
    auto language = ICore::self()->languageController()->languagesForUrl(doc.toUrl()).value(0);
    if (!language) {
        return {};
    }

    auto ret = QVector<KDevelop::Problem::Ptr>();
    qCDebug(LANGUAGE) << "Trying to find assistants for language" << language->name();
    for (const auto& assistant : qAsConst(d->m_registeredAssistants)) {
        if (assistant->supportedLanguage() != language)
            continue;

        if (assistant->isUseful()) {
            qCDebug(LANGUAGE) << "assistant is now useful:" << assistant.data();

            auto p = new KDevelop::StaticAssistantProblem();
            auto range = assistant->displayRange();
            qCDebug(LANGUAGE) << "range:" << range;
            p->setFinalLocation(DocumentRange(doc, range));
            p->setSource(KDevelop::IProblem::SemanticAnalysis);
            p->setSeverity(KDevelop::IProblem::Warning);
            p->setDescription(assistant->title());
            p->setSolutionAssistant(IAssistant::Ptr(assistant.data()));

            ret.append(KDevelop::Problem::Ptr(p));
        }
    }

    return ret;
}

#include "moc_staticassistantsmanager.cpp"
