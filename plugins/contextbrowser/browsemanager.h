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

#ifndef KDEVPLATFORM_PLUGIN_BROWSEMANAGER_H
#define KDEVPLATFORM_PLUGIN_BROWSEMANAGER_H

#include <QObject>
#include <QCursor>
#include <QMap>
#include <QPointer>
#include <qevent.h>
#include <ktexteditor/cursor.h>


class QWidget;

namespace KTextEditor {
    class View;
    class Document;
}

namespace KDevelop {
    class IDocument;
}

class EditorViewWatcher : QObject {
    Q_OBJECT
public:
    ///@param sameWindow If this is true, only views that are child of the same window as the given widget are registered
    EditorViewWatcher(QObject* parent = 0);
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
};

class ContextBrowserPlugin;

/**
 * Integrates the context-browser with the editor views, by listening for navigation events, and implementing html-like source browsing
 */

class BrowseManager : public QObject {
    Q_OBJECT
    public:
        BrowseManager(ContextBrowserPlugin* controller);
    Q_SIGNALS:
        //Emitted when browsing was started using the magic-modifier
        void startDelayedBrowsing(KTextEditor::View* view);
        void stopDelayedBrowsing();
    public slots:
        ///Enabled/disables the browsing mode
        void setBrowsing(bool);
    private slots:
        void eventuallyStartDelayedBrowsing();
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
        ContextBrowserPlugin* m_plugin;
        bool m_browsing;
        int m_browsingByKey; //Whether the browsing was started because of a key
        Watcher m_watcher;
        //Maps widgets to their previously set cursors
        QMap<QPointer<QWidget>, QCursor> m_oldCursors;
        QTimer* m_delayedBrowsingTimer;
        QPointer<KTextEditor::View> m_browingStartedInView;
        KTextEditor::Cursor m_buttonPressPosition;
};

#endif
