/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "workingset.h"

#include <sublime/area.h>
#include <sublime/mainwindow.h>

#include <KTextEditor/Document>
#include <KColorUtils>
#include <KDebug>
#include <KProtocolInfo>

#include <QApplication>

#include <textdocument.h>
#include <core.h>
#include <interfaces/isession.h>
#include <uicontroller.h>
#include <util/pushvalue.h>
#include <documentcontroller.h>
#include <workingsetcontroller.h>

#define SYNC_OFTEN

using namespace KDevelop;

bool WorkingSet::m_loading = false;

namespace {

QIcon generateIcon(const WorkingSetIconParameters& params)
{
    QImage pixmap(16, 16, QImage::Format_ARGB32);
    // fill the background with a transparent color
    pixmap.fill(QColor::fromRgba(qRgba(0, 0, 0, 0)));
    const uint coloredCount = params.coloredCount;
    // coordinates of the rectangles to draw, for 16x16 icons specifically
    QList<QRect> rects;
    rects << QRect(1, 1, 5, 5) << QRect(1, 9, 5, 5) << QRect(9, 1, 5, 5) << QRect(9, 9, 5, 5);
    if ( params.swapDiagonal ) {
        rects.swap(1, 2);
    }

    QPainter painter(&pixmap);
    // color for non-colored squares, paint them brighter if the working set is the active one
    const int inact = 40;
    QColor darkColor = QColor::fromRgb(inact, inact, inact);
    // color for colored squares
    // this code is not fragile, you can just tune the magic formulas at random and see what looks good.
    // just make sure to keep it within the 0-360 / 0-255 / 0-255 space of the HSV model
    QColor brightColor = QColor::fromHsv(params.hue, qMin<uint>(255, 215 + (params.setId*5) % 150),
                                         205 + (params.setId*11) % 50);
    // Y'UV "Y" value, the approximate "lightness" of the color
    // If it is above 0.6, then making the color darker a bit is okay,
    // if it is below 0.35, then the color should be a bit brighter.
    float brightY = 0.299 * brightColor.redF() + 0.587 * brightColor.greenF() + 0.114 * brightColor.blueF();
    if ( brightY > 0.6 ) {
        if ( params.setId % 7 < 2 ) {
            // 2/7 chance to make the color significantly darker
            brightColor = brightColor.darker(120 + (params.setId*7) % 35);
        }
        else if ( params.setId % 5 == 0 ) {
            // 1/5 chance to make it a bit darker
            brightColor = brightColor.darker(110 + (params.setId*3) % 10);
        }
    }
    if ( brightY < 0.35 ) {
        // always make the color brighter to avoid very dark colors (like rgb(0, 0, 255))
        brightColor = brightColor.lighter(120 + (params.setId*13) % 55);
    }
    int at = 0;
    foreach ( const QRect& rect, rects ) {
        QColor currentColor;
        // pick the colored squares; you can get different patterns by re-ordering the "rects" list
        if ( (at + params.setId*7) % 4 < coloredCount ) {
            currentColor = brightColor;
        }
        else {
            currentColor = darkColor;
        }
        // draw the filling of the square
        painter.setPen(QColor(currentColor));
        painter.setBrush(QBrush(currentColor));
        painter.drawRect(rect);
        // draw a slight set-in shadow for the square -- it's barely recognizeable,
        // but it looks way better than without
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QColor(0, 0, 0, 50));
        painter.drawRect(rect);
        painter.setPen(QColor(0, 0, 0, 25));
        painter.drawRect(rect.x() + 1, rect.y() + 1, rect.width() - 2, rect.height() - 2);
        at += 1;
    }
    return QIcon(QPixmap::fromImage(pixmap));
}

}

WorkingSet::WorkingSet(const QString& id)
    : QObject()
    , m_id(id)
    , m_icon(generateIcon(id))
{
}

void WorkingSet::saveFromArea( Sublime::Area* a, Sublime::AreaIndex * area, KConfigGroup setGroup, KConfigGroup areaGroup )
{
    if (area->isSplit()) {
        setGroup.writeEntry("Orientation", area->orientation() == Qt::Horizontal ? "Horizontal" : "Vertical");

        if (area->first()) {
            saveFromArea(a, area->first(), KConfigGroup(&setGroup, "0"), KConfigGroup(&areaGroup, "0"));
        }

        if (area->second()) {
            saveFromArea(a, area->second(), KConfigGroup(&setGroup, "1"), KConfigGroup(&areaGroup, "1"));
        }
    } else {
        setGroup.writeEntry("View Count", area->viewCount());
        areaGroup.writeEntry("View Count", area->viewCount());
        int index = 0;
        foreach (Sublime::View* view, area->views()) {
            //The working set config gets an updated list of files
            QString docSpec = view->document()->documentSpecifier();

            //only save the documents from protocols KIO understands
            //otherwise we try to load kdev:// too early
            if (!KProtocolInfo::isKnownProtocol(QUrl(docSpec))) {
                continue;
            }

            setGroup.writeEntry(QString("View %1").arg(index), docSpec);
            setGroup.writeEntry(QString("View %1 Type").arg(index), view->document()->documentType());
            //The area specific config stores the working set documents in order along with their state
            areaGroup.writeEntry(QString("View %1").arg(index), docSpec);
            areaGroup.writeEntry(QString("View %1 State").arg(index), view->viewState());
            ++index;
        }
    }
}

bool WorkingSet::isEmpty() const
{
    KConfigGroup setConfig(Core::self()->activeSession()->config(), "Working File Sets");
    KConfigGroup group = setConfig.group(m_id);
    return !group.hasKey("Orientation") && group.readEntry("View Count", 0) == 0;
}

struct DisableMainWindowUpdatesFromArea
{
    DisableMainWindowUpdatesFromArea(Sublime::Area* area) : m_area(area) {
        if(area) {
            foreach(Sublime::MainWindow* window, Core::self()->uiControllerInternal()->mainWindows()) {
                if(window->area() == area) {
                    if(window->updatesEnabled()) {
                        wasUpdatesEnabled.insert(window);
                        window->setUpdatesEnabled(false);
                    }
                }
            }
        }
    }

    ~DisableMainWindowUpdatesFromArea() {
        if(m_area) {
            foreach(Sublime::MainWindow* window, wasUpdatesEnabled) {
                window->setUpdatesEnabled(wasUpdatesEnabled.contains(window));
            }
        }
    }

    Sublime::Area* m_area;
    QSet<Sublime::MainWindow*> wasUpdatesEnabled;
};

void loadFileList(QStringList& ret, KConfigGroup group)
{
    if (group.hasKey("Orientation")) {
        QStringList subgroups = group.groupList();

        if (subgroups.contains("0")) {

            {
                KConfigGroup subgroup(&group, "0");
                loadFileList(ret, subgroup);
            }

            if (subgroups.contains("1")) {
                KConfigGroup subgroup(&group, "1");
                loadFileList(ret, subgroup);
            }
        }

    } else {

        int viewCount = group.readEntry("View Count", 0);
        for (int i = 0; i < viewCount; ++i) {
            QString type = group.readEntry(QString("View %1 Type").arg(i), "");
            QString specifier = group.readEntry(QString("View %1").arg(i), "");

            ret << specifier;
        }
    }
}

QStringList WorkingSet::fileList() const
{
    QStringList ret;
    KConfigGroup setConfig(Core::self()->activeSession()->config(), "Working File Sets");
    KConfigGroup group = setConfig.group(m_id);

    loadFileList(ret, group);
    return ret;
}

void WorkingSet::loadToArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex) {
    PushValue<bool> enableLoading(m_loading, true);

    /// We cannot disable the updates here, because (probably) due to a bug in Qt,
    /// which causes the updates to stay disabled forever after some complex operations
    /// on the sub-views. This could be reproduced by creating two working-sets with complex
    /// split-view configurations and switching between them. Re-enabling the updates doesn't help.
//     DisableMainWindowUpdatesFromArea updatesDisabler(area);

    kDebug() << "loading working-set" << m_id << "into area" << area;
    
    QMultiMap<QString, Sublime::View*> recycle;
    
    foreach( Sublime::View* view, area->views() )
        recycle.insert( view->document()->documentSpecifier(), area->removeView(view) );
    
    kDebug() << "recycling" << recycle.size() << "old views";
    
    Q_ASSERT( area->views().empty() );

    KConfigGroup setConfig(Core::self()->activeSession()->config(), "Working File Sets");
    KConfigGroup setGroup = setConfig.group(m_id);
    KConfigGroup areaGroup = setConfig.group(m_id + '|' + area->title());

    loadToArea(area, areaIndex, setGroup, areaGroup, recycle);
    
    // Delete views which were not recycled
    kDebug() << "deleting " << recycle.size() << " old views";
    qDeleteAll( recycle.values() );
    
    area->setActiveView(0);
    
    //activate view in the working set
    /// @todo correctly select one out of multiple equal views
    QString activeView = areaGroup.readEntry("Active View", QString());
    foreach (Sublime::View *v, area->views()) {
        if (v->document()->documentSpecifier() == activeView) {
            area->setActiveView(v);
            break;
        }
    }
    
    if( !area->activeView() && area->views().size() )
        area->setActiveView( area->views()[0] );
    
    if( area->activeView() ) {
        foreach(Sublime::MainWindow* window, Core::self()->uiControllerInternal()->mainWindows()) {
                if(window->area() == area) {
                    window->activateView( area->activeView() );
                }
        }
    }
}

void WorkingSet::loadToArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex, KConfigGroup setGroup, KConfigGroup areaGroup, QMultiMap<QString, Sublime::View*>& recycle)
{
    Q_ASSERT( !areaIndex->isSplit() );
    if (setGroup.hasKey("Orientation")) {
        QStringList subgroups = setGroup.groupList();
        /// @todo also save and restore the ratio

        if (subgroups.contains("0") && subgroups.contains("1")) {
//             kDebug() << "has zero, split:" << split;

            Qt::Orientation orientation = setGroup.readEntry("Orientation", "Horizontal") == "Vertical" ? Qt::Vertical : Qt::Horizontal;
            if(!areaIndex->isSplit()){
                areaIndex->split(orientation);
            }else{
                areaIndex->setOrientation(orientation);
            }

            loadToArea(area, areaIndex->first(), KConfigGroup(&setGroup, "0"), KConfigGroup(&areaGroup, "0"), recycle);

            loadToArea(area, areaIndex->second(), KConfigGroup(&setGroup, "1"), KConfigGroup(&areaGroup, "1"), recycle);
            
            if( areaIndex->first()->viewCount() == 0 )
                areaIndex->unsplit(areaIndex->first());
            else if( areaIndex->second()->viewCount() == 0 )
                areaIndex->unsplit(areaIndex->second());
        }
    } else {
        
        //Load all documents from the workingset into this areaIndex
        int viewCount = setGroup.readEntry("View Count", 0);
        QMap<int, Sublime::View*> createdViews;
        for (int i = 0; i < viewCount; ++i) {
            QString type = setGroup.readEntry(QString("View %1 Type").arg(i), "");
            QString specifier = setGroup.readEntry(QString("View %1").arg(i), "");
            Sublime::View* previousView = area->views().empty() ? 0 : area->views().back();

            QMultiMap<QString, Sublime::View*>::iterator it = recycle.find( specifier );
            if( it != recycle.end() )
            {
                area->addView( *it, areaIndex, previousView );
                recycle.erase( it );
                continue;
            }
            IDocument* doc = Core::self()->documentControllerInternal()->openDocument(QUrl::fromUserInput(specifier),
                             KTextEditor::Cursor::invalid(), IDocumentController::DoNotActivate | IDocumentController::DoNotCreateView);
            Sublime::Document *document = dynamic_cast<Sublime::Document*>(doc);
            if (document) {
                Sublime::View* view = document->createView();
                area->addView(view, areaIndex, previousView);
                createdViews[i] = view;
            } else {
                kWarning() << "Unable to create view of type " << type;
            }
        }
        
        //Load state
        for (int i = 0; i < viewCount; ++i)
        {
            QString state = areaGroup.readEntry(QString("View %1 State").arg(i));
            if (state.length() && createdViews.contains(i))
                createdViews[i]->setState(state);
        }
    }
}

void deleteGroupRecursive(KConfigGroup group) {
//     kDebug() << "deleting" << group.name();
    foreach(const QString& entry, group.entryMap().keys()) {
        group.deleteEntry(entry);
    }
    Q_ASSERT(group.entryMap().isEmpty());

    foreach(const QString& subGroup, group.groupList()) {
        deleteGroupRecursive(group.group(subGroup));
        group.deleteGroup(subGroup);
    }
    //Why doesn't this work?
//     Q_ASSERT(group.groupList().isEmpty());
    group.deleteGroup();

#ifdef SYNC_OFTEN
    group.sync();
#endif
}

void WorkingSet::deleteSet(bool force, bool silent)
{
    if(m_areas.isEmpty() || force) {
        emit aboutToRemove(this);

        KConfigGroup setConfig(Core::self()->activeSession()->config(), "Working File Sets");
        KConfigGroup group = setConfig.group(m_id);
        deleteGroupRecursive(group);
#ifdef SYNC_OFTEN
        setConfig.sync();
#endif

        if(!silent)
            emit setChangedSignificantly();
    }
}

void WorkingSet::saveFromArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex)
{
    kDebug() << "saving" << m_id << "from area";

    bool wasPersistent = isPersistent();

    KConfigGroup setConfig(Core::self()->activeSession()->config(), "Working File Sets");

    KConfigGroup setGroup = setConfig.group(m_id);
    deleteGroupRecursive(setGroup);

    KConfigGroup areaGroup = setConfig.group(m_id + '|' + area->title());
    QString lastActiveView = areaGroup.readEntry("Active View", "");
    deleteGroupRecursive(areaGroup);
    if (area->activeView() && area->activeView()->document())
        areaGroup.writeEntry("Active View", area->activeView()->document()->documentSpecifier());
    else
        areaGroup.writeEntry("Active View", lastActiveView);

    saveFromArea(area, areaIndex, setGroup, areaGroup);

    if(isEmpty())
    {
        deleteGroupRecursive(setGroup);
        deleteGroupRecursive(areaGroup);
    }

    setPersistent(wasPersistent);

#ifdef SYNC_OFTEN
    setConfig.sync();
#endif
    
    emit setChangedSignificantly();
}

void WorkingSet::areaViewAdded(Sublime::AreaIndex*, Sublime::View*) {
    Sublime::Area* area = qobject_cast<Sublime::Area*>(sender());
    Q_ASSERT(area);
    Q_ASSERT(area->workingSet() == m_id);

    kDebug() << "added view in" << area << ", id" << m_id;
    if (m_loading) {
        kDebug() << "doing nothing because loading";
        return;
    }

    changed(area);
}

void WorkingSet::areaViewRemoved(Sublime::AreaIndex*, Sublime::View* view) {
    Sublime::Area* area = qobject_cast<Sublime::Area*>(sender());
    Q_ASSERT(area);
    Q_ASSERT(area->workingSet() == m_id);

    kDebug() << "removed view in" << area << ", id" << m_id;
    if (m_loading) {
        kDebug() << "doing nothing because loading";
        return;
    }
    
    foreach(Sublime::Area* otherArea, m_areas)
    {
        if(otherArea == area)
            continue;
        bool hadDocument = false;
        foreach(Sublime::View* areaView, otherArea->views())
            if(view->document() == areaView->document())
                hadDocument = true;

        if(!hadDocument)
        {
            // We do this to prevent UI flicker. The view has already been removed from
            // one of the connected areas, so the working-set has already recorded the change.
            return;
        }
        
    }
    
    changed(area);
}

void WorkingSet::setPersistent(bool persistent) {
    KConfigGroup setConfig(Core::self()->activeSession()->config(), "Working File Sets");
    KConfigGroup group = setConfig.group(m_id);
    group.writeEntry("persistent", persistent);
#ifdef SYNC_OFTEN
    group.sync();
#endif
    kDebug() << "setting" << m_id << "persistent:" << persistent;
}

bool WorkingSet::isPersistent() const {
    KConfigGroup setConfig(Core::self()->activeSession()->config(), "Working File Sets");
    KConfigGroup group = setConfig.group(m_id);
    return group.readEntry("persistent", false);
}

QIcon WorkingSet::icon() const
{
    return m_icon;
}

bool WorkingSet::isConnected( Sublime::Area* area )
{
  return m_areas.contains( area );
}

QString WorkingSet::id() const
{
  return m_id;
}

bool WorkingSet::hasConnectedAreas() const
{
  return !m_areas.isEmpty();
}

bool WorkingSet::hasConnectedAreas( QList< Sublime::Area* > areas ) const
{
  foreach( Sublime::Area* area, areas )

  if ( m_areas.contains( area ) )
    return true;

  return false;
}

void WorkingSet::connectArea( Sublime::Area* area )
{
  if ( m_areas.contains( area ) ) {
    kDebug() << "tried to double-connect area";
    return;
  }

  kDebug() << "connecting" << m_id << "to area" << area;

//         Q_ASSERT(area->workingSet() == m_id);

  m_areas.push_back( area );
  connect( area, SIGNAL(viewAdded(Sublime::AreaIndex*,Sublime::View*)), this, SLOT(areaViewAdded(Sublime::AreaIndex*,Sublime::View*)) );
  connect( area, SIGNAL(viewRemoved(Sublime::AreaIndex*,Sublime::View*)), this, SLOT(areaViewRemoved(Sublime::AreaIndex*,Sublime::View*)) );
}

void WorkingSet::disconnectArea( Sublime::Area* area )
{
  if ( !m_areas.contains( area ) ) {
    kDebug() << "tried to disconnect not connected area";
    return;
  }

  kDebug() << "disconnecting" << m_id << "from area" << area;

//         Q_ASSERT(area->workingSet() == m_id);

  disconnect( area, SIGNAL(viewAdded(Sublime::AreaIndex*,Sublime::View*)), this, SLOT(areaViewAdded(Sublime::AreaIndex*,Sublime::View*)) );
  disconnect( area, SIGNAL(viewRemoved(Sublime::AreaIndex*,Sublime::View*)), this, SLOT(areaViewRemoved(Sublime::AreaIndex*,Sublime::View*)) );
  m_areas.removeAll( area );
}

void WorkingSet::deleteSet()
{
  deleteSet( false );
}

void WorkingSet::changed( Sublime::Area* area )
{
  if ( m_loading ) {
    return;
  }

  {
    //Do not capture changes done while loading
    PushValue<bool> enableLoading( m_loading, true );

    kDebug() << "recording change done to" << m_id;
    saveFromArea( area, area->rootIndex() );

    for ( QList< QPointer< Sublime::Area > >::iterator it = m_areas.begin(); it != m_areas.end(); ++it ) {
      if (( *it ) != area ) {
        loadToArea(( *it ), ( *it )->rootIndex() );
      }
    }
  }

  emit setChangedSignificantly();
}

