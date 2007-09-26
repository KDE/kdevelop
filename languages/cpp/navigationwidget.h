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
#include <duchainpointer.h>
#include "includeitem.h"

class QWidget;
class QTextBrowser;

namespace Cpp {
  class NavigationContext;
  typedef KSharedPtr<NavigationContext> NavigationContextPointer;

  /**
   * This class deleted itself when its part is deleted, so always use a QPointer when referencing it.
   * The duchain must be read-locked for most operations
   * */
  class NavigationWidget : public QObject {
    Q_OBJECT
    public:
      NavigationWidget(KDevelop::DeclarationPointer declaration);
      NavigationWidget(const IncludeItem& includeItem);
      ~NavigationWidget();
      QWidget* view() const;

      void next();
      void previous();
      void accept();

      /**
       * Creates a compact html description-text
       * */
      static QString shortDescription(KDevelop::Declaration* declaration);
    
      static QString shortDescription(const IncludeItem& includeItem);
      
    private slots:
      void anchorClicked(const QUrl&);
      void targetDestroyed(QObject*);
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
