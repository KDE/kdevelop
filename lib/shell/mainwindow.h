/* This file is part of the KDevelop project
Copyright (C) 2003 F@lk Brettschneider <falkbr@kdevelop.org>
Copyright (C) 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>
Copyright (C) 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#include <kmainwindow.h>

#include "core.h"
#include <sublime/mainwindow.h>

#include "kdevexport.h"

namespace KDevelop
{

class Context;

class IPlugin;
class Document;

/**
KDevelop main window.
Provides methods to control the main window of an application.
*/
class KDEVPLATFORM_EXPORT MainWindow: public Sublime::MainWindow {
    friend class UiController;
    Q_OBJECT
public:
    MainWindow( Sublime::Controller *parent = 0, Qt::WFlags flags = KDE_DEFAULT_WINDOWFLAGS );
    virtual ~MainWindow();

    //FIXME document this
    virtual void fillContextMenu( KMenu *menu, const Context *context );


public Q_SLOTS:
    virtual void setVisible( bool visible );

Q_SIGNALS:
    void finishedLoading();
    void contextMenu( KMenu *menu, const Context *context );

protected:
    //FIXME DOCUMENT!!!  queryClose() must call all of the Core cleanup() methods!
    virtual bool queryClose();

protected slots:
    virtual void loadSettings();
    virtual void saveSettings();
protected:
    virtual void initialize();
    virtual void cleanup();

private:
    class MainWindowPrivate *d;
    friend class MainWindowPrivate;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
