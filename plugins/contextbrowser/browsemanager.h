/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_BROWSEMANAGER_H
#define KDEVPLATFORM_PLUGIN_BROWSEMANAGER_H

#include <QObject>
#include <QPointer>
#include <QUrl>
#include <KTextEditor/Cursor>

#include <vector>

class QTimer;
class QWidget;

namespace KTextEditor {
class View;
}

namespace KDevelop {
class IDocument;
}
class ContextBrowserPlugin;

/**
 * Integrates the context-browser with the editor views, by listening for navigation events, and implementing html-like source browsing
 */

class BrowseManager
    : public QObject
{
    Q_OBJECT

public:
    explicit BrowseManager(ContextBrowserPlugin* controller);

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

    void documentAdded(KDevelop::IDocument* document);
    void viewAdded(KTextEditor::View* view);

    ContextBrowserPlugin* m_plugin;
    bool m_browsing;
    int m_browsingByKey;     //Whether the browsing was started because of a key
    /**
     * Contains widgets whose cursors @c *this temporarily replaced with Qt::PointingHandCursor.
     */
    std::vector<QPointer<QWidget>> m_widgetsWithReplacedCursors;
    QTimer* m_delayedBrowsingTimer;
    QPointer<KTextEditor::View> m_browsingStartedInView;
    KTextEditor::Cursor m_buttonPressPosition;
};

#endif
