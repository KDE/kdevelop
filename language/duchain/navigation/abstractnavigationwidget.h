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

#ifndef KDEVPLATFORM_ABSTRACTNAVIGATIONWIDGET_H
#define KDEVPLATFORM_ABSTRACTNAVIGATIONWIDGET_H


#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QWidget>

#include "../../interfaces/quickopendataprovider.h"
#include <language/languageexport.h>
#include "abstractnavigationcontext.h"

class QWidget;
class QTextBrowser;

namespace KDevelop {
  /**
   * This class deleted itself when its part is deleted, so always use a QPointer when referencing it.
   * The duchain must be read-locked for most operations
   * */
  class KDEVPLATFORMLANGUAGE_EXPORT AbstractNavigationWidget : public QWidget, public QuickOpenEmbeddedWidgetInterface
  {
    Q_OBJECT
    public:
      AbstractNavigationWidget();

      virtual ~AbstractNavigationWidget();

      void setContext(NavigationContextPointer context, int initBrowser = 400);
      
      QSize sizeHint() const;
      
    public slots:
      ///Keyboard-action "next"
      virtual void next();
      ///Keyboard-action "previous"
      virtual void previous();
      ///Keyboard-action "accept"
      virtual void accept();
      virtual void up();
      virtual void down();
      virtual void back();

      ///These are temporarily for gettings these events directly from kate
      ///@todo Do this through a public interface post 4.2
      void embeddedWidgetRight();
      ///Keyboard-action "previous"
      void embeddedWidgetLeft();
      ///Keyboard-action "accept"
      void embeddedWidgetAccept();
      void embeddedWidgetUp();
      void embeddedWidgetDown();
      
      NavigationContextPointer context();

    Q_SIGNALS:
      void sizeHintChanged();
      /// Emitted whenever the current navigation-context has changed
      /// @param wasInitial whether the previous context was the initial context
      /// @param isInitial whether the current context is the initial context
      void contextChanged(bool wasInitial, bool isInitial);
    public slots:
      void navigateDeclaration(KDevelop::IndexedDeclaration decl);
    private slots:
      void anchorClicked(const QUrl&);
    protected:
      virtual void wheelEvent(QWheelEvent* );
      void updateIdealSize() const;

      void initBrowser(int height);
      void update();

      NavigationContextPointer m_startContext;

      TopDUContextPointer m_topContext;

      QPointer<QTextBrowser> m_browser;
      QWidget* m_currentWidget;
      QString m_currentText;
      mutable QSize m_idealTextSize;

    private:
      NavigationContextPointer m_context;
  };
}


#endif
