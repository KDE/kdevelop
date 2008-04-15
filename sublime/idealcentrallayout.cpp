/*
  Copyright 2008 Hamish Rodda <rodda@kde.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "idealcentrallayout.h"

#include <QStyle>
#include <QWidgetItem>

#include <kdebug.h>

#include "idealcentralwidget.h"
#include "ideal.h"
#include "view.h"
#include "switcher.h"
#include "mainwindow.h"

using namespace Sublime;

AreaLayout::AreaLayout(AreaLayout* p)
{
    source = 0;
    parent = p;
    firstChild = 0;
    secondChild = 0;
    splitter = 0;
    orientation = Qt::Horizontal;
    splitterPercentage = 0.5;
    switcher = 0;
    switcherItem = 0;
    activeView = 0;
}

AreaLayout::~ AreaLayout()
{
    clearChildren();

    delete switcher;
    delete switcherItem;

    qDeleteAll(viewItems);
}

void Sublime::AreaLayout::clearChildren()
{
    delete firstChild;
    firstChild = 0;
    delete secondChild;
    secondChild = 0;
    if (splitter) {
        QWidget* widget = splitter->widget();
        delete splitter;
        splitter = 0;
        delete widget;
    }
}

void Sublime::AreaLayout::clearViews()
{
    views.clear();
    qDeleteAll(viewItems);
    viewItems.clear();
    activeView = 0;

    if (switcher) {
        delete switcherItem;
        switcherItem = 0;
        delete switcher;
        switcher = 0;
    }
}


IdealCentralLayout::IdealCentralLayout(MainWindow* mainWindow, IdealCentralWidget* parent)
    : QLayout(parent)
    , m_layoutDirty(true)
    , m_sizeHintDirty(true)
    , m_minDirty(true)
    , m_splitterWidth(parent->style()->pixelMetric(QStyle::PM_SplitterWidth, 0, parent->parentWidget()))
    , m_currentArea(0)
    , m_topArea(0)
{
    setMargin(0);
    connect(mainWindow, SIGNAL(activeViewChanged(Sublime::View*)), this, SLOT(activateView(Sublime::View*)));
}

void IdealCentralLayout::clear()
{
    delete m_topArea;
    m_topArea = 0;

    invalidate();
}

void IdealCentralLayout::setArea(Area * area)
{
    if (m_topArea)
        clear();

    m_topArea = new AreaLayout();
    if (area->rootIndex())
        setupArea(area->rootIndex(), m_topArea);

    m_currentArea = area;

    invalidate();
}

Q_DECLARE_METATYPE(AreaLayout*)

void IdealCentralLayout::setupArea(AreaIndex * area, AreaLayout * layout)
{
    if (layout->source != area) {
        layout->source = area;
        connect(area, SIGNAL(aboutToRemoveAreaIndex(Sublime::AreaIndex*)), this, SLOT(aboutToRemoveAreaIndex(Sublime::AreaIndex*)));
        connect(area, SIGNAL(childAreaIndexesRemoved(Sublime::AreaIndex*)), this, SLOT(refreshAreaIndex(Sublime::AreaIndex*)));
    }

    if (area->isSplitted()) {
        layout->clearViews();

        Q_ASSERT(area->first());
        if (!layout->firstChild) {
            layout->firstChild = new AreaLayout(layout);
        }
        setupArea(area->first(), layout->firstChild);

        Q_ASSERT(area->second());
        if (!layout->secondChild) {
            layout->secondChild = new AreaLayout(layout);
        }
        setupArea(area->second(), layout->secondChild);

        if (!layout->splitter) {
            IdealSplitterHandle* handle = new IdealSplitterHandle(area->orientation() == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal, parentWidget(), qVariantFromValue<AreaLayout*>(layout));
            connect(handle, SIGNAL(resize(int, int, QVariant)), this, SLOT(resize(int, int, QVariant)));
            layout->splitter = new QWidgetItem(handle);
            addChildWidget(handle);
        }

        layout->orientation = area->orientation();

    } else {
        layout->clearChildren();

        foreach (View* view, area->views()) {
            bool found = false;
            foreach (View* v2, layout->views) {
                if (v2 == view) {
                    found = true;
                    break;
                }
            }
            if (found)
                continue;

            layout->views << view;
        }

        if (!layout->views.isEmpty()) {
            if (!layout->activeView)
                layout->activeView = layout->views.first();

            if (!layout->viewItems.contains(layout->activeView)) {
                createViewItem(layout, layout->activeView);
            }

        } else {
            layout->activeView = 0;
        }

        // TODO if we want a no-tab-on-single-document feature
        /*if (layout->views.isEmpty()) {
            if (layout->switcher) {
                delete layout->switcher;
                delete layout->switcherItem;
            }
        } else */{
            if (!layout->switcher) {
                layout->switcher = new Switcher(area, parentWidget());
                addChildWidget(layout->switcher);
                layout->switcherItem = new QWidgetItem(layout->switcher);
                connect(layout->switcher, SIGNAL(currentChanged(int)), this, SLOT(widgetActivated(int)));
            }
        }
    }
}

void IdealCentralLayout::addItem(QLayoutItem * item)
{
    Q_ASSERT(false);
    delete item;
}

class WidgetWalker
{
public:
    enum {
        QWidgetTarget,
        IndexTarget
    } SearchTarget;

    QWidget* targetWidget;
    int targetIndex, resultIndex;
    AreaLayout* resultAreaLayout;
    QLayoutItem* resultItem;
    int currentIndex;

    WidgetWalker(QWidget* widget)
        : SearchTarget(QWidgetTarget)
        , targetWidget(widget)
        , targetIndex(-1)
        , resultIndex(-1)
        , resultAreaLayout(0)
        , resultItem(0)
        , currentIndex(-1)
    {
    }

    WidgetWalker(int index)
        : SearchTarget(IndexTarget)
        , targetWidget(0)
        , targetIndex(index)
        , resultIndex(-1)
        , resultAreaLayout(0)
        , resultItem(0)
        , currentIndex(-1)
    {
    }

    bool found() const
    {
        return resultIndex != -1;
    }

    Area::WalkerMode operator() (AreaLayout *layout)
    {
        foreach (QWidgetItem* item, layout->viewItems)
            if (checkCurrent(layout, item))
                return Area::StopWalker;

        if (layout->splitter)
            if (checkCurrent(layout, layout->splitter))
                return Area::StopWalker;

        if (layout->switcherItem)
            if (checkCurrent(layout, layout->switcherItem))
                return Area::StopWalker;

        return Area::ContinueWalker;
    }

    bool checkCurrent(AreaLayout* layout, QWidgetItem* item)
    {
        bool found = false;
        switch (SearchTarget) {
            case QWidgetTarget:
                if (item->widget() == targetWidget)
                    found = true;
                break;

            case IndexTarget:
                if (currentIndex == targetIndex) {
                    resultIndex = currentIndex;
                    resultAreaLayout = layout;
                    found = true;
                }
                break;
        }

        if (found) {
            resultIndex = currentIndex;
            resultAreaLayout = layout;
            resultItem = item;
            return Area::StopWalker;
        }

        return found;
    }
};

int IdealCentralLayout::count() const
{
    if (!m_topArea)
        return 0;

    WidgetWalker walk(-2);
    m_topArea->walkLayout(walk);
    return walk.currentIndex;
}

int IdealCentralLayout::indexOf(QWidget * widget) const
{
    if (!m_topArea)
        return -1;

    WidgetWalker walk(widget);
    m_topArea->walkLayout(walk);
    return walk.resultIndex;
}

QLayoutItem * IdealCentralLayout::itemAt(int index) const
{
    if (!m_topArea)
        return 0;

    WidgetWalker walk(index);
    m_topArea->walkLayout(walk);
    return walk.resultItem;
}

QLayoutItem * IdealCentralLayout::takeAt(int index)
{
    Q_UNUSED(index)

    // Uh-oh...??
    Q_ASSERT(false);

    return 0;
}

void IdealCentralLayout::setGeometry(const QRect & r)
{
    if (m_layoutDirty || r != geometry()) {
        if (m_topArea)
            doLayout(m_topArea, r);
        m_layoutDirty = false;
    }
}

void IdealCentralLayout::doLayout(AreaLayout* layout, const QRect & rect)
{
    layout->currentRect = rect;

    if (layout->splitter) {
        if (layout->orientation == Qt::Horizontal) {
            int leftWidth = int(rect.width() * layout->splitterPercentage - m_splitterWidth / 2);

            doLayout(layout->firstChild, QRect(rect.x(), rect.y(), leftWidth, rect.height()));
            layout->splitter->setGeometry(QRect(rect.x() + leftWidth, rect.y(), m_splitterWidth, rect.height()));
            doLayout(layout->secondChild, QRect(rect.x() + leftWidth + m_splitterWidth, rect.y(), rect.width() - (leftWidth + m_splitterWidth), rect.height()));

        } else {
            int topHeight = int(rect.height() * layout->splitterPercentage - m_splitterWidth / 2);

            doLayout(layout->firstChild, QRect(rect.x(), rect.y(), rect.width(), topHeight));
            layout->splitter->setGeometry(QRect(rect.x(), rect.y() + topHeight, rect.width(), m_splitterWidth));
            doLayout(layout->secondChild, QRect(rect.x(), rect.y() + topHeight + m_splitterWidth, rect.width(), rect.height() - (topHeight + m_splitterWidth)));
        }

    } else {
        if (!layout->activeView)
            if (!layout->views.isEmpty())
                layout->activeView = layout->views.first();
            else
                return;

        QRect r = rect;
        Q_ASSERT(layout->switcher);
        if (layout->switcher) {
            layout->switcher->show();
            int height = layout->switcherItem->sizeHint().height();
            layout->switcherItem->setGeometry(QRect(rect.x(), rect.y(), rect.width(), height));
            r.setTop(r.y() + height);
        }

        foreach (View* view, layout->views) {
            if (layout->viewItems.contains(view)) {
                QWidgetItem* item = layout->viewItems[view];
                if (view == layout->activeView) {
                    item->setGeometry(r);
                    item->widget()->show();

                } else {
                    item->widget()->hide();
                }
            }
        }
    }
}

QSize IdealCentralLayout::minimumSize() const
{
    if (!m_minDirty)
        return m_min;

    if (m_topArea)
        m_min = doMin(m_topArea);

    m_minDirty = false;
    return m_min;
}

QSize IdealCentralLayout::doMin(AreaLayout * layout) const
{
    QSize ret;

    if (layout->splitter) {
        QSize min1 = doMin(layout->firstChild);
        QSize min2 = doMin(layout->secondChild);

        if (layout->orientation == Qt::Horizontal) {
            ret = QSize(min1.width() + m_splitterWidth + min2.width(), qMax(min1.height(), min2.height()));

        } else {
            ret = QSize(qMax(min1.width(), min2.width()), min1.height() + m_splitterWidth + min2.height());
        }

    } else {
        foreach (QWidgetItem* item, layout->viewItems) {
            // May not yet be instantiated, don't care about their sizes
            Q_ASSERT(item);
            ret = ret.expandedTo(item->minimumSize());
        }

        if (layout->switcher) {
            QSize hint = layout->switcher->minimumSize();
            ret += QSize(0, hint.height());
            ret.setWidth(qMax(ret.width(), hint.width()));
        }
    }

    if (ret.height() < 120)
        ret.setHeight(120);

    if (ret.width() < 120)
        ret.setWidth(120);

    return ret;
}

QSize IdealCentralLayout::sizeHint() const
{
    if (!m_sizeHintDirty)
        return m_hint;

    if (m_topArea)
        m_hint = doHint(m_topArea);

    m_sizeHintDirty = false;
    return m_hint;
}

QSize IdealCentralLayout::doHint(AreaLayout * layout) const
{
    if (layout->splitter) {
        QSize min1 = doHint(layout->firstChild);
        QSize min2 = doHint(layout->secondChild);

        if (layout->orientation == Qt::Horizontal) {
            return QSize(min1.width() + m_splitterWidth + min2.width(), qMax(min1.height(), min2.height()));

        } else {
            return QSize(qMax(min1.width(), min2.width()), min1.height() + m_splitterWidth + min2.height());
        }

    } else {
        QSize ret;

        foreach (QWidgetItem* item, layout->viewItems) {
            // May not yet be instantiated, don't care about their sizes
            Q_ASSERT(item);
            ret = ret.expandedTo(item->sizeHint());
        }

        if (layout->switcher) {
            QSize hint = layout->switcher->sizeHint();
            ret += QSize(0, hint.height());
            ret.setWidth(qMax(ret.width(), hint.width()));
        }

        return ret;
    }
}

Area * IdealCentralLayout::currentArea() const
{
    return m_currentArea;
}

class AreaIndexFinder
{
public:
    AreaIndex* targetAreaIndex;
    AreaLayout* resultAreaLayout;

    AreaIndexFinder(AreaIndex* target)
        : targetAreaIndex(target)
        , resultAreaLayout(0)
    {
    }

    Area::WalkerMode operator() (AreaLayout *layout)
    {
        if (layout->source == targetAreaIndex) {
            resultAreaLayout = layout;
            return Area::StopWalker;
        }

        return Area::ContinueWalker;
    }
};

void IdealCentralLayout::refreshAreaIndex(AreaIndex * area)
{
    AreaIndexFinder f(area);
    m_topArea->walkLayout(f);

    if (f.resultAreaLayout) {
        setupArea(area, f.resultAreaLayout);
    } else {
        setupArea(area, m_topArea);
    }
}

void Sublime::IdealCentralLayout::widgetActivated(int index)
{
    Q_ASSERT(qobject_cast<Switcher*>(sender()));
    AreaIndex* area = static_cast<Switcher*>(sender())->areaIndex();
    if (index < 0 || index >= area->views().count())
        return;

    AreaIndexFinder f(area);
    m_topArea->walkLayout(f);

    if (f.resultAreaLayout) {
        activateView(f.resultAreaLayout, area->views().at(index));
    } else {
        kWarning() << "Could not find layout for area " << area;
    }
}

class ViewFinder
{
public:
    View* targetView;
    AreaLayout* resultAreaLayout;

    ViewFinder(View* target)
        : targetView(target)
        , resultAreaLayout(0)
    {
    }

    Area::WalkerMode operator() (AreaLayout *layout)
    {
        foreach (View* view, layout->views)
            if (view == targetView) {
                resultAreaLayout = layout;
                return Area::StopWalker;
            }

        return Area::ContinueWalker;
    }
};

void IdealCentralLayout::activateView(View* view)
{
    if (!m_topArea)
        return;

    ViewFinder f(view);
    m_topArea->walkLayout(f);

    if (f.resultAreaLayout)
        activateView(f.resultAreaLayout, view);
}

void IdealCentralLayout::activateView(AreaLayout* layout, View* view)
{
    int viewIndex = layout->views.indexOf(view);
    layout->activeView = view;

    if (layout->switcher && layout->switcher->currentIndex() != viewIndex) {
        layout->switcher->blockSignals(true);
        layout->switcher->setCurrentIndex(viewIndex);
        layout->switcher->blockSignals(false);
    }

    createViewItem(layout, view);
    // the correct reason isn't available to us
    view->widget()->setFocus(Qt::OtherFocusReason);

    // TODO just invalidate the changed area
    invalidate();
}

void IdealCentralLayout::createViewItem(AreaLayout* layout, View* view)
{
    if (!layout->viewItems.contains(view)) {
        QWidgetItem* viewItem = new QWidgetItem(view->widget(parentWidget()));
        addChildWidget(view->widget());
        layout->viewItems[view] = viewItem;
    }
}

void Sublime::IdealCentralLayout::resize(int thickness, int, QVariant data)
{
    AreaLayout* layout = qvariant_cast<AreaLayout*>(data);
    Q_ASSERT(layout);
    Q_ASSERT(layout->splitter);
    Q_ASSERT(layout->firstChild && layout->secondChild);

    int width;
    switch (layout->orientation) {
        case Qt::Horizontal:
            width = layout->currentRect.width();
            break;

        case Qt::Vertical:
            width = layout->currentRect.height();
            break;
    }

    QSize min;
    if (thickness / width < layout->splitterPercentage)
        min = doMin(layout->firstChild);
    else
        min = doMin(layout->secondChild);

    int minThickness;

    switch (layout->orientation) {
        case Qt::Horizontal:
            minThickness = min.width();
            break;

        case Qt::Vertical:
            minThickness = min.height();
            break;
    }

    if (thickness < layout->splitterPercentage * width)
        layout->splitterPercentage = double(qMax(minThickness, thickness)) / double(width);
    else
        layout->splitterPercentage = double(width - qMax(minThickness, width - thickness)) / double(width);

    if (layout->splitterPercentage < 0.15)
        layout->splitterPercentage = 0.15;
    if (layout->splitterPercentage > 0.85)
        layout->splitterPercentage = 0.85;

    //kDebug() << layout->currentRect << thickness << width << min << minThickness << layout->splitterPercentage;

    // TODO just invalidate the specific area layout
    invalidate();
}

class AnyViewFinder
{
public:
    View* resultView;

    AnyViewFinder()
        : resultView(0)
    {
    }

    Area::WalkerMode operator() (AreaLayout *layout)
    {
        if (layout->activeView) {
            resultView = layout->activeView;
            return Area::StopWalker;
        }

        return Area::ContinueWalker;
    }
};

void Sublime::IdealCentralLayout::removeView(AreaIndex * area, View * view)
{
    AreaIndexFinder f(area);
    m_topArea->walkLayout(f);

    AreaLayout* layout = f.resultAreaLayout;
    Q_ASSERT(layout);

    if (layout->viewItems.contains(view)) {
        delete layout->viewItems.take(view);
    }

    int idx = layout->views.indexOf(view);
    Q_ASSERT(idx != -1);
    layout->views.removeAt(idx);
    --idx;

    View* newActiveView = 0;
    if (idx >= layout->views.count()) {
        if (!layout->views.isEmpty())
            newActiveView = layout->views.last();
    } else {
        AnyViewFinder f;
        m_topArea->walkLayout(f);
        newActiveView = f.resultView;
    }

    if (newActiveView)
        activateView(newActiveView);

    invalidate();
}

void Sublime::IdealCentralLayout::aboutToRemoveAreaIndex(Sublime::AreaIndex * index)
{
    AreaIndexFinder f(index);
    m_topArea->walkLayout(f);

    AreaLayout* layout = f.resultAreaLayout;
    if (!layout)
        return;

    AreaLayout* parent = f.resultAreaLayout->parent;
    Q_ASSERT(parent);

    parent->clearChildren();

    invalidate();
}

AreaLayout * Sublime::IdealCentralLayout::topLayout() const
{
    return m_topArea;
}

#include "idealcentrallayout.moc"
