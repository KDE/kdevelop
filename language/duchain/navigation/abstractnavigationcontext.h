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

#ifndef ABSTRACTNAVIGATIONCONTEXT_H
#define ABSTRACTNAVIGATIONCONTEXT_H

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
  Colorizer(const QString& color, bool bold=false, bool italic=false) : m_color(color), m_bold(bold), m_italic(italic) {
  }

  QString operator()(const QString& str) const
  {
    QString ret = "<font color=\"#" + m_color + "\">" + str + "</font>";
    if( m_bold )
      ret = "<b>"+ret+"</b>";

    if( m_italic )
      ret = "<i>"+ret+"</i>";
    return ret;
  }

  QString m_color;
  bool m_bold, m_italic;
};

class AbstractNavigationContext;
typedef KSharedPtr<AbstractNavigationContext> NavigationContextPointer;

class KDEVPLATFORMLANGUAGE_EXPORT AbstractNavigationContext : public KShared
{
  public:
    AbstractNavigationContext( KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext = 0 );

    virtual ~AbstractNavigationContext() {
    }

    void nextLink();
    void previousLink();
    void setPrefixSuffix( const QString& prefix, const QString& suffix );
    NavigationContextPointer accept();
    NavigationContextPointer acceptLink(const QString& link);
    NavigationAction currentAction() const;

    virtual QString name() const = 0;
    ///Here the context can return html to be displayed.
    virtual QString html(bool shorten = false);
    ///Here the context can return a widget to be displayed.
    ///The widget stays owned by this navigation-context.
    virtual QWidget* widget() const;

  protected:

    NavigationContextPointer execute(NavigationAction& action);

    void addExternalHtml( const QString& text );

    ///Creates and registers a link to the given declaration, labeled by the given name
    void makeLink( const QString& name, DeclarationPointer declaration, NavigationAction::Type actionType );

    ///Creates a link that executes the given action
    void makeLink( const QString& name, QString targetId, const NavigationAction& action);

    int m_selectedLink; //The link currently selected
    NavigationAction m_selectedLinkAction; //Target of the currently selected link

    virtual NavigationContextPointer registerChild(DeclarationPointer /*declaration*/) { return NavigationContextPointer(); };
    NavigationContextPointer registerChild( AbstractNavigationContext* context );
    QList<NavigationContextPointer> m_children; //Useed to keep alive all children until this is deleted

    //A counter used while building the html-code to count the used links.
    int m_linkCount;
    QString m_currentText; //Here the text is built
    QMap<QString, NavigationAction> m_links;
    QMap<int, NavigationAction> m_intLinks;
    AbstractNavigationContext* m_previousContext;
    QString m_prefix, m_suffix;
    KDevelop::TopDUContextPointer m_topContext;

    static QString declarationKind(DeclarationPointer decl);

    static const Colorizer errorHighlight;
    static const Colorizer labelHighlight;
    static const Colorizer codeHighlight;
    static const Colorizer propertyHighlight;
    static const Colorizer navigationHighlight;
    static const Colorizer importantHighlight;
    static const Colorizer commentHighlight;
    static const Colorizer nameHighlight;
};

}

#endif
