/*
    SPDX-FileCopyrightText: 2011 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "welcomepageview.h"

#include "uihelper.h"
#include "sessionsmodel.h"

#include <KLocalizedContext>

#include <QQmlContext>
#include <QQmlComponent>
#include <QQmlEngine>
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
    KLocalizedContext *localizedContextObject = new KLocalizedContext(engine());
    localizedContextObject->setTranslationDomain(QStringLiteral("kdevwelcomepage"));
    engine()->rootContext()->setContextObject(localizedContextObject);

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

#include "moc_welcomepageview.cpp"
