/*
    SPDX-FileCopyrightText: 2006-2009 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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

#include <algorithm>
#include <utility>

namespace {
/**
 * @return the last two segments of @p document's path or an empty string in case of failure.
 */
QString documentDirAndFilename(const Sublime::Document* document)
{
    auto* const urlDocument = qobject_cast<const Sublime::UrlDocument*>(document);
    if (!urlDocument) {
        return QString{};
    }
    const auto path = urlDocument->url().path();

    const auto lastSlashIndex = path.lastIndexOf(QLatin1Char{'/'});
    if (lastSlashIndex <= 0) {
        return path; // either no slash in path or a single slash is the first symbol of path
    }

    const auto penultimateSlashIndex = path.lastIndexOf(QLatin1Char{'/'}, lastSlashIndex - 1);
    // If there is only one slash in the path, penultimateSlashIndex equals -1 and we correctly return the whole path.
    return path.mid(penultimateSlashIndex + 1);
}

struct ActionInsertionPoint
{
    QAction* previous;
    QAction* next;
};

/**
 * Find a suitable insertion point in a list of actions.
 *
 * @param actions a list of actions ordered by title of their associated document case-insensitively.
 * @param actionDocumentTitle the title of the document associated with the action to be inserted.
 */
ActionInsertionPoint findActionInsertionPoint(const QList<QAction*>& actions, const QString& actionDocumentTitle)
{
    const auto it = std::lower_bound(
        actions.cbegin(), actions.cend(), actionDocumentTitle, [](const QAction* lhs, const QString& rhs) {
            return lhs->data().value<Sublime::View*>()->document()->title().compare(rhs, Qt::CaseInsensitive) < 0;
        });
    ActionInsertionPoint ret;
    ret.next = it == actions.cend() ? nullptr : *it;
    ret.previous = it == actions.cbegin() ? nullptr : *(it - 1);
    return ret;
}

} // namespace

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
        if (event->button() == Qt::MiddleButton) {
            // just close on midbutton, drag can still be done with left mouse button

            int tab = tabAt(event->pos());
            if (tab != -1) {
                emit tabCloseRequested(tab);
            }
            return;
        }
        QTabBar::mousePressEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        // block tabBarDoubleClicked signals with MMB, see https://bugs.kde.org/show_bug.cgi?id=356016
        if (event->button() == Qt::MiddleButton) {
            return;
        }

        QTabBar::mouseDoubleClickEvent(event);
    }

Q_SIGNALS:
    void newTabRequested();

private:
    Container* const m_container;
};

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
    QSpacerItem* shortCutHelpLeftSpacerItem;
    QSpacerItem* shortCutHelpRightSpacerItem;
    QLabel *shortcutHelpLabel;
    QLabel *fileStatus;
    KSqueezedTextLabel *statusCorner;
    QToolButton* documentListButton;
    QMenu* documentListMenu; ///< a popup menu that contains an activating action for each view in this Container
    QHash<View*, QAction*> documentListActionForView;

    /**
     * Set up @p viewAction's text and icon, then insert the action into @a documentListMenu.
     */
    void insertIntoDocumentListMenu(View* view, QAction* viewAction)
    {
        Q_ASSERT(view);
        Q_ASSERT(viewAction);
        Q_ASSERT(viewAction->data().value<Sublime::View*>() == view);

        // FIXME: push this code somehow into shell to get a neat, short path like the document switcher.

        const auto viewDocumentTitle = view->document()->title();
        const auto insertionPoint = findActionInsertionPoint(documentListMenu->actions(), viewDocumentTitle);

        // Check if the document titles of the actions that neighbor viewAction equal viewDocumentTitle.
        // If so, include the containing directory name in the equivalent actions' texts for disambiguation.
        bool includeDirInViewActionText = false;
        for (auto* neighborAction : {insertionPoint.previous, insertionPoint.next}) {
            if (!neighborAction) {
                continue; // no neighboring action on this side
            }
            const auto* const neighborView = neighborAction->data().value<View*>();
            const auto neighborDocumentTitle = neighborView->document()->title();

            if (neighborDocumentTitle.compare(viewDocumentTitle, Qt::CaseInsensitive) != 0) {
                continue; // the titles are not equal, nothing to do
            }
            includeDirInViewActionText = true;

            if (neighborAction->text() != neighborDocumentTitle) {
                Q_ASSERT(neighborAction->text() == documentDirAndFilename(neighborView->document()));
                continue; // neighborAction's text is already disambiguated
            }
            auto newText = documentDirAndFilename(neighborView->document());
            if (!newText.isEmpty()) {
                neighborAction->setText(std::move(newText));
            }
        }

        auto viewActionText = viewDocumentTitle;
        if (includeDirInViewActionText) {
            auto text = documentDirAndFilename(view->document());
            if (!text.isEmpty()) {
                viewActionText = std::move(text);
            }
        }
        viewAction->setText(std::move(viewActionText));

        viewAction->setIcon(view->document()->icon());

        documentListMenu->insertAction(insertionPoint.next, viewAction);

        setAsDockMenu();
    }

    void insertIntoDocumentListMenu(View* view)
    {
        Q_ASSERT(view);
        Q_ASSERT(!documentListActionForView.contains(view));

        auto* const action = new QAction(documentListMenu);
        action->setData(QVariant::fromValue(view));

        documentListActionForView.insert(view, action);
        insertIntoDocumentListMenu(view, action);
    }

    void renameInDocumentListMenu(View* view)
    {
        Q_ASSERT(view);

        auto* const action = documentListActionForView.value(view);
        Q_ASSERT(action);
        // Here and in Container::removeWidget() we don't consider removing the containing directory name
        // from the text of former neighbor actions of the [re]moved action, because if
        // multiple recently open documents shared a file name, it still deserves disambiguation.
        documentListMenu->removeAction(action);
        insertIntoDocumentListMenu(view, action);
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
            optTabBase.initFrom(m_tabBar);
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
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    d->layout = new QBoxLayout(QBoxLayout::LeftToRight);
    d->layout->setContentsMargins(0, 0, 0, 0);
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
    d->documentListButton->setToolTip(i18nc("@info:tooltip", "Show sorted list of opened documents"));
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
    d->shortcutHelpLabel = new QLabel(i18nc("@info", "(Press Ctrl+Tab to switch)"), this);
    auto font = d->shortcutHelpLabel->font();
    font.setPointSize(font.pointSize() - 2);
    font.setItalic(true);
    d->shortcutHelpLabel->setFont(font);
    d->shortCutHelpLeftSpacerItem = new QSpacerItem(0, 0); // fully set in setTabBarHidden()
    d->layout->addSpacerItem(d->shortCutHelpLeftSpacerItem);
    d->shortcutHelpLabel->setAlignment(Qt::AlignCenter);
    d->layout->addWidget(d->shortcutHelpLabel);
    d->shortCutHelpRightSpacerItem = new QSpacerItem(0, 0); // fully set in setTabBarHidden()
    d->layout->addSpacerItem(d->shortCutHelpRightSpacerItem);
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
    d->tabBar->setTabsClosable(configCloseButtonsOnTabs());
    d->tabBar->setMovable(true);
    d->tabBar->setExpanding(false);
    d->tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
}

Container::~Container() = default;

bool Container::configTabBarVisible()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("UiSettings"));
    return group.readEntry("TabBarVisibility", 1);
}

bool Container::configCloseButtonsOnTabs()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("UiSettings"));
    return group.readEntry("CloseButtonsOnTabs", 1);
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
    // from there, which in turn require view to be present in the document list menu.
    d->insertIntoDocumentListMenu(view);

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
            auto* const action = d->documentListActionForView.value(it.value());
            Q_ASSERT(action);
            action->setIcon(doc->icon());
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
                // NOTE: this has languished and is still not upstream as of 6.3.0
                // see https://phabricator.kde.org/D7010
                d->fileNameCorner->updateGeometry();
            }
            int tabIndex = d->stack->indexOf(it.key());
            if (tabIndex != -1) {
                d->tabBar->setTabText(tabIndex, doc->title());
            }

            // Update document list popup title (and icon in case the extension changes and affects the MIME type)
            d->renameInDocumentListMenu(view);
            break;
        }
    }
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

    Q_ASSERT(w);

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
        d->shortCutHelpLeftSpacerItem->changeSize(style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing), 0,
                                                  QSizePolicy::Fixed, QSizePolicy::Fixed);
        d->shortcutHelpLabel->show();
        d->shortCutHelpRightSpacerItem->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
        d->fileNameCorner->show();
    }
    else
    {
        d->fileNameCorner->hide();
        d->fileStatus->hide();
        d->tabBar->show();
        d->shortCutHelpLeftSpacerItem->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
        d->shortcutHelpLabel->hide();
        d->shortCutHelpRightSpacerItem->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    // have spacer item changes taken into account
    d->layout->invalidate();

    View* v = currentView();
    if (v) {
        documentTitleChanged(v->document());
    }
}

void Container::setCloseButtonsOnTabs(bool show)
{
    Q_D(Container);

    d->tabBar->setTabsClosable(show);
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
    viewForWidget(w)->notifyPositionChanged(to);
}

void Container::contextMenu( const QPoint& pos )
{
    Q_D(Container);

    QWidget* senderWidget = qobject_cast<QWidget*>(sender());
    Q_ASSERT(senderWidget);

    int currentTab = d->tabBar->tabAt(pos);

    QMenu menu;
    // Polish before creating a native window below. The style could want change the surface format
    // of the window which will have no effect when the native window has already been created.
    menu.ensurePolished();
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
                                        i18nc("@action:inmenu", "Copy Filename"));
        menu.addSeparator();
        closeTabAction = menu.addAction(QIcon::fromTheme(QStringLiteral("document-close")),
                                        i18nc("@action:inmenu", "Close"));
        closeOtherTabsAction = menu.addAction(QIcon::fromTheme(QStringLiteral("document-close")),
                                              i18nc("@action:inmenu", "Close All Other"));
    }
    QAction* closeAllTabsAction = menu.addAction(QIcon::fromTheme(QStringLiteral("document-close")), i18nc("@action:inmenu", "Close All"));

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
            for (QWidget* tab : std::as_const(otherTabs)) {
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
#include "moc_container.cpp"
