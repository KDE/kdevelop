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

#include "workingsetcontroller.h"
#include <kconfiggroup.h>
#include <kconfig.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kcolorutils.h>
#include <sublime/view.h>
#include <sublime/areaindex.h>
#include <sublime/document.h>
#include <interfaces/idocument.h>
#include "core.h"
#include "documentcontroller.h"
#include <sublime/area.h>
#include "mainwindow.h"
#include <qboxlayout.h>
#include <klocalizedstring.h>

using namespace KDevelop;

//Random set of icons that are well distinguishable from each other. If the user doesn't have them, they won't be used.
QStringList setIcons = QStringList() << "chronometer" << "games-config-tiles" << "im-user" << "irc-voice" << "irc-operator" << "office-chart-pie" << "office-chart-ring" << "speaker" << "view-pim-notes" << "esd" << "akonadi" << "kbugbuster" << "kleopatra" << "nepomuk" << "package_edutainment_art" << "package_edutainment_music" << "package_games_amusement" << "package_games_sports" << "package_network" << "package_office_database" << "package_system_applet" << "package_system_emulator" << "preferences-desktop-notification-bell" << "wine" << "utilities-desktop-extra" << "step" << "preferences-web-browser-cookies" << "preferences-plugin" << "preferences-kcalc-constants" << "preferences-desktop-icons" << "tagua" << "inkscape" << "java" << "accessories-calculator" << "kblogger" << "preferences-desktop-personal" << "emblem-favorite" << "face-smile-big" << "face-embarrassed" << "user-identity" << "mail-tagged" << "media-playlist-suffle" << "weather-clouds";

WorkingSetController::WorkingSetController(Core* core) : m_core(core)
{
}

void WorkingSetController::cleanup()
{
    foreach(WorkingSet* set, m_workingSets)
        delete set;
}


bool WorkingSetController::usingIcon(QString icon)
{
    foreach(WorkingSet* set, m_workingSets)
        if(set->icon() == icon)
            return true;
    return false;
}

bool WorkingSetController::iconValid(QString icon)
{
    return !KIconLoader::global()->iconPath(icon, KIconLoader::Small, true).isNull();
}

WorkingSet* WorkingSetController::getWorkingSet(QString id)
{
    if(!m_workingSets.contains(id)) {
        QString icon;
        for(int a = 0; a < 100; ++a) {
            int pick = (qHash(id) + a) % setIcons.size(); ///@todo Pick icons semantically, by content, and store them in the config
            if(!usingIcon(setIcons[pick])) {
                if(iconValid(setIcons[pick])) {
                    icon = setIcons[pick];
                break;
                }
            }
        }
        if(icon.isEmpty()) {
            kDebug() << "found no icon for working-set" << id;
            icon = "invalid";
        }
        m_workingSets[id] = new WorkingSet(id, icon);
        emit workingSetAdded(id);
    }
    
    return m_workingSets[id];
}

void WorkingSet::saveFromArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex)
{
    ///@todo Make the working-sets session-specific
    KConfigGroup setConfig(KGlobal::config(), "Working Sets");
    KConfigGroup group = setConfig.group(m_id);
    saveFromArea(area, areaIndex, group);
}

void WorkingSet::saveFromArea(Sublime::Area* a, Sublime::AreaIndex * area, KConfigGroup & group)
{
    if (area->isSplitted()) {
        group.writeEntry("Orientation", area->orientation() == Qt::Horizontal ? "Horizontal" : "Vertical");

        if (area->first()) {
            KConfigGroup subgroup(&group, "0");
            subgroup.deleteGroup();
            saveFromArea(a, area->first(), subgroup);
        }

        if (area->second()) {
            KConfigGroup subgroup(&group, "1");
            subgroup.deleteGroup();
            saveFromArea(a, area->second(), subgroup);
        }
    } else {
        group.writeEntry("View Count", area->viewCount());

        int index = 0;
        foreach (Sublime::View* view, area->views()) {
            group.writeEntry(QString("View %1 Type").arg(index), view->document()->documentType());
            group.writeEntry(QString("View %1").arg(index), view->document()->documentSpecifier());
            QString state = view->viewState();
            if (!state.isEmpty())
                group.writeEntry(QString("View %1 State").arg(index), state);

            ++index;
        }
        
        //Do not spam the configuration-file with empty working sets. Do not store them at all.
//         if(area->viewCount() == 0)
//             group.deleteGroup();
    }
}


bool WorkingSet::isEmpty() const
{
    KConfigGroup setConfig(KGlobal::config(), "Working Sets");
    KConfigGroup group = setConfig.group(m_id);
    return group.groupList().isEmpty() && group.readEntry("View Count", 0) == 0;
}

void WorkingSet::loadToArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex, bool clear) {
    m_loading = true;
    kDebug() << "loading working-set" << m_id << "into area" << area;
    
    if(clear) {
        kDebug() << "clearing area with working-set" << area->workingSet();
        area->clearViews();
    }
    
    KConfigGroup setConfig(KGlobal::config(), "Working Sets");
    KConfigGroup group = setConfig.group(m_id);
    loadToArea(area, areaIndex, group);

    kDebug() << "ready";
    m_loading = false;
}

void WorkingSet::loadToArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex, KConfigGroup group)
{
    if (group.hasKey("Orientation")) {
        QStringList subgroups = group.groupList();

        if (subgroups.contains("0")) {
            if (!areaIndex->isSplitted())
                areaIndex->split(group.readEntry("Orientation", "Horizontal") == "Vertical" ? Qt::Vertical : Qt::Horizontal);

            KConfigGroup subgroup(&group, "0");
            loadToArea(area, areaIndex->first(), subgroup);

            if (subgroups.contains("1")) {
                Q_ASSERT(areaIndex->isSplitted());
                KConfigGroup subgroup(&group, "1");
                loadToArea(area, areaIndex->second(), subgroup);
            }
        }

    } else {
        while (areaIndex->isSplitted()) {
            areaIndex = areaIndex->first();
            Q_ASSERT(areaIndex);// Split area index did not contain a first child area index if this fails
        }

        int viewCount = group.readEntry("View Count", 0);
        for (int i = 0; i < viewCount; ++i) {
            QString type = group.readEntry(QString("View %1 Type").arg(i), "");
            QString specifier = group.readEntry(QString("View %1").arg(i), "");

            bool viewExists = false;
            foreach (Sublime::View* view, areaIndex->views()) {
                if (view->document()->documentSpecifier() == specifier) {
                    viewExists = true;
                    break;
                }
            }

            if (viewExists)
                continue;

            IDocument* doc = Core::self()->documentControllerInternal()->openDocument(specifier,
                             KTextEditor::Cursor::invalid(), IDocumentController::DoNotActivate | IDocumentController::DoNotCreateView);
            Sublime::Document *document = dynamic_cast<Sublime::Document*>(doc);
            if (document) {
                Sublime::View* view = document->createView();

                QString state = group.readEntry(QString("View %1 State").arg(i), "");
                if (!state.isEmpty())
                    view->setState(state);

                area->addView(view, areaIndex);
            } else {
                kWarning() << "Unable to create view of type " << type;
            }
        }
    }
}

QWidget* WorkingSetController::createSetManagerWidget(MainWindow* parent, bool local) {
    return new WorkingSetWidget(parent, this, local);
}

WorkingSetWidget::WorkingSetWidget(MainWindow* parent, WorkingSetController* controller, bool mini) : QWidget(parent), m_mini(mini), m_mainWindow(parent) {
    m_layout = new QHBoxLayout(this);
    connect(parent, SIGNAL(areaChanged(Sublime::Area*)), this, SLOT(areaChanged(Sublime::Area*)));
    connect(controller, SIGNAL(workingSetAdded(QString)), this, SLOT(workingSetsChanged()));
    connect(controller, SIGNAL(workingSetRemoved(QString)), this, SLOT(workingSetsChanged()));
    if(parent->area())
        areaChanged(parent->area());
    workingSetsChanged();
}

void WorkingSetWidget::areaChanged(Sublime::Area* area)
{
    if(m_connectedArea) {
        disconnect(m_connectedArea, SIGNAL(changingWorkingSet(Sublime::Area*,QString,QString)), this, SLOT(changingWorkingSet(Sublime::Area*,QString,QString)));
    }
        
    //Queued connect so the change is already applied to the area when we start processing
    connect(area, SIGNAL(changingWorkingSet(Sublime::Area*,QString,QString)), this, SLOT(changingWorkingSet(Sublime::Area*,QString,QString)), Qt::QueuedConnection);
    
    m_connectedArea = area;
    
    changingWorkingSet(area, QString(), area->workingSet());
}

void WorkingSetWidget::changingWorkingSet(Sublime::Area*, QString, QString)
{
    workingSetsChanged();
}

QString htmlColorElement(int element) {
    QString ret = QString("%1").arg(element, 2, 16, QChar('0'));
    return ret;
}

QString htmlColor(QColor color) {
    return "#" + htmlColorElement(color.red()) + htmlColorElement(color.green()) + htmlColorElement(color.blue());
}

void WorkingSetWidget::workingSetsChanged()
{
    kDebug() << "re-creating widget";
    foreach(QToolButton* button, m_buttons.keys())
        delete button;
    m_buttons.clear();
    
    foreach(WorkingSet* set, Core::self()->workingSetControllerInternal()->allWorkingSets()) {
        
        disconnect(set, SIGNAL(setChangedSignificantly()), this, SLOT(workingSetsChanged()));
        connect(set, SIGNAL(setChangedSignificantly()), this, SLOT(workingSetsChanged()));
        
        if(m_mini && set->id() != m_connectedArea->workingSet())
            continue; //In "mini" mode, show only the current working set
        if(set->isEmpty())
            continue;
        QToolButton* butt = new QToolButton(this);
        butt->setIcon(KIcon(set->icon()));
        butt->setToolTip(i18n("Working Set %1", set->id()));
        
        QColor activeBgColor = palette().color(QPalette::Active, QPalette::Highlight);
        QColor normalBgColor = palette().color(QPalette::Active, QPalette::Base);
        QColor useColor;
        if(m_connectedArea && set->id() == m_connectedArea->workingSet())
            useColor = activeBgColor;
        else
            useColor = KColorUtils::mix(normalBgColor, activeBgColor, 0.3);
        
        QString sheet = QString("QToolButton { background : %1}").arg(htmlColor(useColor));
        butt->setStyleSheet(sheet);

        m_layout->addWidget(butt);
        connect(butt, SIGNAL(clicked(bool)), SLOT(buttonTriggered()));
        m_buttons[butt] = set;
    }
    
    update();
}

void WorkingSetWidget::buttonTriggered()
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    Q_ASSERT(button);
    Q_ASSERT(m_buttons.contains(button));
    
    if(m_mainWindow->area()->workingSet() == m_buttons[button]->id()) {
        //Create a new working-set
        m_mainWindow->area()->setWorkingSet(QString("%1_%2").arg(m_mainWindow->area()->objectName()).arg(qrand() % 10000000));
    }else{
        m_mainWindow->area()->setWorkingSet(m_buttons[button]->id());
    }
}

void WorkingSet::changingWorkingSet(Sublime::Area* area, QString from, QString to) {
    kDebug() << "changing working-set from" << from << "to" << to << ", local: " << m_id << "area" << area;
    Q_ASSERT(from == m_id);
    if (from == to)
        return;
    Q_ASSERT(m_areas.contains(area));
    disconnectArea(area);
    WorkingSet* newSet = Core::self()->workingSetControllerInternal()->getWorkingSet(to);
    newSet->connectArea(area);
    kDebug() << "update ready";
}

void WorkingSet::changedWorkingSet(Sublime::Area* area, QString from, QString to) {
    kDebug() << "changed working-set from" << from << "to" << to << ", local: " << m_id << "area" << area;
    Q_ASSERT(to == m_id);
    loadToArea(area, area->rootIndex(), true);
    kDebug() << "update ready";
}

#include "workingsetcontroller.moc"











