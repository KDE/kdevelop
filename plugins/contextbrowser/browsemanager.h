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

#ifndef BROWSEMANAGER_H
#define BROWSEMANAGER_H

#include <QObject>
#include <QCursor>
#include <QMap>
#include <QPointer>
#include <qevent.h>

class QWidget;

namespace KTextEditor {
    class View;
    class Document;
}

namespace KDevelop {
    class IDocument;
}

struct ShiftPressDetector {
    public:
        ShiftPressDetector() : m_hadOtherKey(true) {
        }
        ///Must be called with all key-events
        ///Returns true if the shift-key was released and no other key was pressed between its press and release.
        bool checkKeyEvent(QKeyEvent* e) {
            if(e->type() == QEvent::KeyPress) {
                m_hadOtherKey = true;
                if (e->key() == Qt::Key_Shift && (e->modifiers() & (~Qt::ShiftModifier)) == 0)
                    m_hadOtherKey = false;
                
            }else if(e->type() == QEvent::KeyRelease) {
                if(e->key() == Qt::Key_Shift && !m_hadOtherKey)
                    return true;
            }
            
            return false;
        }
        void clear() {
            m_hadOtherKey = true;
        }
    private:
    bool m_hadOtherKey;
};

class EditorViewWatcher : QObject {
    Q_OBJECT
    public:
    ///@param sameWindow If this is true, only views that are child of the same window as the given widget are registered
    EditorViewWatcher(QWidget* sameWindow = 0);
    QList<KTextEditor::View*> allViews();
    private:
    ///Called for every added view. Reimplement this to catch them.
    virtual void viewAdded(KTextEditor::View*);
    
    private slots:
    void viewDestroyed(QObject* view);
    void viewCreated(KTextEditor::Document*, KTextEditor::View*);
    void documentCreated( KDevelop::IDocument* document );
    private:
    void addViewInternal(KTextEditor::View* view);
    QList<KTextEditor::View*> m_views;
    QWidget* m_childrenOf;
};

class ContextController;

class BrowseManager : public QObject {
    Q_OBJECT
    public:
        BrowseManager(ContextController* controller);
    Q_SIGNALS:
        //Emitted whenever the shift-key has been pressed + released without any other key in between
        void shiftKeyTriggered();
    public slots:
        ///Enabled/disables the browsing mode
        void setBrowsing(bool);
        void resetSiftDetector();
    private:
        void viewAdded(KTextEditor::View* view);
        class Watcher : public EditorViewWatcher {
            public:
            Watcher(BrowseManager* manager);
            virtual void viewAdded(KTextEditor::View*);
            private:
            BrowseManager* m_manager;
        };
        
        void resetChangedCursor();
        void setHandCursor(QWidget* widget);
        
        //Installs/uninstalls the event-filter
        void applyEventFilter(QWidget* object, bool install);
        virtual bool eventFilter(QObject * watched, QEvent * event) ;
        ContextController* m_controller;
        bool m_browsing;
        bool m_browsingByKey; //Whether the browsing was started because of a key
        Watcher m_watcher;
        //Maps widgets to their previously set cursors
        QMap<QPointer<QWidget>, QCursor> m_oldCursors;
        ShiftPressDetector m_shiftDetector;
};

#endif
