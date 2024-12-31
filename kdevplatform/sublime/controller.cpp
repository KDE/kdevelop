/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "controller.h"

#include <QMap>
#include <QList>
#include <QEvent>
#include <QMouseEvent>
#include <QCoreApplication>

#include <KSharedConfig>

#include "area.h"
#include "view.h"
#include "document.h"
#include "mainwindow.h"
#include <debug.h>

namespace Sublime {

struct WidgetFinder {
    explicit WidgetFinder(QWidget *_w) :w(_w), view(nullptr) {}
    Area::WalkerMode operator()(AreaIndex *index)
    {
        for (View* v : std::as_const(index->views())) {
            if (v->hasWidget() && (v->widget() == w))
            {
                view = v;
                return Area::StopWalker;
            }
        }
        return Area::ContinueWalker;
    }

    QWidget* const w;
    View *view;
};

struct ToolWidgetFinder {
    explicit ToolWidgetFinder(QWidget *_w) :w(_w), view(nullptr) {}
    Area::WalkerMode operator()(View *v, Sublime::Position /*position*/)
    {
        if (v->hasWidget() && (v->widget() == w))
        {
            view = v;
            return Area::StopWalker;
        }
        return Area::ContinueWalker;
    }

    QWidget* const w;
    View *view;
};


// class ControllerPrivate

class ControllerPrivate
{
public:
    ControllerPrivate()
    {
    }

    QList<Document*> documents;
    QList<Area*> areas;
    QList<Area*> allAreas;
    QMap<QString, Area*> namedAreas;
    // FIXME: remove this.
    QMap<Area*, MainWindow*> shownAreas;
    QList<MainWindow*> controlledWindows;
    QVector< QList<Area*> > mainWindowAreas;
    bool openAfterCurrent;
    bool arrangeBuddies;
};



// class Controller

Controller::Controller(QObject *parent)
    : QObject(parent)
    , MainWindowOperator()
    , d_ptr(new ControllerPrivate())
{
    init();
}

void Controller::init()
{
    loadSettings();
    QCoreApplication::instance()->installEventFilter(this);
}

Controller::~Controller()
{
    Q_D(Controller);

    qDeleteAll(d->controlledWindows);
}

void Controller::showArea(Area *area, MainWindow *mainWindow)
{
    Q_D(Controller);

    Area *areaToShow = nullptr;
    const auto windowIt = d->shownAreas.find(area);
    //if the area is already shown in another mainwindow then we need to clone it
    if (windowIt != d->shownAreas.end() && (mainWindow != *windowIt))
        areaToShow = new Area(*area);
    else
        areaToShow = area;
    d->shownAreas[areaToShow] = mainWindow;

    showAreaInternal(areaToShow, mainWindow);
}

void Controller::showAreaInternal(Area* area, MainWindow *mainWindow)
{
    MainWindowOperator::setArea(mainWindow, area);
}


void Controller::removeArea(Area *obj)
{
    Q_D(Controller);

    d->areas.removeAll(obj);
}

void Controller::removeDocument(Document *obj)
{
    Q_D(Controller);

    d->documents.removeAll(obj);
}

void Controller::showArea(const QString& areaTypeId, MainWindow *mainWindow)
{
    Q_D(Controller);

    int index = d->controlledWindows.indexOf(mainWindow);
    Q_ASSERT(index != -1);

    Area* area = nullptr;
    for (Area* a : std::as_const(d->mainWindowAreas[index])) {
        qCDebug(SUBLIME) << "Object name: " << a->objectName() << " id "
                     << areaTypeId;
        if (a->objectName() == areaTypeId)
        {
            area = a;
            break;
        }
    }
    Q_ASSERT (area);

    showAreaInternal(area, mainWindow);
}

void Controller::resetCurrentArea(MainWindow *mainWindow)
{
    Q_D(Controller);

    QString id = mainWindow->area()->objectName();

    int areaIndex = 0;
    Area* def = nullptr;
    for (Area* a : std::as_const(d->areas)) {
        if (a->objectName() == id)
        {
            def = a;
            break;
        }
        ++areaIndex;
    }
    Q_ASSERT(def);

    int index = d->controlledWindows.indexOf(mainWindow);
    Q_ASSERT(index != -1);

    Area* prev = d->mainWindowAreas[index][areaIndex];
    d->mainWindowAreas[index][areaIndex] = new Area(*def);
    showAreaInternal(d->mainWindowAreas[index][areaIndex], mainWindow);
    delete prev;
}

const QList<Area*> &Controller::defaultAreas() const
{
    Q_D(const Controller);

    return d->areas;
}


const QList< Area* >& Controller::areas(MainWindow* mainWindow) const
{
    Q_D(const Controller);

    int index = d->controlledWindows.indexOf(mainWindow);
    Q_ASSERT(index != -1);
    return areas(index);
}

const QList<Area*> &Controller::areas(int mainWindow) const
{
    Q_D(const Controller);

    return d->mainWindowAreas[mainWindow];
}

const QList<Area*> &Controller::allAreas() const
{
    Q_D(const Controller);

    return d->allAreas;
}

const QList<Document*> &Controller::documents() const
{
    Q_D(const Controller);

    return d->documents;
}

void Controller::addDefaultArea(Area *area)
{
    Q_D(Controller);

    d->areas.append(area);
    d->allAreas.append(area);
    d->namedAreas[area->objectName()] = area;
    emit areaCreated(area);
}

void Controller::addMainWindow(MainWindow* mainWindow)
{
    Q_D(Controller);

    Q_ASSERT(mainWindow);

    Q_ASSERT (!d->controlledWindows.contains(mainWindow));
    d->controlledWindows << mainWindow;
    d->mainWindowAreas.resize(d->controlledWindows.size());
    int index = d->controlledWindows.size()-1;

    auto& mainWindowAreas = d->mainWindowAreas[index];
    const auto& defaultAreas = this->defaultAreas();
    d->allAreas.reserve(d->allAreas.size() + defaultAreas.size());
    mainWindowAreas.reserve(defaultAreas.size());

    for (const auto* area : defaultAreas) {
        Area *na = new Area(*area);
        d->allAreas.append(na);
        mainWindowAreas.append(na);
        emit areaCreated(na);
    }
    showAreaInternal(d->mainWindowAreas[index][0], mainWindow);
    emit mainWindowAdded( mainWindow );
}

void Controller::addDocument(Document *document)
{
    Q_D(Controller);

    d->documents.append(document);
}

void Controller::areaReleased()
{
    Q_D(Controller);

    auto *w = reinterpret_cast<Sublime::MainWindow*>(sender());
    qCDebug(SUBLIME) << "marking areas as mainwindow-free" << w << d->controlledWindows.contains(w) << d->shownAreas.keys(w);
    const auto areas = d->shownAreas.keys(w);
    for (Area* area : areas) {
        qCDebug(SUBLIME) << "" << area->objectName();
        areaReleased(area);
        disconnect(area, nullptr, w, nullptr);
    }

    d->controlledWindows.removeAll(w);
}

void Controller::areaReleased(Sublime::Area *area)
{
    Q_D(Controller);

    d->shownAreas.remove(area);
    d->namedAreas.remove(area->objectName());
}

Area *Controller::defaultArea(const QString &id) const
{
    Q_D(const Controller);

    return d->namedAreas[id];
}

Area *Controller::area(int mainWindow, const QString& id) const
{
    for (Area* area : areas(mainWindow)) {
        if (area->objectName() == id)
            return area;
    }
    return nullptr;
}

Area* Controller::areaForView(View* view) const
{
    for (Area* area : allAreas()) {
        if(area->views().contains(view))
            return area;
    }

    return nullptr;
}

/*We need this to catch activation of views and tool views
so that we can always tell what view and tool view is active.
"Active" doesn't mean focused. It means that it is focused now
or was focused before and no other view/tool view wasn't focused
after that."*/
//implementation is based upon KParts::PartManager::eventFilter
bool Controller::eventFilter(QObject *obj, QEvent *ev)
{
    Q_D(Controller);

    if (ev->type() != QEvent::MouseButtonPress &&
        ev->type() != QEvent::MouseButtonDblClick &&
        ev->type() != QEvent::FocusIn)
        return false;

    //not a widget? - return
    if (!obj->isWidgetType())
        return false;

    //is dialog or popup? - return
    auto *w = static_cast<QWidget*>(obj);
    if (((w->windowFlags().testFlag(Qt::Dialog)) && w->isModal()) ||
            (w->windowFlags().testFlag(Qt::Popup)) || (w->windowFlags().testFlag(Qt::Tool)))
        return false;

    //not a mouse button that should activate the widget? - return
    if (ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonDblClick)
    {
        auto* mev = static_cast<QMouseEvent*>(ev);
        int activationButtonMask = Qt::LeftButton | Qt::MiddleButton | Qt::RightButton;
        if ((mev->button() & activationButtonMask) == 0)
            return false;
    }

    while (w)
    {
        //not inside sublime mainwindow
        auto *mw = qobject_cast<Sublime::MainWindow*>(w->topLevelWidget());
        if (!mw || !d->controlledWindows.contains(mw))
            return false;

        Area *area = mw->area();

        ///@todo adymo: this is extra slow - optimize
        //find this widget in views
        WidgetFinder widgetFinder(w);
        area->walkViews(widgetFinder, area->rootIndex());
        if (widgetFinder.view && widgetFinder.view != mw->activeView())
        {
            setActiveView(mw, widgetFinder.view);
            ///@todo adymo: shall we filter out the event?
            return false;
        }

        //find this widget in tool views
        ToolWidgetFinder toolFinder(w);
        area->walkToolViews(toolFinder, Sublime::AllPositions);
        if (toolFinder.view && toolFinder.view != mw->activeToolView())
        {
            setActiveToolView(mw, toolFinder.view);
            ///@todo adymo: shall we filter out the event?
            return false;
        }

        w = w->parentWidget();
    }

    return false;
}

const QList< MainWindow * > & Controller::mainWindows() const
{
    Q_D(const Controller);

    return d->controlledWindows;
}


void Controller::notifyToolViewRemoved(Sublime::View *view, Sublime::Position)
{
    emit aboutToRemoveToolView(view);
}

void Controller::notifyToolViewAdded(Sublime::View *view, Sublime::Position)
{
    emit toolViewAdded(view);
}

void Controller::notifyViewRemoved(Sublime::AreaIndex*, Sublime::View *view)
{
    emit aboutToRemoveView(view);
}

void Controller::notifyViewAdded(Sublime::AreaIndex*, Sublime::View *view)
{
    emit viewAdded(view);
}

void Controller::loadSettings()
{
    Q_D(Controller);

    KConfigGroup uiGroup = KSharedConfig::openConfig()->group(QStringLiteral("UiSettings"));
    d->openAfterCurrent = (uiGroup.readEntry("TabBarOpenAfterCurrent", 1) == 1);
    d->arrangeBuddies = (uiGroup.readEntry("TabBarArrangeBuddies", 1) == 1);
}

bool Controller::openAfterCurrent() const
{
    Q_D(const Controller);

    return d->openAfterCurrent;
}

bool Controller::arrangeBuddies() const
{
    Q_D(const Controller);

    return d->arrangeBuddies;
}

}

#include "moc_controller.cpp"
