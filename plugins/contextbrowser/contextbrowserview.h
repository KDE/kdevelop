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

#ifndef CONTEXTBROWSERVIEW_H
#define CONTEXTBROWSERVIEW_H

#include <QWidget>
#include <QVector>
#include <language/duchain/topducontext.h>
#include <language/editor/simplecursor.h>
#include <language/editor/documentcursor.h>

class ContextBrowserPlugin;
class QVBoxLayout;
class QToolButton;
class QCheckBox;
class QMenu;

class ContextController;     // declared below
class DeclarationController; // declared below

class ContextBrowserView : public QWidget {
    Q_OBJECT
    public:
        ContextBrowserView( ContextBrowserPlugin* );
        ~ContextBrowserView();

        //duchain must be locked
        void setContext(KDevelop::DUContext* context);
        void setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext);
        void setSpecialNavigationWidget(QWidget*);
        void updateHistory(KDevelop::DUContext*, const KDevelop::SimpleCursor&);
        void updateMainWidget(QWidget*);

        /** Allows a single update of the view even if in locked state.
          * This is needed to browse while the locked button is checked. */
        void allowLockedUpdate();

    private Q_SLOTS:
        void updateLockIcon(bool); 

    private:
        bool isLocked() const;
        void resetWidget();

    private:
        ContextBrowserPlugin* m_plugin;
        ContextController* m_contextCtrl;
        DeclarationController* m_declarationCtrl;
        QVBoxLayout* m_layout;
        QToolButton* m_lockButton;
        QWidget* m_navigationWidget;
        bool m_allowLockedUpdate;
};

// handles Context related operations for ContextBrowserView
class ContextController : public QObject {
    Q_OBJECT
    public:
        ContextController(ContextBrowserView*);
        virtual ~ContextController();

        //duchain must be locked
        void updateHistory(KDevelop::DUContext* context, const
        KDevelop::SimpleCursor& cursorPosition);
        QWidget* createWidget(KDevelop::DUContext* context);

        QToolButton* previousButton() const;
        QToolButton* nextButton() const;
        QToolButton* resetButton() const;

        struct HistoryEntry {
            //Duchain must be locked
            HistoryEntry(KDevelop::IndexedDUContext ctx = KDevelop::IndexedDUContext(), const KDevelop::SimpleCursor& cursorPosition = KDevelop::SimpleCursor());
            //Duchain must be locked
            void setCursorPosition(const KDevelop::SimpleCursor& cursorPosition);

            //Duchain does not need to be locked
            KDevelop::DocumentCursor computePosition() const;

            KDevelop::IndexedDUContext context;
            KDevelop::DocumentCursor absoluteCursorPosition;
            KDevelop::SimpleCursor relativeCursorPosition; //Cursor position relative to the start line of the context
            QString alternativeString;
        };

    public Q_SLOTS:
        void historyNext();
        void historyPrevious();
        void nextMenuAboutToShow();
        void previousMenuAboutToShow();
        void actionTriggered();
        void resetHistory();

    private:
        bool isPreviousEntry(KDevelop::DUContext*, const KDevelop::SimpleCursor& cursor);
        QString actionTextFor(int historyIndex);
        void updateButtonState();
        void openDocument(int historyIndex);
        void fillHistoryPopup(QMenu* menu, const QList<int>& historyIndices);

    private:
        KDevelop::IndexedDUContext m_context;
        int m_nextHistoryIndex;
        
        QVector<HistoryEntry> m_history;
        QToolButton *m_previousButton;
        QToolButton *m_nextButton;
        QMenu* m_previousMenu, *m_nextMenu;
        ContextBrowserView* m_view;
        QToolButton *m_resetButton;
};

// Handles Declaration related operations for ContextBrowserView
class DeclarationController : public QWidget {
    Q_OBJECT
    public:
        DeclarationController();
        //duchain must be locked
        QWidget* createWidget(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext);

    private:
        KDevelop::DeclarationPointer m_declaration;
};

#endif
