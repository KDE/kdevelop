/* This file is part of the KDevelop project
Copyright 2003 Falk Brettschneider <falkbr@kdevelop.org>
Copyright 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>
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
#ifndef KDEV_MAINWINDOW_H
#define KDEV_MAINWINDOW_H

#include <kxmlguiwindow.h>

#include "core.h"
#include <sublime/mainwindow.h>

#include "shellexport.h"

namespace KTextEditor { class View; }

namespace KDevelop
{

class Context;
class IDocument;

/**
KDevelop main window.
Provides methods to control the main window of an application.
*/
class KDEVPLATFORMSHELL_EXPORT MainWindow: public Sublime::MainWindow {
    friend class UiController;
    Q_OBJECT
public:
    explicit MainWindow( Sublime::Controller *parent = 0, Qt::WFlags flags = KDE_DEFAULT_WINDOWFLAGS );
    virtual ~MainWindow();

    //FIXME document this
    virtual void fillContextMenu( KMenu *menu, const Context *context );

    void setCurrentTextView(KTextEditor::View *view);
    KTextEditor::View *currentTextView();

public Q_SLOTS:
    virtual void setVisible( bool visible );

Q_SIGNALS:
    void finishedLoading();
    void contextMenu( KMenu *menu, const Context *context );

protected:
    //FIXME DOCUMENT!!!  queryClose() must call all of the Core cleanup() methods!
    virtual bool queryClose();

public Q_SLOTS:
    virtual void loadSettings();
    virtual void saveSettings();
protected:
    virtual void initialize();
    virtual void cleanup();

private Q_SLOTS:
    void documentActivated( KDevelop::IDocument* document );
    void documentStateChanged( KDevelop::IDocument* document );
    void documentClosed( KDevelop::IDocument* document );

private:
    class MainWindowPrivate *d;
    friend class MainWindowPrivate;
};

}

#endif

