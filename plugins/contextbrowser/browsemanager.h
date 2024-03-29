/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_BROWSEMANAGER_H
#define KDEVPLATFORM_PLUGIN_BROWSEMANAGER_H

#include <QCursor>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <KTextEditor/Cursor>

class QWidget;

namespace KTextEditor {
class View;
class Document;
}

namespace KDevelop {
class IDocument;
}

class EditorViewWatcher
    : public QObject
{
    Q_OBJECT

public:
    explicit EditorViewWatcher(QObject* parent = nullptr);
    QList<KTextEditor::View*> allViews();

private:
    ///Called for every added view. Reimplement this to catch them.
    virtual void viewAdded(KTextEditor::View*);

private Q_SLOTS:
    void viewDestroyed(QObject* view);
    void viewCreated(KTextEditor::Document*, KTextEditor::View*);
    void documentCreated(KDevelop::IDocument* document);

private:
    void addViewInternal(KTextEditor::View* view);
    QList<KTextEditor::View*> m_views;
};

class ContextBrowserPlugin;
class BrowseManager;

class Watcher
    : public EditorViewWatcher
{
    Q_OBJECT

public:
    explicit Watcher(BrowseManager* manager);
    void viewAdded(KTextEditor::View*) override;

private:
    BrowseManager* m_manager;
};

/**
 * Integrates the context-browser with the editor views, by listening for navigation events, and implementing html-like source browsing
 */

class BrowseManager
    : public QObject
{
    Q_OBJECT

public:
    explicit BrowseManager(ContextBrowserPlugin* controller);

    void viewAdded(KTextEditor::View* view);

    ///Installs/uninstalls the event-filter
    void applyEventFilter(QWidget* object, bool install);
Q_SIGNALS:
    ///Emitted when browsing was started using the magic-modifier
    void startDelayedBrowsing(KTextEditor::View* view);
    void stopDelayedBrowsing();
    void invokeAction(int index);

public Q_SLOTS:
    ///Enabled/disables the browsing mode
    void setBrowsing(bool);

private Q_SLOTS:
    void eventuallyStartDelayedBrowsing();

private:
    struct JumpLocation
    {
        QUrl url;
        KTextEditor::Cursor cursor;

        bool isValid() const { return url.isValid() && cursor.isValid(); }
    };

    void resetChangedCursor();
    JumpLocation determineJumpLoc(KTextEditor::Cursor textCursor, const QUrl& viewUrl) const;
    void setHandCursor(QWidget* widget);
    void avoidMenuAltFocus();
    bool eventFilter(QObject* watched, QEvent* event) override;
    ContextBrowserPlugin* m_plugin;
    bool m_browsing;
    int m_browsingByKey;     //Whether the browsing was started because of a key
    Watcher m_watcher;
    //Maps widgets to their previously set cursors
    QMap<QPointer<QWidget>, QCursor> m_oldCursors;
    QTimer* m_delayedBrowsingTimer;
    QPointer<KTextEditor::View> m_browsingStartedInView;
    KTextEditor::Cursor m_buttonPressPosition;
};

#endif
