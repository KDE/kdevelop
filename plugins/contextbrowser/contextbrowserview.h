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
#include <duchainpointer.h>
#include <simplecursor.h>
#include <documentcursor.h>

class ContextBrowserPlugin;
class QSplitter;
class QGridLayout;
class QPushButton;
class QMenu;

//This widget represents one single context
class ContextWidget : public QWidget {
    Q_OBJECT
    public:
        ContextWidget();
        //duchain must be locked
        void setContext(KDevelop::DUContext* context, const KDevelop::SimpleCursor& cursorPosition, bool noHistory = false);
    public Q_SLOTS:
        void historyNext();
        void historyPrevious();
        void nextMenuAboutToShow();
        void previousMenuAboutToShow();
        void actionTriggered();
    private:
        
        KDevelop::DUContextPointer m_context;
        QGridLayout *m_layout;
        QWidget* m_navigationWidget;
        int m_nextHistoryIndex;
        
        struct HistoryEntry {
            //Duchain must be locked
            HistoryEntry(KDevelop::DUContextPointer ctx = KDevelop::DUContextPointer(), const KDevelop::SimpleCursor& cursorPosition = KDevelop::SimpleCursor());
            //Duchain must be locked
            void setCursorPosition(const KDevelop::SimpleCursor& cursorPosition);
            
            //Duchain does not need to be locked
            KDevelop::DocumentCursor computePosition();

            KDevelop::DUContextPointer context;
            KDevelop::DocumentCursor absoluteCursorPosition;
            KDevelop::SimpleCursor relativeCursorPosition; //Cursor position relative to the start line of the context
        };
        
        QVector<HistoryEntry> m_history;
        QPushButton *m_previousButton, *m_nextButton;
        QMenu* m_previousMenu, *m_nextMenu;
};

//This widget represents one single context
class DeclarationWidget : public QWidget {
    Q_OBJECT
    public:
        DeclarationWidget();
        //duchain must be locked
        void setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext);
        
        //For use with special language objects, like macros in c++
        //duchain must be locked
        void setSpecialNavigationWidget(QWidget* widget);
    private:
        KDevelop::DeclarationPointer m_declaration;
        QWidget* m_navigationWidget;
        QGridLayout *m_layout;
};

class ContextBrowserView : public QWidget {
    Q_OBJECT
    public:
        ContextBrowserView( ContextBrowserPlugin* );
        ~ContextBrowserView();
        
        DeclarationWidget* declarationWidget();
        ContextWidget* contextWidget();
    private:
        virtual void resizeEvent ( QResizeEvent * event );
        ContextBrowserPlugin* m_plugin;
        QSplitter* m_splitter;
        
        ContextWidget* m_contextWidget;
        DeclarationWidget* m_declarationWidget;
};

#endif
