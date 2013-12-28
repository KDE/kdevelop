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
#include "sessionsmodel.h"

#include <QDeclarativeContext>
#include <QDeclarativeComponent>
#include <QDeclarativeError>
#include <QDebug>

#include <shell/core.h>
#include <shell/uicontroller.h>

#include <sublime/area.h>
#include <sublime/mainwindow.h>

#include <kdeclarative.h>
#include <KDebug>
#include <Plasma/Theme>
#include <qdeclarative.h>

using namespace KDevelop;

WelcomePageView::WelcomePageView(QWidget* parent)
    : QDeclarativeView(parent)
{
    qRegisterMetaType<QObject*>("KDevelop::IProjectController*");
    qRegisterMetaType<QObject*>("KDevelop::IPluginController*");
    qRegisterMetaType<QObject*>("PatchReviewPlugin*");
    qmlRegisterType<SessionsModel>("org.kdevelop.welcomepage", 4, 3, "SessionsModel");
    
    QPalette p = palette();
    p.setColor(QPalette::Active, QPalette::Text, QColor(Qt::black));
    setPalette(p);

    //setup kdeclarative library
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.initialize();
    //binds things like kconfig and icons
    kdeclarative.setupBindings();

    setResizeMode(QDeclarativeView::SizeRootObjectToView);

    UiHelper* helper = new UiHelper(this);
    rootContext()->setContextProperty("kdev", helper);
    rootContext()->setContextProperty("ICore", KDevelop::ICore::self());
    areaChanged(ICore::self()->uiController()->activeArea());

    setSource(QUrl("qrc:/qml/main.qml"));
    if(!errors().isEmpty()) {
        kWarning() << "welcomepage errors:" << errors();
    }
    areaChanged(Core::self()->uiControllerInternal()->activeSublimeWindow()->area());
    connect(Core::self()->uiControllerInternal()->activeSublimeWindow(), SIGNAL(areaChanged(Sublime::Area*)),
        this, SLOT(areaChanged(Sublime::Area*)));
}

void WelcomePageView::areaChanged(Sublime::Area* area)
{
    rootContext()->setContextProperty("area", area->objectName());
}

void trySetupWelcomePageView()
{
    WelcomePageView* v = new WelcomePageView;

    // make sure plasma component is available
    QDeclarativeComponent component(v->engine());
    component.setData("import org.kde.plasma.components 0.1\nimport QtQuick 1.0\nText { text: \"Hello world!\" }", QUrl());

    if (component.isError()) {
        kWarning() << "Welcome Page not supported. errors: " << component.errors();
        delete v;
    } else {
        Core::self()->uiControllerInternal()->activeSublimeWindow()->setBackgroundCentralWidget(v);
    }
}
