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
#include <QDir>
#include <QEvent>
#include <QSpacerItem>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>
#include <QStackedWidget>
#include <QStyleFactory>
#include <QStyleOptionTabBarBase>
#include <QStylePainter>
#include <QTabBar>
#include <QToolButton>
#include <QWindow>

#include <KAcceleratorManager>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "view.h"
#include "urldocument.h"

#include <KSqueezedTextLabel>

namespace Sublime {

// class ContainerTabBar

class ContainerTabBar : public QTabBar
{
    Q_OBJECT

public:
    explicit ContainerTabBar(Container* container)
        : QTabBar(container), m_container(container)
    {
        if (QApplication::style()->objectName() == QLatin1String("macintosh")) {
            static QPointer<QStyle> qTabBarStyle = QStyleFactory::create(QStringLiteral("fusion"));
            if (qTabBarStyle) {
                setStyle(qTabBarStyle);
            }
        }
        // configure the QTabBar style so it behaves as appropriately as possible,
        // even if we end up using the native Macintosh style because the user's
        // Qt doesn't have the Fusion style installed.
        setDocumentMode(true);
        setUsesScrollButtons(true);
        setElideMode(Qt::ElideNone);

        installEventFilter(this);
    }

    bool event(QEvent* ev) override {
        if(ev->type() == QEvent::ToolTip)
        {
            ev->accept();

            auto* helpEvent = static_cast<QHelpEvent*>(ev);
            int tab = tabAt(helpEvent->pos());

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

            int tab = tabAt(event->pos());
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
    Container* const m_container;
};

bool sortViews(const View* const lhs, const View* const rhs)
{
        return lhs->document()->title().compare(rhs->document()->title(), Qt::CaseInsensitive) < 0;
}

#ifdef Q_OS_MACOS
// only one of these per process:
static QMenu* currentDockMenu = nullptr;
#endif

class ContainerPrivate
{
public:
    QBoxLayout* layout;
    QHash<QWidget*, View*> viewForWidget;

    ContainerTabBar *tabBar;
    QStackedWidget *stack;
    KSqueezedTextLabel *fileNameCorner;
    QLabel *shortcutHelpLabel;
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

        for (View* view : qAsConst(viewForWidget)) {
            views << view;
        }

        std::sort(views.begin(), views.end(), sortViews);

        for (int i = 0; i < views.size(); ++i) {
            View *view = views.at(i);
            QString visibleEntryTitle;
            // if filename is not unique, prepend containing directory
            if ((i < views.size() - 1 && view->document()->title() == views.at(i + 1)->document()->title())
                || (i > 0 && view->document()->title() == views.at(i - 1)->document()->title())
            ) {
                auto urlDoc = qobject_cast<Sublime::UrlDocument*>(view->document());
                if (!urlDoc) {
                    visibleEntryTitle = view->document()->title();
                }
                else {
                    auto url = urlDoc->url().toString();
                    int secondOffset = url.lastIndexOf(QLatin1Char('/'));
                    secondOffset = url.lastIndexOf(QLatin1Char('/'), secondOffset - 1);
                    visibleEntryTitle = url.right(url.length() - url.lastIndexOf(QLatin1Char('/'), secondOffset) - 1);
                }
            } else {
                visibleEntryTitle = view->document()->title();
            }
            QAction* action = documentListMenu->addAction(visibleEntryTitle);
            action->setData(QVariant::fromValue(view));
            documentListActionForView[view] = action;
            action->setIcon(view->document()->icon());
            ///FIXME: push this code somehow into shell, such that we can access the project model for
            ///       icons and also get a neat, short path like the document switcher.
        }

        setAsDockMenu();
    }

    void setAsDockMenu()
    {
#ifdef Q_OS_MACOS
        if (documentListMenu != currentDockMenu) {
            documentListMenu->setAsDockMenu();
            currentDockMenu = documentListMenu;
        }
#endif
    }

    ~ContainerPrivate()
    {
#ifdef Q_OS_MACOS
        if (documentListMenu == currentDockMenu) {
            QMenu().setAsDockMenu();
            currentDockMenu = nullptr;
        }
#endif
    }
};

class UnderlinedLabel: public KSqueezedTextLabel {
Q_OBJECT
public:
    explicit UnderlinedLabel(QTabBar *tabBar, QWidget* parent = nullptr)
        :KSqueezedTextLabel(parent), m_tabBar(tabBar)
    {
    }

protected:
    void paintEvent(QPaintEvent *ev) override
    {
#ifndef Q_OS_OSX
        // getting the underlining right on OS X is tricky; omitting
        // the underlining attracts the eye less than not getting it
        // exactly right.
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
#endif

        KSqueezedTextLabel::paintEvent(ev);
    }

    QTabBar *m_tabBar;
};


class StatusLabel: public UnderlinedLabel {
Q_OBJECT
public:
    explicit StatusLabel(QTabBar *tabBar, QWidget* parent = nullptr):
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
    : QWidget(parent)
    , d_ptr(new ContainerPrivate())
{
    Q_D(Container);

    KAcceleratorManager::setNoAccel(this);

    auto *l = new QBoxLayout(QBoxLayout::TopToBottom, this);
    l->setMargin(0);
    l->setSpacing(0);

    d->layout = new QBoxLayout(QBoxLayout::LeftToRight);
    d->layout->setMargin(0);
    d->layout->setSpacing(0);

    d->documentListMenu = new QMenu(this);
    d->documentListButton = new QToolButton(this);
    d->documentListButton->setIcon(QIcon::fromTheme(QStringLiteral("format-list-unordered")));
    d->documentListButton->setMenu(d->documentListMenu);
#ifdef Q_OS_MACOS
    // for maintaining the Dock menu:
    setFocusPolicy(Qt::StrongFocus);
#endif
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
    d->fileNameCorner->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->layout->addWidget(d->fileNameCorner);
    d->shortcutHelpLabel = new QLabel(i18n("(Press Ctrl+Tab to switch)"), this);
    auto font = d->shortcutHelpLabel->font();
    font.setPointSize(font.pointSize() - 2);
    font.setItalic(true);
    d->shortcutHelpLabel->setFont(font);
    d->layout->addSpacerItem(new QSpacerItem(style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing), 0,
                                             QSizePolicy::Fixed, QSizePolicy::Fixed));
    d->shortcutHelpLabel->setAlignment(Qt::AlignCenter);
    d->layout->addWidget(d->shortcutHelpLabel);
    d->layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
    d->statusCorner = new StatusLabel(d->tabBar, this);
    d->layout->addWidget(d->statusCorner);
    l->addLayout(d->layout);

    d->stack = new QStackedWidget(this);
    l->addWidget(d->stack);

    connect(d->tabBar, &ContainerTabBar::currentChanged, this, &Container::widgetActivated);
    connect(d->tabBar, &ContainerTabBar::tabCloseRequested, this, QOverload<int>::of(&Container::requestClose));
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

Container::~Container() = default;

bool Container::configTabBarVisible()
{
    KConfigGroup group = KSharedConfig::openConfig()->group("UiSettings");
    return group.readEntry("TabBarVisibility", 1);
}

void Container::setLeftCornerWidget(QWidget* widget)
{
    Q_D(Container);

    if(d->leftCornerWidget.data() == widget) {
        if(d->leftCornerWidget)
            d->leftCornerWidget.data()->setParent(nullptr);
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

QList<View*> Container::views() const
{
    Q_D(const Container);

    return d->viewForWidget.values();
}

void Container::requestClose(int idx)
{
    emit requestClose(widget(idx));
}

void Container::widgetActivated(int idx)
{
    Q_D(Container);

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
    Q_D(Container);

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
    Q_D(Container);

    const auto statusText = view->viewStatus();
    d->statusCorner->setText(statusText);
    d->statusCorner->setVisible(!statusText.isEmpty());
}

void Container::statusIconChanged(Document* doc)
{
    Q_D(Container);

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
    Q_D(Container);

    QHashIterator<QWidget*, View*> it = d->viewForWidget;
    while (it.hasNext()) {
        Sublime::View* view = it.next().value();
        if (view->document() == doc) {
            if (currentView() == view) {
                d->fileNameCorner->setText( doc->title(Document::Extended) );
                // TODO KF6: remove this as soon as it is included upstream and we reqire
                // that version
                // see https://phabricator.kde.org/D7010
                d->fileNameCorner->updateGeometry();
            }
            int tabIndex = d->stack->indexOf(it.key());
            if (tabIndex != -1) {
                d->tabBar->setTabText(tabIndex, doc->title());
            }
            break;
        }
    }
    // Update document list popup title
    d->updateDocumentListPopupMenu();
}

int Container::count() const
{
    Q_D(const Container);

    return d->stack->count();
}

QWidget* Container::currentWidget() const
{
    Q_D(const Container);

    return d->stack->currentWidget();
}

void Container::setCurrentWidget(QWidget* w)
{
    Q_D(Container);

    if (d->stack->currentWidget() == w) {
        return;
    }
    d->stack->setCurrentWidget(w);
    d->tabBar->setCurrentIndex(d->stack->indexOf(w));
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
    Q_D(const Container);

    return d->stack->widget(i);
}

int Container::indexOf(QWidget* w) const
{
    Q_D(const Container);

    return d->stack->indexOf(w);
}

void Container::removeWidget(QWidget *w)
{
    Q_D(Container);

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

bool Container::hasWidget(QWidget* w) const
{
    Q_D(const Container);

    return d->stack->indexOf(w) != -1;
}

View *Container::viewForWidget(QWidget *w) const
{
    Q_D(const Container);

    return d->viewForWidget.value(w);
}

void Container::setTabBarHidden(bool hide)
{
    Q_D(Container);

    if (hide)
    {
        d->tabBar->hide();
        d->fileStatus->show();
        d->shortcutHelpLabel->show();
        d->fileNameCorner->show();
    }
    else
    {
        d->fileNameCorner->hide();
        d->fileStatus->hide();
        d->tabBar->show();
        d->shortcutHelpLabel->hide();
    }
    View* v = currentView();
    if (v) {
        documentTitleChanged(v->document());
    }
}

void Container::resetTabColors(const QColor& color)
{
    Q_D(Container);

    for (int i = 0; i < count(); i++){
        d->tabBar->setTabTextColor(i, color);
    }
}

void Container::setTabColor(const View* view, const QColor& color)
{
    Q_D(Container);

    for (int i = 0; i < count(); i++){
        if (view == viewForWidget(widget(i))) {
            d->tabBar->setTabTextColor(i, color);
        }
    }
}

void Container::setTabColors(const QHash<const View*, QColor>& colors)
{
    Q_D(Container);

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
    Q_D(Container);

    QWidget *w = d->stack->widget(from);
    d->stack->removeWidget(w);
    d->stack->insertWidget(to, w);
    d->viewForWidget[w]->notifyPositionChanged(to);
}

void Container::contextMenu( const QPoint& pos )
{
    Q_D(Container);

    QWidget* senderWidget = qobject_cast<QWidget*>(sender());
    Q_ASSERT(senderWidget);

    int currentTab = d->tabBar->tabAt(pos);

    QMenu menu;
    // At least for positioning on Wayland the window the menu belongs to
    // needs to be set. We cannot set senderWidget as parent because some actions (e.g. split view)
    // result in sync destruction of the senderWidget, which then would also prematurely
    // destruct the menu object
    // Workaround (best known currently, check again API of Qt >5.9):
    menu.winId(); // trigger being a native widget already, to ensure windowHandle created
    auto parentWindowHandle = senderWidget->windowHandle();
    if (!parentWindowHandle) {
        parentWindowHandle = senderWidget->nativeParentWidget()->windowHandle();
    }
    menu.windowHandle()->setTransientParent(parentWindowHandle);

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
                                        i18n("Close"));
        closeOtherTabsAction = menu.addAction(QIcon::fromTheme(QStringLiteral("document-close")),
                                              i18n("Close All Other"));
    }
    QAction* closeAllTabsAction = menu.addAction(QIcon::fromTheme(QStringLiteral("document-close")), i18n("Close All"));

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
            for (QWidget* tab : qAsConst(otherTabs)) {
                emit requestClose(tab);
            }
        } else if ( triggered == closeAllTabsAction ) {
            // activate last tab
            widgetActivated(count() - 1);

            // close all
            for ( int i = 0; i < count(); ++i ) {
                emit requestClose(widget(i));
            }
        } else if( triggered == copyPathAction ) {
            auto view = viewForWidget( widget( currentTab ) );
            auto urlDocument = qobject_cast<UrlDocument*>( view->document() );
            if( urlDocument ) {
                QString toCopy = urlDocument->url().toDisplayString(QUrl::PreferLocalFile);
                if (urlDocument->url().isLocalFile()) {
                    toCopy = QDir::toNativeSeparators(toCopy);
                }
                QApplication::clipboard()->setText(toCopy);
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
    Q_D(const Container);

    return d->tabBar->currentIndex() == tab;
}

QRect Container::tabRect(int tab) const
{
    Q_D(const Container);

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
    Q_D(Container);

    auto* view = action->data().value< Sublime::View* >();
    Q_ASSERT(view);
    QWidget* widget = d->viewForWidget.key(view);
    Q_ASSERT(widget);
    setCurrentWidget(widget);
}
Sublime::View* Container::currentView() const
{
    Q_D(const Container);

    return d->viewForWidget.value(widget( d->tabBar->currentIndex() ));
}

void Container::focusInEvent(QFocusEvent* event)
{
    Q_D(Container);

    d->setAsDockMenu();
    QWidget::focusInEvent(event);
}

}

#include "container.moc"
