/*
    SPDX-FileCopyrightText: 2002 Falk Brettschneider <falkbr@kdevelop.org>
    SPDX-FileCopyrightText: 2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006, 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_MAINWINDOW_PRIVATE_H
#define KDEVPLATFORM_MAINWINDOW_PRIVATE_H

#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QMap>
#include <KXMLGUIClient>

#include <language/util/navigationtooltip.h>

class KActionCollection;
class QMenu;

namespace Sublime {
class View;
class Container;
}

namespace KParts {
class Part;
}

namespace KTextEditor {
class View;
}

namespace KTextEditorIntegration {
class MainWindow;
}

namespace KDevelop {

class IPlugin;
class MainWindow;
class StatusBar;

class MainWindowPrivate: public QObject {
    Q_OBJECT
public:
    explicit MainWindowPrivate(MainWindow *mainWindow);
    ~MainWindowPrivate() override;

    QPointer<QWidget> centralPlugin;
    QMetaObject::Connection activeDocumentReadWriteConnection;

    void setupActions();
    void setupGui();
    void setupStatusBar();
    void registerStatus(QObject*);

    void tabContextMenuRequested(Sublime::View *view, QMenu* menu);
    void tabToolTipRequested(Sublime::View* view, Sublime::Container* container, int tab);
    void dockBarContextMenuRequested(Qt::DockWidgetArea area, const QPoint& position);

public Q_SLOTS:
    void addPlugin( KDevelop::IPlugin *plugin );
    void removePlugin( KDevelop::IPlugin *plugin );
    void updateSourceFormatterGuiClient(bool hasFormatters);

    void activePartChanged(KParts::Part *part);
    void mergeView(Sublime::View *view);
    void changeActiveView(Sublime::View *view);
    void xmlguiclientDestroyed(QObject* obj);

    //actions
    void fileNew();

    void gotoNextWindow();
    void gotoPreviousWindow();

    void selectPrevItem();
    void selectNextItem();

    void viewAddNewToolView();

    void newWindow();
    void splitHorizontal();
    void splitVertical();
    void split(Qt::Orientation orientation);
    void toggleFullScreen(bool fullScreen);

    void gotoNextSplit();
    void gotoPreviousSplit();

    void settingsDialog();

    void quitAll();

//    void fixToolbar();

    ///Returns true if we're currently changing the active view through changeActiveView()
    bool changingActiveView() const ;

    void configureNotifications();
    void showLoadedPlugins();

    void toggleArea(bool b);
    void showErrorMessage(const QString& message, int timeout);
    void pluginDestroyed(QObject*);

    /// the following slots always activate the m_tabView before calling the normal slot above
    /// @see m_tabView
    /// @see tabContextMenuRequested
    void contextMenuFileNew();
    void contextMenuSplitHorizontal();
    void contextMenuSplitVertical();

    /// reload all open documents
    void reloadAll();

    KTextEditorIntegration::MainWindow *kateWrapper() const;

private:
    KActionCollection *actionCollection();

    MainWindow* const m_mainWindow;
    StatusBar* m_statusBar;
    QWidget* lastXMLGUIClientView;
    QPointer<QWidget> m_workingSetCornerWidget;

    QMap<IPlugin*, KXMLGUIClient*> m_pluginCustomClients;

    bool m_changingActiveView;
    /// the view of the tab that got it's context menu connected
    Sublime::View* m_tabView;
    QPair<Sublime::View*, QPointer<NavigationToolTip> > m_tabTooltip;
    KTextEditorIntegration::MainWindow* m_kateWrapper;
};

}

#endif

