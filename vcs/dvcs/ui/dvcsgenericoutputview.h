/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
  *                                                                        *
 *   Adapted for DVCS                                                      *
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

#ifndef DVCS_GENERIC_OUTPUTVIEW_H
#define DVCS_GENERIC_OUTPUTVIEW_H

#include <QWidget>
#include <KJob>

#include "ui_cvsgenericoutputview.h"

namespace KDevelop
{
    class DVcsJob;
    class DistributedVersionControlPlugin;
}

/**
 * Shows plain text.
 *
 * Text can either be added directly by calling appendText().
 *
 * Or by connecting a job's result() signal to slotJobFinished().
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class DVCSgenericOutputView : public QWidget, private Ui::CvsGenericOutputViewBase {
    Q_OBJECT
public:
    explicit DVCSgenericOutputView(KDevelop::DistributedVersionControlPlugin *plugin, KDevelop::DVcsJob* job=0, QWidget* parent=0);
    virtual ~DVCSgenericOutputView();

public slots:
    void appendText(const QString& text);
    void slotJobFinished(KJob* job);

private:
    KDevelop::DistributedVersionControlPlugin* m_plugin;
};

#endif
