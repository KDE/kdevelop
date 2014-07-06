/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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

#include <kurl.h>

#include <language/languageexport.h>

#include "interfaces/isourceformatter.h"

namespace KTextEditor {
class Document;
}

namespace KDevelop {

class BasicRefactoring;
class IndexedString;
class ParseJob;
class ILanguage;
class TopDUContext;
class DocumentRange;
class SimpleCursor;
class SimpleRange;
class ICodeHighlighting;
class DocumentChangeTracker;
class ICreateClassHelper;

class KDEVPLATFORMLANGUAGE_EXPORT ILanguageSupport {
public:
    virtual ~ILanguageSupport() {}

    /** @return the name of the language.*/
    virtual QString name() const = 0;
    /** @return the parse job that is used by background parser to parse given @p url.*/
    virtual ParseJob *createParseJob(const IndexedString &url) = 0;
    /** @return the language for this support.*/
    virtual ILanguage *language();
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
     *  @return the standard context used by this language for the given @param url.
      * */
    virtual TopDUContext *standardContext(const KUrl& url, bool proxyContext = false);

    /**
      * Should return a code-highlighting instance for this language, or zero.
      */
    virtual ICodeHighlighting* codeHighlighting() const;

    /**
      * Should return a BasicRefactoring instance that controls the language-agnostic refactoring rules, or zero
      */
    virtual BasicRefactoring* refactoring() const;

    /**
     * Should return a document change-tracker for this language that tracks the changes in the given document 
     * */
    virtual DocumentChangeTracker* createChangeTrackerForDocument(KTextEditor::Document* document) const;

    /**
     * Should return a class creating helper for this language, or zero.
     *
     * If zero is returned, a default class helper will be created.
     * Reimplementing this method is therefore not necessary to have classes created in this language.
     * */
    virtual ICreateClassHelper* createClassHelper() const;

    /**
     * The following functions are used to allow navigation-features, tooltips, etc. for non-duchain language objects.
     * In C++, they are used to allow highlighting and navigation of macro-uses.
     * */

    /**Should return the local range within the given url that belongs to the
      *special language-object that contains @param position, or (KUrl(), SimpleRange:invalid()) */
    virtual SimpleRange specialLanguageObjectRange(const KUrl& url, const SimpleCursor& position);

    /**Should return the source-range and source-document that the
      *special language-object that contains @param position refers to, or SimpleRange:invalid(). */
    virtual QPair<KUrl, SimpleCursor> specialLanguageObjectJumpCursor(const KUrl& url, const SimpleCursor& position);
    
    /**Should return a navigation-widget for the
      *special language-object that contains @param position refers, or 0.
      *If you setProperty("DoNotCloseOnCursorMove", true) on the widget returned,
      *then the widget will not close when the cursor moves in the document, which
      *enables you to change the document contents from the widget without immediately closing the widget.*/
    virtual QWidget* specialLanguageObjectNavigationWidget(const KUrl& url, const SimpleCursor& position);
    
    /**Should return a tiny piece of code which makes it possible for KDevelop to derive the indentation
      *settings from an automatic source formatter. Example for C++: "class C{\n class D {\n void c() {\n int m;\n }\n }\n};\n"
      *The sample must be completely unindented (no line must start with leading whitespace),
      *and it must contain at least 4 indentation levels!
      *The default implementation returns an empty string.*/
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
    
    enum WhitespaceSensitivity {
        Insensitive = 0,
        IndentOnly = 1,
        Sensitive = 2
    };
    
    /**Specifies whether this language is sensitive to whitespace changes.
      * - The default "Insensitive" will only schedule a document for reparsing when
      *   a change in a non-whitespace area happens (non-whitespace chars added or whitespace
      *   added where it was surrounded by characters)
      * - "IndentOnly" will additionally schedule the document for reparsing if a whitespace
      *   change occurs at the beginning of the line (more exactly, if all characters before the
      *   changed ones are whitespace)
      * - "Sensitive" will always schedule the document for reparsing, no matter what was changed.
      */
    virtual WhitespaceSensitivity whitespaceSensititivy() const;
};

}

Q_DECLARE_INTERFACE( KDevelop::ILanguageSupport, "org.kdevelop.ILanguageSupport")

#endif
