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

#include <QtCore/QMap>
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QStylePainter>
#include <QtGui/QStackedWidget>
#include <QtGui/QStyleOptionTabBarBase>
#include <QtGui/qstyle.h>

#include <kdebug.h>
#include <klocale.h>
#include <ktabbar.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kglobal.h>
#include <kdeversion.h>
#include <kacceleratormanager.h>
#include <kmenu.h>
#include <kicon.h>

#include "view.h"
#include "document.h"
#include <qpointer.h>
#include <QEvent>
#include <QKeyEvent>
#include "urldocument.h"
#include <ksqueezedtextlabel.h>

namespace Sublime {

// struct ContainerPrivate

class ContainerTabBar : public KTabBar {
    public:
    ContainerTabBar(Container* container) : KTabBar(container), m_container(container) {
    }
    
    virtual bool event(QEvent* ev) {
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
        
        return KTabBar::event(ev);
    }
    virtual void mousePressEvent(QMouseEvent* event) {
        if (event->button() == Qt::MidButton) {
            // just close on midbutton, drag can still be done with left mouse button
            return;
        }
        KTabBar::mousePressEvent(event);
    }
    Container* m_container;
};

struct ContainerPrivate {
    QMap<QWidget*, View*> viewForWidget;

    ContainerTabBar *tabBar;
    QStackedWidget *stack;
    KSqueezedTextLabel *fileNameCorner;
    QLabel *fileStatus;
    KSqueezedTextLabel *statusCorner;
    QPointer<QWidget> leftCornerWidget;
};

class UnderlinedLabel: public KSqueezedTextLabel {
public:
    UnderlinedLabel(KTabBar *tabBar, QWidget* parent = 0)
        :KSqueezedTextLabel(parent), m_tabBar(tabBar)
    {
    }

protected:
    virtual void paintEvent(QPaintEvent *ev)
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

    KTabBar *m_tabBar;
};


class StatusLabel: public UnderlinedLabel {
public:
    StatusLabel(KTabBar *tabBar, QWidget* parent = 0):
        UnderlinedLabel(tabBar, parent)
    {
        setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    }

    virtual QSize minimumSizeHint() const
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

    m_tabBarLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    m_tabBarLayout->setMargin(0);
    m_tabBarLayout->setSpacing(0);

    d->tabBar = new ContainerTabBar(this);
    d->tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tabBarLayout->addWidget(d->tabBar);
    d->fileStatus = new QLabel( this );
    d->fileStatus->setFixedSize( QSize( 16, 16 ) );
    m_tabBarLayout->addWidget(d->fileStatus);
    d->fileNameCorner = new UnderlinedLabel(d->tabBar, this);
    m_tabBarLayout->addWidget(d->fileNameCorner);
    d->statusCorner = new StatusLabel(d->tabBar, this);
    m_tabBarLayout->addWidget(d->statusCorner);
    l->addLayout(m_tabBarLayout);

    d->stack = new QStackedWidget(this);
    l->addWidget(d->stack);

    connect(d->tabBar, SIGNAL(currentChanged(int)), this, SLOT(widgetActivated(int)));
    connect(d->tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeRequest(int)));
    connect(d->tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int, int)));
    connect(d->tabBar, SIGNAL(wheelDelta(int)), this, SLOT(wheelScroll(int)));
    connect(d->tabBar, SIGNAL(contextMenu(int,QPoint)), this, SLOT(contextMenu(int,QPoint)));
    connect(d->tabBar, SIGNAL(mouseMiddleClick(int)), this, SLOT(closeRequest(int)));

    KConfigGroup group = KGlobal::config()->group("UiSettings");
    setTabBarHidden(group.readEntry("TabBarVisibility", 1) == 0);
    d->tabBar->setTabsClosable(true);
    d->tabBar->setMovable(true);
    d->tabBar->setExpanding(false);
}

void Container::setLeftCornerWidget(QWidget* widget)
{
    if(d->leftCornerWidget == widget) {
        if(d->leftCornerWidget)
            d->leftCornerWidget->setParent(0);
    }else{
        delete d->leftCornerWidget;
    }
    d->leftCornerWidget = widget;
    if(!widget)
        return;
    widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_tabBarLayout->insertWidget(0, widget);
    widget->show();
}

Container::~Container()
{
    delete d;
}

void Container::wheelScroll(int delta)
{
    int nextIndex = -1;
    if( delta > 0 )
    {
        if( d->tabBar->currentIndex() == 0 )
        {
            nextIndex = d->tabBar->count() - 1;
        } else
        {
            nextIndex = d->tabBar->currentIndex() -1;
        }
    } else 
    {
        if( d->tabBar->currentIndex() == d->tabBar->count() - 1 )
        {
            nextIndex = 0;
        } else
        {
            nextIndex = d->tabBar->currentIndex() + 1;
        }
    }
    widgetActivated( nextIndex );
}

void Container::closeRequest(int idx)
{
    emit closeRequest((widget(idx)));
}

void Container::widgetActivated(int idx)
{
    if (idx < 0)
        return;
    if (QWidget* w = d->stack->widget(idx)) {
        if(d->viewForWidget.contains(w))
            emit activateView(d->viewForWidget[w]);
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
    setCurrentWidget(d->stack->currentWidget());
    
    // This fixes a strange layouting bug, that could be reproduced like this: Open a few files in KDevelop, activate the rightmost tab.
    // Then temporarily switch to another area, and then switch back. After that, the tab-bar was gone.
    // The problem could only be fixed by closing/opening another view.
    d->tabBar->setMinimumHeight(d->tabBar->sizeHint().height());
    
    connect(view, SIGNAL(statusChanged(Sublime::View*)), this, SLOT(statusChanged(Sublime::View*)));
    connect(view->document(), SIGNAL(statusIconChanged(Sublime::Document*)), this, SLOT(statusIconChanged(Sublime::Document*)));
    connect(view->document(), SIGNAL(titleChanged(Sublime::Document*)), this, SLOT(documentTitleChanged(Sublime::Document*)));
}

void Container::statusChanged(Sublime::View* view)
{
    d->statusCorner->setText(view->viewStatus());
}


void Container::statusIconChanged(Document* doc)
{
    QMapIterator<QWidget*, View*> it = d->viewForWidget;
    while (it.hasNext()) {
        if (it.next().value()->document() == doc) {
            d->fileStatus->setPixmap( doc->statusIcon().pixmap( QSize( 16,16 ) ) );
            int tabIndex = d->stack->indexOf(it.key());
            if (tabIndex != -1) {
                d->tabBar->setTabIcon(tabIndex, doc->statusIcon());
            }
            break;
        }
    }
}

void Container::documentTitleChanged(Sublime::Document* doc)
{
    QMapIterator<QWidget*, View*> it = d->viewForWidget;
    while (it.hasNext()) {
        if (it.next().value()->document() == doc) {
            QString txt = doc->title();
            //TODO: Maybe add new virtual in Document to support supplying this
            // extended information from subclasses like IDocument which can use
            // the rest of the kdevplatform API
            UrlDocument* udoc = dynamic_cast<UrlDocument*>( doc );
            if( udoc ) {
                QString pretty;
                if( udoc->url().isLocalFile() ) {
                    pretty = udoc->url().toLocalFile();
                } else {
                    pretty = udoc->url().prettyUrl();
                }
                txt = txt + " (" + pretty + ")";
            }
            d->fileNameCorner->setText( txt );
            int tabIndex = d->stack->indexOf(it.key());
            if (tabIndex != -1) {
                d->tabBar->setTabText(tabIndex, doc->title());
            }
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
    if (View *view = d->viewForWidget[w])
    {
        statusChanged(view);
        statusIconChanged( view->document() );
        documentTitleChanged( view->document() );
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

void Sublime::Container::removeWidget(QWidget *w)
{
    if (w) {
        int widgetIdx = d->stack->indexOf(w);
        d->stack->removeWidget(w);
        d->tabBar->removeTab(widgetIdx);
        if (d->tabBar->currentIndex() != -1) {
            QWidget* w = widget( d->tabBar->currentIndex() );
            if( w ) {
                statusIconChanged( d->viewForWidget[w]->document() );
                documentTitleChanged( d->viewForWidget[w]->document() );
            }
        }
        View* view = d->viewForWidget.take(w);
        if (view)
        {
            disconnect(view->document(), SIGNAL(titleChanged(Sublime::Document*)), this, SLOT(documentTitleChanged(Sublime::Document*)));
            disconnect(view->document(), SIGNAL(statusIconChanged(Sublime::Document*)), this, SLOT(statusIconChanged(Sublime::Document*)));
            disconnect(view, SIGNAL(statusChanged(Sublime::View*)), this, SLOT(statusChanged(Sublime::View*)));
        }
    }
}

bool Container::hasWidget(QWidget *w)
{
    return d->stack->indexOf(w) != -1;
}

View *Container::viewForWidget(QWidget *w) const
{
    return d->viewForWidget[w];
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
}

void Container::tabMoved(int from, int to)
{
    QWidget *w = d->stack->widget(from);
    d->stack->removeWidget(w);
    d->stack->insertWidget(to, w);
    d->viewForWidget[w]->notifyPositionChanged(to);
}

void Container::contextMenu( int currentTab, const QPoint& pos )
{
    KMenu menu;

    emit tabContextMenuRequested(viewForWidget(widget(currentTab)), &menu);

    menu.addSeparator();
    QAction* closeTabAction = menu.addAction( KIcon("document-close"), i18n( "Close File" ) );
    QAction* closeOtherTabsAction = menu.addAction( KIcon("document-close"), i18n( "Close Other Files" ) );
    QAction* closeAllTabsAction = menu.addAction( KIcon("document-close"), i18n( "Close All Files" ) );

    QAction* triggered = menu.exec(pos);

    if (triggered) {
        if ( triggered == closeTabAction ) {
            closeRequest(currentTab);
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
                closeRequest(tab);
            }
        } else if ( triggered == closeAllTabsAction ) {
            // activate last tab
            widgetActivated(count() - 1);
            // close all
            QList<QWidget*> tabs;
            for ( int i = 0; i < count(); ++i ) {
                closeRequest(widget(i));
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



}

#include "container.moc"

