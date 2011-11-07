/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
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

#include "importdialog.h"

#include <KMessageBox>
#include <KDebug>

#include "../dvcsplugin.h"
#include "../dvcsjob.h"
#include "importmetadatawidget.h"

#include <vcslocation.h>

using KDevelop::DistributedVersionControlPlugin;

ImportDialog::ImportDialog(DistributedVersionControlPlugin* plugin, const KUrl& url, QWidget *parent)
    : KDialog(parent), m_url(url), m_plugin(plugin)
{
    m_widget = new ImportMetadataWidget(this);
    m_widget->setSourceLocation( KDevelop::VcsLocation(m_url) );
    m_widget->setSourceLocationEditable( true );
    setMainWidget(m_widget);
}

ImportDialog::~ImportDialog()
{
}

void ImportDialog::accept()
{
    KDevelop::VcsJob *job = m_plugin->init(m_widget->source());
    if (job) {
        connect(job, SIGNAL(result(KJob*)),
                m_plugin, SLOT(jobFinished(KJob*)));
        job->start();
    }
    KDialog::accept();
}

#include "importdialog.moc"
