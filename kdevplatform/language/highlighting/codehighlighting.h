/*
    SPDX-FileCopyrightText: 2007-2010 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CODEHIGHLIGHTING_H
#define KDEVPLATFORM_CODEHIGHLIGHTING_H

#include <QObject>
#include <QHash>
#include <QRecursiveMutex>

#include <serialization/indexedstring.h>
#include <language/duchain/ducontext.h>
#include <language/interfaces/icodehighlighting.h>
#include <language/backgroundparser/documentchangetracker.h>

#include <KTextEditor/Attribute>
#include <KTextEditor/MovingRange>

#include <memory>
#include <vector>

namespace KDevelop {
class DUContext;
class Declaration;

using ColorMap = QVector<KDevelop::Declaration*>;

class CodeHighlighting;

enum class CodeHighlightingType {
    Error,

    LocalClassMember,
    LocalMemberFunction,
    InheritedClassMember,
    InheritedMemberFunction,
    LocalVariable,
    MemberVariable,
    NamespaceVariable,
    GlobalVariable,
    FunctionVariable,

    Class,
    Namespace,
    Function,
    ForwardDeclaration,
    Enum,
    Enumerator,
    TypeAlias,
    Macro, ///< Declaration of a macro such as "#define FOO"
    MacroFunctionLike, ///< Declaration of a function like macro such as "#define FOO()"
    HighlightUses,
};

inline size_t qHash(CodeHighlightingType type, size_t seed = 0) noexcept
{
    return ::qHash(static_cast<int>(type), seed);
}

enum class CodeHighlightingContext {
    Definition,
    Declaration,
    Reference,
};

struct HighlightedRange
{
    RangeInRevision range;
    KTextEditor::Attribute::Ptr attribute;
    bool operator<(const HighlightedRange& rhs) const
    {
        return range.start < rhs.range.start;
    }
};

/**
 * Code highlighting instance that is used to apply code highlighting to one specific top context
 * */

class KDEVPLATFORMLANGUAGE_EXPORT CodeHighlightingInstance
{
public:
    explicit CodeHighlightingInstance(const CodeHighlighting* highlighting) : m_useClassCache(false)
        , m_highlighting(highlighting)
    {
    }
    virtual ~CodeHighlightingInstance()
    {
    }

    virtual void highlightDeclaration(KDevelop::Declaration* declaration, const QColor& color);
    virtual void highlightUse(KDevelop::DUContext* context, int index, const QColor& color);
    virtual void highlightUses(KDevelop::DUContext* context);

    void highlightDUChain(KDevelop::TopDUContext* context);
    void highlightDUChain(KDevelop::DUContext* context, QHash<KDevelop::Declaration*, uint> colorsForDeclarations,
        ColorMap);

    KDevelop::Declaration* localClassFromCodeContext(KDevelop::DUContext* context) const;
    /**
     * @param context Should be the context from where the declaration is used, if a use is highlighted.
     * */
    virtual CodeHighlightingType typeForDeclaration(KDevelop::Declaration* dec, KDevelop::DUContext* context) const;
    /**
     * Decides whether to apply auto-generated rainbow colors to @p dec.
     * Default implementation only applies that to local variables in functions.
     */
    virtual bool useRainbowColor(KDevelop::Declaration* dec) const;

    //A temporary hash for speedup
    mutable QHash<KDevelop::DUContext*, KDevelop::Declaration*> m_contextClasses;

    //Here the colors of function context are stored until they are merged into the function body
    mutable QHash<KDevelop::IndexedDUContext, QHash<KDevelop::Declaration*, uint>> m_functionColorsForDeclarations;
    mutable QHash<KDevelop::IndexedDUContext, ColorMap> m_functionDeclarationsForColors;

    mutable bool m_useClassCache;
    const CodeHighlighting* m_highlighting;

    QVector<HighlightedRange> m_highlight;
};

/**
 * General class representing the code highlighting for one language
 * */
class KDEVPLATFORMLANGUAGE_EXPORT CodeHighlighting : public QObject, public KDevelop::ICodeHighlighting
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ICodeHighlighting)

public:

    explicit CodeHighlighting(QObject* parent);
    ~CodeHighlighting() override;

    /// This function is thread-safe
    /// @warning The duchain must not be locked when this is called (->possible deadlock)
    void highlightDUChain(ReferencedTopDUContext context) override;

    //color should be zero when undecided
    KTextEditor::Attribute::Ptr attributeForType(CodeHighlightingType type, CodeHighlightingContext context,
                                                 const QColor& color) const;
    KTextEditor::Attribute::Ptr attributeForDepth(int depth) const;

    /// This function is thread-safe
    /// Returns whether a highlighting is already given for the given url
    bool hasHighlighting(IndexedString url) const override;

private:
    //Returns whether the given attribute was set by the code highlighting, and not by something else
    //Always returns true when the attribute is zero
    bool isCodeHighlight(KTextEditor::Attribute::Ptr attr) const;

protected:
    //Can be overridden to create an own instance type
    virtual CodeHighlightingInstance* createInstance() const;

private:
    using MovingRangePtr = std::unique_ptr<KTextEditor::MovingRange>;
    /// Highlighting of one specific document
    struct DocumentHighlighting
    {
        IndexedString m_document;
        qint64 m_waitingRevision;
        // The ranges are sorted by range start, so they can easily be matched
        QVector<HighlightedRange> m_waiting;
        std::vector<MovingRangePtr> m_highlightedRanges;
    };

    QHash<DocumentChangeTracker*, DocumentHighlighting*> m_highlights;

    friend class CodeHighlightingInstance;

    mutable QHash<CodeHighlightingType, KTextEditor::Attribute::Ptr> m_definitionAttributes;
    mutable QHash<CodeHighlightingType, KTextEditor::Attribute::Ptr> m_declarationAttributes;
    mutable QHash<CodeHighlightingType, KTextEditor::Attribute::Ptr> m_referenceAttributes;
    mutable QList<KTextEditor::Attribute::Ptr> m_depthAttributes;
    // Should be used to enable/disable the colorization of local variables and their uses
    bool m_localColorization;
    // Should be used to enable/disable the colorization of global types and their uses
    bool m_globalColorization;

    mutable QRecursiveMutex m_dataMutex;

private Q_SLOTS:
    void clearHighlightingForDocument(const KDevelop::IndexedString& document);
    void applyHighlighting(void* highlighting);

    /// when the colors change we must invalidate our local caches
    void adaptToColorChanges();

    void aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*);
    void aboutToRemoveText(const KTextEditor::Range&);
};
}

Q_DECLARE_TYPEINFO(KDevelop::HighlightedRange, Q_MOVABLE_TYPE);

#endif
