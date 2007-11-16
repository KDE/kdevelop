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

#ifndef CPP_NAVIGATIONWIDGET_H
#define CPP_NAVIGATIONWIDGET_H

#include <QObject>
#include <QPointer>
#include <KSharedPtr>
#include <ktextbrowser.h>
#include <duchainpointer.h>
#include "includeitem.h"
#include <quickopendataprovider.h>

class QWidget;
class QTextBrowser;

namespace Cpp {
  class NavigationContext;
  typedef KSharedPtr<NavigationContext> NavigationContextPointer;

  /**
   * This class deleted itself when its part is deleted, so always use a QPointer when referencing it.
   * The duchain must be read-locked for most operations
   * */
  class NavigationWidget : public KTextBrowser, public KDevelop::QuickOpenEmbeddedWidgetInterface {
    Q_OBJECT
    public:
      /**
       * @param htmlPrefix and @param htmlSuffix can be used to add own text before/behind the
       * normal text in the navigation-widget.
       * The texts may contain links to files, using a simple special Syntax:
       * KDEV_FILE_LINK{File}. Every occurence of KDEV_FILE_LINK will be replaced.
       * */
      NavigationWidget(KDevelop::DeclarationPointer declaration, const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString());
      /**
       * @param htmlPrefix and @param htmlSuffix can be used to add own text before/behind the
       * normal text in the navigation-widget.
       * The texts may contain links to files, see above.
       * */
      NavigationWidget(const IncludeItem& includeItem, const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString());
      ~NavigationWidget();

      /**
       * Creates a compact html description-text
       * */
      static QString shortDescription(KDevelop::Declaration* declaration);
    
      static QString shortDescription(const IncludeItem& includeItem);

    public slots:
      ///Keyboard-action "next"
      virtual void next();
      ///Keyboard-action "previous"
      virtual void previous();
      ///Keyboard-action "accept"
      virtual void accept();
    private slots:
      void anchorClicked(const QUrl&);
    private:

      void initBrowser(int height);
      void update();
      
      void setContext(NavigationContextPointer context);

      NavigationContextPointer m_context, m_startContext;
      
      QPointer<QTextBrowser> m_browser;
      KDevelop::DeclarationPointer m_declaration;
  };
}

#endif
