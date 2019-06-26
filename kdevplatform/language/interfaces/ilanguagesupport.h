/***************************************************************************
*   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>                   *
*   Copyright 2014 Kevin Funk <kfunk@kde.org>                             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License as       *
*   published by the Free Software Foundation; either version 2 of the    *
*   License, or (at your option) any later version.                       *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
***************************************************************************/

#ifndef KDEVPLATFORM_ILANGUAGESUPPORT_H
#define KDEVPLATFORM_ILANGUAGESUPPORT_H

#include <QScopedPointer>
#include <QUrl>

#include <language/languageexport.h>

#include "interfaces/isourceformatter.h"

class QReadWriteLock;

namespace KTextEditor {
class Cursor;
class Range;
class Document;
}

namespace KDevelop {
class BasicRefactoring;
class IndexedString;
class ParseJob;
class TopDUContext;
class ICodeHighlighting;
class ICreateClassHelper;
class ILanguageSupportPrivate;

class KDEVPLATFORMLANGUAGE_EXPORT ILanguageSupport
{
public:
    ILanguageSupport();
    virtual ~ILanguageSupport();

    /** @return the name of the language.*/
    virtual QString name() const = 0;
    /** @return the parse job that is used by background parser to parse given @p url.*/
    virtual ParseJob* createParseJob(const IndexedString& url) = 0;
    /**
     * Only important for languages that can parse multiple different versions of a file, like C++ due to the preprocessor.
     * The default-implementation for other languages is "return DUChain::chainForDocument(url);"
     *
     * @param proxyContext      Whether the returned context should be a proxy-contexts. In C++, a proxy-contexts has no direct content.
     *                          It mainly just imports an actual content-context, and it holds all the imports. It can also represent
     *                          multiple different versions of the same content in the eyes of the preprocessor. Also, a proxy-context may contain the problem-
     *                          descriptions of preprocessor problems.
     *                          The proxy-context should be preferred whenever the problem-list is required, or for deciding whether a document needs to be updated
     *                          (only the proxy-context knows about all the dependencies, since it contains most of the imports)
     *
     *  @warning The DUChain must be locked before calling this, @see KDevelop::DUChainReadLocker
     *
     *  @return the standard context used by this language for the given @p url
     **/
    virtual TopDUContext* standardContext(const QUrl& url, bool proxyContext = false);

    /**
     * Should return a code-highlighting instance for this language, or zero.
     */
    virtual ICodeHighlighting* codeHighlighting() const;

    /**
     * Should return a BasicRefactoring instance that controls the language-agnostic refactoring rules, or zero
     */
    virtual BasicRefactoring* refactoring() const;

    /**
     * Should return a class creating helper for this language, or zero.
     *
     * If zero is returned, a default class helper will be created.
     * Reimplementing this method is therefore not necessary to have classes created in this language.
     * */
    virtual ICreateClassHelper* createClassHelper() const;

    /**
     * Every thread that does background-parsing should read-lock its language's parse-mutex while parsing.
     * Any other thread may write-lock the parse-mutex in order to wait for all parsing-threads to finish the parsing.
     * The parse-mutex only needs to be locked while working on the du-chain, not while preprocessing or reading.
     * Tip: use QReadLocker for read-locking.
     * The duchain must always be unlocked when you try to lock a parseLock!
     */
    virtual QReadWriteLock* parseLock() const;

    /**
     * The following functions are used to allow navigation-features, tooltips, etc. for non-duchain language objects.
     * In C++, they are used to allow highlighting and navigation of macro-uses.
     * */

    /**Should return the local range within the given url that belongs to the
     * special language-object that contains @p position , or (QUrl(), KTextEditor::Range:invalid()) */
    virtual KTextEditor::Range specialLanguageObjectRange(const QUrl& url, const KTextEditor::Cursor& position);

    /**Should return the source-range and source-document that the
     * special language-object that contains @p position refers to, or KTextEditor::Range:invalid(). */
    virtual QPair<QUrl, KTextEditor::Cursor> specialLanguageObjectJumpCursor(const QUrl& url,
                                                                             const KTextEditor::Cursor& position);

    /**Should return a navigation-widget for the
     * special language-object that contains @p position refers to as well as the range the object takes there,
     * or nullptr and an invalid range.
     * If you setProperty("DoNotCloseOnCursorMove", true) on the widget returned,
     * then the widget will not close when the cursor moves in the document, which
     * enables you to change the document contents from the widget without immediately closing the widget.*/
    virtual QPair<QWidget*, KTextEditor::Range> specialLanguageObjectNavigationWidget(const QUrl& url,
                                                                                      const KTextEditor::Cursor& position);

    /**Should return a tiny piece of code which makes it possible for KDevelop to derive the indentation
     * settings from an automatic source formatter. Example for C++: "class C{\n class D {\n void c() {\n int m;\n }\n }\n};\n"
     * The sample must be completely unindented (no line must start with leading whitespace),
     * and it must contain at least 4 indentation levels!
     * The default implementation returns an empty string.*/
    virtual QString indentationSample() const;

    /**
     * Can return a list of source formatting items for this language.
     * For example, if your language wants to use the CustomScript engine with
     * a specific executable, return an item with "customscript" as the engine
     * and a style describing your options as the style (in this case, especially
     * the command to execute in the "content" member).
     * Multiple items can be returned. Make sure to set the mime type(s) of your language
     * on the returned items.
     */
    virtual SourceFormatterItemList sourceFormatterItems() const;

    enum ReparseDelaySpecialValues {
        DefaultDelay = -1,
        NoUpdateRequired = -2
    };

    /**
     * @brief Enables the language to control how long the background parser waits until a changed document is reparsed.
     *
     * You can return DefaultDelay to use the default delay, or NoUpdateRequired to indicate that
     * this change does not require a re-parse at all.
     *
     * The default implementation returns DefaultDelay if the change was not whitespace-only,
     * and NoUpdateRequired otherwise.
     *
     * @param doc the document which was modified
     * @param changedRange the range which was modified
     * @param changedText the text which was inserted or removed
     * @param removal whether text was removed or inserted
     * @return int duration in ms to wait until re-parsing or a value of the ReparseDelaySpecialValues enum.
     */
    virtual int suggestedReparseDelayForChange(KTextEditor::Document* doc, const KTextEditor::Range& changedRange,
                                               const QString& changedText, bool removal) const;

private:
    const QScopedPointer<class ILanguageSupportPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ILanguageSupport)
};
}

Q_DECLARE_INTERFACE(KDevelop::ILanguageSupport, "org.kdevelop.ILanguageSupport")

#endif
