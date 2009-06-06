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
#include <sublime/view.h>
#include <sublime/areaindex.h>
#include <sublime/document.h>
#include <interfaces/idocument.h>
#include "core.h"
#include "documentcontroller.h"
#include <sublime/area.h>
#include "mainwindow.h"

using namespace KDevelop;

WorkingSetController::WorkingSetController(Core* core) : m_core(core)
{
}

void WorkingSetController::cleanup()
{
    foreach(WorkingSet* set, m_workingSets)
        delete set;
}

WorkingSet* WorkingSetController::getWorkingSet(QString id)
{
    if(!m_workingSets.contains(id))
        m_workingSets[id] = new WorkingSet(id);
    
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
    }
}

void WorkingSet::loadToArea(Sublime::Area* area, Sublime::AreaIndex* areaIndex) {
    KConfigGroup setConfig(KGlobal::config(), "Working Sets");
    KConfigGroup group = setConfig.group(m_id);
    loadToArea(area, areaIndex, group);
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
    QLabel* ret = new QLabel("", parent); ///@todo Create working-set manager widget
    return ret;
}

#include "workingsetcontroller.moc"
