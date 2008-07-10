/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef GITMAINVIEW_H
#define GITMAINVIEW_H

#include <QWidget>
#include <KJob>
#include <QToolButton>

#include "ui_cvsmainview.h"

class GitPlugin;
class GitGenericOutputView;

/**
 * This class is the main output view of KDevelop's Git plugin.
 * It only constists out of a KTabWidget.
 *
 * When created, a GitGenericOutputView will be inserted.
 *
 * Inserting text into that default output view is possible via the
 * slotJobFinished() slot.
 *
 * Additional tabs can be added via slotAddTab().
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class GitMainView : public QWidget, private Ui::CvsMainViewBase {
    Q_OBJECT
public:
    GitMainView(GitPlugin *plugin, QWidget* parent);
    virtual ~GitMainView();

public slots:
    /**
     * Inserts @p tag into the KTabWidget and calls it @p label .
     * This slot gets connected to GitPlugin::addNewTabToMainView().
     */
    void slotAddTab(QWidget* tab, const QString& label);

    /**
     * When this slot gets called, the output of the job will be written to
     * the default outputview of the KTabWidget.
     * This slot gets connected to GitPlugin::jobFinished().
     */
    void slotJobFinished(KJob* job);

    /**
     * Closes the current active tab (if it's not the first tab)
     */
    void slotTabClose();

private:
    GitPlugin* m_plugin;
    GitGenericOutputView* m_mainview;
    QToolButton* m_closeButton;
};

#endif
