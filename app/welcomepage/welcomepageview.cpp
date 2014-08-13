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
#include "welcomepagedocument.h"

#include <QQmlContext>
#include <QQmlComponent>
#include <QQmlError>
#include <QDebug>

#include <shell/core.h>
#include <shell/uicontroller.h>

#include <sublime/area.h>
#include <sublime/mainwindow.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <KDeclarative/KDeclarative>
#include <qlabel.h>

using namespace KDevelop;

WelcomePageWidget::WelcomePageWidget(const QList<IProject*> & projects, QWidget* parent)
    : QQuickWidget(parent)
{
    qRegisterMetaType<QObject*>("KDevelop::IProjectController*");
    qRegisterMetaType<QObject*>("KDevelop::IPluginController*");
    qRegisterMetaType<QObject*>("PatchReviewPlugin*");
    qmlRegisterType<SessionsModel>("org.kdevelop.welcomepage", 4, 3, "SessionsModel");

    //setup kdeclarative library
    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.setupBindings();

    setResizeMode(QQuickWidget::SizeRootObjectToView);

    UiHelper* helper = new UiHelper(this);
    rootContext()->setContextProperty("kdev", helper);
    rootContext()->setContextProperty("ICore", KDevelop::ICore::self());
    areaChanged(ICore::self()->uiController()->activeArea());

    setSource(QUrl("qrc:/qml/main.qml"));
    if(!errors().isEmpty()) {
        qWarning() << "welcomepage errors:" << errors();
    }
    connect(Core::self()->uiControllerInternal()->activeSublimeWindow(), SIGNAL(areaChanged(Sublime::Area*)),
        this, SLOT(areaChanged(Sublime::Area*)));
}

void WelcomePageWidget::areaChanged(Sublime::Area* area)
{
    rootContext()->setContextProperty("area", area->objectName());
}

WelcomePageView::WelcomePageView(Sublime::Document* doc, Sublime::View::WidgetOwnership ws)
    : View(doc, ws)
{
}

QWidget* WelcomePageView::createWidget(QWidget* parent)
{
    return new WelcomePageWidget(QList<IProject*>(), parent);
}
