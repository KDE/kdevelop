/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "context.h"

#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/typealiastype.h>
#include <language/duchain/types/typeutils.h>
#include <language/duchain/stringhelpers.h>
#include <language/codecompletion/codecompletionmodel.h>
#include <language/codecompletion/normaldeclarationcompletionitem.h>
#include <language/codegen/documentchangeset.h>
#include <util/foregroundlock.h>
#include <custom-definesandincludes/idefinesandincludesmanager.h>
#include <project/projectmodel.h>

#include "../util/clangdebug.h"
#include "../util/clangtypes.h"
#include "../util/clangutils.h"
#include "../duchain/clangdiagnosticevaluator.h"
#include "../duchain/parsesession.h"
#include "../duchain/duchainutils.h"
#include "../duchain/navigationwidget.h"
#include "../clangsettings/clangsettingsmanager.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <memory>

#include <KTextEditor/Document>
#include <KTextEditor/View>

using namespace KDevelop;

namespace {
/// Maximum return-type string length in completion items
const int MAX_RETURN_TYPE_STRING_LENGTH = 20;

/// Priority of code-completion results. NOTE: Keep in sync with Clang code base.
enum CodeCompletionPriority {
  /// Priority for the next initialization in a constructor initializer list.
  CCP_NextInitializer = 7,
  /// Priority for an enumeration constant inside a switch whose condition is of the enumeration type.
  CCP_EnumInCase = 7,

  CCP_LocalDeclarationMatch = 8,

  CCP_DeclarationMatch = 12,

  CCP_LocalDeclarationSimiliar = 17,
  /// Priority for a send-to-super completion.
  CCP_SuperCompletion = 20,

  CCP_DeclarationSimiliar = 25,
  /// Priority for a declaration that is in the local scope.
  CCP_LocalDeclaration = 34,
  /// Priority for a member declaration found from the current method or member function.
  CCP_MemberDeclaration = 35,
  /// Priority for a language keyword (that isn't any of the other categories).
  CCP_Keyword = 40,
  /// Priority for a code pattern.
  CCP_CodePattern = 40,
  /// Priority for a non-type declaration.
  CCP_Declaration = 50,
  /// Priority for a type.
  CCP_Type = CCP_Declaration,
  /// Priority for a constant value (e.g., enumerator).
  CCP_Constant = 65,
  /// Priority for a preprocessor macro.
  CCP_Macro = 70,
  /// Priority for a nested-name-specifier.
  CCP_NestedNameSpecifier = 75,
  /// Priority for a result that isn't likely to be what the user wants, but is included for completeness.
  CCP_Unlikely = 80
};

/**
 * Common base class for Clang code completion items.
 */
template<class Base>
class CompletionItem : public Base
{
public:
    CompletionItem(const QString& display, const QString& prefix)
        : Base()
        , m_display(display)
        , m_prefix(prefix)
        , m_unimportant(false)
    {
    }

    ~CompletionItem() override = default;

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* /*model*/) const override
    {
        if (role == Qt::DisplayRole) {
            if (index.column() == CodeCompletionModel::Prefix) {
                return m_prefix;
            } else if (index.column() == CodeCompletionModel::Name) {
                return m_display;
            }
        }
        return {};
    }

    void markAsUnimportant()
    {
        m_unimportant = true;
    }

protected:
    QString m_display;
    QString m_prefix;
    bool m_unimportant;
};

class OverrideItem : public CompletionItem<CompletionTreeItem>
{
public:
    OverrideItem(const QString& nameAndParams, const QString& returnType)
        : CompletionItem<CompletionTreeItem>(
              nameAndParams,
              i18n("Override %1", returnType)
          )
        , m_returnType(returnType)
    {
    }

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const override
    {
        if (role == Qt::DecorationRole) {
            if (index.column() == KTextEditor::CodeCompletionModel::Icon) {
                return QIcon::fromTheme(QStringLiteral("CTparents"));
            }
        }
        return CompletionItem<CompletionTreeItem>::data(index, role, model);
    }

    void execute(KTextEditor::View* view, const KTextEditor::Range& word) override
    {
        QString replacement = m_returnType + QLatin1Char(' ') + m_display.replace(QRegularExpression(QStringLiteral("\\s*=\\s*0")), QString());

        bool appendSpecifer = true;
        if (const auto* project =
            KDevelop::ICore::self()->projectController()->findProjectForUrl(view->document()->url())) {
            const auto arguments = KDevelop::IDefinesAndIncludesManager::manager()->parserArguments(
                project->filesForPath(IndexedString(view->document()->url().path())).first());
            const auto match = QRegularExpression(QStringLiteral(R"(-std=c\+\+(\w+))")).match(arguments);

            appendSpecifer = match.hasMatch(); // assume non-modern if no standard is specified
            if (appendSpecifer) {
                const auto standard = match.capturedView(1);
                appendSpecifer = (standard != QLatin1String("98") && standard != QLatin1String("03"));
            }
        }

        if (appendSpecifer) {
            replacement.append(QLatin1String(" override;"));
        } else {
            replacement.append(QLatin1Char(';'));
        }

        DocumentChange overrideChange(IndexedString(view->document()->url()),
                                            word,
                                            QString{},
                                            replacement);
        overrideChange.m_ignoreOldText = true;
        DocumentChangeSet changes;
        changes.addChange(overrideChange);
        changes.applyAllChanges();
    }

private:
    QString m_returnType;
};

/**
 * Specialized completion item class for items which are represented by a Declaration
 */
class DeclarationItem : public CompletionItem<NormalDeclarationCompletionItem>
{
public:
    DeclarationItem(Declaration* dec, const QString& display, const QString& prefix, const QString& replacement)
        : CompletionItem<NormalDeclarationCompletionItem>(display, prefix)
        , m_replacement(replacement)
    {
        m_declaration = dec;
    }

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const override
    {
        if (role == CodeCompletionModel::MatchQuality && m_matchQuality) {
            return m_matchQuality;
        }

        auto ret = CompletionItem<NormalDeclarationCompletionItem>::data(index, role, model);
        if (ret.isValid()) {
            return ret;
        }
        return NormalDeclarationCompletionItem::data(index, role, model);
    }

    void execute(KTextEditor::View* view, const KTextEditor::Range& word) override
    {
        QString repl = m_replacement;
        DUChainReadLocker lock;

        if(!m_declaration){
            return;
        }

        if(m_declaration->isFunctionDeclaration()) {
            const auto functionType = m_declaration->type<FunctionType>();

            // protect against buggy code that created the m_declaration,
            // to mark it as a function but not assign a function type
            if (!functionType)
                return;

            auto doc = view->document();

            // Function pointer?
            bool funcptr = false;
            const auto line = doc->line(word.start().line());
            auto pos = word.end().column() - 1;
            while ( pos > 0 && (line.at(pos).isLetterOrNumber() || line.at(pos) == QLatin1Char(':')) ) {
                pos--;
                if ( line.at(pos) == QLatin1Char('&') ) {
                    funcptr = true;
                    break;
                }
            }

            auto restEmpty = doc->characterAt(word.end() + KTextEditor::Cursor{0, 1}) == QChar();

            bool didAddParentheses = false;
            if ( !funcptr && doc->characterAt(word.end()) != QLatin1Char('(') ) {
                repl += QLatin1String("()");
                didAddParentheses = true;
            }
            view->document()->replaceText(word, repl);
            if (functionType->indexedArgumentsSize() && didAddParentheses) {
                view->setCursorPosition(word.start() + KTextEditor::Cursor(0, repl.size() - 1));
            }
            auto returnTypeIntegral = functionType->returnType().dynamicCast<IntegralType>();
            if ( restEmpty && !funcptr && returnTypeIntegral && returnTypeIntegral->dataType() == IntegralType::TypeVoid ) {
                // function returns void and rest of line is empty -- nothing can be done with the result
                if (functionType->indexedArgumentsSize() ) {
                    // we placed the cursor inside the ()
                    view->document()->insertText(view->cursorPosition() + KTextEditor::Cursor(0, 1), QStringLiteral(";"));
                }
                else {
                    // we placed the cursor after the ()
                    view->document()->insertText(view->cursorPosition(), QStringLiteral(";"));
                    view->setCursorPosition(view->cursorPosition() + KTextEditor::Cursor{0, 1});
                }
            }
        } else {
            view->document()->replaceText(word, repl);
        }
    }

    bool createsExpandingWidget() const override
    {
        return true;
    }

    QWidget* createExpandingWidget(const CodeCompletionModel* /*model*/) const override
    {
        return new ClangNavigationWidget(m_declaration, AbstractNavigationWidget::EmbeddableWidget);
    }

    int matchQuality() const
    {
        return m_matchQuality;
    }

    ///Sets match quality from 0 to 10. 10 is the best fit.
    void setMatchQuality(int value)
    {
        m_matchQuality = value;
    }

    void setInheritanceDepth(int depth)
    {
        m_inheritanceDepth = depth;
    }

    int argumentHintDepth() const override
    {
        return m_depth;
    }

    void setArgumentHintDepth(int depth)
    {
        m_depth = depth;
    }

protected:
    int m_matchQuality = 0;
    int m_depth = 0;
    QString m_replacement;
};

class ImplementsItem : public DeclarationItem
{
public:
    static QString replacement(const FuncImplementInfo& info)
    {
        QString replacement = info.templatePrefix;
        if (!info.isDestructor && !info.isConstructor) {
            replacement += info.returnType + QLatin1Char(' ');
        }
        replacement += info.prototype + QLatin1String("\n{\n}\n");
        return replacement;
    }

    explicit ImplementsItem(const FuncImplementInfo& item)
        : DeclarationItem(item.declaration.data(), item.prototype,
            i18n("Implement %1", item.isConstructor ? QStringLiteral("<constructor>") :
                                   item.isDestructor ? QStringLiteral("<destructor>") : item.returnType),
            replacement(item)
          )
    {
    }

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const override
    {
        if (index.column() == CodeCompletionModel::Arguments) {
            // our display string already contains the arguments
            return {};
        }
        return DeclarationItem::data(index, role, model);
    }

    void execute(KTextEditor::View* view, const KTextEditor::Range& word) override
    {
        auto* const document = view->document();

        DocumentChangeSet changes;
        KTextEditor::Cursor rangeStart = word.start();

        // try and replace leading typed text that match the proposed implementation
        const QString leading = document->line(word.end().line()).left(word.end().column());
        const QString leadingNoSpace = removeWhitespace(leading);
        if (!leadingNoSpace.isEmpty() && (removeWhitespace(m_display).startsWith(leadingNoSpace)
            || removeWhitespace(m_replacement).startsWith(leadingNoSpace))) {
            const int removeSize = leading.end() - std::find_if_not(leading.begin(), leading.end(),
                                        [](QChar c){ return c.isSpace(); });
            rangeStart = {word.end().line(), word.end().column() - removeSize};
        }

        DocumentChange change(IndexedString(view->document()->url()),
                              KTextEditor::Range(rangeStart, word.end()),
                              QString(),
                              m_replacement);
        change.m_ignoreOldText = true;
        changes.addChange(change);
        changes.applyAllChanges();

        // Place cursor after the opening brace
        // arbitrarily chose 4, as it would accommodate the template and return types on their own line
        const auto searchRange = KTextEditor::Range(rangeStart, rangeStart.line() + 4, 0);
        const auto results = view->document()->searchText(searchRange, QStringLiteral("{"));
        if (!results.isEmpty()) {
            view->setCursorPosition(results.first().end());
        }
    }
};

class ArgumentHintItem : public DeclarationItem
{
public:
    struct CurrentArgumentRange
    {
        int start;
        int end;
    };

    ArgumentHintItem(Declaration* decl,  const QString& prefix, const QString& name, const QString& arguments, const CurrentArgumentRange& range)
        : DeclarationItem(decl, name, prefix, {})
        , m_range(range)
        , m_arguments(arguments)
    {}

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const override
    {
        if (role == CodeCompletionModel::CustomHighlight && index.column() == CodeCompletionModel::Arguments && argumentHintDepth()) {
            QTextCharFormat boldFormat;
            boldFormat.setFontWeight(QFont::Bold);
            const QList<QVariant> highlighting {
                QVariant(m_range.start),
                QVariant(m_range.end),
                boldFormat,
            };
            return highlighting;
        }

        if (role == CodeCompletionModel::HighlightingMethod && index.column() == CodeCompletionModel::Arguments && argumentHintDepth()) {
            return QVariant(CodeCompletionModel::CustomHighlighting);
        }

        if (index.column() == CodeCompletionModel::Arguments) {
            return m_arguments;
        }

        return DeclarationItem::data(index, role, model);
    }

private:
    CurrentArgumentRange m_range;
    QString m_arguments;
};

/**
 * A minimalistic completion item for macros and such
 */
class SimpleItem : public CompletionItem<CompletionTreeItem>
{
public:
    SimpleItem(const QString& display, const QString& prefix, const QString& replacement, const QIcon& icon = QIcon())
        : CompletionItem<CompletionTreeItem>(display, prefix)
        , m_replacement(replacement)
        , m_icon(icon)
    {
    }

    void execute(KTextEditor::View* view, const KTextEditor::Range& word) override
    {
        view->document()->replaceText(word, m_replacement);
    }

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const override
    {
        if (role == Qt::DecorationRole && index.column() == KTextEditor::CodeCompletionModel::Icon) {
            return m_icon;
        }
        if (role == CodeCompletionModel::UnimportantItemRole) {
            return m_unimportant;
        }
        return CompletionItem<CompletionTreeItem>::data(index, role, model);
    }

private:
    QString m_replacement;
    QIcon m_icon;
};

/**
 * Return true in case position @p position represents a cursor inside a comment
 */
bool isInsideComment(CXTranslationUnit unit, CXFile file, const KTextEditor::Cursor& position)
{
    if (!position.isValid()) {
        return false;
    }

    // TODO: This may get very slow for a large TU, investigate if we can improve this function
    auto begin = clang_getLocation(unit, file, 1, 1);
    auto end = clang_getLocation(unit, file, position.line() + 1, position.column() + 1);
    CXSourceRange range = clang_getRange(begin, end);

    // tokenize the whole range from the start until 'position'
    // if we detect a comment token at this position, return true
    const ClangTokens tokens(unit, range);
    for (CXToken token : tokens) {
        CXTokenKind tokenKind = clang_getTokenKind(token);
        if (tokenKind != CXToken_Comment) {
            continue;
        }

        auto range = ClangRange(clang_getTokenExtent(unit, token));
        if (range.toRange().contains(position)) {
            return true;
        }
    }
    return false;
}

/**
 * @return whether @p result is a builtin provided by KDevelop's automatically included GCC
 *         compatibility header, which libclang does not recognize as a builtin.
 */
bool isGccCompatibilityBuiltin(CXCompletionResult result)
{
    // The "#pragma clang system_header" line in plugins/clang/duchain/gccCompatibility/additional_floating_types.h
    // suppresses __KDevelopClangGccCompat Namespace and __float80 TypedefDecl completions, but not _FloatX TypedefDecl
    // completions. That's because clang_codeCompleteAt() filters out identifiers declared in a system header that start
    // with two underscores, but not an underscore followed by a capital letter, possibly due to a libclang bug.
    // The included typedefs substitute for separate floating-point types provided by GCC. Thus they are similar to
    // other builtin completions, such as int and double. For this reason, they are marked as builtins rather than
    // simply removed. The missing __float80 builtin completion shouldn't be a problem in practice, so we keep the
    // "#pragma clang system_header" line to make this function slightly simpler and more efficient.

    if (result.CursorKind != CXCursor_TypedefDecl || clang_getNumCompletionChunks(result.CompletionString) != 1
        || clang_getCompletionChunkKind(result.CompletionString, 0) != CXCompletionChunk_TypedText) {
        return false;
    }

    const ClangString clangString{clang_getCompletionChunkText(result.CompletionString, 0)};
    const auto text = clangString.c_str();
    const auto textSize = std::strlen(text);

    if (textSize != 8 && textSize != 9) {
        return false;
    }

    constexpr auto prefixSize = 6;
    if (!std::equal(text, text + prefixSize, "_Float")) {
        return false;
    }
    // TODO: explicitly capture [text, textSize] once building KDevelop with Visual Studio 2019 is no longer supported.
    const auto suffixEquals = [=](const char* suffix) {
        Q_ASSERT(std::strlen(suffix) == textSize - prefixSize);
        return std::equal(text + prefixSize, text + textSize, suffix);
    };
    if (textSize == 8) {
        return suffixEquals("32") || suffixEquals("64");
    } else {
        return suffixEquals("32x") || suffixEquals("64x") || suffixEquals("128");
    }
}

QString& elideStringRight(QString& str, int length)
{
    if (str.size() > length + 3) {
        return str.replace(length, str.size() - length, QStringLiteral("..."));
    }
    return str;
}

constexpr int maxBestMatchCompletionPriority = CCP_SuperCompletion;

/**
 * @return Value suited for @ref CodeCompletionModel::MatchQuality in the range [1, 10] (the higher the better)
 *
 * See https://clang.llvm.org/doxygen/CodeCompleteConsumer_8h_source.html for list of priorities
 * They (currently) are in the range [0, 80] (the lower, the better). Nevertheless, we are only setting priority
 * until maxBestMatchCompletionPriority (20), so we better build the value around it.
 */
int matchQualityFromBestMatchCompletionPriority(int completionPriority)
{
    Q_ASSERT(completionPriority >= 0);
    Q_ASSERT(completionPriority <= maxBestMatchCompletionPriority);

    constexpr int maxMatchQuality = 10;
    auto matchQuality = maxMatchQuality - maxMatchQuality * completionPriority / maxBestMatchCompletionPriority;
    Q_ASSERT(matchQuality >= 0);

    // KTextEditor considers a completion with matchQuality == 0 not suitable.
    // DeclarationItem::data() considers matchQuality == 0 invalid and does not return it.
    // Avoid zero special case by increasing matchQuality 0 to 1.
    constexpr int minSuitableMatchQuality = 1;
    matchQuality = std::max(minSuitableMatchQuality, matchQuality);

    Q_ASSERT(matchQuality >= minSuitableMatchQuality);
    Q_ASSERT(matchQuality <= maxMatchQuality);
    return matchQuality;
}

int adjustPriorityForType(const AbstractType::Ptr& type, int completionPriority)
{
    const auto modifier = 4;
    if (type) {
        const auto whichType = type->whichType();
        if (whichType == AbstractType::TypePointer || whichType == AbstractType::TypeReference) {
            // Clang considers all pointers as similar, this is not what we want.
            completionPriority += modifier;
        } else if (whichType == AbstractType::TypeStructure) {
            // Clang considers all classes as similar too...
            completionPriority += modifier;
        } else if (whichType == AbstractType::TypeDelayed) {
            completionPriority += modifier;
        } else if (whichType == AbstractType::TypeAlias) {
            auto aliasedType = type.staticCast<TypeAliasType>();
            return adjustPriorityForType(aliasedType->type(), completionPriority);
        } else if (whichType == AbstractType::TypeFunction) {
            auto functionType = type.staticCast<FunctionType>();
            return adjustPriorityForType(functionType->returnType(), completionPriority);
        }
    } else {
        completionPriority += modifier;
    }

    return completionPriority;
}

/// Adjusts priority for the @p decl
int adjustPriorityForDeclaration(Declaration* decl, int completionPriority)
{
    if(completionPriority < CCP_LocalDeclarationSimiliar || completionPriority > CCP_SuperCompletion){
        return completionPriority;
    }

    return adjustPriorityForType(decl->abstractType(), completionPriority);
}

/**
 * @return Whether the declaration represented by identifier @p identifier qualifies as completion result
 *
 * For example, we don't want to offer SomeClass::SomeClass as completion item to the user
 * (otherwise we'd end up generating code such as 's.SomeClass();')
 */
bool isValidCompletionIdentifier(const QualifiedIdentifier& identifier)
{
    const int count = identifier.count();
    if (identifier.count() < 2) {
        return true;
    }

    const Identifier scope = identifier.at(count-2);
    const Identifier id = identifier.last();
    if (scope == id) {
        return false; // is constructor
    }
    const QString idString = id.toString();
    if (idString.startsWith(QLatin1Char('~')) && scope.toString() == QStringView{idString}.mid(1)) {
        return false; // is destructor
    }
    return true;
}

/**
 * @return Whether the declaration represented by identifier @p identifier qualifies as "special" completion result
 *
 * "Special" completion results are items that are likely not regularly used.
 *
 * Examples:
 * - 'SomeClass::operator=(const SomeClass&)'
 */
bool isValidSpecialCompletionIdentifier(const QualifiedIdentifier& identifier)
{
    if (identifier.count() < 2) {
        return false;
    }

    const Identifier id = identifier.last();
    const QString idString = id.toString();
    if (idString.startsWith(QLatin1String("operator="))) {
        return true; // is assignment operator
    }
    return false;
}

Declaration* findDeclaration(const QualifiedIdentifier& qid, const DUContextPointer& ctx, const CursorInRevision& position, QSet<Declaration*>& handled)
{
    Declaration* ret = nullptr;
    const auto& importedContexts = ctx->topContext()->importedParentContexts();
    auto visitor = [&](const IndexedDeclaration& indexedDeclaration) {
        // if the context is not included, then this match is not correct for our consideration
        // this fixes issues where we used to include matches from files that did not have
        // anything to do with the current TU, e.g. the main from a different file or stuff like that
        // it also reduces the chance of us picking up a function of the same name from somewhere else
        // also, this makes sure the context has the correct language and we don't get confused by stuff
        // from other language plugins
        if (std::none_of(importedContexts.begin(), importedContexts.end(),
                         [indexedDeclaration](const DUContext::Import& import) {
                             return import.topContextIndex() == indexedDeclaration.indexedTopContext().index();
                         })) {
            return PersistentSymbolTable::VisitorState::Continue;
        }

        auto declaration = indexedDeclaration.declaration();
        if (!declaration) {
            // Mitigate problems such as: Cannot load a top-context from file "/home/kfunk/.cache/kdevduchain/kdevelop-{foo}/topcontexts/6085"
            //  - the required language-support for handling ID 55 is probably not loaded
            qCWarning(KDEV_CLANG) << "Detected an invalid declaration for" << qid;

            return PersistentSymbolTable::VisitorState::Continue;
        }

        if (declaration->kind() == Declaration::Instance && !declaration->isFunctionDeclaration()) {
            return PersistentSymbolTable::VisitorState::Break;
        }
        if (!handled.contains(declaration)) {
            handled.insert(declaration);

            ret = declaration;
            return PersistentSymbolTable::VisitorState::Break;
        }

        return PersistentSymbolTable::VisitorState::Continue;
    };
    PersistentSymbolTable::self().visitDeclarations(qid, visitor);
    if (ret) {
        return ret;
    }

    const auto foundDeclarations = ctx->findDeclarations(qid, position);
    for (auto dec : foundDeclarations) {
        if (!handled.contains(dec)) {
            handled.insert(dec);
            return dec;
        }
    }

    return nullptr;
}

/// If any parent of this context is a class, the closest class declaration is returned, nullptr otherwise
Declaration* classDeclarationForContext(const DUContextPointer& context, const CursorInRevision& position)
{
    auto parent = context;
    while (parent) {
        if (parent->type() == DUContext::Class) {
            break;
        }

        if (auto owner = parent->owner()) {
            // Work-around for out-of-line methods. They have Helper context instead of Class context
            if (owner->context() && owner->context()->type() == DUContext::Helper) {
                auto qid = owner->qualifiedIdentifier();
                qid.pop();

                QSet<Declaration*> tmp;
                auto decl = findDeclaration(qid, context, position, tmp);

                if (decl && decl->internalContext() && decl->internalContext()->type() == DUContext::Class) {
                    parent = decl->internalContext();
                    break;
                }
            }
        }
        parent = parent->parentContext();
    }

    return parent ? parent->owner() : nullptr;
}

class LookAheadItemMatcher
{
public:
    explicit LookAheadItemMatcher(const TopDUContextPointer& ctx)
        : m_topContext(ctx)
        , m_enabled(ClangSettingsManager::self()->codeCompletionSettings().lookAhead)
    {}

    /// Adds all local declarations for @p declaration into possible look-ahead items.
    void addDeclarations(Declaration* declaration)
    {
        if (!m_enabled) {
            return;
        }

        if (declaration->kind() != Declaration::Instance) {
            return;
        }

        auto type = typeForDeclaration(declaration);
        auto identifiedType = dynamic_cast<const IdentifiedType*>(type.data());
        if (!identifiedType) {
            return;
        }

        addDeclarationsForType(identifiedType, declaration);
    }

    /// Add type for matching. This type'll be used for filtering look-ahead items
    /// Only items with @p type will be returned through @sa matchedItems
    /// @param matchQuality @p type's match quality
    void addMatchedType(const IndexedType& type, int matchQuality)
    {
        Q_ASSERT_X(matchQuality > 0, Q_FUNC_INFO, "The completion must be suitable.");
        if (!type.isValid()) {
            return;
        }
        auto& lookAheadMatchQuality = matchedTypeToMatchQuality[type];
        // Use the highest among match qualities associated with the type, because currently
        // supported look-ahead completions are as useful as CCP_LocalDeclaration, and the
        // corresponding match quality is unlikely to be lower than @p matchQuality.
        // The following statement correctly assigns @p matchQuality if @p type was just inserted into
        // matchedTypeToMatchQuality, because then lookAheadMatchQuality == int{} == 0 < matchQuality.
        lookAheadMatchQuality = std::max(lookAheadMatchQuality, matchQuality);
    }

    /// @return look-ahead items that math given types. @sa addMatchedType
    QList<CompletionTreeItemPointer> matchedItems() const
    {
        QList<CompletionTreeItemPointer> lookAheadItems;
        for (const auto& pair : std::as_const(possibleLookAheadDeclarations)) {
            auto decl = pair.first;
            if (const auto matchQuality = matchedTypeToMatchQuality.value(decl->indexedType())) {
                auto parent = pair.second;
                const QLatin1String access = (parent->abstractType()->whichType() == AbstractType::TypePointer)
                                 ? QLatin1String("->") : QLatin1String(".");
                const QString text = parent->identifier().toString() + access + decl->identifier().toString();
                auto item = new DeclarationItem(decl, text, {}, text);
                item->setMatchQuality(matchQuality);
                lookAheadItems.append(CompletionTreeItemPointer(item));
            }
        }

        return lookAheadItems;
    }

private:
    AbstractType::Ptr typeForDeclaration(const Declaration* decl)
    {
        return TypeUtils::targetType(decl->abstractType(), m_topContext.data());
    }

    void addDeclarationsForType(const IdentifiedType* identifiedType, Declaration* declaration)
    {
        if (auto typeDecl = identifiedType->declaration(m_topContext.data())) {
            if (dynamic_cast<ClassDeclaration*>(typeDecl->logicalDeclaration(m_topContext.data()))) {
                if (!typeDecl->internalContext()) {
                    return;
                }

                const auto& localDeclarations = typeDecl->internalContext()->localDeclarations();
                for (auto localDecl : localDeclarations) {
                    if(localDecl->identifier().isEmpty()){
                        continue;
                    }

                    if(auto classMember = dynamic_cast<ClassMemberDeclaration*>(localDecl)){
                        // TODO: Also add protected/private members if completion is inside this class context.
                        if(classMember->accessPolicy() != Declaration::Public){
                            continue;
                        }
                    }

                    if (!localDecl->abstractType()) {
                        continue;
                    }

                    if (localDecl->abstractType()->whichType() == AbstractType::TypeIntegral) {
                        if (auto integralType = declaration->abstractType().dynamicCast<IntegralType>()) {
                            if (integralType->dataType() == IntegralType::TypeVoid) {
                                continue;
                            }
                        }
                    }

                    possibleLookAheadDeclarations.insert({localDecl, declaration});
                }
            }
        }
    }

    // Declaration and it's context
    using DeclarationContext = QPair<Declaration*, Declaration*>;

    /// Types of declarations that look-ahead completion items can have and their match qualities
    QHash<IndexedType, int> matchedTypeToMatchQuality;

    // List of declarations that can be added to the Look Ahead group
    // Second declaration represents context
    QSet<DeclarationContext> possibleLookAheadDeclarations;

    TopDUContextPointer m_topContext;

    bool m_enabled;
};

struct MemberAccessReplacer : public QObject
{
    Q_OBJECT

public:
    enum Type {
        None,
        DotToArrow,
        ArrowToDot
    };
    Q_ENUM(Type)

public Q_SLOTS:
    void replaceCurrentAccess(MemberAccessReplacer::Type type)
    {
        if (auto document = ICore::self()->documentController()->activeDocument()) {
            if (auto textDocument = document->textDocument()) {
                auto activeView = document->activeTextView();
                if (!activeView) {
                    return;
                }

                auto cursor = activeView->cursorPosition();

                QString oldAccess, newAccess;
                if (type == ArrowToDot) {
                    oldAccess = QStringLiteral("->");
                    newAccess = QStringLiteral(".");
                } else {
                    oldAccess = QStringLiteral(".");
                    newAccess = QStringLiteral("->");
                }

                auto oldRange = KTextEditor::Range(cursor - KTextEditor::Cursor(0, oldAccess.length()), cursor);

                // This code needed for testReplaceMemberAccess test
                // Maybe we should do a similar thing for '->' to '.' direction, but this is not so important
                while (textDocument->text(oldRange) == QLatin1String(" ") && oldRange.start().column() >= 0) {
                    oldRange = KTextEditor::Range({oldRange.start().line(), oldRange.start().column() - 1},
                                                  {oldRange.end().line(), oldRange.end().column() - 1});
                }

                if (oldRange.start().column() >= 0 && textDocument->text(oldRange) == oldAccess) {
                    textDocument->replaceText(oldRange, newAccess);
                }
            }
        }
    }
};
static MemberAccessReplacer s_memberAccessReplacer;

bool areAllResultsOverloadCandidates(const CXCodeCompleteResults& results)
{
#if CINDEX_VERSION_MINOR >= 30
    return std::all_of(results.Results, results.Results + results.NumResults, [](const CXCompletionResult& result) {
        return result.CursorKind == CXCursor_OverloadCandidate;
    });
#else
    // CXCursor_OverloadCandidate is unavailable in this version, so we return true only if there are no results.
    return results.NumResults == 0;
#endif
}
} // namespace

ClangCodeCompletionContext::ClangCodeCompletionContext(const DUContextPointer& context,
                                                       const ParseSessionData::Ptr& sessionData,
                                                       const QUrl& url,
                                                       const KTextEditor::Cursor& position,
                                                       const QString& text,
                                                       const QString& followingText
                                                      )
    : CodeCompletionContext(context, text + followingText, CursorInRevision::castFromSimpleCursor(position), 0)
    , m_results(nullptr, clang_disposeCodeCompleteResults)
    , m_parseSessionData(sessionData)
{
    qRegisterMetaType<MemberAccessReplacer::Type>();
    const QByteArray file = url.toLocalFile().toUtf8();
    ParseSession session(m_parseSessionData);

    QVector<UnsavedFile> otherUnsavedFiles;
    {
        ForegroundLock lock;
        otherUnsavedFiles = ClangUtils::unsavedFiles();
    }
    QVector<CXUnsavedFile> allUnsaved;

    {
        const unsigned int completeOptions = clang_defaultCodeCompleteOptions();

        CXUnsavedFile unsaved;
        unsaved.Filename = file.constData();
        const QByteArray content = m_text.toUtf8();
        unsaved.Contents = content.constData();
        unsaved.Length = content.size();

        allUnsaved.reserve(otherUnsavedFiles.size() + 1);
        for (const auto& f : std::as_const(otherUnsavedFiles)) {
            allUnsaved.append(f.toClangApi());
        }
        allUnsaved.append(unsaved);

        m_results.reset(clang_codeCompleteAt(session.unit(), file.constData(),
                        position.line() + 1, position.column() + 1,
                        allUnsaved.data(), allUnsaved.size(),
                        completeOptions));

        if (!m_results) {
            qCWarning(KDEV_CLANG) << "Something went wrong during 'clang_codeCompleteAt' for file" << file;
            return;
        }

        auto numDiagnostics = clang_codeCompleteGetNumDiagnostics(m_results.get());
        for (uint i = 0; i < numDiagnostics; i++) {
            auto diagnostic = clang_codeCompleteGetDiagnostic(m_results.get(), i);
            auto diagnosticType = ClangDiagnosticEvaluator::diagnosticType(diagnostic);
            clang_disposeDiagnostic(diagnostic);
            if (diagnosticType == ClangDiagnosticEvaluator::ReplaceWithArrowProblem || diagnosticType == ClangDiagnosticEvaluator::ReplaceWithDotProblem) {
                MemberAccessReplacer::Type replacementType;
                if (diagnosticType == ClangDiagnosticEvaluator::ReplaceWithDotProblem) {
                    replacementType = MemberAccessReplacer::ArrowToDot;
                } else {
                    replacementType = MemberAccessReplacer::DotToArrow;
                }

                QMetaObject::invokeMethod(&s_memberAccessReplacer, "replaceCurrentAccess", Qt::QueuedConnection,
                                          Q_ARG(MemberAccessReplacer::Type, replacementType));

                m_valid = false;
                return;
            }
        }

        auto addMacros = ClangSettingsManager::self()->codeCompletionSettings().macros;
        if (!addMacros) {
            m_filters |= NoMacros;
        }
    }

    if (!m_results->NumResults) {
        const auto trimmedText = QStringView{text}.trimmed();
        if (trimmedText.endsWith(QLatin1Char('.'))) {
            // TODO: This shouldn't be needed if Clang provided diagnostic.
            // But it doesn't always do it, so let's try to manually determine whether '.' is used instead of '->'
            m_text = trimmedText.left(trimmedText.size() - 1) + QLatin1String("->");

            CXUnsavedFile unsaved;
            unsaved.Filename = file.constData();
            const QByteArray content = m_text.toUtf8();
            unsaved.Contents = content.constData();
            unsaved.Length = content.size();
            allUnsaved[allUnsaved.size() - 1] = unsaved;

            m_results.reset(clang_codeCompleteAt(session.unit(), file.constData(),
                                                 position.line() + 1, position.column() + 1 + 1,
                                                 allUnsaved.data(), allUnsaved.size(),
                                                 clang_defaultCodeCompleteOptions()));

            m_valid = false;

            // Overload candidate completions do not make sense after '->', but Clang proposes such a completion after
            // a digit sometimes. Ignore the overload candidate completions to prevent a wrong '.' to '->' replacement.
            if (!m_results || areAllResultsOverloadCandidates(*m_results)) {
                return; // do not replace '.' with '->', because there are no useful completions after '->'
            }

            const auto diagnostic = clang_codeCompleteGetDiagnostic(m_results.get(), 0);
            const ClangString str(clang_getDiagnosticCategoryText(diagnostic));

            // This is unfortunately not documented anywhere in clang
            const bool isParseIssue = (qstrcmp(str.c_str(), "Parse Issue") == 0);
            clang_disposeDiagnostic(diagnostic);

            // Do not perform subsitution if we get a parsing issue with the ->
            if (isParseIssue) {
                return;
            }

            QMetaObject::invokeMethod(&s_memberAccessReplacer, "replaceCurrentAccess", Qt::QueuedConnection,
                                      Q_ARG(MemberAccessReplacer::Type, MemberAccessReplacer::DotToArrow));

            return;
        }
    }

    // check 'isValidPosition' after parsing the new content
    auto clangFile = session.file(file);
    if (!isValidPosition(session.unit(), clangFile)) {
        m_valid = false;
        return;
    }

    m_completionHelper.computeCompletions(session, clangFile, position);
}

ClangCodeCompletionContext::~ClangCodeCompletionContext()
{
}

bool ClangCodeCompletionContext::isValidPosition(CXTranslationUnit unit, CXFile file) const
{
    if (isInsideComment(unit, file, m_position.castToSimpleCursor())) {
        clangDebug() << "Invalid completion context: Inside comment";
        return false;
    }
    return true;
}

QList<CompletionTreeItemPointer> ClangCodeCompletionContext::completionItems(bool& abort, bool /*fullCompletion*/)
{
    if (!m_valid || !m_duContext || !m_results) {
        return {};
    }

    const auto ctx = DUContextPointer(m_duContext->findContextAt(m_position));

    /// Normal completion items, such as 'void Foo::foo()'
    QList<CompletionTreeItemPointer> items;
    /// Stuff like 'Foo& Foo::operator=(const Foo&)', etc. Not regularly used by our users.
    QList<CompletionTreeItemPointer> specialItems;
    /// Macros from the current context
    QList<CompletionTreeItemPointer> macros;
    /// Builtins reported by Clang
    QList<CompletionTreeItemPointer> builtin;

    // two sets of handled declarations to prevent duplicates and make sure we show
    // all available overloads
    QSet<Declaration*> handled;
    // this is only used for the CXCursor_OverloadCandidate completion items
    QSet<Declaration*> overloadsHandled;

    LookAheadItemMatcher lookAheadMatcher(TopDUContextPointer(ctx->topContext()));

    // If ctx is/inside the Class context, this represents that context.
    const auto currentClassContext = classDeclarationForContext(ctx, m_position);

    // HACK: try to build a fallback parent ID from the USR
    //       otherwise we won't identify typedefed anon structs correctly :(
    auto parentFromUSR = [this]() -> QString {
        const auto containerUSR = ClangString(clang_codeCompleteGetContainerUSR(m_results.get())).toString();
        const auto lastAt = containerUSR.lastIndexOf(QLatin1Char('@'));
        if (lastAt <= 0 || containerUSR[lastAt - 1] != QLatin1Char('A')) // we use this hack only for _A_non stuff
            return {};

        return containerUSR.mid(lastAt + 1);
    };
    const auto fallbackParentFromUSR = parentFromUSR();

    clangDebug() << "Clang found" << m_results->NumResults << "completion results";

    for (uint i = 0; i < m_results->NumResults; ++i) {
        if (abort) {
            return {};
        }

        auto result = m_results->Results[i];
        #if CINDEX_VERSION_MINOR >= 30
        const bool isOverloadCandidate = result.CursorKind == CXCursor_OverloadCandidate;
        #else
        const bool isOverloadCandidate = false;
        #endif

        const auto availability = clang_getCompletionAvailability(result.CompletionString);
        if (availability == CXAvailability_NotAvailable) {
            continue;
        }

        const bool isMacroDefinition = result.CursorKind == CXCursor_MacroDefinition;
        if (isMacroDefinition && m_filters & NoMacros) {
            continue;
        }

        const bool isBuiltin = result.CursorKind == CXCursor_NotImplemented || isGccCompatibilityBuiltin(result);
        if (isBuiltin && m_filters & NoBuiltins) {
            continue;
        }

        const bool isDeclaration = !isMacroDefinition && !isBuiltin;
        if (isDeclaration && m_filters & NoDeclarations) {
            continue;
        }

        if (availability == CXAvailability_NotAccessible && (!isDeclaration || !currentClassContext)) {
            continue;
        }

        // the string that would be needed to type, usually the identifier of something. Also we use it as name for code completion declaration items.
        QString typed;
        // the return type of a function e.g.
        QString resultType;
        // the replacement text when an item gets executed
        QString replacement;

        QString arguments;

        ArgumentHintItem::CurrentArgumentRange argumentRange;
        //BEGIN function signature parsing
        // nesting depth of parentheses
        int parenDepth = 0;
        enum FunctionSignatureState {
            // not yet inside the function signature
            Before,
            // any token is part of the function signature now
            Inside,
            // finished parsing the function signature
            After
        };
        // current state
        FunctionSignatureState signatureState = Before;
        //END function signature parsing

        const auto processChunks = [&](CXCompletionString completionString, const auto& self) -> void {
            const uint chunks = clang_getNumCompletionChunks(completionString);
            for (uint j = 0; j < chunks; ++j) {
                const auto kind = clang_getCompletionChunkKind(completionString, j);
                if (kind == CXCompletionChunk_Optional) {
                    completionString = clang_getCompletionChunkCompletionString(completionString, j);
                    if (completionString) {
                        self(completionString, self);
                    }
                    continue;
                }

                // We don't need function signature for declaration items, we can get it directly from the declaration. Also adding the function signature to the "display" would break the "Detailed completion" option.
                if (isDeclaration && !typed.isEmpty()) {
                    // TODO: When parent context for CXCursor_OverloadCandidate is fixed remove this check
                    if (!isOverloadCandidate) {
                        break;
                    }
                }

                const QString string = ClangString(clang_getCompletionChunkText(completionString, j)).toString();

                switch (kind) {
                case CXCompletionChunk_TypedText:
                    typed = string;
                    replacement += string;
                    break;
                case CXCompletionChunk_ResultType:
                    resultType = string;
                    continue;
                case CXCompletionChunk_Placeholder:
                    if (signatureState == Inside) {
                        arguments += string;
                    }
                    continue;
                case CXCompletionChunk_LeftParen:
                    if (signatureState == Before && !parenDepth) {
                        signatureState = Inside;
                    }
                    parenDepth++;
                    break;
                case CXCompletionChunk_RightParen:
                    --parenDepth;
                    if (signatureState == Inside && !parenDepth) {
                        arguments += QLatin1Char(')');
                        signatureState = After;
                    }
                    break;
                case CXCompletionChunk_Text:
                    if (isOverloadCandidate) {
                        typed += string;
                    }
                    else if (result.CursorKind == CXCursor_EnumConstantDecl) {
                        replacement += string;
                    }
                    else if (result.CursorKind == CXCursor_EnumConstantDecl) {
                        replacement += string;
                    }
                    break;
                case CXCompletionChunk_CurrentParameter:
                    argumentRange.start = arguments.size();
                    argumentRange.end = string.size();
                    break;
                default:
                    break;
                }
                if (signatureState == Inside) {
                    arguments += string;
                }
            }
        };

        processChunks(result.CompletionString, processChunks);

        // we have our own implementation of an override helper
        // TODO: use the clang-provided one, if available
        if (typed.endsWith(QLatin1String(" override")))
            continue;

        // TODO: No closing paren if default parameters present
        if (isOverloadCandidate && !arguments.endsWith(QLatin1Char(')'))) {
            arguments += QLatin1Char(')');
        }
        // ellide text to the right for overly long result types (templates especially)
        elideStringRight(resultType, MAX_RETURN_TYPE_STRING_LENGTH);

        static const auto noIcon = QIcon(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                                QStringLiteral("kdevelop/pics/namespace.png")));

        if (isDeclaration) {
            const Identifier id(typed);
            QualifiedIdentifier qid;
            auto parent = ClangString(clang_getCompletionParent(result.CompletionString, nullptr)).toString();
            if (parent.isEmpty() && !fallbackParentFromUSR.isEmpty()) {
                parent = fallbackParentFromUSR;
            }
            if (!parent.isEmpty()) {
                qid = QualifiedIdentifier(parent);
            }
            qid.push(id);

            if (!isValidCompletionIdentifier(qid)) {
                continue;
            }

            if (isOverloadCandidate && resultType.isEmpty() && parent.isEmpty()) {
                // workaround: find constructor calls for non-namespaced classes
                // TODO: return the namespaced class as parent in libclang
                qid.push(id);
            }

            auto found = findDeclaration(qid, ctx, m_position, isOverloadCandidate ? overloadsHandled : handled);

            CompletionTreeItemPointer item;
            if (found) {
                // TODO: Bug in Clang: protected members from base classes not accessible in derived classes.
                if (availability == CXAvailability_NotAccessible) {
                    if (auto cl = dynamic_cast<ClassMemberDeclaration*>(found)) {
                        if (cl->accessPolicy() != Declaration::Protected) {
                            continue;
                        }

                        auto declarationClassContext = classDeclarationForContext(DUContextPointer(found->context()), m_position);

                        uint steps = 10;
                        auto inheriters = DUChainUtils::inheriters(declarationClassContext, steps);
                        if(!inheriters.contains(currentClassContext)){
                            continue;
                        }
                    } else {
                        continue;
                    }
                }

                DeclarationItem* declarationItem = nullptr;
                if (isOverloadCandidate) {
                    declarationItem = new ArgumentHintItem(found, resultType, typed, arguments, argumentRange);
                    declarationItem->setArgumentHintDepth(1);
                } else {
                    declarationItem = new DeclarationItem(found, typed, resultType, replacement);
                }

                const auto completionPriority =
                    adjustPriorityForDeclaration(found, clang_getCompletionPriority(result.CompletionString));
                const bool bestMatch = completionPriority <= maxBestMatchCompletionPriority;

                // don't set best match property for reserved identifiers, also prefer declarations from current file
                const bool isReserved = found->indexedIdentifier().identifier().isReserved();
                if (bestMatch && !isReserved) {
                    const auto matchQuality = matchQualityFromBestMatchCompletionPriority(completionPriority);
                    declarationItem->setMatchQuality(matchQuality);

                    // TODO: LibClang missing API to determine expected code completion type.
                    if (auto functionType = found->type<FunctionType>()) {
                        lookAheadMatcher.addMatchedType(IndexedType(functionType->returnType()), matchQuality);
                    }
                    lookAheadMatcher.addMatchedType(found->indexedType(), matchQuality);
                } else {
                    declarationItem->setInheritanceDepth(completionPriority);

                    lookAheadMatcher.addDeclarations(found);
                }
                if (isReserved) {
                    declarationItem->markAsUnimportant();
                }

                item = declarationItem;
            } else {
                if (isOverloadCandidate) {
                    // TODO: No parent context for CXCursor_OverloadCandidate items, hence qid is broken -> no declaration found
                    auto ahi = new ArgumentHintItem({}, resultType, typed, arguments, argumentRange);
                    ahi->setArgumentHintDepth(1);
                    item = ahi;
                } else {
                    // still, let's trust that Clang found something useful and put it into the completion result list
                    clangDebug() << "Could not find declaration for" << qid;
                    auto instance = new SimpleItem(typed + arguments, resultType, replacement, noIcon);
                    instance->markAsUnimportant();
                    item = CompletionTreeItemPointer(instance);
                }
            }

            if (isValidSpecialCompletionIdentifier(qid)) {
                // If it's a special completion identifier e.g. "operator=(const&)" and we don't have a declaration for it, don't add it into completion list, as this item is completely useless and pollutes the test case.
                // This happens e.g. for "class A{}; a.|".  At | we have "operator=(const A&)" as a special completion identifier without a declaration.
                if(item->declaration()){
                    specialItems.append(item);
                }
            } else {
                items.append(item);
            }
            continue;
        }

        if (isMacroDefinition) {
            // TODO: grouping of macros and built-in stuff
            const auto text = QString(typed + arguments);
            auto instance = new SimpleItem(text, resultType, replacement, noIcon);
            auto item = CompletionTreeItemPointer(instance);
            if ( text.startsWith(QLatin1Char('_')) ) {
                instance->markAsUnimportant();
            }
            macros.append(item);
        } else if (isBuiltin) {
            auto instance = new SimpleItem(typed, resultType, replacement, noIcon);
            auto item = CompletionTreeItemPointer(instance);
            builtin.append(item);
        } else {
            Q_UNREACHABLE();
        }
    }

    if (abort) {
        return {};
    }

    addImplementationHelperItems();
    addOverwritableItems();

    eventuallyAddGroup(i18n("Special"), 700, specialItems);
    eventuallyAddGroup(i18n("Look-ahead Matches"), 800, lookAheadMatcher.matchedItems());
    eventuallyAddGroup(i18n("Builtin"), 900, builtin);
    eventuallyAddGroup(i18n("Macros"), 1000, macros);
    return items;
}

void ClangCodeCompletionContext::eventuallyAddGroup(const QString& name, int priority,
                                                    const QList<CompletionTreeItemPointer>& items)
{
    if (items.isEmpty()) {
        return;
    }

    auto* node = new CompletionCustomGroupNode(name, priority);
    node->appendChildren(items);
    m_ungrouped << CompletionTreeElementPointer(node);
}

void ClangCodeCompletionContext::addOverwritableItems()
{
    const auto overrideList = m_completionHelper.overrides();
    if (overrideList.isEmpty()) {
        return;
    }

    QList<CompletionTreeItemPointer> overrides;
    QList<CompletionTreeItemPointer> overridesAbstract;
    for (const auto& info : overrideList) {
        QStringList params;
        params.reserve(info.params.size());
        for (const auto& param : info.params) {
            params << param.type + QLatin1Char(' ') + param.id;
        }
        QString nameAndParams = info.name + QLatin1Char('(') + params.join(QLatin1String(", ")) + QLatin1Char(')');
        if(info.isConst)
            nameAndParams = nameAndParams + QLatin1String(" const");
        if(info.isPureVirtual)
            nameAndParams = nameAndParams + QLatin1String(" = 0");

        auto item = CompletionTreeItemPointer(new OverrideItem(nameAndParams, info.returnType));
        if (info.isPureVirtual)
            overridesAbstract << item;
        else
            overrides << item;
    }
    eventuallyAddGroup(i18n("Abstract Override"), 0, overridesAbstract);
    eventuallyAddGroup(i18n("Virtual Override"), 0, overrides);
}

void ClangCodeCompletionContext::addImplementationHelperItems()
{
    const auto implementsList = m_completionHelper.implements();
    if (implementsList.isEmpty()) {
        return;
    }

    QList<CompletionTreeItemPointer> implements;
    implements.reserve(implementsList.size());
    for (const auto& info : implementsList) {
        implements << CompletionTreeItemPointer(new ImplementsItem(info));
    }
    eventuallyAddGroup(i18n("Implement Function"), 0, implements);
}


QList<CompletionTreeElementPointer> ClangCodeCompletionContext::ungroupedElements()
{
    return m_ungrouped;
}

ClangCodeCompletionContext::ContextFilters ClangCodeCompletionContext::filters() const
{
    return m_filters;
}

void ClangCodeCompletionContext::setFilters(const ClangCodeCompletionContext::ContextFilters& filters)
{
    m_filters = filters;
}

#include "context.moc"
