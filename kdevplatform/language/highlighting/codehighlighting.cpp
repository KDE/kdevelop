/*
    SPDX-FileCopyrightText: 2007-2010 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codehighlighting.h"

#include "../../interfaces/icore.h"
#include "../../interfaces/ilanguagecontroller.h"
#include "../../interfaces/icompletionsettings.h"
#include "../../util/foregroundlock.h"
#include <debug.h>

#include "../duchain/declaration.h"
#include "../duchain/types/functiontype.h"
#include "../duchain/types/enumeratortype.h"
#include "../duchain/types/typealiastype.h"
#include "../duchain/types/enumerationtype.h"
#include "../duchain/types/structuretype.h"
#include "../duchain/functiondefinition.h"
#include "../duchain/use.h"

#include "colorcache.h"
#include "configurablecolors.h"
#include <duchain/parsingenvironment.h>
#include <backgroundparser/backgroundparser.h>
#include <backgroundparser/urlparselock.h>

#include <KTextEditor/Document>

using namespace KTextEditor;

static const float highlightingZDepth = -500;

#define ifDebug(x)

namespace KDevelop {
///@todo Don't highlighting everything, only what is visible on-demand

CodeHighlighting::CodeHighlighting(QObject* parent)
    : QObject(parent)
    , m_localColorization(true)
    , m_globalColorization(true)
{
    qRegisterMetaType<KDevelop::IndexedString>("KDevelop::IndexedString");

    adaptToColorChanges();

    connect(ColorCache::self(), &ColorCache::colorsGotChanged,
            this, &CodeHighlighting::adaptToColorChanges);
}

CodeHighlighting::~CodeHighlighting()
{
    qDeleteAll(m_highlights);
}

void CodeHighlighting::adaptToColorChanges()
{
    QMutexLocker lock(&m_dataMutex);
    // disable local highlighting if the ratio is set to 0
    m_localColorization = ICore::self()->languageController()->completionSettings()->localColorizationLevel() > 0;
    // disable global highlighting if the ratio is set to 0
    m_globalColorization = ICore::self()->languageController()->completionSettings()->globalColorizationLevel() > 0;

    m_declarationAttributes.clear();
    m_definitionAttributes.clear();
    m_depthAttributes.clear();
    m_referenceAttributes.clear();
}

KTextEditor::Attribute::Ptr CodeHighlighting::attributeForType(CodeHighlightingType type,
                                                               CodeHighlightingContext context,
                                                               const QColor& color) const
{
    QMutexLocker lock(&m_dataMutex);
    KTextEditor::Attribute::Ptr a;
    switch (context) {
    case CodeHighlightingContext::Definition:
        a = m_definitionAttributes[type];
        break;

    case CodeHighlightingContext::Declaration:
        a = m_declarationAttributes[type];
        break;

    case CodeHighlightingContext::Reference:
        a = m_referenceAttributes[type];
        break;
    }

    if (!a || color.isValid()) {
        a = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute(*ColorCache::self()->defaultColors()->attribute(
                                                                       type)));

        if (context == CodeHighlightingContext::Definition || context == CodeHighlightingContext::Declaration) {
            if (ICore::self()->languageController()->completionSettings()->boldDeclarations()) {
                a->setFontBold();
            }
        }

        if (color.isValid()) {
            a->setForeground(color);
//       a->setBackground(QColor(mix(0xffffff-color, backgroundColor(), 255-backgroundTinting)));
        } else {
            switch (context) {
            case CodeHighlightingContext::Definition:
                m_definitionAttributes.insert(type, a);
                break;
            case CodeHighlightingContext::Declaration:
                m_declarationAttributes.insert(type, a);
                break;
            case CodeHighlightingContext::Reference:
                m_referenceAttributes.insert(type, a);
                break;
            }
        }
    }

    return a;
}

ColorMap emptyColorMap()
{
    ColorMap ret(ColorCache::self()->validColorCount() + 1, nullptr);
    return ret;
}

CodeHighlightingInstance* CodeHighlighting::createInstance() const
{
    return new CodeHighlightingInstance(this);
}

bool CodeHighlighting::hasHighlighting(IndexedString url) const
{
    DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(url);
    if (tracker) {
        QMutexLocker lock(&m_dataMutex);
        const auto highlightingIt = m_highlights.constFind(tracker);
        return highlightingIt != m_highlights.constEnd() && !(*highlightingIt)->m_highlightedRanges.isEmpty();
    }
    return false;
}

void CodeHighlighting::highlightDUChain(ReferencedTopDUContext context)
{
    ENSURE_CHAIN_NOT_LOCKED

    IndexedString url;

    {
        DUChainReadLocker lock;
        if (!context)
            return;

        url = context->url();
    }

    // This prevents the background-parser from updating the top-context while we're working with it
    UrlParseLock urlLock(context->url());

    DUChainReadLocker lock;

    qint64 revision = context->parsingEnvironmentFile()->modificationRevision().revision;

    qCDebug(LANGUAGE) << "highlighting du chain" << url.toUrl();

    if (!m_localColorization && !m_globalColorization) {
        qCDebug(LANGUAGE) << "highlighting disabled";
        QMetaObject::invokeMethod(this, "clearHighlightingForDocument", Qt::QueuedConnection,
                                  Q_ARG(KDevelop::IndexedString, url));
        return;
    }

    CodeHighlightingInstance* instance = createInstance();

    lock.unlock();

    instance->highlightDUChain(context.data());

    auto* highlighting = new DocumentHighlighting;
    highlighting->m_document = url;
    highlighting->m_waitingRevision = revision;
    highlighting->m_waiting = instance->m_highlight;
    std::sort(highlighting->m_waiting.begin(), highlighting->m_waiting.end());

    QMetaObject::invokeMethod(this, "applyHighlighting", Qt::QueuedConnection, Q_ARG(void*, highlighting));

    delete instance;
}

void CodeHighlightingInstance::highlightDUChain(TopDUContext* context)
{
    m_contextClasses.clear();
    m_useClassCache = true;

    //Highlight
    highlightDUChain(context, QHash<Declaration*, uint>(), emptyColorMap());

    m_functionColorsForDeclarations.clear();
    m_functionDeclarationsForColors.clear();

    m_useClassCache = false;
    m_contextClasses.clear();
}

void CodeHighlightingInstance::highlightDUChain(DUContext* context, QHash<Declaration*, uint> colorsForDeclarations,
                                                ColorMap declarationsForColors)
{
    DUChainReadLocker lock;

    TopDUContext* top = context->topContext();

    //Merge the colors from the function arguments
    const auto importedParentContexts = context->importedParentContexts();
    for (const DUContext::Import& imported : importedParentContexts) {
        if (!imported.context(top) ||
            (imported.context(top)->type() != DUContext::Other && imported.context(top)->type() != DUContext::Function))
            continue;
        //For now it's enough simply copying them, because we only pass on colors within function bodies.
        const auto functionColorsIt = m_functionColorsForDeclarations.constFind(imported.context(top));
        if (functionColorsIt != m_functionColorsForDeclarations.constEnd())
            colorsForDeclarations = *functionColorsIt;
        const auto functionDeclarationsIt = m_functionDeclarationsForColors.constFind(imported.context(top));
        if (functionDeclarationsIt != m_functionDeclarationsForColors.constEnd())
            declarationsForColors = *functionDeclarationsIt;
    }

    QList<Declaration*> takeFreeColors;

    bool noRainbow = ICore::self()->languageController()->completionSettings()->localColorizationLevel() == 0;
    const auto localDeclarations = context->localDeclarations();
    for (Declaration* dec : localDeclarations) {
        if (noRainbow || !useRainbowColor(dec)) {
            highlightDeclaration(dec, QColor(QColor::Invalid));
            continue;
        }
        //Initially pick a color using the hash, so the chances are good that the same identifier gets the same color always.
        uint colorNum = dec->identifier().hash() % ColorCache::self()->primaryColorCount();

        if (declarationsForColors[colorNum]) {
            takeFreeColors << dec; //Use one of the colors that stays free
            continue;
        }

        colorsForDeclarations[dec] = colorNum;
        declarationsForColors[colorNum] = dec;

        highlightDeclaration(dec, ColorCache::self()->generatedColor(colorNum));
    }

    for (Declaration* dec : std::as_const(takeFreeColors)) {
        uint colorNum = dec->identifier().hash() % ColorCache::self()->primaryColorCount();
        uint oldColorNum = colorNum;
        while (declarationsForColors[colorNum]) {
            colorNum = (colorNum + 1) % ColorCache::self()->primaryColorCount();
            if (colorNum == oldColorNum) {
                colorNum = ColorCache::self()->primaryColorCount();
                break;
            }
        }

        if (colorNum < ColorCache::self()->primaryColorCount()) {
            // Use primary color
            colorsForDeclarations[dec] = colorNum;
            declarationsForColors[colorNum] = dec;
            highlightDeclaration(dec, ColorCache::self()->generatedColor(colorNum));
        } else {
            // Try to use supplementary color
            colorNum = ColorCache::self()->primaryColorCount();
            while (declarationsForColors[colorNum]) {
                colorNum++;
                if (colorNum == ColorCache::self()->validColorCount()) {
                    //If no color could be found, use default color
                    highlightDeclaration(dec, QColor(QColor::Invalid));
                    break;
                }
            }
            if (colorNum < ColorCache::self()->validColorCount()) {
                // Use supplementary color
                colorsForDeclarations[dec] = colorNum;
                declarationsForColors[colorNum] = dec;
                highlightDeclaration(dec, ColorCache::self()->generatedColor(colorNum));
            }
        }
    }

    for (int a = 0; a < context->usesCount(); ++a) {
        Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[a].m_declarationIndex);
        QColor color(QColor::Invalid);
        const auto colorsIt = colorsForDeclarations.constFind(decl);
        if (colorsIt != colorsForDeclarations.constEnd())
            color = ColorCache::self()->generatedColor(*colorsIt);
        highlightUse(context, a, color);
    }

    if (context->type() == DUContext::Other || context->type() == DUContext::Function) {
        const auto indexed = IndexedDUContext(context);
        m_functionColorsForDeclarations[indexed] = colorsForDeclarations;
        m_functionDeclarationsForColors[indexed] = declarationsForColors;
    }

    const QVector<DUContext*> children = context->childContexts();

    lock.unlock(); // Periodically release the lock, so that the UI won't be blocked too much

    for (DUContext* child : children) {
        highlightDUChain(child,  colorsForDeclarations, declarationsForColors);
    }
}

KTextEditor::Attribute::Ptr CodeHighlighting::attributeForDepth(int depth) const
{
    while (depth >= m_depthAttributes.count()) {
        KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute());
        a->setBackground(QColor(Qt::white).darker(100 + (m_depthAttributes.count() * 25)));
        a->setBackgroundFillWhitespace(true);
        if (depth % 2)
            a->setOutline(Qt::red);
        m_depthAttributes.append(a);
    }

    return m_depthAttributes[depth];
}

KDevelop::Declaration* CodeHighlightingInstance::localClassFromCodeContext(KDevelop::DUContext* context) const
{
    if (!context)
        return nullptr;

    const auto classIt = m_contextClasses.constFind(context);
    if (classIt != m_contextClasses.constEnd())
        return *classIt;

    DUContext* startContext = context;

    while (context->type() == DUContext::Other) {
        //Move context to the top context of type "Other". This is needed because every compound-statement creates a new sub-context.
        auto parent = context->parentContext();
        if (!parent || (parent->type() != DUContext::Other && parent->type() != DUContext::Function)) {
            break;
        }
        context = context->parentContext();
    }

    ///Step 1: Find the function-declaration for the function we are in
    Declaration* functionDeclaration = nullptr;

    if (auto* def = dynamic_cast<FunctionDefinition*>(context->owner())) {
        const auto classIt = m_contextClasses.constFind(context);
        if (classIt != m_contextClasses.constEnd())
            return *classIt;

        functionDeclaration = def->declaration(startContext->topContext());
    }

    if (!functionDeclaration && context->owner())
        functionDeclaration = context->owner();

    if (!functionDeclaration) {
        if (m_useClassCache)
            m_contextClasses[context] = nullptr;
        return nullptr;
    }

    Declaration* decl  = functionDeclaration->context()->owner();

    if (m_useClassCache)
        m_contextClasses[context] = decl;

    return decl;
}

CodeHighlightingType CodeHighlightingInstance::typeForDeclaration(Declaration* dec, DUContext* context) const
{
    /**
     * We highlight in 3 steps by priority:
     * 1. Is the item in the local class or an inherited class? If yes, highlight.
     * 2. What kind of item is it? If it's a type/function/enumerator, highlight by type.
     * 3. Else, highlight by scope.
     *
     * */

    if (!dec)
        return CodeHighlightingType::Error;

    auto type = CodeHighlightingType::LocalVariable;
    if (dec->kind() == Declaration::Namespace)
        return CodeHighlightingType::Namespace;

    if (dec->kind() == Declaration::Macro) {
        return CodeHighlightingType::Macro;
    }

    if (context && dec->context() && dec->context()->type() == DUContext::Class) {
        //It is a use.
        //Determine the class we're in
        Declaration* klass = localClassFromCodeContext(context);
        if (klass) {
            if (klass->internalContext() == dec->context()) {
                // Using Member of the local class
                if (dec->type<KDevelop::FunctionType>())
                    type = CodeHighlightingType::LocalMemberFunction;
                else
                    type = CodeHighlightingType::LocalClassMember;
            } else if (klass->internalContext() && klass->internalContext()->imports(dec->context())) {
                // Using Member of an inherited clas
                if (dec->type<KDevelop::FunctionType>())
                    type = CodeHighlightingType::InheritedMemberFunction;
                else
                    type = CodeHighlightingType::InheritedClassMember;
            }
        }
    }

    if (type == CodeHighlightingType::LocalVariable) {
        if (dec->kind() == Declaration::Type || dec->type<KDevelop::FunctionType>() ||
            dec->type<KDevelop::EnumeratorType>()) {
            if (dec->isForwardDeclaration())
                type = CodeHighlightingType::ForwardDeclaration;
            else if (dec->type<KDevelop::FunctionType>())
                type = CodeHighlightingType::Function;
            else if (dec->type<StructureType>())
                type = CodeHighlightingType::Class;
            else if (dec->type<KDevelop::TypeAliasType>())
                type = CodeHighlightingType::TypeAlias;
            else if (dec->type<EnumerationType>())
                type = CodeHighlightingType::Enum;
            else if (dec->type<KDevelop::EnumeratorType>())
                type = CodeHighlightingType::Enumerator;
        }
    }

    if (type == CodeHighlightingType::LocalVariable) {
        switch (dec->context()->type()) {
        case DUContext::Namespace:
            type = CodeHighlightingType::NamespaceVariable;
            break;
        case DUContext::Class:
            type = CodeHighlightingType::MemberVariable;
            break;
        case DUContext::Function:
            type = CodeHighlightingType::FunctionVariable;
            break;
        case DUContext::Global:
            type = CodeHighlightingType::GlobalVariable;
            break;
        default:
            break;
        }
    }

    return type;
}

bool CodeHighlightingInstance::useRainbowColor(Declaration* dec) const
{
    return dec->context()->type() == DUContext::Function ||
           (dec->context()->type() == DUContext::Other && dec->context()->owner());
}

void CodeHighlightingInstance::highlightDeclaration(Declaration* declaration, const QColor& color)
{
    HighlightedRange h;
    h.range = declaration->range();
    h.attribute = m_highlighting->attributeForType(typeForDeclaration(declaration, nullptr),
                                                   CodeHighlightingContext::Declaration, color);
    m_highlight.push_back(h);
}

void CodeHighlightingInstance::highlightUse(DUContext* context, int index, const QColor& color)
{
    Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[index].m_declarationIndex);

    auto type = typeForDeclaration(decl, context);

    if (type != CodeHighlightingType::Error
        || ICore::self()->languageController()->completionSettings()->highlightSemanticProblems()) {
        HighlightedRange h;
        h.range = context->uses()[index].m_range;
        h.attribute = m_highlighting->attributeForType(type, CodeHighlightingContext::Reference, color);
        m_highlight.push_back(h);
    }
}

void CodeHighlightingInstance::highlightUses(DUContext* context)
{
    for (int a = 0; a < context->usesCount(); ++a)
        highlightUse(context, a, QColor(QColor::Invalid));
}

void CodeHighlighting::clearHighlightingForDocument(const IndexedString& document)
{
    VERIFY_FOREGROUND_LOCKED
    QMutexLocker lock(&m_dataMutex);
    DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(document);
    auto highlightingIt = m_highlights.find(tracker);
    if (highlightingIt != m_highlights.end()) {
        disconnect(tracker->document(), nullptr, this, nullptr);
        disconnect(tracker, &DocumentChangeTracker::destroyed, this, nullptr);
        delete *highlightingIt;
        m_highlights.erase(highlightingIt);
    }
}

void CodeHighlighting::applyHighlighting(void* _highlighting)
{
    auto* highlighting =
        static_cast<CodeHighlighting::DocumentHighlighting*>(_highlighting);

    VERIFY_FOREGROUND_LOCKED
    QMutexLocker lock(&m_dataMutex);
    DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(
        highlighting->m_document);

    if (!tracker) {
        qCDebug(LANGUAGE) << "no document found for the planned highlighting of" << highlighting->m_document.str();
        delete highlighting;
        return;
    }

    if (!tracker->holdingRevision(highlighting->m_waitingRevision)) {
        qCDebug(LANGUAGE) << "not holding revision" << highlighting->m_waitingRevision << "not applying highlighting;"
                          << "probably a new parse job has already updated the context";
        delete highlighting;
        return;
    }

    QVector<MovingRange*> oldHighlightedRanges;

    const auto highlightingIt = m_highlights.find(tracker);
    if (highlightingIt != m_highlights.end()) {
        oldHighlightedRanges.swap((*highlightingIt)->m_highlightedRanges);
        delete *highlightingIt;
        *highlightingIt = highlighting;
    } else {
        // we newly add this tracker, so add the connection
        connect(tracker->document(), &Document::aboutToInvalidateMovingInterfaceContent, this,
                &CodeHighlighting::aboutToInvalidateMovingInterfaceContent);
        // This can't use new style connect syntax since aboutToRemoveText is only part of KTextEditor::DocumentPrivate
        connect(tracker->document(), SIGNAL(aboutToRemoveText(KTextEditor::Range)),
                this, SLOT(aboutToRemoveText(KTextEditor::Range)));
        connect(tracker, &DocumentChangeTracker::destroyed, this, [this, tracker]() {
            // Called when a document is destroyed
            VERIFY_FOREGROUND_LOCKED
            QMutexLocker lock(&m_dataMutex);
            disconnect(tracker->document(), nullptr, this, nullptr);
            Q_ASSERT(m_highlights.contains(tracker));
            delete m_highlights[tracker];
            m_highlights.remove(tracker);
        });
        m_highlights.insert(tracker, highlighting);
    }

    // Now create MovingRanges (match old ones with the incoming ranges)

    KTextEditor::Range tempRange;

    QVector<MovingRange*>::iterator movingIt = oldHighlightedRanges.begin();
    QVector<HighlightedRange>::iterator rangeIt = highlighting->m_waiting.begin();

    while (rangeIt != highlighting->m_waiting.end()) {
        // Translate the range into the current revision
        KTextEditor::Range transformedRange = tracker->transformToCurrentRevision(rangeIt->range,
                                                                                  highlighting->m_waitingRevision);

        while (movingIt != oldHighlightedRanges.end() &&
               ((*movingIt)->start().line() < transformedRange.start().line() ||
                ((*movingIt)->start().line() == transformedRange.start().line() &&
                 (*movingIt)->start().column() < transformedRange.start().column()))) {
            delete *movingIt; // Skip ranges that are in front of the current matched range
            ++movingIt;
        }

        tempRange = transformedRange;

        if (movingIt == oldHighlightedRanges.end() ||
            transformedRange.start().line() != (*movingIt)->start().line() ||
            transformedRange.start().column() != (*movingIt)->start().column() ||
            transformedRange.end().line() != (*movingIt)->end().line() ||
            transformedRange.end().column() != (*movingIt)->end().column()) {
            Q_ASSERT(rangeIt->attribute);
            // The moving range is behind or unequal, create a new range
            highlighting->m_highlightedRanges.push_back(tracker->document()->newMovingRange(tempRange));
            highlighting->m_highlightedRanges.back()->setAttribute(rangeIt->attribute);
            highlighting->m_highlightedRanges.back()->setZDepth(highlightingZDepth);
        } else
        {
            // Update the existing moving range
            (*movingIt)->setAttribute(rangeIt->attribute);
            (*movingIt)->setRange(tempRange);
            highlighting->m_highlightedRanges.push_back(*movingIt);
            ++movingIt;
        }
        ++rangeIt;
    }

    for (; movingIt != oldHighlightedRanges.end(); ++movingIt)
        delete *movingIt; // Delete unmatched moving ranges behind
}

void CodeHighlighting::aboutToInvalidateMovingInterfaceContent(Document* doc)
{
    clearHighlightingForDocument(IndexedString(doc->url()));
}

void CodeHighlighting::aboutToRemoveText(const KTextEditor::Range& range)
{
    if (range.onSingleLine()) // don't try to optimize this
        return;

    VERIFY_FOREGROUND_LOCKED
    QMutexLocker lock(&m_dataMutex);
    Q_ASSERT(qobject_cast<KTextEditor::Document*>(sender()));
    auto* doc = static_cast<KTextEditor::Document*>(sender());

    DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()
                                     ->trackerForUrl(IndexedString(doc->url()));
    const auto highlightingIt = m_highlights.constFind(tracker);
    if (highlightingIt != m_highlights.constEnd()) {
        QVector<MovingRange*>& ranges = (*highlightingIt)->m_highlightedRanges;
        QVector<MovingRange*>::iterator it = ranges.begin();
        while (it != ranges.end()) {
            if (range.contains((*it)->toRange())) {
                delete (*it);
                it = ranges.erase(it);
            } else {
                ++it;
            }
        }
    }
}
}

#include "moc_codehighlighting.cpp"
