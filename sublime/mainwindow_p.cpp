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
#include "ideal.h"
#include <KToolBar>
#include <KSelectAction>
#include <ktoggleaction.h>

namespace Sublime {

MainWindowPrivate::MainWindowPrivate(MainWindow *w, Controller* controller)
:controller(controller), area(0), activeView(0), activeToolView(0), centralWidget(0),
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

    action = new KAction(i18n("Show Top Dock"), this);
    action->setCheckable(true);
    action->setShortcut(Qt::META | Qt::CTRL | Qt::Key_T);
    connect(action, SIGNAL(toggled(bool)), SLOT(showTopDock(bool)));
    ac->addAction("show_top_dock", action);

    action = new KAction(i18n("Focus Editor"), this);
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

    action = new KAction(i18n("Remove view"), this);
    connect(action, SIGNAL(triggered(bool)), SLOT(removeView()));
    ac->addAction("remove_view", action);

    action = new KAction(i18n("Anchor Current Dock"), this);
    action->setCheckable(true);
    action->setEnabled(false);
    action->setShortcut(Qt::META | Qt::CTRL | Qt::Key_A);
    connect(action, SIGNAL(toggled(bool)), SLOT(anchorCurrentDock(bool)));
    ac->addAction("anchor_current_dock", action);

    action = new KAction(i18n("Maximize Current Dock"), this);
    action->setCheckable(true);
    action->setEnabled(false);
    connect(action, SIGNAL(toggled(bool)), SLOT(maximizeCurrentDock(bool)));
    ac->addAction("maximize_current_dock", action);

    action = new KActionMenu(i18n("Tool Views"), this);
    ac->addAction("docks_submenu", action);

    recreateCentralWidget();
}


MainWindowPrivate::~MainWindowPrivate()
{
    delete m_leftTabbarCornerWidget;
}

void MainWindowPrivate::showLeftDock(bool b)
{
    idealMainWidget->showLeftDock(b);
}

void MainWindowPrivate::showBottomDock(bool b)
{
    idealMainWidget->showBottomDock(b);
}

void MainWindowPrivate::showRightDock(bool b)
{
    idealMainWidget->showRightDock(b);
}

void MainWindowPrivate::showTopDock(bool b)
{
    idealMainWidget->showTopDock(b);
}

void MainWindowPrivate::anchorCurrentDock(bool b)
{
    idealMainWidget->anchorCurrentDock(b);
}

void MainWindowPrivate::maximizeCurrentDock(bool b)
{
    idealMainWidget->maximizeCurrentDock(b);
}

void MainWindowPrivate::focusEditor()
{
    idealMainWidget->focusEditor();
}

void MainWindowPrivate::toggleDocksShown()
{
    idealMainWidget->toggleDocksShown();
}

void MainWindowPrivate::removeView()
{
    idealMainWidget->removeView();
}

void MainWindowPrivate::selectNextDock()
{
    idealMainWidget->selectNextDock();
}

void MainWindowPrivate::selectPreviousDock()
{
    idealMainWidget->selectPreviousDock();
}

Area::WalkerMode MainWindowPrivate::IdealToolViewCreator::operator() (View *view, Sublime::Position position)
{
    if (!d->docks.contains(view))
    {
        d->docks << view;
        d->idealMainWidget->addView(d->positionToDockArea(position), view);
    }
    return Area::ContinueWalker;
}

Area::WalkerMode MainWindowPrivate::ViewCreator::operator() (AreaIndex *index)
{
    kDebug() << "reconstructing views for area index" << index;
    QSplitter *parent = 0;
    QSplitter *splitter = d->m_indexSplitters.value(index);
    if (!splitter)
    {
        //no splitter - we shall create it and populate with views
        if (!index->parent())
        {
            kDebug() << "reconstructing root area";
            //this is root area
            splitter = new QSplitter(d->centralWidget);
            d->m_indexSplitters[index] = splitter;
            d->centralWidget->layout()->addWidget(splitter);
        }
        else
        {
            parent = d->m_indexSplitters[index->parent()];
            Q_ASSERT(parent);
            kDebug() << "adding new splitter to" << parent;
            splitter = new QSplitter(parent);
            d->m_indexSplitters[index] = splitter;
            parent->addWidget(splitter);
        }
    }
    splitter->show();

    if (index->isSplitted()) //this is a visible splitter
        splitter->setOrientation(index->orientation());
    else
    {
        Container *container = 0;
        if (!splitter->widget(0))
        {
            qDebug() << "new container";
            //we need to create view container
            container = new Container(splitter);
            connect(container, SIGNAL(activateView(Sublime::View*)), d->m_mainWindow, SLOT(activateView(Sublime::View*)));
            connect(container, SIGNAL(tabContextMenuRequested(Sublime::View*,KMenu*)),
                    d->m_mainWindow, SLOT(tabContextMenuRequested(Sublime::View*,KMenu*)));
            connect(container, SIGNAL(tabToolTipRequested(Sublime::View*,QPoint)),
                    d->m_mainWindow, SLOT(tabToolTipRequested(Sublime::View*,QPoint)));
            connect(container, SIGNAL(closeRequest(QWidget*)),
                    d, SLOT(widgetCloseRequest(QWidget*)), Qt::QueuedConnection);
            splitter->addWidget(container);
        }
        else
            container = qobject_cast<Container*>(splitter->widget(0));
        container->show();

        int position = 0;
        foreach (View *view, index->views())
        {
            QWidget *widget = view->widget(container);
            if (widget && !container->hasWidget(widget))
            {
                widget->installEventFilter(d);
                foreach (QWidget* w, widget->findChildren<QWidget*>())
                    w->installEventFilter(d);
                container->addWidget(view, position);
                d->viewContainers[view] = container;
                d->widgetToView[widget] = view;
            }
            position++;
        }
    }
    return Area::ContinueWalker;
}

void MainWindowPrivate::reconstruct()
{
    if(m_leftTabbarCornerWidget) {
        m_leftTabbarCornerWidget->hide();
        m_leftTabbarCornerWidget->setParent(0);
    }
    
    IdealToolViewCreator toolViewCreator(this);
    area->walkToolViews(toolViewCreator, Sublime::AllPositions);

    ViewCreator viewCreator(this);
    area->walkViews(viewCreator, area->rootIndex());

    idealMainWidget->blockSignals(true);
    IdealMainLayout *l = idealMainWidget->mainLayout();
    l->setWidthForRole(IdealMainLayout::Left, area->thickness(Sublime::Left));
    l->setWidthForRole(IdealMainLayout::Right, area->thickness(Sublime::Right));
    l->setWidthForRole(IdealMainLayout::Bottom,
                       area->thickness(Sublime::Bottom));
    l->setWidthForRole(IdealMainLayout::Top, area->thickness(Sublime::Top));
    kDebug() << "RECONSTRUCT" << area << "  " << area->shownToolView(Sublime::Left) << "\n";
    foreach (View *view, area->toolViews())
    {
        QString id = view->document()->documentSpecifier();
        if (!id.isEmpty())
        {
            Sublime::Position pos = area->toolViewPosition(view);
            if (area->shownToolView(pos) == id)
                idealMainWidget->raiseView(view);
        }
    }
    idealMainWidget->blockSignals(false);
    
    setTabBarLeftCornerWidget(m_leftTabbarCornerWidget);
}

void MainWindowPrivate::clearArea()
{
    if(m_leftTabbarCornerWidget)
        m_leftTabbarCornerWidget->setParent(0);
    
    //reparent toolview widgets to 0 to prevent their deletion together with dockwidgets
    foreach (View *view, area->toolViews())
    {
        // FIXME should we really delete here??
        bool nonDestructive = true;
        idealMainWidget->removeView(view, nonDestructive);

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
    recreateCentralWidget();
    m_mainWindow->setActiveView(0);
    m_indexSplitters.clear();
    area = 0;
    viewContainers.clear();
    
    setTabBarLeftCornerWidget(m_leftTabbarCornerWidget);
}

void MainWindowPrivate::recreateCentralWidget()
{
    idealMainWidget = new IdealMainWidget(m_mainWindow, m_mainWindow->actionCollection());
    m_mainWindow->setCentralWidget(idealMainWidget);

    centralWidget = new QWidget();
    idealMainWidget->setCentralWidget(centralWidget);

    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setMargin(0);
    centralWidget->setLayout(layout);

    connect(idealMainWidget,
            SIGNAL(dockShown(Sublime::View*, Sublime::Position, bool)),
            this,
            SLOT(slotDockShown(Sublime::View*, Sublime::Position, bool)));

    connect(idealMainWidget->mainLayout(),
            SIGNAL(widgetResized(IdealMainLayout::Role, int)),
            this,
            SLOT(widgetResized(IdealMainLayout::Role, int)));

    connect(idealMainWidget, SIGNAL(dockBarContextMenuRequested(Qt::DockWidgetArea, const QPoint&)),
            m_mainWindow, SLOT(dockBarContextMenuRequested(Qt::DockWidgetArea, const QPoint&)));
}

void MainWindowPrivate::
slotDockShown(Sublime::View* view, Sublime::Position pos, bool shown)
{
    if (ignoreDockShown)
        return;

    QString id;
    if (shown)
        id = view->document()->documentSpecifier();
    kDebug() << "View " << view->document()->documentSpecifier() << " " << shown;
    area->setShownToolView(pos, id);
}

void MainWindowPrivate::viewRemovedInternal(AreaIndex* index, View* view)
{
    Q_UNUSED(index);
    Q_UNUSED(view);
    // A formerly non-empty working-set has become empty, and a relayout of the area-selector may be required
    if(m_mainWindow->area()->views().size() == 0)
        m_mainWindow->setupAreaSelector();
}

void MainWindowPrivate::viewAdded(Sublime::AreaIndex *index, Sublime::View *view)
{
    if(m_leftTabbarCornerWidget) {
        m_leftTabbarCornerWidget->hide();
        m_leftTabbarCornerWidget->setParent(0);
    }
    
    ViewCreator viewCreator(this);
    QSplitter *splitter = m_indexSplitters[index];
    if (index->isSplitted() && (splitter->count() == 1) &&
            qobject_cast<Sublime::Container*>(splitter->widget(0)))
    {
        Container *container = qobject_cast<Sublime::Container*>(splitter->widget(0));
        //we need to remove extra container before reconstruction
        //first reparent widgets in container so that they are not deleted
        while (container->count())
        {
            container->widget(0)->setParent(0);
        }
        //and then delete the container
        delete container;
    }
    area->walkViews(viewCreator, index);
    emit m_mainWindow->viewAdded( view );
    
    setTabBarLeftCornerWidget(m_leftTabbarCornerWidget);
    
    // A formerly empty working-set may become non-empty, and a relayout of the area-selector may be required
    if(m_mainWindow->area()->views().size() == 1)
        m_mainWindow->setupAreaSelector();
}

void Sublime::MainWindowPrivate::raiseToolView(Sublime::View * view)
{
    idealMainWidget->raiseView(view);
}

void MainWindowPrivate::aboutToRemoveView(Sublime::AreaIndex *index, Sublime::View *view)
{
    if (!m_indexSplitters.contains(index))
        return;

    QSplitter *splitter = m_indexSplitters[index];
    kDebug() << "index " << index << " root " << area->rootIndex();
    kDebug() << "splitter " << splitter << " container " << splitter->widget(0);
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
            m_leftTabbarCornerWidget->hide();
            m_leftTabbarCornerWidget->setParent(0);
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

            parentSplitter->setUpdatesEnabled(false);
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
            parentSplitter->setUpdatesEnabled(true);

            kDebug() << "after deleation " << parent << " has "
                         << parentSplitter->count() << " elements";

            
            //find the container somewhere to activate
            Container *containerToActivate = parentSplitter->findChild<Sublime::Container*>();
            //activate the current view there
            if (containerToActivate) {
                m_mainWindow->setActiveView(containerToActivate->viewForWidget(containerToActivate->currentWidget()));
                setTabBarLeftCornerWidget(m_leftTabbarCornerWidget);
                return;
            }
        }
    }

    setTabBarLeftCornerWidget(m_leftTabbarCornerWidget);
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

    idealMainWidget->removeView(toolView);
    // TODO are Views unique?
    docks.removeAll(toolView);
}

void MainWindowPrivate::toolViewMoved(
    Sublime::View *toolView, Sublime::Position position)
{
    if (!docks.contains(toolView))
        return;

    idealMainWidget->moveView(toolView, positionToDockArea(position));
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

bool MainWindowPrivate::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
        idealMainWidget->centralWidgetFocused();

    return false;
}

void MainWindowPrivate::widgetResized(IdealMainLayout::Role role, int thickness)
{
    area->setThickness(IdealMainLayout::positionForRole(role), thickness);
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
    //Resize to hold the whole length up to the menu
    static bool zeroSizeHint = false;
    if ( zeroSizeHint )
        return QSize();
    zeroSizeHint = true;
    int available = bar()->parentWidget()->width() - bar()->sizeHint().width() - 10;
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

AreaTabWidget::AreaTabWidget ( QMenuBar* parent ) : QWidget ( parent ), areaSideWidget(0) {
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
    if(widget != m_leftTabbarCornerWidget)
        delete m_leftTabbarCornerWidget;
    m_leftTabbarCornerWidget = widget;
    
    if(!widget || !area || viewContainers.isEmpty())
        return;
    
    AreaIndex* putToIndex = area->rootIndex();
    QSplitter* splitter = m_indexSplitters[putToIndex];
    while(putToIndex->isSplitted()) {
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

