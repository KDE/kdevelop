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
#include "container.h"

#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QEvent>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>
#include <QStackedWidget>
#include <QStyleOptionTabBarBase>
#include <QStylePainter>
#include <QTabBar>
#include <QToolButton>

#include <KAcceleratorManager>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "view.h"
#include "document.h"
#include <ksqueezedtextlabel.h>

namespace Sublime {

// struct ContainerPrivate

class ContainerTabBar : public QTabBar
{
    Q_OBJECT

public:
    ContainerTabBar(Container* container)
        : QTabBar(container), m_container(container)
    {
        installEventFilter(this);
    }

    bool event(QEvent* ev) override {
        if(ev->type() == QEvent::ToolTip)
        {
            ev->accept();

            int tab = tabAt(mapFromGlobal(QCursor::pos()));

            if(tab != -1)
            {
                m_container->showTooltipForTab(tab);
            }

            return true;
        }

        return QTabBar::event(ev);
    }
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::MidButton) {
            // just close on midbutton, drag can still be done with left mouse button

            int tab = tabAt(mapFromGlobal(QCursor::pos()));
            if (tab != -1) {
                emit tabCloseRequested(tab);
            }
            return;
        }
        QTabBar::mousePressEvent(event);
    }

    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if (obj != this) {
            return QObject::eventFilter(obj, event);
        }

        // TODO Qt6: Move to mouseDoubleClickEvent when fixme in qttabbar.cpp is resolved
        // see "fixme Qt 6: move to mouseDoubleClickEvent(), here for BC reasons." in qtabbar.cpp
        if (event->type() == QEvent::MouseButtonDblClick) {
            // block tabBarDoubleClicked signals with RMB, see https://bugs.kde.org/show_bug.cgi?id=356016
            auto mouseEvent = static_cast<const QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::MidButton) {
                return true;
            }
        }

        return QObject::eventFilter(obj, event);
    }

Q_SIGNALS:
    void newTabRequested();

private:
    Container* m_container;
};

bool sortViews(const View* const lhs, const View* const rhs)
{
        return lhs->document()->title().compare(rhs->document()->title(), Qt::CaseInsensitive) < 0;
}

struct ContainerPrivate {
    QBoxLayout* layout;
    QHash<QWidget*, View*> viewForWidget;

    ContainerTabBar *tabBar;
    QStackedWidget *stack;
    KSqueezedTextLabel *fileNameCorner;
    QLabel *fileStatus;
    KSqueezedTextLabel *statusCorner;
    QPointer<QWidget> leftCornerWidget;
    QToolButton* documentListButton;
    QMenu* documentListMenu;
    QHash<View*, QAction*> documentListActionForView;

    /**
     * Updates the context menu which is shown when
     * the document list button in the tab bar is clicked.
     *
     * It shall build a popup menu which contains all currently
     * enabled views using the title their document provides.
     */
    void updateDocumentListPopupMenu()
    {
        qDeleteAll(documentListActionForView);
        documentListActionForView.clear();
        documentListMenu->clear();

        // create a lexicographically sorted list
        QVector<View*> views;
        views.reserve(viewForWidget.size());

        foreach(View* view, viewForWidget){
            views << view;
        }

        std::sort(views.begin(), views.end(), sortViews);

        foreach(View* view, views) {
            QAction* action = documentListMenu->addAction(view->document()->title());
            action->setData(QVariant::fromValue(view));
            documentListActionForView[view] = action;
            action->setIcon(view->document()->icon());
            ///FIXME: push this code somehow into shell, such that we can access the project model for
            ///       icons and also get a neat, short path like the document switcher.
        }
    }
};

class UnderlinedLabel: public KSqueezedTextLabel {
Q_OBJECT
public:
    UnderlinedLabel(QTabBar *tabBar, QWidget* parent = 0)
        :KSqueezedTextLabel(parent), m_tabBar(tabBar)
    {
    }

protected:
    void paintEvent(QPaintEvent *ev) override
    {
        if (m_tabBar->isVisible() && m_tabBar->count() > 0)
        {
            QStylePainter p(this);
            QStyleOptionTabBarBase optTabBase;
            optTabBase.init(m_tabBar);
            optTabBase.shape = m_tabBar->shape();
            optTabBase.tabBarRect = m_tabBar->rect();
            optTabBase.tabBarRect.moveRight(0);

            QStyleOptionTab tabOverlap;
            tabOverlap.shape = m_tabBar->shape();
            int overlap = style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap, m_tabBar);
            if( overlap > 0 )
            {
                QRect rect;
                rect.setRect(0, height()-overlap, width(), overlap);
                optTabBase.rect = rect;
            }
            if( m_tabBar->drawBase() )
            {
                p.drawPrimitive(QStyle::PE_FrameTabBarBase, optTabBase);
            }
        }

        KSqueezedTextLabel::paintEvent(ev);
    }

    QTabBar *m_tabBar;
};


class StatusLabel: public UnderlinedLabel {
Q_OBJECT
public:
    StatusLabel(QTabBar *tabBar, QWidget* parent = 0):
        UnderlinedLabel(tabBar, parent)
    {
        setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    }

    QSize minimumSizeHint() const override
    {
        QRect rect = style()->itemTextRect(fontMetrics(), QRect(), Qt::AlignRight, true, i18n("Line: 00000 Col: 000"));
        rect.setHeight(m_tabBar->height());
        return rect.size();
    }
};

// class Container

Container::Container(QWidget *parent)
    :QWidget(parent), d(new ContainerPrivate())
{
    KAcceleratorManager::setNoAccel(this);

    QBoxLayout *l = new QBoxLayout(QBoxLayout::TopToBottom, this);
    l->setMargin(0);
    l->setSpacing(0);

    d->layout = new QBoxLayout(QBoxLayout::LeftToRight);
    d->layout->setMargin(0);
    d->layout->setSpacing(0);

    d->documentListMenu = new QMenu(this);
    d->documentListButton = new QToolButton(this);
    d->documentListButton->setIcon(QIcon::fromTheme(QStringLiteral("format-list-unordered")));
    d->documentListButton->setMenu(d->documentListMenu);
    d->documentListButton->setPopupMode(QToolButton::InstantPopup);
    d->documentListButton->setAutoRaise(true);
    d->documentListButton->setToolTip(i18n("Show sorted list of opened documents"));
    d->documentListButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    d->layout->addWidget(d->documentListButton);
    d->tabBar = new ContainerTabBar(this);
    d->tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    d->layout->addWidget(d->tabBar);
    d->fileStatus = new QLabel( this );
    d->fileStatus->setFixedSize( QSize( 16, 16 ) );
    d->layout->addWidget(d->fileStatus);
    d->fileNameCorner = new UnderlinedLabel(d->tabBar, this);
    d->layout->addWidget(d->fileNameCorner);
    d->statusCorner = new StatusLabel(d->tabBar, this);
    d->layout->addWidget(d->statusCorner);
    l->addLayout(d->layout);

    d->stack = new QStackedWidget(this);
    l->addWidget(d->stack);

    connect(d->tabBar, &ContainerTabBar::currentChanged, this, &Container::widgetActivated);
    connect(d->tabBar, &ContainerTabBar::tabCloseRequested, this, static_cast<void(Container::*)(int)>(&Container::requestClose));
    connect(d->tabBar, &ContainerTabBar::newTabRequested, this, &Container::newTabRequested);
    connect(d->tabBar, &ContainerTabBar::tabMoved, this, &Container::tabMoved);
    connect(d->tabBar, &ContainerTabBar::customContextMenuRequested, this, &Container::contextMenu);
    connect(d->tabBar, &ContainerTabBar::tabBarDoubleClicked, this, &Container::doubleClickTriggered);
    connect(d->documentListMenu, &QMenu::triggered, this, &Container::documentListActionTriggered);


    setTabBarHidden(!configTabBarVisible());
    d->tabBar->setTabsClosable(true);
    d->tabBar->setMovable(true);
    d->tabBar->setExpanding(false);
    d->tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
}

bool Container::configTabBarVisible()
{
    KConfigGroup group = KSharedConfig::openConfig()->group("UiSettings");
    return group.readEntry("TabBarVisibility", 1);
}

void Container::setLeftCornerWidget(QWidget* widget)
{
    if(d->leftCornerWidget.data() == widget) {
        if(d->leftCornerWidget)
            d->leftCornerWidget.data()->setParent(0);
    }else{
        delete d->leftCornerWidget.data();
        d->leftCornerWidget.clear();
    }
    d->leftCornerWidget = widget;
    if(!widget)
        return;
    widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    d->layout->insertWidget(0, widget);
    widget->show();
}

Container::~Container()
{
    delete d;
}

QList<View*> Container::views() const
{
    return d->viewForWidget.values();
}

void Container::requestClose(int idx)
{
    emit requestClose(widget(idx));
}

void Container::widgetActivated(int idx)
{
    if (idx < 0)
        return;
    if (QWidget* w = d->stack->widget(idx)) {
        Sublime::View* view = d->viewForWidget.value(w);
        if(view)
            emit activateView(view);
    }
}

void Container::addWidget(View *view, int position)
{
    QWidget *w = view->widget(this);
    int idx = 0;
    if (position != -1)
    {
        idx = d->stack->insertWidget(position, w);
    }
    else
        idx = d->stack->addWidget(w);
    d->tabBar->insertTab(idx, view->document()->statusIcon(), view->document()->title());
    Q_ASSERT(view);
    d->viewForWidget[w] = view;

    // Update document list context menu. This has to be called before
    // setCurrentWidget, because we call the status icon and title update slots
    // already, which in turn need the document list menu to be setup.
    d->updateDocumentListPopupMenu();

    setCurrentWidget(d->stack->currentWidget());

    // This fixes a strange layouting bug, that could be reproduced like this: Open a few files in KDevelop, activate the rightmost tab.
    // Then temporarily switch to another area, and then switch back. After that, the tab-bar was gone.
    // The problem could only be fixed by closing/opening another view.
    d->tabBar->setMinimumHeight(d->tabBar->sizeHint().height());

    connect(view, &View::statusChanged, this, &Container::statusChanged);
    connect(view->document(), &Document::statusIconChanged, this, &Container::statusIconChanged);
    connect(view->document(), &Document::titleChanged, this, &Container::documentTitleChanged);
}

void Container::statusChanged(Sublime::View* view)
{
    d->statusCorner->setText(view->viewStatus());
}


void Container::statusIconChanged(Document* doc)
{
    QHashIterator<QWidget*, View*> it = d->viewForWidget;
    while (it.hasNext()) {
        if (it.next().value()->document() == doc) {
            d->fileStatus->setPixmap( doc->statusIcon().pixmap( QSize( 16,16 ) ) );
            int tabIndex = d->stack->indexOf(it.key());
            if (tabIndex != -1) {
                d->tabBar->setTabIcon(tabIndex, doc->statusIcon());
            }

            // Update the document title's menu associated action
            // using the View* index map
            Q_ASSERT(d->documentListActionForView.contains(it.value()));
            d->documentListActionForView[it.value()]->setIcon(doc->icon());
            break;
        }
    }
}

void Container::documentTitleChanged(Sublime::Document* doc)
{
    QHashIterator<QWidget*, View*> it = d->viewForWidget;
    while (it.hasNext()) {
        Sublime::View* view = it.next().value();
        if (view->document() == doc) {
            if (currentView() == view) {
                d->fileNameCorner->setText( doc->title(Document::Extended) + i18n(" <i><small>(Press Ctrl+Tab to switch)</small></i>") );
            }
            int tabIndex = d->stack->indexOf(it.key());
            if (tabIndex != -1) {
                d->tabBar->setTabText(tabIndex, doc->title());
            }

            // Update document list popup title
            Q_ASSERT(d->documentListActionForView.contains(view));
            d->documentListActionForView[view]->setText(doc->title());
            break;
        }
    }
}

int Container::count() const
{
    return d->stack->count();
}

QWidget* Container::currentWidget() const
{
    return d->stack->currentWidget();
}

void Container::setCurrentWidget(QWidget* w)
{
    d->stack->setCurrentWidget(w);
    //prevent from emitting activateView() signal on tabbar active tab change
    //this function is called from MainWindow::activateView()
    //which does the activation without any additional signals
    d->tabBar->blockSignals(true);
    d->tabBar->setCurrentIndex(d->stack->indexOf(w));
    d->tabBar->blockSignals(false);
    if (View* view = viewForWidget(w))
    {
        statusChanged(view);
        if (!d->tabBar->isVisible())
        {
            // repaint icon and document title only in tabbar-less mode
            // tabbar will do repainting for us
            statusIconChanged( view->document() );
            documentTitleChanged( view->document() );
        }
    }
}

QWidget* Container::widget(int i) const
{
    return d->stack->widget(i);
}

int Container::indexOf(QWidget* w) const
{
    return d->stack->indexOf(w);
}

void Container::removeWidget(QWidget *w)
{
    if (w) {
        int widgetIdx = d->stack->indexOf(w);
        d->stack->removeWidget(w);
        d->tabBar->removeTab(widgetIdx);
        if (d->tabBar->currentIndex() != -1 && !d->tabBar->isVisible()) {
            // repaint icon and document title only in tabbar-less mode
            // tabbar will do repainting for us
            View* view = currentView();
            if( view ) {
                statusIconChanged( view->document() );
                documentTitleChanged( view->document() );
            }
        }
        View* view = d->viewForWidget.take(w);
        if (view)
        {
            disconnect(view->document(), &Document::titleChanged, this, &Container::documentTitleChanged);
            disconnect(view->document(), &Document::statusIconChanged, this, &Container::statusIconChanged);
            disconnect(view, &View::statusChanged, this, &Container::statusChanged);

            // Update document list context menu
            Q_ASSERT(d->documentListActionForView.contains(view));
            delete d->documentListActionForView.take(view);
        }
    }
}

bool Container::hasWidget(QWidget *w)
{
    return d->stack->indexOf(w) != -1;
}

View *Container::viewForWidget(QWidget *w) const
{
    return d->viewForWidget.value(w);
}

void Container::setTabBarHidden(bool hide)
{
    if (hide)
    {
        d->tabBar->hide();
        d->fileNameCorner->show();
        d->fileStatus->show();
    }
    else
    {
        d->fileNameCorner->hide();
        d->fileStatus->hide();
        d->tabBar->show();
    }
    View* v = currentView();
    if (v) {
        documentTitleChanged(v->document());
    }
}

void Container::resetTabColors(const QColor& color)
{
    for (int i = 0; i < count(); i++){
        d->tabBar->setTabTextColor(i, color);
    }
}

void Container::setTabColor(const View* view, const QColor& color)
{
    for (int i = 0; i < count(); i++){
        if (view == viewForWidget(widget(i))) {
            d->tabBar->setTabTextColor(i, color);
        }
    }
}

void Container::setTabColors(const QHash<const View*, QColor>& colors)
{
    for (int i = 0; i < count(); i++) {
        auto view = viewForWidget(widget(i));
        auto color = colors[view];
        if (color.isValid()) {
            d->tabBar->setTabTextColor(i, color);
        }
    }
}

void Container::tabMoved(int from, int to)
{
    QWidget *w = d->stack->widget(from);
    d->stack->removeWidget(w);
    d->stack->insertWidget(to, w);
    d->viewForWidget[w]->notifyPositionChanged(to);
}

void Container::contextMenu( const QPoint& pos )
{
    QWidget* senderWidget = qobject_cast<QWidget*>(sender());
    Q_ASSERT(senderWidget);

    int currentTab = d->tabBar->tabAt(pos);

    QMenu menu;

    Sublime::View* view = viewForWidget(widget(currentTab));
    emit tabContextMenuRequested(view, &menu);

    menu.addSeparator();
    QAction* copyPathAction = nullptr;
    QAction* closeTabAction = nullptr;
    QAction* closeOtherTabsAction = nullptr;
    if (view) {
        copyPathAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-copy")),
                                        i18n("Copy Filename"));
        menu.addSeparator();
        closeTabAction = menu.addAction(QIcon::fromTheme(QStringLiteral("document-close")),
                                        i18n("Close File"));
        closeOtherTabsAction = menu.addAction(QIcon::fromTheme(QStringLiteral("document-close")),
                                              i18n("Close Other Files"));
    }
    QAction* closeAllTabsAction = menu.addAction( QIcon::fromTheme(QStringLiteral("document-close")), i18n( "Close All Files" ) );

    QAction* triggered = menu.exec(senderWidget->mapToGlobal(pos));

    if (triggered) {
        if ( triggered == closeTabAction ) {
            requestClose(currentTab);
        } else if ( triggered == closeOtherTabsAction ) {
            // activate the remaining tab
            widgetActivated(currentTab);
            // first get the widgets to be closed since otherwise the indices will be wrong
            QList<QWidget*> otherTabs;
            for ( int i = 0; i < count(); ++i ) {
                if ( i != currentTab ) {
                    otherTabs << widget(i);
                }
            }
            // finally close other tabs
            foreach( QWidget* tab, otherTabs ) {
                requestClose(tab);
            }
        } else if ( triggered == closeAllTabsAction ) {
            // activate last tab
            widgetActivated(count() - 1);

            // close all
            for ( int i = 0; i < count(); ++i ) {
                requestClose(widget(i));
            }
        } else if( triggered == copyPathAction ) {
            auto view = viewForWidget( widget( currentTab ) );
            auto urlDocument = qobject_cast<UrlDocument*>( view->document() );
            if( urlDocument ) {
                QApplication::clipboard()->setText( urlDocument->url().toString() );
            }
        } // else the action was handled by someone else
    }
}

void Container::showTooltipForTab(int tab)
{
    emit tabToolTipRequested(viewForWidget(widget(tab)), this, tab);
}

bool Container::isCurrentTab(int tab) const
{
    return d->tabBar->currentIndex() == tab;
}

QRect Container::tabRect(int tab) const
{
    return d->tabBar->tabRect(tab).translated(d->tabBar->mapToGlobal(QPoint(0, 0)));
}

void Container::doubleClickTriggered(int tab)
{
    if (tab == -1) {
        emit newTabRequested();
    } else {
        emit tabDoubleClicked(viewForWidget(widget(tab)));
    }
}

void Container::documentListActionTriggered(QAction* action)
{
    Sublime::View* view = action->data().value< Sublime::View* >();
    Q_ASSERT(view);
    QWidget* widget = d->viewForWidget.key(view);
    Q_ASSERT(widget);
    setCurrentWidget(widget);
}
Sublime::View* Container::currentView() const
{
    return d->viewForWidget.value(widget( d->tabBar->currentIndex() ));
}

}

#include "container.moc"
