/* This file is part of KDevelop
    Copyright 2011 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "welcomepageview.h"
#include "uihelper.h"
#include <QDeclarativeContext>
#include <QDebug>
#include <shell/core.h>
#include <shell/uicontroller.h>
#include <sublime/area.h>
#include <sublime/mainwindow.h>

WelcomePageView::WelcomePageView(QWidget* parent)
    : QDeclarativeView(parent)
{
    qRegisterMetaType<QObject*>("KDevelop::IProjectController*");
    connect(KDevelop::Core::self()->uiControllerInternal()->activeSublimeWindow(), SIGNAL(areaChanged(Sublime::Area*)), this, SLOT(areaChanged(Sublime::Area*)));
    
    setResizeMode(QDeclarativeView::SizeRootObjectToView);
    
    UiHelper* helper = new UiHelper(this);
    rootContext()->setContextProperty("kdev", helper);
    rootContext()->setContextProperty("ICore", KDevelop::ICore::self());
    rootContext()->setContextProperty("area", KDevelop::ICore::self()->uiController()->activeArea()->title());
    
    setSource(QUrl("qrc:/main.qml"));
}

void WelcomePageView::areaChanged(Sublime::Area* area)
{
    rootContext()->setContextProperty("area", area->title());
}
