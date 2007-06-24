/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "uicontroller.h"

#include <QMap>
#include <QPointer>
#include <QApplication>
#include <QListWidget>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <ksettings/dialog.h>
#include <ksettings/dispatcher.h>
#include <kcmultidialog.h>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/tooldocument.h>

#include "core.h"
#include "shellextension.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "mainwindow.h"
#include "partdocument.h"
#include "textdocument.h"

namespace KDevelop {

class UiControllerPrivate {
public:
    UiControllerPrivate(UiController *controller): cfgDlg(0), m_controller(controller)
    {
        AreaParams defaultAreaParams = ShellExtension::getInstance()->defaultArea();
        defaultArea = new Sublime::Area(m_controller, defaultAreaParams.name, defaultAreaParams.title);
        defaultMainWindow = new MainWindow(controller);
    }

    Sublime::Area *defaultArea;
    Core *core;
    MainWindow* defaultMainWindow;

    QMap<IToolViewFactory*, Sublime::ToolDocument*> factoryDocuments;

    KSettings::Dialog* cfgDlg;

private:
    UiController *m_controller;
};


class UiToolViewFactory: public Sublime::ToolFactory {
public:
    UiToolViewFactory(IToolViewFactory *factory): m_factory(factory) {}
    ~UiToolViewFactory() { delete m_factory; }
    virtual QWidget* create(Sublime::ToolDocument *doc, QWidget *parent = 0)
    {
        Q_UNUSED( doc );
        return m_factory->create(parent);
    }

private:
    IToolViewFactory *m_factory;
};


class ViewSelectorItem: public QListWidgetItem {
public:
    ViewSelectorItem(const QString &text, QListWidget *parent = 0, int type = Type)
        :QListWidgetItem(text, parent, type) {}
    IToolViewFactory *factory;
};

UiController::UiController(Core *core)
    :Sublime::Controller(0), IUiController(), d(new UiControllerPrivate(this))
{
    KSettings::Dispatcher::registerComponent( KGlobal::mainComponent(),
                                    defaultMainWindow(), SLOT( loadSettings() ) );
    KSettings::Dispatcher::registerComponent( KComponentData("kdevplatform"),
                                    defaultMainWindow(), SLOT( loadSettings() ) );
    d->core = core;
}

UiController::~UiController()
{
    delete d;
}

void UiController::switchToArea(const QString &areaName, SwitchMode switchMode)
{
    Q_UNUSED( switchMode );
    MainWindow *main = new MainWindow(this);
    KSettings::Dispatcher::registerComponent( KGlobal::mainComponent(),
                                    main, SLOT( loadSettings() ) );
    KSettings::Dispatcher::registerComponent( KComponentData("kdevplatform"),
                                    main, SLOT( loadSettings() ) );
    showArea(area(areaName), main);
    main->initialize();
    main->show();
}


void UiController::addToolView(const QString & name, IToolViewFactory *factory)
{
    kDebug() << k_funcinfo << endl;
    Sublime::ToolDocument *doc = new Sublime::ToolDocument(name, this, new UiToolViewFactory(factory));
    d->factoryDocuments[factory] = doc;
    d->defaultArea->addToolView(doc->createView(),
        Sublime::dockAreaToPosition(factory->defaultPosition(d->defaultArea->objectName())));
}

void KDevelop::UiController::removeToolView(IToolViewFactory *factory)
{
    kDebug() << k_funcinfo << endl;
    //delete the tooldocument
    Sublime::ToolDocument *doc = d->factoryDocuments[factory];

    ///@todo adymo: on document deletion all its views shall be also deleted
    foreach (Sublime::View *view, doc->views())
#ifdef __GNUC__
#warning removeToolView deletes view - once per area (CID 3303)
#endif
        foreach (Sublime::Area *area, areas())
            area->removeToolView(view);

    d->factoryDocuments.remove(factory);
    delete doc;
}

Sublime::Area *UiController::activeArea()
{
    Sublime::MainWindow *m = activeSublimeWindow();
    if (m)
        return activeSublimeWindow()->area();
    return 0;
}

Sublime::MainWindow *UiController::activeSublimeWindow()
{
    QWidget *active = QApplication::activeWindow();
    if (!active)
        return 0;
    return qobject_cast<Sublime::MainWindow*>(active);
}

MainWindow *UiController::defaultMainWindow()
{
    return d->defaultMainWindow;
}

Sublime::Area * KDevelop::UiController::defaultArea()
{
    return d->defaultArea;
}

void UiController::initialize()
{
    defaultMainWindow()->initialize();
}

void UiController::openEmptyDocument()
{
    PartDocument *doc = new PartDocument(KUrl(), Core::self());
    Sublime::View *view = doc->createView();
    activeArea()->addView(view);
    activeSublimeWindow()->activateView(view);
}

void UiController::addNewToolView(MainWindow *mw)
{
    if (!mw || !mw->area())
        return;
    KDialog *dia = new KDialog(mw);
    dia->setCaption(i18n("Select Tool View to Add"));
    dia->setButtons(KDialog::Ok | KDialog::Cancel);
    QListWidget *list = new QListWidget(dia);

    for (QMap<IToolViewFactory*, Sublime::ToolDocument*>::const_iterator it = d->factoryDocuments.begin();
        it != d->factoryDocuments.end(); ++it)
    {
        ViewSelectorItem *item = new ViewSelectorItem(it.value()->title(), list);
        item->factory = it.key();
        list->addItem(item);
    }

    dia->setMainWidget(list);
    if (dia->exec() == QDialog::Accepted && list->currentItem())
    {
        ViewSelectorItem *current = static_cast<ViewSelectorItem*>(list->currentItem());
        Sublime::ToolDocument *doc = d->factoryDocuments[current->factory];
        mw->area()->addToolView(doc->createView(),
            Sublime::dockAreaToPosition(current->factory->defaultPosition(mw->area()->objectName())));
    }
    delete dia;
}

void UiController::showSettingsDialog()
{
    QStringList blacklist = d->core->pluginControllerInternal()->projectPlugins();
    kDebug() << k_funcinfo << "blacklist" << blacklist << endl;
    if(!d->cfgDlg)
    {
        d->cfgDlg = new KSettings::Dialog( QStringList() << "kdevplatform",
                                           activeMainWindow() );
        d->cfgDlg->setComponentBlacklist( blacklist );
    }
// The following doesn't work for some reason if the parent != activeMainWin,
// the show() call doesn't show the dialog
//     if( d->cfgDlg->dialog()->parentWidget() != activeMainWindow() )
//         d->cfgDlg->dialog()->setParent( activeMainWindow() );
    d->cfgDlg->show();
}

Sublime::Controller* UiController::controller()
{
    return this;
}

KParts::MainWindow *UiController::activeMainWindow()
{
    return (KParts::MainWindow*)(activeSublimeWindow());
}

}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
