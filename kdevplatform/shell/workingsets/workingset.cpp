/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "workingset.h"
#include "debug_workingset.h"

#include <sublime/area.h>
#include <sublime/document.h>
#include <sublime/mainwindow.h>
#include <sublime/view.h>

#include <KProtocolInfo>
#include <KTextEditor/View>

#include <textdocument.h>
#include <core.h>
#include <interfaces/isession.h>
#include <uicontroller.h>
#include <util/pushvalue.h>
#include <documentcontroller.h>

#include <QFileInfo>
#include <QPainter>
#include <QSplitter>

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
    QList<QRect> rects{
        {1, 1, 5, 5},
        {1, 9, 5, 5},
        {9, 1, 5, 5},
        {9, 9, 5, 5},
    };
    if (params.swapDiagonal) {
        rects.swapItemsAt(1, 2);
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
    for (const QRect& rect : std::as_const(rects)) {
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

QSplitter* loadToAreaPrivate(Sublime::Area *area, Sublime::AreaIndex *areaIndex, const KConfigGroup &setGroup, QMultiMap<QString, Sublime::View*> &recycle)
{
    Q_ASSERT(!areaIndex->isSplit());

    QSplitter *parentSplitter = nullptr;

    if (setGroup.hasKey("Orientation")) {
        QStringList subgroups = setGroup.groupList();
        if (!subgroups.contains(QStringLiteral("0"))) {
            if (subgroups.contains(QStringLiteral("1"))) {
                parentSplitter = loadToAreaPrivate(area, areaIndex, KConfigGroup(&setGroup, QStringLiteral("1")), recycle);
            }
        } else if (!subgroups.contains(QStringLiteral("1"))) {
            parentSplitter = loadToAreaPrivate(area, areaIndex, KConfigGroup(&setGroup, QStringLiteral("0")), recycle);
        } else {
            areaIndex->split(setGroup.readEntry("Orientation", "Horizontal") == QLatin1String("Vertical") ? Qt::Vertical : Qt::Horizontal);

            parentSplitter = loadToAreaPrivate(area, areaIndex->first(), KConfigGroup(&setGroup, QStringLiteral("0")), recycle);
            if (!parentSplitter) {
                areaIndex->unsplit(areaIndex->first());
                parentSplitter = loadToAreaPrivate(area, areaIndex, KConfigGroup(&setGroup, QStringLiteral("1")), recycle);
            } else if (auto *splitter = loadToAreaPrivate(area, areaIndex->second(), KConfigGroup(&setGroup, QStringLiteral("1")), recycle)) {
                splitter->setSizes(setGroup.readEntry("Sizes", QList<int>({1, 1})));
                parentSplitter = qobject_cast<QSplitter*>(splitter->parent());
            } else {
                areaIndex->unsplit(areaIndex->second());
                emit area->viewAdded(areaIndex, nullptr);
            }
        }
    } else {
        //Load all documents from the workingset into this areaIndex
        int viewCount = setGroup.readEntry("View Count", 0);
        QVector<Sublime::View*> createdViews(viewCount, nullptr);
        for (int i = 0; i < viewCount; ++i) {
            QString specifier = setGroup.readEntry(QStringLiteral("View %1").arg(i), QString());
            if (specifier.isEmpty()) {
                continue;
            }

            Sublime::View* previousView = areaIndex->views().empty() ? nullptr : areaIndex->views().back();

            QMultiMap<QString, Sublime::View*>::iterator it = recycle.find(specifier);
            if (it != recycle.end()) {
                area->addView(*it, areaIndex, previousView);
                createdViews[i] = *it;
                recycle.erase(it);
                continue;
            }
            auto url = QUrl::fromUserInput(specifier);
            if (url.isLocalFile() && !QFileInfo::exists(url.path())) {
                // This code runs both on KDevelop start and when active area changes from Code to Debug or vice versa.
                // If a document is already open, it can be opened again, even if its file does not exist on disk
                // (an untitled document, opened nonexistent file or a file deleted from disk externally after opening).
                // documentForUrl() returns a valid pointer for an already open document URL.
                if (!Core::self()->documentControllerInternal()->documentForUrl(url)) {
                    qCWarning(WORKINGSET) << "Unable to find file" << specifier;
                    continue;
                }
            }
            IDocument* doc = Core::self()->documentControllerInternal()->openDocument(url,
                             KTextEditor::Cursor::invalid(), IDocumentController::DoNotActivate | IDocumentController::DoNotCreateView);
            if (auto document = dynamic_cast<Sublime::Document*>(doc)) {
                Sublime::View *view = document->createView();
                area->addView(view, areaIndex, previousView);
                createdViews[i] = view;
            } else {
                qCWarning(WORKINGSET) << "Unable to create view" << specifier;
            }
        }

        //Load state
        int activeIndex = setGroup.readEntry(QStringLiteral("Active View"), -1);
        for (int i = 0; i < viewCount; ++i) {
            if (!createdViews[i]) {
                continue;
            }
            KConfigGroup viewGroup(&setGroup, QStringLiteral("View %1 Config").arg(i));
            if (viewGroup.exists()) {
                createdViews[i]->readSessionConfig(viewGroup);
                if (auto textView = qobject_cast<TextView*>(createdViews[i])) {
                    if (auto kateView = textView->textView()) {
                        auto cursors = viewGroup.readEntry("Selection", QList<int>());
                        if (cursors.size() == 4) {
                            kateView->setSelection(KTextEditor::Range(cursors.at(0), cursors.at(1), cursors.at(2), cursors.at(3)));
                        }
                    }
                }
            }
            if (i == activeIndex) {
                area->setActiveView(createdViews[i]);
            }
            if (!parentSplitter) {
                auto p = createdViews[i]->widget()->parentWidget();
                while (p && !(parentSplitter = qobject_cast<QSplitter*>(p))) {
                    p = p->parentWidget();
                }
                if (parentSplitter) {
                    parentSplitter = qobject_cast<QSplitter*>(parentSplitter->parent());
                }
            }
        }
    }

    return parentSplitter;
}

QSplitter* saveFromAreaPrivate(Sublime::AreaIndex *area, KConfigGroup setGroup, const Sublime::View *activeView)
{
    QSplitter *parentSplitter = nullptr;

    if (area->isSplit()) {
        if (!area->first()) {
            parentSplitter = saveFromAreaPrivate(area->second(), setGroup, activeView);
        } else if (!area->second()) {
            parentSplitter = saveFromAreaPrivate(area->first(), setGroup, activeView);
        } else {
            parentSplitter = saveFromAreaPrivate(area->first(), KConfigGroup(&setGroup, QStringLiteral("0")), activeView);
            if (!parentSplitter) {
                parentSplitter = saveFromAreaPrivate(area->second(), setGroup, activeView);
            } else if (saveFromAreaPrivate(area->second(), KConfigGroup(&setGroup, QStringLiteral("1")), activeView)) {
                setGroup.writeEntry("Orientation", area->orientation() == Qt::Horizontal ? "Horizontal" : "Vertical");
                setGroup.writeEntry("Sizes", parentSplitter->sizes());
            } else {
                // move up settings of group "0"
                KConfigGroup(&setGroup, QStringLiteral("0")).copyTo(&setGroup);
                setGroup.deleteGroup(QStringLiteral("0"));
            }
        }
    } else {
        int index = 0;
        int activeIndex = -1;
        const auto views = area->views();
        for (Sublime::View *view : views) {
            //The working set config gets an updated list of files
            QString docSpec = view->document()->documentSpecifier();

            if (view == activeView) {
                activeIndex = index;
            }

            //only save the documents from protocols KIO understands
            //otherwise we try to load kdev:// too early
            if (!KProtocolInfo::isKnownProtocol(QUrl(docSpec))) {
                continue;
            }

            setGroup.writeEntry(QStringLiteral("View %1").arg(index), docSpec);
            KConfigGroup viewGroup(&setGroup, QStringLiteral("View %1 Config").arg(index));
            view->writeSessionConfig(viewGroup);
            if (auto textView = qobject_cast<TextView*>(view)) {
                if (auto kateView = textView->textView()) {
                    auto range = kateView->selectionRange();
                    if (range.isValid()) {
                        viewGroup.writeEntry("Selection", QList<int>({range.start().line(), range.start().column(),
                                                                      range.end().line(), range.end().column()}));
                    }
                }
            }
            ++index;

            if (!parentSplitter && view->hasWidget()) {
                auto p = view->widget()->parentWidget();
                while (p && !(parentSplitter = qobject_cast<QSplitter*>(p))) {
                    p = p->parentWidget();
                }
            }
        }
        if (index > 0) {
            setGroup.writeEntry("View Count", index);
            if (activeIndex >= 0) {
                setGroup.writeEntry(QStringLiteral("Active View"), std::min(activeIndex, index - 1));
            }
        }
    }

    return parentSplitter ? qobject_cast<QSplitter*>(parentSplitter->parent()) : nullptr;
}


}

WorkingSet::WorkingSet(const QString& id)
    : QObject()
    , m_id(id)
    , m_icon(generateIcon(WorkingSetIconParameters(id)))
{
}

bool WorkingSet::isEmpty() const
{
    KConfigGroup setConfig(Core::self()->activeSession()->config(), QStringLiteral("Working File Sets"));
    KConfigGroup group = setConfig.group(m_id);
    return !group.hasKey("Orientation") && group.readEntry("View Count", 0) == 0;
}

struct DisableMainWindowUpdatesFromArea
{
    explicit DisableMainWindowUpdatesFromArea(Sublime::Area* area) : m_area(area) {
        if(area) {
            const auto windows = Core::self()->uiControllerInternal()->mainWindows();
            for (Sublime::MainWindow* window : windows) {
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
            for (Sublime::MainWindow* window : std::as_const(wasUpdatesEnabled)) {
                window->setUpdatesEnabled(wasUpdatesEnabled.contains(window));
            }
        }
    }

private:
    Q_DISABLE_COPY(DisableMainWindowUpdatesFromArea)

    Sublime::Area* m_area;
    QSet<Sublime::MainWindow*> wasUpdatesEnabled;
};

void loadFileList(QStringList& ret, const KConfigGroup& group)
{
    if (group.hasKey("Orientation")) {
        QStringList subgroups = group.groupList();

        if (subgroups.contains(QStringLiteral("0"))) {

            {
                KConfigGroup subgroup(&group, QStringLiteral("0"));
                loadFileList(ret, subgroup);
            }

            if (subgroups.contains(QStringLiteral("1"))) {
                KConfigGroup subgroup(&group, QStringLiteral("1"));
                loadFileList(ret, subgroup);
            }
        }

    } else {
        int viewCount = group.readEntry("View Count", 0);
        ret.reserve(ret.size() + viewCount);
        for (int i = 0; i < viewCount; ++i) {
            QString specifier = group.readEntry(QStringLiteral("View %1").arg(i), QString());
            auto url = QUrl::fromUserInput(specifier);
            if (url.isLocalFile() && !QFileInfo::exists(url.path())) {
                continue;
            }
            ret << specifier;
        }
    }
}

QStringList WorkingSet::fileList() const
{
    QStringList ret;
    KConfigGroup setConfig(Core::self()->activeSession()->config(), QStringLiteral("Working File Sets"));
    KConfigGroup group = setConfig.group(m_id);

    loadFileList(ret, group);
    return ret;
}

QSet<QString> WorkingSet::fileSet() const
{
    const QStringList fileList = this->fileList();
    return QSet<QString>(fileList.begin(), fileList.end());
}

void WorkingSet::loadToArea(Sublime::Area* area) {
    PushValue<bool> enableLoading(m_loading, true);

    /// We cannot disable the updates here, because (probably) due to a bug in Qt,
    /// which causes the updates to stay disabled forever after some complex operations
    /// on the sub-views. This could be reproduced by creating two working-sets with complex
    /// split-view configurations and switching between them. Re-enabling the updates doesn't help.
//     DisableMainWindowUpdatesFromArea updatesDisabler(area);

    qCDebug(WORKINGSET) << "loading working-set" << m_id << "into area" << area;

    QMultiMap<QString, Sublime::View*> recycle;

    const auto viewsBefore = area->views();
    for (Sublime::View* view : viewsBefore) {
        recycle.insert( view->document()->documentSpecifier(), area->removeView(view) );
    }

    qCDebug(WORKINGSET) << "recycling up to" << recycle.size() << "old views";

    Q_ASSERT( area->views().empty() );

    KConfigGroup setConfig(Core::self()->activeSession()->config(), QStringLiteral("Working File Sets"));
    KConfigGroup setGroup = setConfig.group(m_id);

    // Migrate from former by-area configs to a shared config
    KConfigGroup areaGroup = setConfig.group(m_id + QLatin1Char('|') + area->title());
    if (areaGroup.exists()) {
        if (setGroup.readEntry("Active View", QString()).isEmpty()) {
            setGroup.writeEntry("Active View", areaGroup.readEntry("Active View", QString()));
        }
        int viewCount = setGroup.readEntry("View Count", 0);
        QMultiMap<QString, KConfigGroup> oldViewConfigs;
        for (int i = 0; i < viewCount; ++i) {
            KConfigGroup viewGroup(&setGroup, QStringLiteral("View %1 Config").arg(i));
            auto specifier = setGroup.readEntry(QStringLiteral("View %1").arg(i), QString());
            if (viewGroup.exists() || specifier.isEmpty()) {
                continue;
            }
            if (oldViewConfigs.empty()) { // cache all view configs from the old area config
                int oldViewCount = areaGroup.readEntry("View Count", 0);
                for (int j = 0; j < oldViewCount; ++j) {
                    auto oldSpecifier = areaGroup.readEntry(QStringLiteral("View %1").arg(j), QString());
                    if (!oldSpecifier.isEmpty()) {
                        oldViewConfigs.insert(oldSpecifier, KConfigGroup(&areaGroup, QStringLiteral("View %1 Config").arg(j)));
                    }
                }
            }
            auto it = oldViewConfigs.find(specifier);
            if (it != oldViewConfigs.end()) {
                viewGroup.copyTo(&(*it));
                oldViewConfigs.erase(it);
            }
            if (oldViewConfigs.empty()) {
                break;
            }
        }
        setConfig.deleteGroup(areaGroup.name());
    }

    loadToAreaPrivate(area, area->rootIndex(), setGroup, recycle);

    // Delete views which were not recycled
    qCDebug(WORKINGSET) << "deleting" << recycle.size() << "old views";
    qDeleteAll(recycle);

    if (area->views().isEmpty()) {
        return;
    }

    if (!area->activeView()) {
        area->setActiveView(area->views().at(0));
    }

    const auto windows = Core::self()->uiControllerInternal()->mainWindows();
    for (Sublime::MainWindow* window : windows) {
        if (window->area() == area) {
            window->activateView(area->activeView());
        }
    }
}

void WorkingSet::deleteSet(bool force, bool silent)
{
    if(m_areas.isEmpty() || force) {
        emit aboutToRemove(this);

        KConfigGroup setConfig(Core::self()->activeSession()->config(), QStringLiteral("Working File Sets"));
        KConfigGroup group = setConfig.group(m_id);
        group.deleteGroup();
#ifdef SYNC_OFTEN
        setConfig.sync();
#endif

        if(!silent)
            emit setChangedSignificantly();
    }
}

void WorkingSet::saveFromArea(Sublime::Area* area)
{
    qCDebug(WORKINGSET) << "saving" << m_id << "from area";

    bool wasPersistent = isPersistent();

    KConfigGroup setConfig(Core::self()->activeSession()->config(), QStringLiteral("Working File Sets"));

    KConfigGroup setGroup = setConfig.group(m_id);
    setGroup.deleteGroup();

    saveFromAreaPrivate(area->rootIndex(), setGroup, area->activeView());

    if (isEmpty()) {
        setGroup.deleteGroup();
    } else {
        setPersistent(wasPersistent);
    }

#ifdef SYNC_OFTEN
    setConfig.sync();
#endif

    emit setChangedSignificantly();
}

void WorkingSet::areaViewAdded(Sublime::AreaIndex*, Sublime::View*) {
    auto* area = qobject_cast<Sublime::Area*>(sender());
    Q_ASSERT(area);
    Q_ASSERT(area->workingSet() == m_id);

    qCDebug(WORKINGSET) << "added view in" << area << ", id" << m_id;
    if (m_loading) {
        qCDebug(WORKINGSET) << "doing nothing because loading";
        return;
    }

    changed(area);
}

void WorkingSet::areaViewRemoved(Sublime::AreaIndex*, Sublime::View* view) {
    auto* area = qobject_cast<Sublime::Area*>(sender());
    Q_ASSERT(area);
    Q_ASSERT(area->workingSet() == m_id);

    qCDebug(WORKINGSET) << "removed view in" << area << ", id" << m_id;
    if (m_loading) {
        qCDebug(WORKINGSET) << "doing nothing because loading";
        return;
    }

    const auto areasBefore = m_areas; // TODO: check if areas could be changed, otherwise use m_areas directly
    for (Sublime::Area* otherArea : areasBefore) {
        if(otherArea == area)
            continue;
        const auto otherAreaViews = otherArea->views();
        bool hadDocument = std::any_of(otherAreaViews.begin(), otherAreaViews.end(), [&](Sublime::View* otherView) {
            return (view->document() == otherView->document());
        });

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
    KConfigGroup setConfig(Core::self()->activeSession()->config(), QStringLiteral("Working File Sets"));
    KConfigGroup group = setConfig.group(m_id);
    group.writeEntry("persistent", persistent);
#ifdef SYNC_OFTEN
    group.sync();
#endif
    qCDebug(WORKINGSET) << "setting" << m_id << "persistent:" << persistent;
}

bool WorkingSet::isPersistent() const {
    KConfigGroup setConfig(Core::self()->activeSession()->config(), QStringLiteral("Working File Sets"));
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

bool WorkingSet::hasConnectedArea(Sublime::Area *area) const
{
  return m_areas.contains(area);
}

void WorkingSet::connectArea( Sublime::Area* area )
{
  if ( m_areas.contains( area ) ) {
    qCDebug(WORKINGSET) << "tried to double-connect area";
    return;
  }

  qCDebug(WORKINGSET) << "connecting" << m_id << "to area" << area;

//         Q_ASSERT(area->workingSet() == m_id);

  m_areas.push_back( area );
  connect( area, &Sublime::Area::viewAdded, this, &WorkingSet::areaViewAdded );
  connect( area, &Sublime::Area::viewRemoved, this, &WorkingSet::areaViewRemoved );
}

void WorkingSet::disconnectArea( Sublime::Area* area )
{
  if ( !m_areas.contains( area ) ) {
    qCDebug(WORKINGSET) << "tried to disconnect not connected area";
    return;
  }

  qCDebug(WORKINGSET) << "disconnecting" << m_id << "from area" << area;

//         Q_ASSERT(area->workingSet() == m_id);

  disconnect( area, &Sublime::Area::viewAdded, this, &WorkingSet::areaViewAdded );
  disconnect( area, &Sublime::Area::viewRemoved, this, &WorkingSet::areaViewRemoved );
  m_areas.removeAll( area );
}

void WorkingSet::changed( Sublime::Area* area )
{
  if ( m_loading ) {
    return;
  }

  {
    //Do not capture changes done while loading
    PushValue<bool> enableLoading( m_loading, true );

    qCDebug(WORKINGSET) << "recording change done to" << m_id;
    saveFromArea(area);
  }

  emit setChangedSignificantly();
}

#include "moc_workingset.cpp"
