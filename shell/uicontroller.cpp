/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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
#include <QAction>
#include <QPointer>
#include <QApplication>
#include <QListWidget>
#include <QToolBar>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <ksettings/dialog.h>
#include <ksettings/dispatcher.h>
#include <kcmultidialog.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>

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
#include "documentcontroller.h"

namespace KDevelop {

class UiControllerPrivate {
public:
    UiControllerPrivate(UiController *controller): cfgDlg(0), m_controller(controller)
    {
        // FIXME: remove this 'defaultArea' thing.
        AreaParams defaultAreaParams = ShellExtension::getInstance()->defaultArea();
        QMap<QString, Sublime::Position> desired;

        desired["org.kdevelop.ClassBrowserView"] = Sublime::Left;
        desired["org.kdevelop.DocumentsView"] = Sublime::Left;
        desired["org.kdevelop.ProjectsView"] = Sublime::Left;
        desired["org.kdevelop.FileManagerView"] = Sublime::Left;
        desired["org.kdevelop.ProblemReporterView"] = Sublime::Bottom;
        Sublime::Area* a = 
            new Sublime::Area(m_controller, "code", i18n("Code"));
        a->setDesiredToolViews(desired);
        controller->addArea(a);

        desired.clear();
        desired["org.kdevelop.debugger.VariablesView"] = Sublime::Left;
        desired["org.kdevelop.debugger.BreakpointsView"] = Sublime::Bottom;
        desired["org.kdevelop.debugger.StackView"] = Sublime::Bottom;
        desired["org.kdevelop.debugger.ConsoleView"] = Sublime::Bottom;
        a = new Sublime::Area(m_controller, "debug", i18n("Debug"));
        a->setDesiredToolViews(desired);
        controller->addArea(a);

        defaultMainWindow = new MainWindow(controller);
        controller->addMainWindow(defaultMainWindow);
        activeSublimeWindow = defaultMainWindow;
    }

    void widgetChanged(QWidget*, QWidget* now)
    {
        Sublime::MainWindow* win = qobject_cast<Sublime::MainWindow*>(now);
        if( win )
        {
            activeSublimeWindow = win;
        }
    }

    Sublime::Area *defaultArea;
    Core *core;
    MainWindow* defaultMainWindow;

    QMap<IToolViewFactory*, Sublime::ToolDocument*> factoryDocuments;

    KSettings::Dialog* cfgDlg;

    Sublime::MainWindow* activeSublimeWindow;
    QList<Sublime::MainWindow*> sublimeWindows;

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


    QList<QAction*> toolBarActions( QWidget* viewWidget ) const
    {
        return m_factory->toolBarActions( viewWidget );
    }

    QString id() const { return m_factory->id(); }

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
    // FIXME: restore.
#if 0
    KSettings::Dispatcher::registerComponent( KGlobal::mainComponent(),
                                    defaultMainWindow(), SLOT( loadSettings() ) );
    KSettings::Dispatcher::registerComponent( KComponentData("kdevplatform"),
                                    defaultMainWindow(), SLOT( loadSettings() ) );
#endif
    d->core = core;
    connect( QApplication::instance(),
             SIGNAL( focusChanged( QWidget*, QWidget* ) ),
            this, SLOT( widgetChanged( QWidget*, QWidget* ) ) );
}

UiController::~UiController()
{
    delete d;
}

void UiController::mainWindowDeleted(MainWindow* mw)
{
    if (d->defaultMainWindow == mw)
        d->defaultMainWindow = 0L;

    if (d->activeSublimeWindow == mw)
        d->activeSublimeWindow = 0L;
}

// FIXME: currently, this always create new window. Probably,
// should just rename it.
void UiController::switchToArea(const QString &areaName, SwitchMode switchMode)
{
    Q_UNUSED( switchMode );
    KParts::MainWindow *oldMain = activeMainWindow();

    MainWindow *main = new MainWindow(this);
    // FIXME: what this is supposed to do?
    KSettings::Dispatcher::registerComponent( KGlobal::mainComponent(),
                                    main, SLOT( loadSettings() ) );
    KSettings::Dispatcher::registerComponent( KComponentData("kdevplatform"),
                                    main, SLOT( loadSettings() ) );

    addMainWindow(main);
    showArea(areaName, main);
    main->initialize();

    // WTF? First, enabling this code causes crashes since we
    // try to disconnect some already-deleted action, or something.
    // Second, this code will disconnection the clients from guiFactory
    // of the previous main window. Ick!
#if 0
    //we need to add all existing guiclients to the new mainwindow
    //@todo adymo: add only ones that belong to the area (when the area code is there)
    foreach (KXMLGUIClient *client, oldMain->guiFactory()->clients())
        main->guiFactory()->addClient(client);
#endif

    main->show();
}


void UiController::addToolView(const QString & name, IToolViewFactory *factory)
{
    kDebug(9501) ;
    Sublime::ToolDocument *doc = new Sublime::ToolDocument(name, this, new UiToolViewFactory(factory));
    d->factoryDocuments[factory] = doc;
}

void KDevelop::UiController::raiseToolView(Sublime::View * view)
{
    d->defaultArea->raiseToolView(view);
}

void KDevelop::UiController::removeToolView(IToolViewFactory *factory)
{
    kDebug(9501) ;
    //delete the tooldocument
    Sublime::ToolDocument *doc = d->factoryDocuments[factory];

    ///@todo adymo: on document deletion all its views shall be also deleted
    foreach (Sublime::View *view, doc->views()) {
        foreach (Sublime::Area *area, areas())
            if (area->removeToolView(view))
                view->deleteLater();
    }

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
    return d->activeSublimeWindow;
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

void UiController::cleanup()
{
    foreach (Sublime::MainWindow* w, mainWindows())
        w->saveSettings();
    saveAllAreas(KGlobal::config());
}

void UiController::addNewToolView(MainWindow *mw)
{
    if (!mw || !mw->area())
        return;
    KDialog *dia = new KDialog(mw);
    dia->setCaption(i18n("Select Tool View to Add"));
    dia->setButtons(KDialog::Ok | KDialog::Cancel);
    QListWidget *list = new QListWidget(dia);

    list->setSortingEnabled(true);
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
            Sublime::dockAreaToPosition(current->factory->defaultPosition()));
    }
    delete dia;
}

void UiController::showSettingsDialog()
{
    QStringList blacklist = d->core->pluginControllerInternal()->projectPlugins();
    kDebug(9501) << "blacklist" << blacklist;
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

void UiController::saveArea(Sublime::Area * area, KConfigGroup & group)
{
    area->save(group);
    saveArea(area->rootIndex(), group);
}

void UiController::saveArea(Sublime::AreaIndex * area, KConfigGroup & group)
{
    if (area->isSplitted()) {
        group.writeEntry("Orientation", area->orientation() == Qt::Horizontal ? "Horizontal" : "Vertical");

        if (area->first()) {
            KConfigGroup subgroup(&group, "0");
            subgroup.deleteGroup();
            saveArea(area->first(), subgroup);
        }

        if (area->second()) {
            KConfigGroup subgroup(&group, "1");
            subgroup.deleteGroup();
            saveArea(area->second(), subgroup);
        }
    } else {
        group.writeEntry("View Count", area->viewCount());

        int index = 0;
        foreach (Sublime::View* view, area->views()) {
            group.writeEntry(QString("View %1 Type").arg(index), view->document()->documentType());
            group.writeEntry(QString("View %1").arg(index), view->document()->documentSpecifier());
            QString state = view->viewState();
            if (!state.isEmpty())
                group.writeEntry(QString("View %1 State").arg(index), state);

            ++index;
        }
    }
}

void UiController::loadArea(Sublime::Area * area, const KConfigGroup & group)
{
    area->load(group);
    loadArea(area, area->rootIndex(), group);
}

void UiController::loadArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex, const KConfigGroup& group)
{
    if (group.hasKey("Orientation")) {
        QStringList subgroups = group.groupList();

        if (subgroups.contains("0")) {
            if (!areaIndex->isSplitted())
                areaIndex->split(group.readEntry("Orientation", "Horizontal") == "Vertical" ? Qt::Vertical : Qt::Horizontal);

            KConfigGroup subgroup(&group, "0");
            loadArea(area, areaIndex->first(), subgroup);

            if (subgroups.contains("1")) {
                Q_ASSERT(areaIndex->isSplitted());
                KConfigGroup subgroup(&group, "1");
                loadArea(area, areaIndex->second(), subgroup);
            }
        }

    } else {
        while (areaIndex->isSplitted()) {
            areaIndex = areaIndex->first();
            Q_ASSERT(areaIndex);// Split area index did not contain a first child area index if this fails
        }

        int viewCount = group.readEntry("View Count", 0);
        for (int i = 0; i < viewCount; ++i) {
            QString type = group.readEntry(QString("View %1 Type").arg(i), "");
            QString specifier = group.readEntry(QString("View %1").arg(i), "");

            bool viewExists = false;
            foreach (Sublime::View* view, areaIndex->views()) {
                if (view->document()->documentSpecifier() == specifier) {
                    viewExists = true;
                    break;
                }
            }

            if (viewExists)
                continue;


            IDocument* doc = Core::self()->documentControllerInternal()->openDocument(specifier,
                KTextEditor::Cursor::invalid(), IDocumentController::DoNotActivate | IDocumentController::DoNotCreateView);
            Sublime::Document *document = dynamic_cast<Sublime::Document*>(doc);
            if (document) {
                Sublime::View* view = document->createView();

                QString state = group.readEntry(QString("View %1 State").arg(i), "");
                if (!state.isEmpty())
                    view->setState(state);

                area->addView(view, areaIndex);
            } else {
                kWarning() << "Unable to create view of type " << type;
            }
        }
    }
}

void UiController::saveAllAreas(KSharedConfig::Ptr config)
{
    KConfigGroup uiConfig(config, "User Interface");
    int wc = mainWindows().size();
    uiConfig.writeEntry("Main Windows Count", wc);
    int ac = areas().size();
    uiConfig.writeEntry("Area Count", ac);
    for (int w = 0; w < wc; ++w)
        for (int a = 0; a < ac; ++a)
        {
            Sublime::Area* area = areas(w)[a];
            KConfigGroup areaConfig(
                &uiConfig, QString("Main Window %1, Area %2").arg(w).arg(a));

            kDebug(9501) << "Saving area " << area->objectName() << " for mw " << w;

            areaConfig.deleteGroup();            
            // FIXME: don't use objectName
            areaConfig.writeEntry("id", area->objectName());
            saveArea(area, areaConfig);         
            areaConfig.sync();
        }
}

void UiController::loadAllAreas(KSharedConfig::Ptr config)
{
    KConfigGroup uiConfig(config, "User Interface");
    int wc = uiConfig.readEntry("Main Windows Count", 0);
    int ac = uiConfig.readEntry("Area Count", 0);

    /* It is expected the main windows are restored before
       restoring areas.  */
    if (wc > mainWindows().size())
        wc = mainWindows().size();

    QList<Sublime::Area*> changedAreas;

    for (int w = 0; w < wc; ++w)
        for (int a = 0; a < ac; ++a)
        {
            KConfigGroup areaConfig(
                &uiConfig, QString("Main Window %1, Area %2").arg(w).arg(a));
            
            QString id = areaConfig.readEntry("id", QString());
            if (id.isNull())
                continue;

            Sublime::Area* area = this->area(w, id);
            if (!area)
                // FIXME: probably, should still add it and add
                // new default one. Alternatively, if deleting
                // area type should warn the user.
                continue;
            
            loadArea(area, areaConfig);

            changedAreas << area;
        }

    QMap<IToolViewFactory*, Sublime::ToolDocument*>::const_iterator i, e;
    for (i = d->factoryDocuments.begin(), 
             e = d->factoryDocuments.end(); i != e; ++i)
    {   
        IToolViewFactory* factory = i.key();
   
        foreach (Sublime::Area* area, allAreas()) {
            if (!area->wantToolView(factory->id()))
                continue;
            
            Sublime::View* view = i.value()->createView();
            area->addToolView(
                view,
                Sublime::dockAreaToPosition(factory->defaultPosition()));
            
            connect(view, SIGNAL(raise(Sublime::View*)), 
                    SLOT(raiseToolView(Sublime::View*)));
            
            factory->viewCreated(view);
        }
    }

    foreach (Sublime::MainWindow* mw, mainWindows())
        if (changedAreas.contains(mw->area()))
            showAreaInternal(mw->area(), mw);


}

}

#include "uicontroller.moc"
