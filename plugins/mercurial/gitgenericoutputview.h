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

#ifndef GITGENERICOUTPUTVIEW_H
#define GITGENERICOUTPUTVIEW_H

#include <QWidget>
#include <KJob>

#include "ui_cvsgenericoutputview.h"

class GitPlugin;
class GitJob;

/**
 * Shows plain text.
 *
 * Text can either be added directly by calling appendText().
 *
 * Or by connecting a job's result() signal to slotJobFinished().
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class GitGenericOutputView : public QWidget, private Ui::CvsGenericOutputViewBase {
    Q_OBJECT
public:
    explicit GitGenericOutputView(GitPlugin *plugin, GitJob* job=0, QWidget* parent=0);
    virtual ~GitGenericOutputView();

public slots:
    void appendText(const QString& text);
    void slotJobFinished(KJob* job);

private:
    GitPlugin* m_plugin;
};

#endif
