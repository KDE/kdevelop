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
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QLabel>
#include <KLocalizedString>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <KParts/MainWindow>
#include <QDebug>
#include "uihelper.h"

WelcomePageView::WelcomePageView(const QString &title, Sublime::Controller *controller)
    : Sublime::Document(title, controller)
{
    qRegisterMetaType<QObject*>("KDevelop::IProjectController*");
}

QWidget* WelcomePageView::createViewWidget(QWidget* parent)
{
    
    QDeclarativeView* view = new QDeclarativeView(parent);
    view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    
    UiHelper* helper = new UiHelper(view);
    view->rootContext()->setContextProperty("kdev", helper);
    view->rootContext()->setContextProperty("ICore", KDevelop::ICore::self());
    
    view->setSource(QUrl("qrc:/main.qml"));
    return view;
}

QString WelcomePageView::documentType() const
{
    return i18n("Welcome");
}

QString WelcomePageView::documentSpecifier() const
{
    return i18n("Welcome to KDevelop");
}
