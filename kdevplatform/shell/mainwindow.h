/* This file is part of the KDevelop project
Copyright 2003 Falk Brettschneider <falkbr@kdevelop.org>
Copyright 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright 2003 Amilcar do Carmo Lucas <amilcar@kdevelop.org>
Copyright 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
Copyright 2006 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#ifndef KDEVPLATFORM_MAINWINDOW_H
#define KDEVPLATFORM_MAINWINDOW_H

#include <sublime/mainwindow.h>

#include "shellexport.h"

namespace KTextEditor {
class View;
class Document;
}

namespace KTextEditorIntegration {
class MainWindow;
}

namespace KDevelop
{

class IDocument;
class MainWindowPrivate;

/**
KDevelop main window.
Provides methods to control the main window of an application.
*/
class KDEVPLATFORMSHELL_EXPORT MainWindow: public Sublime::MainWindow {
    friend class UiController;
    Q_OBJECT
    Q_CLASSINFO( "D-Bus Interface", "org.kdevelop.MainWindow" )
public:
    explicit MainWindow(Sublime::Controller *parent = nullptr, Qt::WindowFlags flags = {});
    ~MainWindow() override;

    /*! @p status must implement KDevelop::IStatus */
    void registerStatus(QObject* status);
    KTextEditorIntegration::MainWindow *kateWrapper() const;
    void split(Qt::Orientation orientation);

public Q_SLOTS:
    /*! Shows an error message in the status bar.
        @p message The message
        @p timeout The timeout in milliseconds how long to show the message */
    void showErrorMessage(const QString& message, int timeout);

    virtual Q_SCRIPTABLE void ensureVisible();
    virtual Q_SCRIPTABLE QString windowTitle() {
        return Sublime::MainWindow::windowTitle();
    }

    void setVisible( bool visible ) override;
    void configureShortcuts();
    void loadSettings() override;

Q_SIGNALS:
    void finishedLoading();

protected:
    //FIXME DOCUMENT!!!  queryClose() must call all of the Core cleanup() methods!
    bool queryClose() override;
    //reimplemented from KXMLGUIBuilder to support visible menubar separators
    QAction *createCustomElement(QWidget *parent, int index, const QDomElement &element) override;

    virtual void initialize();
    virtual void cleanup();
    void initializeStatusBar() override;
    bool event( QEvent* ) override;
    void dragEnterEvent( QDragEnterEvent* ) override;
    void dropEvent( QDropEvent* ) override;
    void applyMainWindowSettings(const KConfigGroup& config) override;
    void createGUI(KParts::Part* part);

protected Q_SLOTS:
    void tabContextMenuRequested(Sublime::View* , QMenu* ) override;
    void tabToolTipRequested(Sublime::View* view, Sublime::Container* container, int tab) override;
    void dockBarContextMenuRequested(Qt::DockWidgetArea, const QPoint&) override;
    void newTabRequested() override;

private Q_SLOTS:
    void documentActivated(const QPointer<KTextEditor::Document>& textDocument);
    void updateCaption();

    void updateTabColor(IDocument* doc);
    void updateAllTabColors();

    void shortcutsChanged();

private:
    void initializeCorners();

private:
    MainWindowPrivate* d_ptr;
    Q_DECLARE_PRIVATE(MainWindow)
    friend class MainWindowPrivate;
};

}

#endif

