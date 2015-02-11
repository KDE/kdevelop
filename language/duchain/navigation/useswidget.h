/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_USESWIDGET_H
#define KDEVPLATFORM_USESWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QSharedPointer>

#include <language/duchain/indexeddeclaration.h>
#include <language/duchain/indexedtopducontext.h>
#include <language/languageexport.h>
#include "usescollector.h"
#include <language/editor/persistentmovingrange.h>

class QLabel;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class QBoxLayout;
class QPushButton;
class QProgressBar;

namespace KDevelop {
    class CodeRepresentation;
  
    class IndexedDeclaration;
    ///A widget representing one use of a Declaration in a speicific context
    class KDEVPLATFORMLANGUAGE_EXPORT OneUseWidget : public QWidget {
      Q_OBJECT
      public:
        OneUseWidget(IndexedDeclaration declaration, IndexedString document, KTextEditor::Range range, const CodeRepresentation& code);
        ~OneUseWidget();
        
      private slots:
        void jumpTo();
      private:
        virtual void resizeEvent ( QResizeEvent * event ) override;
        
        PersistentMovingRange::Ptr m_range;
        IndexedDeclaration m_declaration;
        IndexedString m_document;
        QString m_sourceLine;
        
        QLabel* m_label;
        QLabel* m_icon;
        QHBoxLayout* m_layout;
    };
    
    
    class KDEVPLATFORMLANGUAGE_EXPORT NavigatableWidgetList : public QScrollArea {
      Q_OBJECT
      public:
        NavigatableWidgetList(bool allowScrolling = false, uint maxHeight = 0, bool vertical = true);
        ~NavigatableWidgetList();
        void addItem(QWidget* widget, int pos = -1);
        void addHeaderItem(QWidget* widget, Qt::Alignment alignment = 0);
        ///Whether items were added to this list using addItem(..)
        bool hasItems() const;
        ///Deletes all items that were added using addItem
        void deleteItems();
        QList<QWidget*> items() const;
        void setShowHeader(bool show);
      protected:
        QBoxLayout* m_itemLayout;
        QVBoxLayout* m_layout;
      private:
        QHBoxLayout* m_headerLayout;
        QToolButton *m_previousButton, *m_nextButton;
        bool m_allowScrolling, m_useArrows;
    };
    
    class KDEVPLATFORMLANGUAGE_EXPORT ContextUsesWidget : public NavigatableWidgetList {
      Q_OBJECT
      public:
        ContextUsesWidget(const CodeRepresentation& code, QList<IndexedDeclaration> usedDeclaration, IndexedDUContext context);
      Q_SIGNALS:
        void navigateDeclaration(KDevelop::IndexedDeclaration);
      private Q_SLOTS:
        void linkWasActivated(QString);
      private:
        IndexedDUContext m_context;
    };

    class KDEVPLATFORMLANGUAGE_EXPORT DeclarationWidget : public NavigatableWidgetList {
      Q_OBJECT
      public:
        DeclarationWidget(const KDevelop::CodeRepresentation& code, const KDevelop::IndexedDeclaration& declaration);
    };
    
    /**
     * Represents the uses of a declaration within one top-context
     */
    class KDEVPLATFORMLANGUAGE_EXPORT TopContextUsesWidget : public NavigatableWidgetList {
        Q_OBJECT
        public:
          TopContextUsesWidget(IndexedDeclaration declaration, QList<IndexedDeclaration> localDeclarations, IndexedTopDUContext topContext);
          void setExpanded(bool);
          int usesCount() const;
        private slots:
            void labelClicked();
        private:
            IndexedTopDUContext m_topContext;
            IndexedDeclaration m_declaration;
            QLabel* m_icon;
            QLabel* m_toggleButton;
            QList<IndexedDeclaration> m_allDeclarations;
            int m_usesCount;
    };

    /**
     * A widget that allows browsing through all the uses of a declaration, and also through all declarations of it.
     */
    class KDEVPLATFORMLANGUAGE_EXPORT UsesWidget : public NavigatableWidgetList {
      Q_OBJECT
        public:
            ///This class can be overridden to do additional processing while the uses-widget shows the uses.
            struct KDEVPLATFORMLANGUAGE_EXPORT UsesWidgetCollector : public UsesCollector {
              public:
              void setWidget(UsesWidget* widget );
              UsesWidgetCollector(IndexedDeclaration decl);
              virtual void processUses(KDevelop::ReferencedTopDUContext topContext) override;
              virtual void maximumProgress(uint max) override;
              virtual void progress(uint processed, uint total) override;
              UsesWidget* m_widget;
            };
            virtual QSize sizeHint () const override;
            ///@param customCollector allows specifying an own subclass of UsesWidgetCollector.
            UsesWidget(const IndexedDeclaration& declaration, QSharedPointer<UsesWidgetCollector> customCollector = {});
            ~UsesWidget();
            void setAllExpanded(bool expanded);
            unsigned int countAllUses() const;
        Q_SIGNALS:
            void navigateDeclaration(KDevelop::IndexedDeclaration);
        private:
            const QString headerLineText() const;
            QLabel* m_headerLine;
            QSharedPointer<UsesWidgetCollector> m_collector;
            QProgressBar* m_progressBar;
        public slots:
            void headerLinkActivated(QString linkName);
            void redrawHeaderLine();
    };
}

#endif
