/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
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
#include "area.h"

#include <QMap>
#include <QList>
#include <QStringList>
#include <QAction>
#include <QPointer>

#include "view.h"
#include "document.h"
#include "areaindex.h"
#include "controller.h"
#include "sublimedebug.h"

namespace Sublime {

// struct AreaPrivate

struct AreaPrivate {
    AreaPrivate()
    {
        rootIndex = new RootAreaIndex();
        currentIndex = rootIndex;
        controller = 0;
    }
    AreaPrivate(const AreaPrivate &p)
    {
        rootIndex = new RootAreaIndex(*(p.rootIndex));
        currentIndex = rootIndex;
        controller = p.controller;
        toolViewPositions.clear();
        desiredToolViews = p.desiredToolViews;
        shownToolViews = p.shownToolViews;
        workingSet = p.workingSet;

        title = p.title;
        iconName = p.iconName;
    }
    ~AreaPrivate()
    {
        delete rootIndex;
    }

    struct ViewFinder {
        ViewFinder(View *_view): view(_view), index(0) {}
        Area::WalkerMode operator() (AreaIndex *idx) {
            if (idx->hasView(view))
            {
                index = idx;
                return Area::StopWalker;
            }
            return Area::ContinueWalker;
        }
        View *view;
        AreaIndex *index;
    };

    struct ViewLister {
        Area::WalkerMode operator()(AreaIndex *idx) {
            views += idx->views();
            return Area::ContinueWalker;
        }
        QList<View*> views;
    };

    QString title;

    RootAreaIndex *rootIndex;
    AreaIndex *currentIndex;
    Controller *controller;

    QList<View*> toolViews;
    QMap<View *, Sublime::Position> toolViewPositions;
    QMap<QString, Sublime::Position> desiredToolViews;
    QMap<Sublime::Position, QStringList> shownToolViews;
    QMap<Sublime::Position, int> thickness;
    QString iconName;
    QString workingSet;
    QPointer<View> activeView;
    QList<QAction*> m_actions;
};

// class Area

Area::Area(Controller *controller, const QString &name, const QString &title)
    :QObject(controller), d( new AreaPrivate() )
{
    // FIXME: using objectName seems fishy. Introduce areaType method,
    // or some such.
    setObjectName(name);
    d->title = title;
    d->controller = controller;
    d->iconName = "kdevelop";
    d->workingSet.clear();
    qCDebug(SUBLIME) << "initial working-set:" << d->workingSet;
    initialize();
}

Area::Area(const Area &area)
    : QObject(area.controller()), d( new AreaPrivate( *(area.d) ) )
{
    setObjectName(area.objectName());

    //clone toolviews
    d->toolViews.clear();
    foreach (View *view, area.toolViews())
        addToolView(view->document()->createView(), area.toolViewPosition(view));
    initialize();
}

void Area::initialize()
{
    connect(this, &Area::viewAdded,
            d->controller, &Controller::notifyViewAdded);
    connect(this, &Area::aboutToRemoveView,
            d->controller, &Controller::notifyViewRemoved);
    connect(this, &Area::toolViewAdded,
            d->controller, &Controller::notifyToolViewAdded);
    connect(this, &Area::aboutToRemoveToolView,
            d->controller, &Controller::notifyToolViewRemoved);
    connect(this, &Area::toolViewMoved,
            d->controller, &Controller::toolViewMoved);

    /* In theory, ownership is passed to us, so should not bother detecting
    deletion outside.  */
    connect(this, &Area::destroyed, d->controller,
            [this] (QObject* obj) { d->controller->removeArea(static_cast<Area*>(obj)); });
}

Area::~Area()
{
    delete d;
}

View* Area::activeView()
{
    return d->activeView.data();
}

void Area::setActiveView(View* view)
{
    d->activeView = view;
}

void Area::addView(View *view, AreaIndex *index, View *after)
{
    //View *after = 0;
    if (!after  &&  controller()->openAfterCurrent()) {
        after = activeView();
    }
    index->add(view, after);
    connect(view, &View::positionChanged, this, &Area::positionChanged);
    qCDebug(SUBLIME) << "view added in" << this;
    connect(this, &Area::destroyed, view, &View::deleteLater);
    emit viewAdded(index, view);
}

void Area::addView(View *view, View *after)
{
    AreaIndex *index = d->currentIndex;
    if (after)
    {
        AreaIndex *i = indexOf(after);
        if (i)
            index = i;
    }
    addView(view, index);
}

void Area::addView(View *view, View *viewToSplit, Qt::Orientation orientation)
{
    AreaIndex *indexToSplit = indexOf(viewToSplit);
    addView(view, indexToSplit, orientation);
}

void Area::addView(View* view, AreaIndex* indexToSplit, Qt::Orientation orientation)
{
    indexToSplit->split(view, orientation);
    emit viewAdded(indexToSplit, view);
    connect(this, &Area::destroyed, view, &View::deleteLater);
}

View* Area::removeView(View *view)
{
    AreaIndex *index = indexOf(view);
    Q_ASSERT(index);

    emit aboutToRemoveView(index, view);
    index->remove(view);
    emit viewRemoved(index, view);

    return view;
}

AreaIndex *Area::indexOf(View *view)
{
    AreaPrivate::ViewFinder f(view);
    walkViews(f, d->rootIndex);
    return f.index;
}

RootAreaIndex *Area::rootIndex() const
{
    return d->rootIndex;
}

void Area::addToolView(View *view, Position defaultPosition)
{
    d->toolViews.append(view);
    QString id = view->document()->documentSpecifier();
    Position position = defaultPosition;
    if (d->desiredToolViews.contains(id))
        position = d->desiredToolViews[id];
    d->desiredToolViews[id] = position;
    d->toolViewPositions[view] = position;
    emit toolViewAdded(view, position);
}

void Sublime::Area::raiseToolView(View * toolView)
{
    emit requestToolViewRaise(toolView);
}

View* Area::removeToolView(View *view)
{
    if (!d->toolViews.contains(view))
        return 0;

    emit aboutToRemoveToolView(view, d->toolViewPositions[view]);
    QString id = view->document()->documentSpecifier();
    qCDebug(SUBLIME) << this << "removed tool view " << id;
    d->desiredToolViews.remove(id);
    d->toolViews.removeAll(view);
    d->toolViewPositions.remove(view);
    return view;
}

void Area::moveToolView(View *toolView, Position newPosition)
{
    if (!d->toolViews.contains(toolView))
        return;

    QString id = toolView->document()->documentSpecifier();
    d->desiredToolViews[id] = newPosition;
    d->toolViewPositions[toolView] = newPosition;
    emit toolViewMoved(toolView, newPosition);
}

QList<View*> &Area::toolViews() const
{
    return d->toolViews;
}

Position Area::toolViewPosition(View *toolView) const
{
    return d->toolViewPositions[toolView];
}

Controller *Area::controller() const
{
    return d->controller;
}

QList<View*> Sublime::Area::views()
{
    AreaPrivate::ViewLister lister;
    walkViews(lister, d->rootIndex);
    return lister.views;
}

QString Area::title() const
{
    return d->title;
}

void Area::setTitle(const QString &title)
{
    d->title = title;
}

void Area::save(KConfigGroup& group) const
{
    QStringList desired;
    QMap<QString, Sublime::Position>::iterator i, e;
    for (i = d->desiredToolViews.begin(), e = d->desiredToolViews.end(); i != e; ++i)
    {
        desired << i.key() + ':' + QString::number(static_cast<int>(i.value()));
    }
    group.writeEntry("desired views", desired);
    qCDebug(SUBLIME) << "save " << this << "wrote" << group.readEntry("desired views", "");
    group.writeEntry("view on left", shownToolViews(Sublime::Left));
    group.writeEntry("view on right", shownToolViews(Sublime::Right));
    group.writeEntry("view on top", shownToolViews(Sublime::Top));
    group.writeEntry("view on bottom", shownToolViews(Sublime::Bottom));
    group.writeEntry("thickness left", thickness(Sublime::Left));
    group.writeEntry("thickness right", thickness(Sublime::Right));
    group.writeEntry("thickness bottom", thickness(Sublime::Bottom));
    group.writeEntry("thickness top", thickness(Sublime::Top));
    group.writeEntry("working set", d->workingSet);
}

void Area::load(const KConfigGroup& group)
{
    qCDebug(SUBLIME) << "loading areas config";
    d->desiredToolViews.clear();
    QStringList desired = group.readEntry("desired views", QStringList());
    foreach (const QString &s, desired)
    {
        int i = s.indexOf(':');
        if (i != -1)
        {
            QString id = s.left(i);
            int pos_i = s.mid(i+1).toInt();
            Sublime::Position pos = static_cast<Sublime::Position>(pos_i);
            if (pos != Sublime::Left && pos != Sublime::Right
                && pos != Sublime::Top && pos != Sublime::Bottom)
                pos = Sublime::Bottom;
            d->desiredToolViews[id] = pos;
        }
    }
    setShownToolViews(Sublime::Left, group.readEntry("view on left", QStringList()));
    setShownToolViews(Sublime::Right,
                     group.readEntry("view on right", QStringList()));
    setShownToolViews(Sublime::Top, group.readEntry("view on top", QStringList()));
    setShownToolViews(Sublime::Bottom,
                     group.readEntry("view on bottom", QStringList()));
    setThickness(Sublime::Left, group.readEntry("thickness left", -1));
    setThickness(Sublime::Right, group.readEntry("thickness right", -1));
    setThickness(Sublime::Bottom, group.readEntry("thickness bottom", -1));
    setThickness(Sublime::Top, group.readEntry("thickness top", -1));
    setWorkingSet(group.readEntry("working set", d->workingSet));
}

bool Area::wantToolView(const QString& id)
{
    return (d->desiredToolViews.contains(id));
}

void Area::setShownToolViews(Sublime::Position pos, const QStringList& ids)
{
    d->shownToolViews[pos] = ids;
}

QStringList Area::shownToolViews(Sublime::Position pos) const
{
    return d->shownToolViews[pos];
}

void Area::setDesiredToolViews(
    const QMap<QString, Sublime::Position>& desiredToolViews)
{
    d->desiredToolViews = desiredToolViews;
}

void Area::setThickness(Sublime::Position pos, int thickness)
{
    d->thickness[pos] = thickness;
}

int Area::thickness(Sublime::Position pos) const
{
    if (!d->thickness.count(pos))
        return -1;
    return (d->thickness)[pos];
}

QString Area::iconName() const
{
    return d->iconName;
}

void Area::setIconName(const QString& iconName)
{
    d->iconName = iconName;
}

void Area::positionChanged(View *view, int newPos)
{
    qCDebug(SUBLIME) << view << newPos;
    AreaIndex *index = indexOf(view);
    index->views().move(index->views().indexOf(view), newPos);
}


QString Area::workingSet() const
{
    return d->workingSet;
}


void Area::setWorkingSet(QString name)
{
    if(name != d->workingSet) {
        qCDebug(SUBLIME) << this << "setting new working-set" << name;
        QString oldName = d->workingSet;
        emit changingWorkingSet(this, oldName, name);
        d->workingSet = name;
        emit changedWorkingSet(this, oldName, name);
    }
}

bool Area::closeView(View* view, bool silent)
{
    QPointer<Document> doc = view->document();

    // We don't just delete the view, because if silent is false, we might need to ask the user.
    if(doc && !silent)
    {
        // Do some counting to check whether we need to ask the user for feedback
        qCDebug(SUBLIME) << "Closing view for" << view->document()->documentSpecifier() << "views" << view->document()->views().size() << "in area" << this;
        int viewsInCurrentArea = 0; // Number of views for the same document in the current area
        int viewsInOtherAreas = 0; // Number of views for the same document in other areas
        int viewsInOtherWorkingSets = 0; // Number of views for the same document in areas with different working-set

        foreach(View* otherView, doc.data()->views())
        {
            Area* area = controller()->areaForView(otherView);
            if(area == this)
                viewsInCurrentArea += 1;
            if(!area || (area != this))
                viewsInOtherAreas += 1;
            if(area && area != this && area->workingSet() != workingSet())
                viewsInOtherWorkingSets += 1;
        }

        if(viewsInCurrentArea == 1 && (viewsInOtherAreas == 0 || viewsInOtherWorkingSets == 0))
        {
            // Time to ask the user for feedback, because the document will be completely closed
            // due to working-set synchronization
            if( !doc.data()->askForCloseFeedback() )
                return false;
        }
    }

    // otherwise we can silently close the view,
    // the document will still have an opened view somewhere
    delete removeView(view);

    return true;
}

void Area::clearViews(bool silent)
{
    foreach(Sublime::View* view, views())
        closeView(view, silent);
}

QList<QAction*> Area::actions() const
{
    return d->m_actions;
}

void Area::addAction(QAction* action)
{
    connect(action, &QAction::destroyed, this, &Area::actionDestroyed);
    d->m_actions.append(action);
}

void Area::actionDestroyed(QObject* action)
{
    d->m_actions.removeAll(qobject_cast<QAction*>(action));
}

}

