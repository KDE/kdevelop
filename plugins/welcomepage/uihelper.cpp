/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "uihelper.h"
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <KParts/MainWindow>
#include <KActionCollection>
#include <interfaces/iprojectcontroller.h>
#include <QDebug>
#include <QMenuBar>
#include <qdesktopservices.h>
#include <sublime/area.h>
#include <sublime/view.h>

using namespace KDevelop;

UiHelper::UiHelper(QObject* parent): QObject(parent)
{}

QAction* findActionRec(const QStringList& path, const QList<QAction*>& actions)
{
    QStringList newPath = path;
    QString current = newPath.takeFirst();
    
    foreach(QAction* a, actions) {
        if(a->objectName() == current) {
            if(newPath.isEmpty())
                return a;
            else if(a->menu())
                return findActionRec(newPath, a->menu()->actions());
            else
                qDebug() << "shouldn't get here:" << path;
        }
    }
    
    qWarning() << "error: action path not found: " << path;
    return 0;
}

QAction* UiHelper::retrieveMenuAction(const QString& menuPath)
{
    QMenuBar* m = ICore::self()->uiController()->activeMainWindow()->menuBar();
    
    QAction* a=findActionRec(menuPath.split('/'), m->actions());
    return a;
}

void UiHelper::setArea(const QString& name)
{
    ICore::self()->uiController()->switchToArea(name, IUiController::ThisWindow);
}

void UiHelper::raiseToolView(const QString& id)
{
    QList< Sublime::View* > views = ICore::self()->uiController()->activeArea()->toolViews();
    foreach(Sublime::View* v, views) {
        QWidget* w=v->widget();
        if(w && id==w->objectName())
            ICore::self()->uiController()->raiseToolView(w);
    }
}

void UiHelper::showMenu(const QString& name)
{
    QAction* action = retrieveMenuAction(name);
    Q_ASSERT(action);
    Q_ASSERT(action->menu());
    
    action->menu()->popup(QCursor::pos());
}
