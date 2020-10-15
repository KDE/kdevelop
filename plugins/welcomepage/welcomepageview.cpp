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

#include <KDeclarative/KDeclarative>

#include <QQmlContext>
#include <QQmlComponent>
#include <QQmlError>
#include <QDebug>

#include <shell/core.h>
#include <shell/uicontroller.h>

#include <sublime/area.h>
#include <sublime/mainwindow.h>
#include <interfaces/iprojectcontroller.h>

using namespace KDevelop;

WelcomePageWidget::WelcomePageWidget(const QList<IProject*> & /*projects*/, QWidget* parent)
    : QQuickWidget(parent)
{
    qRegisterMetaType<QObject*>("KDevelop::IProjectController*");
    qRegisterMetaType<QObject*>("KDevelop::IPluginController*");
    qRegisterMetaType<QObject*>("PatchReviewPlugin*");
    qRegisterMetaType<QAction*>();
    qmlRegisterType<SessionsModel>("org.kdevelop.welcomepage", 4, 3, "SessionsModel");

    //setup kdeclarative library
    KDeclarative::KDeclarative::setupEngine(engine());
    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.setTranslationDomain(QStringLiteral("kdevwelcomepage"));
    kdeclarative.setupContext();

    setResizeMode(QQuickWidget::SizeRootObjectToView);

    auto* helper = new UiHelper(this);
    rootContext()->setContextProperty(QStringLiteral("kdev"), helper);
    rootContext()->setContextProperty(QStringLiteral("ICore"), KDevelop::ICore::self());
    areaChanged(ICore::self()->uiController()->activeArea());

    setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if(!errors().isEmpty()) {
        qWarning() << "welcomepage errors:" << errors();
    }
    connect(Core::self()->uiControllerInternal()->activeSublimeWindow(), &Sublime::MainWindow::areaChanged,
        this, &WelcomePageWidget::areaChanged);
}

void WelcomePageWidget::areaChanged(Sublime::Area* area)
{
    rootContext()->setContextProperty(QStringLiteral("area"), area->objectName());
}
