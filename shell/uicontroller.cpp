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
#include <kmenubar.h>
#include <kcomponentdata.h>
#include <KCMUtils/ksettings/dispatcher.h>
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
#include "assistantpopup.h"
#include <kactioncollection.h>
#include <ktexteditor/view.h>
#include "workingsetcontroller.h"
#include "workingsets/workingset.h"

namespace KDevelop {

class UiControllerPrivate {
public:
    UiControllerPrivate(UiController *controller)
    : areasRestored(false), m_controller(controller)
    {
        if (Core::self()->workingSetControllerInternal())
            Core::self()->workingSetControllerInternal()->initializeController(m_controller);

        QMap<QString, Sublime::Position> desired;

        desired["org.kdevelop.ClassBrowserView"] = Sublime::Left;
        desired["org.kdevelop.DocumentsView"] = Sublime::Left;
        desired["org.kdevelop.ProjectsView"] = Sublime::Left;
        desired["org.kdevelop.FileManagerView"] = Sublime::Left;
        desired["org.kdevelop.ProblemReporterView"] = Sublime::Bottom;
        desired["org.kdevelop.OutputView"] = Sublime::Bottom;
        desired["org.kdevelop.ContextBrowser"] = Sublime::Bottom;
        desired["org.kdevelop.KonsoleView"] = Sublime::Bottom;
        desired["org.kdevelop.SnippetView"] = Sublime::Right;
        desired["org.kdevelop.ExternalScriptView"] = Sublime::Right;
        Sublime::Area* a =
            new Sublime::Area(m_controller, "code", i18n("Code"));
        a->setDesiredToolViews(desired);
        a->setIconName("document-edit");
        m_controller->addDefaultArea(a);

        desired.clear();
        desired["org.kdevelop.debugger.VariablesView"] = Sublime::Left;
        desired["org.kdevelop.debugger.BreakpointsView"] = Sublime::Bottom;
        desired["org.kdevelop.debugger.StackView"] = Sublime::Bottom;
        desired["org.kdevelop.debugger.ConsoleView"] = Sublime::Bottom;
        desired["org.kdevelop.KonsoleView"] = Sublime::Bottom;
        a = new Sublime::Area(m_controller, "debug", i18n("Debug"));
        a->setDesiredToolViews(desired);
        a->setIconName("tools-report-bug");
        m_controller->addDefaultArea(a);

        desired.clear();
        desired["org.kdevelop.ProjectsView"] = Sublime::Left;
        desired["org.kdevelop.PatchReview"] = Sublime::Bottom;
        
        a = new Sublime::Area(m_controller, "review", i18n("Review"));
        a->setDesiredToolViews(desired);
        a->setIconName("applications-engineering");
        m_controller->addDefaultArea(a);

        if(!(Core::self()->setupFlags() & Core::NoUi)) 
        {
            defaultMainWindow = new MainWindow(m_controller);
            m_controller->addMainWindow(defaultMainWindow);
            activeSublimeWindow = defaultMainWindow;
        }
        else
        {
            activeSublimeWindow = defaultMainWindow = 0;
        }
    }

    void widgetChanged(QWidget*, QWidget* now)
    {
        if (now) {
            Sublime::MainWindow* win = qobject_cast<Sublime::MainWindow*>(now->window());
            if( win )
            {
                activeSublimeWindow = win;
            }
        }
    }

    Core *core;
    MainWindow* defaultMainWindow;

    QMap<IToolViewFactory*, Sublime::ToolDocument*> factoryDocuments;

    Sublime::MainWindow* activeSublimeWindow;
    bool areasRestored;

    //Currently shown assistant popup.
    QPointer<AssistantPopup> currentShownAssistant;

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

    virtual QList< QAction* > contextMenuActions(QWidget* viewWidget) const
    {
        return m_factory->contextMenuActions( viewWidget );
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


class NewToolViewListWidget: public QListWidget {
    Q_OBJECT

public:
    NewToolViewListWidget(MainWindow *mw, QWidget* parent = 0)
        :QListWidget(parent), m_mw(mw)
    {
        connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(addNewToolViewByDoubleClick(QModelIndex)));
    }

Q_SIGNALS:
    void addNewToolView(MainWindow *mw, QListWidgetItem *item);

private Q_SLOTS:
    void addNewToolViewByDoubleClick(QModelIndex index)
    {
        QListWidgetItem *item = itemFromIndex(index);
        // Disable item so that the toolview can not be added again.
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        emit addNewToolView(m_mw, item);
    }

private:
    MainWindow *m_mw;
};


UiController::UiController(Core *core)
    :Sublime::Controller(0), IUiController(), d(new UiControllerPrivate(this))
{
    setObjectName("UiController");
    d->core = core;

    if (!defaultMainWindow() || (Core::self()->setupFlags() & Core::NoUi))
        return;

    connect( QApplication::instance(),
             SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(widgetChanged(QWidget*,QWidget*)) );

    setupActions();
}

UiController::~UiController()
{
    delete d;
}

void UiController::setupActions()
{
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
    if (switchMode == ThisWindow) {
        showArea(areaName, activeSublimeWindow());
        return;
    }

    MainWindow *main = new MainWindow(this);
    // FIXME: what this is supposed to do?
    // Answer: Its notifying the mainwindow to reload its settings when one of
    // the KCM's changes its settings and it works
    KSettings::Dispatcher::registerComponent( KComponentData::mainComponent().componentName(), main, "loadSettings" );
    KSettings::Dispatcher::registerComponent( Core::self()->componentData().componentName(), main, "loadSettings" );

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


QWidget* UiController::findToolView(const QString& name, IToolViewFactory *factory, FindFlags flags)
{
    if(!d->areasRestored || !activeArea())
        return 0;

    QList< Sublime::View* > views = activeArea()->toolViews();
    foreach(Sublime::View* view, views) {
        Sublime::ToolDocument *doc = dynamic_cast<Sublime::ToolDocument*>(view->document());
        if(doc && doc->title() == name && view->widget()) {
            if(flags & Raise)
                view->requestRaise();
            return view->widget();
        }
    }
    
    QWidget* ret = 0;
    
    if(flags & Create)
    {
        if(!d->factoryDocuments.contains(factory))
            d->factoryDocuments[factory] = new Sublime::ToolDocument(name, this, new UiToolViewFactory(factory));
        
        Sublime::ToolDocument *doc = d->factoryDocuments[factory];

        Sublime::View* view = addToolViewToArea(factory, doc, activeArea());
        if(view)
            ret = view->widget();
        
        if(flags & Raise)
            findToolView(name, factory, Raise);
    }

    return ret;
}

void UiController::raiseToolView(QWidget* toolViewWidget)
{
    if(!d->areasRestored)
        return;

    QList< Sublime::View* > views = activeArea()->toolViews();
    foreach(Sublime::View* view, views) {
        if(view->widget() == toolViewWidget) {
            view->requestRaise();
            return;
        }
    }
}

void UiController::addToolView(const QString & name, IToolViewFactory *factory)
{
    kDebug() ;
    Sublime::ToolDocument *doc = new Sublime::ToolDocument(name, this, new UiToolViewFactory(factory));
    d->factoryDocuments[factory] = doc;

    /* Until areas are restored, we don't know which views should be really
       added, and which not, so we just record view availability.  */
    if (d->areasRestored) {
         foreach (Sublime::Area* area, allAreas()) {
             addToolViewToArea(factory, doc, area);
         }
    }
}

void KDevelop::UiController::raiseToolView(Sublime::View * view)
{
    foreach( Sublime::Area* area, allAreas() ) {
        if( area->toolViews().contains( view ) )
            area->raiseToolView( view );
    }
}

void KDevelop::UiController::removeToolView(IToolViewFactory *factory)
{
    kDebug() ;
    //delete the tooldocument
    Sublime::ToolDocument *doc = d->factoryDocuments[factory];

    ///@todo adymo: on document deletion all its views shall be also deleted
    foreach (Sublime::View *view, doc->views()) {
        foreach (Sublime::Area *area, allAreas())
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

void UiController::initialize()
{
    defaultMainWindow()->initialize();
}

void UiController::cleanup()
{
    foreach (Sublime::MainWindow* w, mainWindows())
        w->saveSettings();
    saveAllAreas(KSharedConfig::openConfig());
}

void UiController::selectNewToolViewToAdd(MainWindow *mw)
{
    if (!mw || !mw->area())
        return;
    KDialog *dia = new KDialog(mw);
    dia->setCaption(i18n("Select Tool View to Add"));
    dia->setButtons(KDialog::Ok | KDialog::Cancel);
    NewToolViewListWidget *list = new NewToolViewListWidget(mw, dia);

    list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    list->setSortingEnabled(true);
    for (QMap<IToolViewFactory*, Sublime::ToolDocument*>::const_iterator it = d->factoryDocuments.constBegin();
        it != d->factoryDocuments.constEnd(); ++it)
    {
        ViewSelectorItem *item = new ViewSelectorItem(it.value()->title(), list);
        item->factory = it.key();
        if (!item->factory->allowMultiple() && toolViewPresent(it.value(), mw->area())) {
            // Disable item if the toolview is already present.
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
        list->addItem(item);
    }

    list->setFocus();
    connect(list, SIGNAL(addNewToolView(MainWindow*,QListWidgetItem*)), this, SLOT(addNewToolView(MainWindow*,QListWidgetItem*)));
    dia->setMainWidget(list);
    if (dia->exec() == QDialog::Accepted)
    {
        foreach (QListWidgetItem* item, list->selectedItems())
        {
            addNewToolView(mw, item);
        }
    }
    delete dia;
}

void UiController::addNewToolView(MainWindow *mw, QListWidgetItem* item)
{
    ViewSelectorItem *current = static_cast<ViewSelectorItem*>(item);
    Sublime::ToolDocument *doc = d->factoryDocuments[current->factory];
    Sublime::View *view = doc->createView();
    mw->area()->addToolView(view,
        Sublime::dockAreaToPosition(current->factory->defaultPosition()));
    current->factory->viewCreated(view);
}

void UiController::showSettingsDialog()
{
    QStringList blacklist = d->core->pluginControllerInternal()->projectPlugins();
    foreach(const KPluginInfo& info, d->core->pluginControllerInternal()->allPluginInfos()) {
        if (!blacklist.contains(info.pluginName()) && !info.isPluginEnabled()) {
            blacklist << info.pluginName();
        }
    }
    kDebug() << "blacklist" << blacklist;
    KSettings::Dialog cfgDlg( QStringList() << "kdevplatform",
                                        activeMainWindow() );
    cfgDlg.setMinimumSize( 550, 300 );
    cfgDlg.setComponentBlacklist( blacklist );
    cfgDlg.exec();
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
    if (!area->workingSet().isEmpty()) {
        WorkingSet* set = Core::self()->workingSetControllerInternal()->getWorkingSet(area->workingSet());
        set->saveFromArea(area, area->rootIndex());
    }
}

void UiController::loadArea(Sublime::Area * area, const KConfigGroup & group)
{
    area->load(group);
    if (!area->workingSet().isEmpty()) {
        WorkingSet* set = Core::self()->workingSetControllerInternal()->getWorkingSet(area->workingSet());
        Q_ASSERT(set->isConnected(area));
        Q_UNUSED(set);
    }
}

void UiController::saveAllAreas(KSharedConfig::Ptr config)
{
    KConfigGroup uiConfig(config, "User Interface");
    int wc = mainWindows().size();
    uiConfig.writeEntry("Main Windows Count", wc);
    for (int w = 0; w < wc; ++w)
    {
        KConfigGroup mainWindowConfig(&uiConfig,
                                      QString("Main Window %1").arg(w));

        foreach (Sublime::Area* defaultArea, defaultAreas())
        {
            // FIXME: using object name seems ugly.
            QString type = defaultArea->objectName();
            Sublime::Area* area = this->area(w, type);
            KConfigGroup areaConfig(&mainWindowConfig, "Area " + type);

            areaConfig.deleteGroup();
            areaConfig.writeEntry("id", type);
            saveArea(area, areaConfig);
            areaConfig.sync();
        }
    }
    uiConfig.sync();
}

void UiController::loadAllAreas(KSharedConfig::Ptr config)
{
    KConfigGroup uiConfig(config, "User Interface");
    int wc = uiConfig.readEntry("Main Windows Count", 1);

    /* It is expected the main windows are restored before
       restoring areas.  */
    if (wc > mainWindows().size())
        wc = mainWindows().size();

    QList<Sublime::Area*> changedAreas;

    /* Offer all toolviews to the default areas.  */
    foreach (Sublime::Area *area, defaultAreas())
    {
        QMap<IToolViewFactory*, Sublime::ToolDocument*>::const_iterator i, e;
        for (i = d->factoryDocuments.constBegin(),
                 e = d->factoryDocuments.constEnd(); i != e; ++i)
        {
            addToolViewIfWanted(i.key(), i.value(), area);
        }
    }

    /* Restore per-windows areas.  */
    for (int w = 0; w < wc; ++w)
    {
        KConfigGroup mainWindowConfig(&uiConfig,
                                      QString("Main Window %1").arg(w));
        
        Sublime::MainWindow *mw = mainWindows()[w];

        /* We loop over default areas.  This means that if
           the config file has an area of some type that is not
           in default set, we'd just ignore it.  I think it's fine --
           the model were a given mainwindow can has it's own
           area types not represented in the default set is way
           too complex.  */
        foreach (Sublime::Area* defaultArea, defaultAreas())
        {
            QString type = defaultArea->objectName();
            Sublime::Area* area = this->area(w, type);

            KConfigGroup areaConfig(&mainWindowConfig, "Area " + type);

            kDebug() << "Trying to restore area " << type;

            /* This is just an easy check that a group exists, to
               avoid "restoring" area from empty config group, wiping
               away programmatically installed defaults.  */
            if (areaConfig.readEntry("id", "") == type)
            {
                kDebug() << "Restoring area " << type;
                loadArea(area, areaConfig);
            }

            // At this point we know which toolviews the area wants.
            // Tender all tool views we have.
            QMap<IToolViewFactory*, Sublime::ToolDocument*>::const_iterator i, e;
            for (i = d->factoryDocuments.constBegin(),
                     e = d->factoryDocuments.constEnd(); i != e; ++i)
            {
                addToolViewIfWanted(i.key(), i.value(), area);
            }
        }

        // Force reload of the changes.
        showAreaInternal(mw->area(), mw);

        mw->enableAreaSettingsSave();
    }

    d->areasRestored = true;
}

void UiController::addToolViewToDockArea(IToolViewFactory* factory, Qt::DockWidgetArea area)
{
    addToolViewToArea(factory, d->factoryDocuments[factory], activeArea(), Sublime::dockAreaToPosition(area));
}

bool UiController::toolViewPresent(Sublime::ToolDocument* doc, Sublime::Area* area)
{
    foreach (Sublime::View *view, doc->views()) {
        if( area->toolViews().contains( view ) )
            return true;
    }
    return false;
}

void UiController::addToolViewIfWanted(IToolViewFactory* factory,
                           Sublime::ToolDocument* doc,
                           Sublime::Area* area)
{
    if (area->wantToolView(factory->id()))
    {
        addToolViewToArea(factory, doc, area);
    }
}

Sublime::View* UiController::addToolViewToArea(IToolViewFactory* factory,
                                     Sublime::ToolDocument* doc,
                                     Sublime::Area* area, Sublime::Position p)
{
    Sublime::View* view = doc->createView();
    area->addToolView(
        view,
        p == Sublime::AllPositions ? Sublime::dockAreaToPosition(factory->defaultPosition()) : p);

    connect(view, SIGNAL(raise(Sublime::View*)),
            SLOT(raiseToolView(Sublime::View*)));

    factory->viewCreated(view);
    return view;
}

void UiController::registerStatus(QObject* status)
{
    Sublime::MainWindow* w = activeSublimeWindow();
    if (!w) return;
    MainWindow* mw = qobject_cast<KDevelop::MainWindow*>(w);
    if (!mw) return;
    mw->registerStatus(status);
}

void UiController::showErrorMessage(const QString& message, int timeout)
{
    Sublime::MainWindow* w = activeSublimeWindow();
    if (!w) return;
    MainWindow* mw = qobject_cast<KDevelop::MainWindow*>(w);
    if (!mw) return;
    QMetaObject::invokeMethod(mw, "showErrorMessage", Q_ARG(QString, message), Q_ARG(int, timeout));
}

void UiController::hideAssistant(const KDevelop::IAssistant::Ptr& assistant)
{
    if(d->currentShownAssistant && d->currentShownAssistant->assistant() == assistant) {
        disconnect(d->currentShownAssistant->assistant().data(), SIGNAL(hide()), this, SLOT(assistantHide()));
        disconnect(d->currentShownAssistant->assistant().data(), SIGNAL(actionsChanged()), this, SLOT(assistantActionsChanged()));
        
        AssistantPopup* oldPopup = d->currentShownAssistant;
        d->currentShownAssistant = 0;
        oldPopup->hide();
        oldPopup->deleteLater(); //We have to do deleteLater, so we don't get problems when an assistant hides itself
    }
}

void UiController::popUpAssistant(const KDevelop::IAssistant::Ptr& assistant)
{
    assistantHide();
    if(!assistant)
        return;
    
    Sublime::View* view = d->activeSublimeWindow->activeView();
    if( !view )
    {
        kDebug() << "no active view in mainwindow";
        return;
    }
    TextEditorWidget* textWidget = dynamic_cast<TextEditorWidget*>(view->widget());
    if(textWidget && textWidget->editorView()) {

        d->currentShownAssistant = new AssistantPopup(textWidget->editorView(), assistant);
        if(assistant->actions().count())
            d->currentShownAssistant->show();

        connect(assistant.data(), SIGNAL(hide()), SLOT(assistantHide()), Qt::DirectConnection);
        connect(assistant.data(), SIGNAL(actionsChanged()), SLOT(assistantActionsChanged()), Qt::DirectConnection);
    }
}

const QMap< IToolViewFactory*, Sublime::ToolDocument* >& UiController::factoryDocuments() const
{
    return d->factoryDocuments;
}

void UiController::assistantHide() {
    if(d->currentShownAssistant)
        hideAssistant(d->currentShownAssistant->assistant());
}

void UiController::assistantActionsChanged() {
    if(d->currentShownAssistant)
        popUpAssistant(d->currentShownAssistant->assistant());
}

}


#include "uicontroller.moc"
#include "moc_uicontroller.cpp"
