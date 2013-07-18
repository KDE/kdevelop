/***************************************************************************
 *   Copyright 2006-2009 Alexander Dymo  <adymo@kdevelop.org>              *
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
#include "mainwindow_p.h"

#include <QtGui/QMenu>
#include <QtGui/QLayout>
#include <QtGui/QSplitter>
#include <QtGui/QDockWidget>
#include <QtGui/QWidgetAction>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QCommonStyle>
#include <QTimer>

#include <kdebug.h>
#include <klocale.h>
#include <kactionmenu.h>
#include <kacceleratormanager.h>
#include <kactioncollection.h>

#include "area.h"
#include "view.h"
#include "areaindex.h"
#include "document.h"
#include "container.h"
#include "controller.h"
#include "mainwindow.h"
#include "idealcontroller.h"
#include "holdupdates.h"
#include "idealbuttonbarwidget.h"
#include <KToolBar>
#include <KSelectAction>
#include <ktoggleaction.h>

class IdealToolBar : public QToolBar
{
    Q_OBJECT
    public:
        explicit IdealToolBar(const QString& title, Sublime::IdealButtonBarWidget* buttons, QMainWindow* parent)
            : QToolBar(title, parent), m_buttons(buttons)
        {
            setMovable(false);
            setFloatable(false);
            setObjectName(title);
            layout()->setMargin(0);
            
            addWidget(m_buttons);
        }

        void hideWhenEmpty()
        {
            refresh();
            
            QTimer* t = new QTimer(this);
            t->setInterval(100);
            t->setSingleShot(true);
            connect(t, SIGNAL(timeout()), SLOT(refresh()));
            connect(this, SIGNAL(visibilityChanged(bool)), t, SLOT(start()));
            connect(m_buttons, SIGNAL(emptyChanged()), t, SLOT(start()));
        }

    public slots:
        void refresh()
        {
            if(m_buttons->isEmpty()==isVisible()) {
                setVisible(!m_buttons->isEmpty());
            }
        }

    private:
        Sublime::IdealButtonBarWidget* m_buttons;
};

namespace Sublime {

MainWindowPrivate::MainWindowPrivate(MainWindow *w, Controller* controller)
:controller(controller), area(0), activeView(0), activeToolView(0), bgCentralWidget(0),
 ignoreDockShown(false), autoAreaSettingsSave(false), m_mainWindow(w)
{
    KActionCollection *ac = m_mainWindow->actionCollection();

    KAction* action = new KAction(i18n("Show Left Dock"), this);
    action->setCheckable(true);
    action->setShortcut(Qt::META | Qt::CTRL | Qt::Key_L);
    connect(action, SIGNAL(toggled(bool)), SLOT(showLeftDock(bool)));
    ac->addAction("show_left_dock", action);

    action = new KAction(i18n("Show Right Dock"), this);
    action->setCheckable(true);
    action->setShortcut(Qt::META | Qt::CTRL | Qt::Key_R);
    connect(action, SIGNAL(toggled(bool)), SLOT(showRightDock(bool)));
    ac->addAction("show_right_dock", action);

    action = new KAction(i18n("Show Bottom Dock"), this);
    action->setCheckable(true);
    action->setShortcut(Qt::META | Qt::CTRL | Qt::Key_B);
    connect(action, SIGNAL(toggled(bool)), SLOT(showBottomDock(bool)));
    ac->addAction("show_bottom_dock", action);

    action = new KAction(i18nc("@action", "Focus Editor"), this);
    action->setShortcuts(QList<QKeySequence>() << (Qt::META | Qt::CTRL | Qt::Key_E) << Qt::META + Qt::Key_C);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(focusEditor()));
    ac->addAction("focus_editor", action);

    action = new KAction(i18n("Hide/Restore Docks"), this);
    action->setShortcut(Qt::META | Qt::CTRL | Qt::Key_H);
    connect(action, SIGNAL(triggered(bool)), SLOT(toggleDocksShown()));
    ac->addAction("hide_all_docks", action);

    action = new KAction(i18n("Next Tool View"), this);
    action->setShortcut(Qt::META | Qt::CTRL | Qt::Key_N);
    connect(action, SIGNAL(triggered(bool)), SLOT(selectNextDock()));
    ac->addAction("select_next_dock", action);

    action = new KAction(i18n("Previous Tool View"), this);
    action->setShortcut(Qt::META | Qt::CTRL | Qt::Key_P);
    connect(action, SIGNAL(triggered(bool)), SLOT(selectPreviousDock()));
    ac->addAction("select_previous_dock", action);

    action = new KActionMenu(i18n("Tool Views"), this);
    ac->addAction("docks_submenu", action);

    idealController = new IdealController(m_mainWindow);

    IdealToolBar* leftToolBar = new IdealToolBar(i18n("Left Button Bar"), idealController->leftBarWidget, m_mainWindow);
    leftToolBar->hideWhenEmpty();
    m_mainWindow->addToolBar(Qt::LeftToolBarArea, leftToolBar);

    IdealToolBar* rightToolBar = new IdealToolBar(i18n("Right Button Bar"), idealController->rightBarWidget, m_mainWindow);
    rightToolBar->hideWhenEmpty();
    m_mainWindow->addToolBar(Qt::RightToolBarArea, rightToolBar);

    IdealToolBar* bottomToolBar = new IdealToolBar(i18n("Bottom Button Bar"), idealController->bottomBarWidget, m_mainWindow);
    m_mainWindow->addToolBar(Qt::BottomToolBarArea, bottomToolBar);

    // adymo: intentionally do not add a toolbar for top buttonbar
    // this doesn't work well with toolbars added via xmlgui

    centralWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    centralWidget->setLayout(layout);
    layout->setMargin(0);
    splitterCentralWidget = new QSplitter(centralWidget);
    layout->addWidget(splitterCentralWidget);
    m_mainWindow->setCentralWidget(centralWidget);

    connect(idealController,
            SIGNAL(dockShown(Sublime::View*,Sublime::Position,bool)),
            this,
            SLOT(slotDockShown(Sublime::View*,Sublime::Position,bool)));

    connect(idealController,
            SIGNAL(widgetResized(Qt::DockWidgetArea,int)),
            this,
            SLOT(widgetResized(Qt::DockWidgetArea,int)));

   connect(idealController, SIGNAL(dockBarContextMenuRequested(Qt::DockWidgetArea,QPoint)),
            m_mainWindow, SLOT(dockBarContextMenuRequested(Qt::DockWidgetArea,QPoint)));
}


MainWindowPrivate::~MainWindowPrivate()
{
    delete m_leftTabbarCornerWidget.data();
    m_leftTabbarCornerWidget.clear();
}

void MainWindowPrivate::showLeftDock(bool b)
{
    idealController->showLeftDock(b);
}

void MainWindowPrivate::showBottomDock(bool b)
{
    idealController->showBottomDock(b);
}

void MainWindowPrivate::showRightDock(bool b)
{
    idealController->showRightDock(b);
}

void MainWindowPrivate::setBackgroundCentralWidget(QWidget* w)
{
    delete bgCentralWidget;
    QLayout* l=m_mainWindow->centralWidget()->layout();
    l->addWidget(w);
    bgCentralWidget=w;
    setBackgroundVisible(area->views().isEmpty());
}

void MainWindowPrivate::setBackgroundVisible(bool v)
{
    if(!bgCentralWidget)
        return;
    
    bgCentralWidget->setVisible(v);
    splitterCentralWidget->setVisible(!v);
}

void MainWindowPrivate::focusEditor()
{
    if (View* view = m_mainWindow->activeView())
        if (view->hasWidget())
            view->widget()->setFocus(Qt::ShortcutFocusReason);
}

void MainWindowPrivate::toggleDocksShown()
{
    idealController->toggleDocksShown();
}

void MainWindowPrivate::selectNextDock()
{
    idealController->goPrevNextDock(IdealController::NextDock);
}

void MainWindowPrivate::selectPreviousDock()
{
    idealController->goPrevNextDock(IdealController::PrevDock);
}

Area::WalkerMode MainWindowPrivate::IdealToolViewCreator::operator() (View *view, Sublime::Position position)
{
    if (!d->docks.contains(view))
    {
        d->docks << view;

        //add view
        d->idealController->addView(d->positionToDockArea(position), view);
    }
    return Area::ContinueWalker;
}

Area::WalkerMode MainWindowPrivate::ViewCreator::operator() (AreaIndex *index)
{
    QSplitter *splitter = d->m_indexSplitters.value(index);
    if (!splitter)
    {
        //no splitter - we shall create it and populate with views
        if (!index->parent())
        {
            kDebug() << "reconstructing root area";
            //this is root area
            splitter = d->splitterCentralWidget;
            d->m_indexSplitters[index] = splitter;
            d->centralWidget->layout()->addWidget(splitter);
        }
        else
        {
            if (!d->m_indexSplitters.value(index->parent())) {
                // can happen in working set code, as that adds a view to a child index first
                // hence, recursively reconstruct the parent indizes first
                operator()(index->parent());
            }
            QSplitter *parent = d->m_indexSplitters.value(index->parent());
            splitter = new QSplitter(parent);
            d->m_indexSplitters[index] = splitter;
            
            if(index == index->parent()->first())
                parent->insertWidget(0, splitter);
            else
                parent->addWidget(splitter);
        }
        Q_ASSERT(splitter);
    }

    if (index->isSplit()) //this is a visible splitter
        splitter->setOrientation(index->orientation());
    else
    {
        Container *container = 0;
        
        while(splitter->count() && qobject_cast<QSplitter*>(splitter->widget(0)))
        {
            // After unsplitting, we might have to remove old splitters
            QWidget* widget = splitter->widget(0);
            kDebug() << "deleting" << widget;
            widget->setParent(0);
            delete widget;
        }
        
        if (!splitter->widget(0))
        {
            //we need to create view container
            container = new Container(splitter);
            connect(container, SIGNAL(activateView(Sublime::View*)), d->m_mainWindow, SLOT(activateView(Sublime::View*)));
            connect(container, SIGNAL(tabContextMenuRequested(Sublime::View*,KMenu*)),
                    d->m_mainWindow, SLOT(tabContextMenuRequested(Sublime::View*,KMenu*)));
            connect(container, SIGNAL(tabToolTipRequested(Sublime::View*,Sublime::Container*,int)),
                    d->m_mainWindow, SLOT(tabToolTipRequested(Sublime::View*,Sublime::Container*,int)));
            connect(container, SIGNAL(closeRequest(QWidget*)),
                    d, SLOT(widgetCloseRequest(QWidget*)), Qt::QueuedConnection);
            splitter->addWidget(container);
        }
        else
            container = qobject_cast<Container*>(splitter->widget(0));
        container->show();

        int position = 0;
        bool hadActiveView = false;
        Sublime::View* activeView = d->activeView;
        
        foreach (View *view, index->views())
        {
            QWidget *widget = view->widget(container);
            
            if (widget)
            {
                if(!container->hasWidget(widget))
                {
                    container->addWidget(view, position);
                    d->viewContainers[view] = container;
                    d->widgetToView[widget] = view;
                }
                if(activeView == view)
                {
                    hadActiveView = true;
                    container->setCurrentWidget(widget);
                }else if(topViews.contains(view) && !hadActiveView)
                    container->setCurrentWidget(widget);
            }
            position++;
        }
    }
    return Area::ContinueWalker;
}

void MainWindowPrivate::reconstructViews(QList<View*> topViews)
{
    ViewCreator viewCreator(this, topViews);
    area->walkViews(viewCreator, area->rootIndex());
    setBackgroundVisible(area->views().isEmpty());
}

void MainWindowPrivate::reconstruct()
{
    if(m_leftTabbarCornerWidget) {
        m_leftTabbarCornerWidget.data()->hide();
        m_leftTabbarCornerWidget.data()->setParent(0);
    }

    idealController->setWidthForArea(Qt::LeftDockWidgetArea, area->thickness(Sublime::Left));
    idealController->setWidthForArea(Qt::BottomDockWidgetArea, area->thickness(Sublime::Bottom));
    idealController->setWidthForArea(Qt::RightDockWidgetArea, area->thickness(Sublime::Right));

    IdealToolViewCreator toolViewCreator(this);
    area->walkToolViews(toolViewCreator, Sublime::AllPositions);

    reconstructViews();

    m_mainWindow->blockSignals(true);

    kDebug() << "RECONSTRUCT" << area << "  " << area->shownToolViews(Sublime::Left) << "\n";
    foreach (View *view, area->toolViews())
    {
        QString id = view->document()->documentSpecifier();
        if (!id.isEmpty())
        {
            Sublime::Position pos = area->toolViewPosition(view);
            if (area->shownToolViews(pos).contains(id))
                idealController->raiseView(view, IdealController::GroupWithOtherViews);
        }
    }
    m_mainWindow->blockSignals(false);
    
    setTabBarLeftCornerWidget(m_leftTabbarCornerWidget.data());
}

void MainWindowPrivate::clearArea()
{
    if(m_leftTabbarCornerWidget)
        m_leftTabbarCornerWidget.data()->setParent(0);
    
    //reparent toolview widgets to 0 to prevent their deletion together with dockwidgets
    foreach (View *view, area->toolViews())
    {
        // FIXME should we really delete here??
        bool nonDestructive = true;
        idealController->removeView(view, nonDestructive);

        if (view->hasWidget())
            view->widget()->setParent(0);
    }

    docks.clear();

    //reparent all view widgets to 0 to prevent their deletion together with central
    //widget. this reparenting is necessary when switching areas inside the same mainwindow
    foreach (View *view, area->views())
    {
        if (view->hasWidget())
            view->widget()->setParent(0);
    }
    cleanCentralWidget();
    m_mainWindow->setActiveView(0);
    m_indexSplitters.clear();
    area = 0;
    viewContainers.clear();
    
    setTabBarLeftCornerWidget(m_leftTabbarCornerWidget.data());
}

void MainWindowPrivate::cleanCentralWidget()
{
    while(splitterCentralWidget->count())
        delete splitterCentralWidget->widget(0);

    setBackgroundVisible(true);
}

struct ShownToolViewFinder {
    ShownToolViewFinder() {}
    Area::WalkerMode operator()(View *v, Sublime::Position /*position*/)
    {
        if (v->hasWidget() && v->widget()->isVisible())
            views << v;
        return Area::ContinueWalker;
    }
    QList<View *> views;
};

void MainWindowPrivate::slotDockShown(Sublime::View* /*view*/, Sublime::Position pos, bool /*shown*/)
{
    if (ignoreDockShown)
        return;

    ShownToolViewFinder finder;
    m_mainWindow->area()->walkToolViews(finder, pos);

    QStringList ids;
    foreach (View *v, finder.views) {
        ids << v->document()->documentSpecifier();
    }
    area->setShownToolViews(pos, ids);
}

void MainWindowPrivate::viewRemovedInternal(AreaIndex* index, View* view)
{
    Q_UNUSED(index);
    Q_UNUSED(view);
    // A formerly non-empty working-set has become empty, and a relayout of the area-selector may be required
    if(m_mainWindow->area()->views().size() == 0)
        m_mainWindow->setupAreaSelector();
    
    setBackgroundVisible(area->views().isEmpty());
}

void MainWindowPrivate::viewAdded(Sublime::AreaIndex *index, Sublime::View *view)
{
    if(m_leftTabbarCornerWidget) {
        m_leftTabbarCornerWidget.data()->hide();
        m_leftTabbarCornerWidget.data()->setParent(0);
    }

    {
         // Remove container objects in the hierarchy from the parents,
         // because they are not needed anymore, and might lead to broken splitter hierarchy and crashes.
        for(Sublime::AreaIndex* current = index; current; current = current->parent())
        {
        QSplitter *splitter = m_indexSplitters[current];
        if (current->isSplit() && splitter)
        {
            // Also update the orientation
            splitter->setOrientation(current->orientation());
            
            for(int w = 0; w < splitter->count(); ++w)
            {
                Container *container = qobject_cast<Sublime::Container*>(splitter->widget(w));
                //we need to remove extra container before reconstruction
                //first reparent widgets in container so that they are not deleted
                if(container)
                {
                    while (container->count())
                    {
                        container->widget(0)->setParent(0);
                    }
                    //and then delete the container
                    delete container;
                }
            }
        }
        }
    }

    ViewCreator viewCreator(this);
    area->walkViews(viewCreator, index);
    emit m_mainWindow->viewAdded( view );
    
    setTabBarLeftCornerWidget(m_leftTabbarCornerWidget.data());
    
    // A formerly empty working-set may become non-empty, and a relayout of the area-selector may be required
    if(m_mainWindow->area()->views().size() == 1)
        m_mainWindow->setupAreaSelector();
    
    setBackgroundVisible(false);
}

void Sublime::MainWindowPrivate::raiseToolView(Sublime::View * view)
{
    idealController->raiseView(view);
}

void MainWindowPrivate::aboutToRemoveView(Sublime::AreaIndex *index, Sublime::View *view)
{
    if (!m_indexSplitters[index])
        return;

    QSplitter *splitter = m_indexSplitters[index];
    kDebug() << "index " << index << " root " << area->rootIndex();
    kDebug() << "splitter " << splitter << " container " << splitter->widget(0);
    kDebug() << "structure: " << index->print() << " whole structure: " << area->rootIndex()->print();
    //find the container for the view and remove the widget
    Container *container = qobject_cast<Container*>(splitter->widget(0));
    if (!container) {
        kWarning() << "Splitter does not have a left widget!";
        return;
    }
    
    emit m_mainWindow->aboutToRemoveView( view );

    if (view->widget())
        widgetToView.remove(view->widget());
    viewContainers.remove(view);

    const bool wasActive = m_mainWindow->activeView() == view;
    if (container->count() > 1)
    {
        //container is not empty or this is a root index
        //just remove a widget
		if( view->widget() ) {
			container->removeWidget(view->widget());
			view->widget()->setParent(0);
			//activate what is visible currently in the container if the removed view was active
			if (wasActive)
				return m_mainWindow->setActiveView(container->viewForWidget(container->currentWidget()));
		}
    }
    else
    {
        if(m_leftTabbarCornerWidget) {
            m_leftTabbarCornerWidget.data()->hide();
            m_leftTabbarCornerWidget.data()->setParent(0);
        }
        
        // We've about to remove the last view of this container.  It will
        // be empty, so have to delete it, as well.

        // If we have a container, then it should be the only child of
        // the splitter.
        Q_ASSERT(splitter->count() == 1);
        container->removeWidget(view->widget());

        if (view->widget())
            view->widget()->setParent(0);
        else
            kWarning() << "View does not have a widget!";

        Q_ASSERT(container->count() == 0);
        // We can be called from signal handler of container
        // (which is tab widget), so defer deleting it.
        container->deleteLater();
        container->setParent(0);

        /* If we're not at the top level, we get to collapse split views.  */
        if (index->parent())
        {
            /* The splitter used to have container as the only child, now it's
               time to get rid of it.  Make sure deleting splitter does not
               delete container -- per above comment, we'll delete it later.  */
            QCommonStyle* tmpStyle = new QCommonStyle; // temp style since container uses it's parents style, which gets zeroed
            container->setStyle(tmpStyle);
            connect(container, SIGNAL(destroyed(QObject*)), tmpStyle, SLOT(deleteLater()));
            container->setParent(0);
            m_indexSplitters.remove(index);
            delete splitter;

            AreaIndex *parent = index->parent();
            QSplitter *parentSplitter = m_indexSplitters[parent];

            AreaIndex *sibling = parent->first() == index ? parent->second() : parent->first();
            QSplitter *siblingSplitter = m_indexSplitters[sibling];

            if(siblingSplitter)
            {
                HoldUpdates du(parentSplitter);
                //save sizes and orientation of the sibling splitter
                parentSplitter->setOrientation(siblingSplitter->orientation());
                QList<int> sizes = siblingSplitter->sizes();

                /* Parent has two children -- 'index' that we've deleted and
                'sibling'.  We move all children of 'sibling' into parent,
                and delete 'sibling'.  sibling either contains a single
                Container instance, or a bunch of further QSplitters.  */
                while (siblingSplitter->count() > 0)
                {
                    //reparent contents into parent splitter
                    QWidget *siblingWidget = siblingSplitter->widget(0);
                    siblingWidget->setParent(parentSplitter);
                    parentSplitter->addWidget(siblingWidget);
                }

                m_indexSplitters.remove(sibling);
                delete siblingSplitter;
                parentSplitter->setSizes(sizes);
            }

            kDebug() << "after deleation " << parent << " has "
                         << parentSplitter->count() << " elements";

            
            //find the container somewhere to activate
            Container *containerToActivate = parentSplitter->findChild<Sublime::Container*>();
            //activate the current view there
            if (containerToActivate) {
                m_mainWindow->setActiveView(containerToActivate->viewForWidget(containerToActivate->currentWidget()));
                setTabBarLeftCornerWidget(m_leftTabbarCornerWidget.data());
                return;
            }
        }
    }

    setTabBarLeftCornerWidget(m_leftTabbarCornerWidget.data());
    if ( wasActive ) {
        m_mainWindow->setActiveView(0L);
    }
}

void MainWindowPrivate::toolViewAdded(Sublime::View* /*toolView*/, Sublime::Position position)
{
    IdealToolViewCreator toolViewCreator(this);
    area->walkToolViews(toolViewCreator, position);
}

void MainWindowPrivate::aboutToRemoveToolView(Sublime::View *toolView, Sublime::Position /*position*/)
{
    if (!docks.contains(toolView))
        return;

    idealController->removeView(toolView);
    // TODO are Views unique?
    docks.removeAll(toolView);
}

void MainWindowPrivate::toolViewMoved(
    Sublime::View *toolView, Sublime::Position position)
{
    if (!docks.contains(toolView))
        return;

    idealController->moveView(toolView, positionToDockArea(position));
}

Qt::DockWidgetArea MainWindowPrivate::positionToDockArea(Position position)
{
    switch (position)
    {
        case Sublime::Left: return Qt::LeftDockWidgetArea;
        case Sublime::Right: return Qt::RightDockWidgetArea;
        case Sublime::Bottom: return Qt::BottomDockWidgetArea;
        case Sublime::Top: return Qt::TopDockWidgetArea;
        default: return Qt::LeftDockWidgetArea;
    }
}

void MainWindowPrivate::switchToArea(QAction *action)
{
    kDebug() << "for" << action;
    controller->showArea(m_actionAreas[action], m_mainWindow);
}

void MainWindowPrivate::updateAreaSwitcher(Sublime::Area *area)
{
    if (m_areaActions.contains(area))
        m_areaActions[area]->setChecked(true);
}

void MainWindowPrivate::activateFirstVisibleView()
{
    if (area->views().count() > 0)
        m_mainWindow->activateView(area->views().first());
}

void MainWindowPrivate::widgetResized(Qt::DockWidgetArea /*dockArea*/, int /*thickness*/)
{
    //TODO: adymo: remove all thickness business
}

void MainWindowPrivate::widgetCloseRequest(QWidget* widget)
{
    if (widgetToView.contains(widget))
    {
        View *view = widgetToView[widget];
        area->closeView(view);
    }
}

void MainWindowPrivate::toggleArea ( int index ) {
    m_mainWindow->controller()->showArea(areaSwitcher->tabBar->areaId(index), m_mainWindow);
}

void AreaTabBar::paintEvent ( QPaintEvent* ev ) {
    QTabBar::paintEvent(ev);
    if ( currentIndex() != -1 ) {
        QStylePainter p ( this );
        //Draw highlight behind current area
        QRect activeRect = tabRect ( currentIndex() );
        QRect tabArea = activeRect;
        QImage img ( tabArea.width(), tabArea.height(), QImage::Format_ARGB32 );
        img.fill ( 0 );
        QPainter paintImg ( &img );
        QColor color ( palette().color ( QPalette::Active, QPalette::Highlight ) );
        color.setAlpha ( 70 );
        QRect paint = tabArea;
        const int margin = 8;
        paint.setLeft ( margin );
        paint.setTop ( margin );
        paint.setRight ( activeRect.width()-margin );
        paint.setBottom ( activeRect.height()-margin );

        paintImg.fillRect ( paint, color );
        expblur<16,7> ( img, margin/2 );
        p.drawImage ( tabArea, img );
    }
}

void AreaTabWidget::paintEvent ( QPaintEvent* ev ) {
    QWidget::paintEvent ( ev );

    if ( tabBar->isVisible() && tabBar->count() > 0 ) {
        QStylePainter p ( this );

        QStyleOptionTabBarBase optTabBase;
        optTabBase.init ( tabBar );
        optTabBase.shape = tabBar->shape();
        optTabBase.tabBarRect = tabBar->rect();
        optTabBase.tabBarRect.moveTopLeft(tabBar->pos());

        int sideWidth = width()-tabBar->width();

        QStyleOptionTab tabOverlap;
        tabOverlap.shape = tabBar->shape();
        int overlap = style()->pixelMetric ( QStyle::PM_TabBarBaseOverlap, &tabOverlap, tabBar );
        QRect rect;
        rect.setRect ( 0, height()-overlap, sideWidth, overlap );
        optTabBase.rect = rect;

        QImage img ( sideWidth, height(), QImage::Format_ARGB32 );
        img.fill ( 0 );
        QStylePainter p2 ( &img, tabBar );
        p2.drawPrimitive ( QStyle::PE_FrameTabBarBase, optTabBase );

        {
            //Blend the painted line out to the left
            QLinearGradient blendOut ( 0, 0, sideWidth, 0 );
            blendOut.setColorAt ( 0, QColor ( 255, 255, 255, 255 ) );
            blendOut.setColorAt ( 1, QColor ( 0, 0, 0, 0 ) );
            QBrush blendBrush ( blendOut );

            p2.setCompositionMode ( QPainter::CompositionMode_DestinationOut );
            p2.fillRect ( img.rect(), blendBrush );
        }

        p.setCompositionMode ( QPainter::CompositionMode_SourceOver );
        p.drawImage ( img.rect(), img );
    }
}

QSize AreaTabWidget::sizeHint() const {
    QMenuBar* menuBar = qobject_cast<QMenuBar*>(parent());
    if ( !menuBar ) {
        return QWidget::sizeHint();
    }
    //Resize to hold the whole length up to the menu
    static bool zeroSizeHint = false;
    if ( zeroSizeHint )
        return QSize();
    zeroSizeHint = true;
    int available = menuBar->parentWidget()->width() - menuBar->sizeHint().width() - 10;
    zeroSizeHint = false;
    QSize orig = tabBar->sizeHint();
    int addFade = available - orig.width();
    
    int perfectFade = 500;
    if(areaSideWidget)
        perfectFade += areaSideWidget->sizeHint().width();
    
    if(addFade > perfectFade)
        addFade = perfectFade;
    
    int wantWidth = addFade + orig.width();
    
    if ( wantWidth > orig.width() )
        orig.setWidth ( wantWidth );
    return orig;
}

AreaTabWidget::AreaTabWidget ( QWidget* parent ) : QWidget ( parent ), areaSideWidget(0) {
    m_layout = new QHBoxLayout ( this );
    m_layout->setAlignment ( Qt::AlignRight );
    tabBar = new AreaTabBar ( this );
    m_layout->addWidget ( tabBar );
    m_layout->setContentsMargins ( 0,0,0,0 );
    m_leftLayout = new QVBoxLayout;
    
    m_leftLayout->setContentsMargins(11, 4, 11, 8); ///@todo These margins are a bit too hardcoded, should depend on style
    m_layout->insertLayout(0, m_leftLayout);
}

QSize AreaTabBar::tabSizeHint ( int index ) const {
    //Since we move all contents into the button, we give approximately the button size as size-hint
    //QTabBar seems to add useless space for the non-existing tab text
    QSize ret = QTabBar::tabSizeHint(index);
    
    if(ret.width() > buttons[index]->sizeHint().width()+16)
        ret.setWidth(buttons[index]->sizeHint().width() + 16); ///@todo Where does the offset come from?
    return ret;
}


void AreaTabButton::setIsCurrent ( bool arg1 ) {
    m_isCurrent = arg1;
}

void AreaTabBar::setCurrentIndex ( int index ) {
    m_currentIndex = index;
    foreach(AreaTabButton* button, buttons) {
        button->setIsCurrent(buttons.indexOf(button) == index);
    }
    QTabBar::setCurrentIndex ( index );
    foreach(AreaTabButton* button, buttons) {
        button->update();
    }
    update();
}

AreaTabBar::AreaTabBar ( QWidget* parent ) : QTabBar ( parent ), m_currentIndex ( -1 ) {
    setShape ( QTabBar::RoundedNorth );
    setDocumentMode ( true );
    setExpanding ( false );
    setLayoutDirection ( Qt::RightToLeft );
    setDrawBase ( false );
    setUsesScrollButtons ( false );
    setFocusPolicy( Qt::NoFocus );
    QPalette pal = palette();
}

AreaTabButton::AreaTabButton ( QString text, QIcon icon, uint iconSize, QWidget* parent, bool isCurrent, QWidget* _customButtonWidget )
	: QWidget ( parent ), customButtonWidget(_customButtonWidget), m_isCurrent ( isCurrent )
{
    QHBoxLayout* layout = new QHBoxLayout ( this );
    iconLabel = new QLabel ( this );
    iconLabel->setPixmap ( icon.pixmap ( QSize ( iconSize, iconSize ) ) );
    iconLabel->setAutoFillBackground ( false );
    textLabel = new QLabel ( this );
    textLabel->setText ( text );
    textLabel->setAutoFillBackground ( false );
    if(customButtonWidget) {
        customButtonWidget->setParent(this);
        layout->addWidget(customButtonWidget);
    }
    layout->addWidget ( textLabel );
    layout->addWidget ( iconLabel );
    layout->setMargin ( 0 );
}

void MainWindowPrivate::setTabBarLeftCornerWidget(QWidget* widget)
{
    if(widget != m_leftTabbarCornerWidget.data()) {
        delete m_leftTabbarCornerWidget.data();
        m_leftTabbarCornerWidget.clear();
    }
    m_leftTabbarCornerWidget = widget;
    
    if(!widget || !area || viewContainers.isEmpty())
        return;
    
    AreaIndex* putToIndex = area->rootIndex();
    QSplitter* splitter = m_indexSplitters[putToIndex];
    while(putToIndex->isSplit()) {
        putToIndex = putToIndex->first();
        splitter = m_indexSplitters[putToIndex];
    }
    
//     Q_ASSERT(splitter || putToIndex == area->rootIndex());
    
    Container* c = 0;
    if(splitter) {
        c = qobject_cast<Container*>(splitter->widget(0));
    }else{
        c = viewContainers.values()[0];
    }
    Q_ASSERT(c);
    
    c->setLeftCornerWidget(widget);
}

}

#include "mainwindow_p.moc"
#include "moc_mainwindow_p.cpp"

