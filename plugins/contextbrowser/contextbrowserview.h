/*
  * This file is part of KDevelop
 *
 * Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_PLUGIN_CONTEXTBROWSERVIEW_H
#define KDEVPLATFORM_PLUGIN_CONTEXTBROWSERVIEW_H

#include <QWidget>
#include <QVector>
#include <QPointer>

#include <language/duchain/topducontext.h>
#include <language/editor/documentcursor.h>
#include <serialization/indexedstring.h>

class ContextBrowserPlugin;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QCheckBox;
class QMenu;
class KComboBox;

namespace KDevelop {
class IDocument;
}

class ContextBrowserView : public QWidget {
    Q_OBJECT
    public:
        ContextBrowserView( ContextBrowserPlugin*, QWidget* parent );
        ~ContextBrowserView();

        //duchain must be locked
        void setContext(KDevelop::DUContext* context);
        void setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext, bool force = false);
        void setSpecialNavigationWidget(QWidget*);
        void updateMainWidget(QWidget*);

        /** Allows a single update of the view even if in locked state.
          * This is needed to browse while the locked button is checked. */
        void allowLockedUpdate();

        ///Returns the currently locked declaration, or invalid of none is locked atm.
        KDevelop::IndexedDeclaration lockedDeclaration() const;

        void setNavigationWidget(QWidget* widget);
        
        QWidget* navigationWidget() {
            return m_navigationWidget;
        }
        
        //duchain must be locked
        QWidget* createWidget(KDevelop::DUContext* context);
        
        //duchain must be locked
        QWidget* createWidget(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext);

        KDevelop::IndexedDeclaration declaration() const;

        ///Returns whether the view is currently locked
        bool isLocked() const;
        
    private Q_SLOTS:
        void updateLockIcon(bool); 
        void declarationMenu();
        void navigationContextChanged(bool wasInitial, bool isInitial);

    private:
        virtual void showEvent(QShowEvent* event) override;
        virtual bool event(QEvent* event) override;
        
        virtual void focusInEvent(QFocusEvent* event) override;
        virtual void focusOutEvent(QFocusEvent* event) override;
        void resetWidget();

    private:
        
        KDevelop::IndexedDeclaration m_declaration;
        
        ContextBrowserPlugin* m_plugin;
        QVBoxLayout* m_layout;
        QToolButton* m_lockButton;
        QToolButton* m_declarationMenuButton;
        QHBoxLayout* m_buttons;

        QPointer<QWidget> m_navigationWidget;
        KDevelop::DeclarationId m_navigationWidgetDeclaration;
        bool m_allowLockedUpdate;
        KDevelop::IndexedTopDUContext m_lastUsedTopContext;

        KDevelop::IndexedDUContext m_context;
        // Whether the lock-button was activated automatically due to user navigation
        bool m_autoLocked;
};

#endif
