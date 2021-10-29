/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "uihelper.h"

#include "debug.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <KParts/MainWindow>
#include <QDebug>
#include <QMenuBar>
#include <sublime/area.h>
#include <sublime/view.h>

using namespace KDevelop;

UiHelper::UiHelper(QObject* parent): QObject(parent)
{}

QAction* findActionRec(const QStringList& path, const QList<QAction*>& actions)
{
    QStringList newPath = path;
    QString current = newPath.takeFirst();
    
    for (QAction* a : actions) {
        if(a->objectName() == current) {
            if(newPath.isEmpty())
                return a;
            else if(a->menu())
                return findActionRec(newPath, a->menu()->actions());
            else
                qCDebug(PLUGIN_WELCOMEPAGE) << "shouldn't get here:" << path;
        }
    }
    
    qWarning() << "error: action path not found: " << path;
    return nullptr;
}

QAction* UiHelper::retrieveMenuAction(const QString& menuPath)
{
    QMenuBar* m = ICore::self()->uiController()->activeMainWindow()->menuBar();

    QAction* a = findActionRec(menuPath.split(QLatin1Char('/')), m->actions());
    return a;
}

void UiHelper::setArea(const QString& name)
{
    ICore::self()->uiController()->switchToArea(name, IUiController::ThisWindow);
}

void UiHelper::raiseToolView(const QString& id)
{
    const QList<Sublime::View*> views = ICore::self()->uiController()->activeArea()->toolViews();
    for (Sublime::View* v : views) {
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
