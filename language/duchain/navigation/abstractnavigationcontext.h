/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_ABSTRACTNAVIGATIONCONTEXT_H
#define KDEVPLATFORM_ABSTRACTNAVIGATIONCONTEXT_H

#include <KDE/KSharedPtr>

#include "../../languageexport.h"
#include "../duchainpointer.h"
#include "navigationaction.h"
#include "../types/structuretype.h"



namespace KDevelop {

/** A helper-class for elegant colorization of html-strings .
 *
 * Initialize it with a html-color like "990000". and colorize strings
 * using operator()
 */
struct KDEVPLATFORMLANGUAGE_EXPORT Colorizer
{
  enum FormattingFlag {
    Nothing = 0x0,
    Bold = 0x1,
    Italic = 0x2,
    Fixed = 0x4
  };
  Q_DECLARE_FLAGS(Formatting, FormattingFlag)
  Colorizer(const QString& color, Formatting formatting = Nothing)
    : m_color(color), m_formatting(formatting)
  {
  }

  QString operator()(const QString& str) const;

  QString m_color;
  Formatting m_formatting;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Colorizer::Formatting);

class AbstractNavigationContext;
typedef KSharedPtr<AbstractNavigationContext> NavigationContextPointer;

class KDEVPLATFORMLANGUAGE_EXPORT AbstractNavigationContext : public KShared
{
  public:
    AbstractNavigationContext( KDevelop::TopDUContextPointer topContext = KDevelop::TopDUContextPointer(), AbstractNavigationContext* previousContext = 0 );

    virtual ~AbstractNavigationContext() {
    }

    void nextLink();
    void previousLink();
    void up();
    void down();
    void setPrefixSuffix( const QString& prefix, const QString& suffix );
    NavigationContextPointer accept();
    NavigationContextPointer back();
    NavigationContextPointer accept(IndexedDeclaration decl);
    NavigationContextPointer acceptLink(const QString& link);
    NavigationAction currentAction() const;

    virtual QString name() const = 0;
    ///Here the context can return html to be displayed.
    virtual QString html(bool shorten = false);
    ///Here the context can return a widget to be displayed.
    ///The widget stays owned by this navigation-context.
    ///The widget may have a signal "navigateDeclaration(KDevelop::IndexedDeclaration)".
    ///If that signal is emitted, the new declaration is navigated in the navigation-wdiget.
    virtual QWidget* widget() const;
    
    ///Whether the widget returned by widget() should take the maximum possible spsace.
    ///The default implementation returns true.
    virtual bool isWidgetMaximized() const;

    ///Returns whether this context's string has already been computed, and is up to date.
    ///After clear() was called, this returns false again.
    bool alreadyComputed() const;
    
    void setTopContext(TopDUContextPointer context);

    TopDUContextPointer topContext() const;

    NavigationContextPointer executeLink(QString link);

    NavigationContextPointer execute(const NavigationAction& action);

  protected:
    
    /// Returns the html font-size prefix (aka. <small> or similar) for the given mode
    QString fontSizePrefix(bool shorten) const;
    /// Returns the html font-size suffix (aka. <small> or similar) for the given mode
    QString fontSizeSuffix(bool shorten) const;
    
    virtual void setPreviousContext(AbstractNavigationContext* previous);
    
    struct TextHandler {
      TextHandler(AbstractNavigationContext* c) : context(c) {
      }
      void operator+=(const QString& str) const {
        context->addHtml(str);
      }
      AbstractNavigationContext* context;
    };
    
    ///Override this to execute own key-actions using NavigationAction
    virtual NavigationContextPointer executeKeyAction(QString key);

    ///Adds given the text to currentHtml()
    void addHtml(QString html);
    ///Returns the html text being built in its current state
    QString currentHtml() const;
    ///Returns a convenience object that allows writing "modifyHtml() += "Hallo";"
    TextHandler modifyHtml() {
      return TextHandler(this);
    }

    //Clears the computed html and links
    void clear();

    void addExternalHtml( const QString& text );

    ///Creates and registers a link to the given declaration, labeled by the given name
    virtual void makeLink( const QString& name, DeclarationPointer declaration, NavigationAction::Type actionType );

    ///Creates a link that executes the given action and adds it to the current context
    void makeLink( const QString& name, QString targetId, const NavigationAction& action);
    
    ///Creates a link that executes the given action and returns it
    QString createLink(const QString& name, QString targetId, const NavigationAction& action);

    int m_selectedLink; //The link currently selected
    NavigationAction m_selectedLinkAction; //Target of the currently selected link

    NavigationContextPointer registerChild(DeclarationPointer /*declaration*/);
    NavigationContextPointer registerChild( AbstractNavigationContext* context );
    QList<NavigationContextPointer> m_children; //Useed to keep alive all children until this is deleted

    bool m_shorten;

    int m_currentLine;

    //A counter used while building the html-code to count the used links.
    int m_linkCount;
    //Something else than -1 if the current position is represented by a line-number, not a link.
    int m_currentPositionLine;
    QMap<QString, NavigationAction> m_links;
    QMap<int, int> m_linkLines; //Holds the line for each link
    QMap<int, NavigationAction> m_intLinks;
    AbstractNavigationContext* m_previousContext;
    QString m_prefix, m_suffix;
    KDevelop::TopDUContextPointer m_topContext;

    virtual QString declarationKind(DeclarationPointer decl);

    static const Colorizer typeHighlight;
    static const Colorizer errorHighlight;
    static const Colorizer labelHighlight;
    static const Colorizer codeHighlight;
    static const Colorizer propertyHighlight;
    static const Colorizer navigationHighlight;
    static const Colorizer importantHighlight;
    static const Colorizer commentHighlight;
    static const Colorizer nameHighlight;
  private:
    QString m_currentText; //Here the text is built
};

}

#endif
