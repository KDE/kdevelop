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

#include "gitplugin.h"
#include "gitproxy.h"
#include "gitjob.h"

#include <vcs/vcsmapping.h>
#include <vcs/vcslocation.h>

#include "importmetadatawidget.h"

ImportDialog::ImportDialog(GitPlugin* plugin, const KUrl& url, QWidget *parent)
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
    GitJob *job = dynamic_cast<GitJob*>( m_plugin->init(m_widget->getRepositoryLocation()) );
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SLOT( jobFinished(KJob*) ));
        job->start();
    }
}

void ImportDialog::jobFinished(KJob * job)
{
    if (job->error()) {
        KMessageBox::error(this, i18n("Error on initialization"), i18n("Initialization Error"));
        return;
    }

    // The job finished, now let's check the output is everything was OK
    GitJob* gitjob = dynamic_cast<GitJob*>(job);

    static QRegExp re_file("^[IN]\\s(.*)");
    bool error = false;
    QStringList lines = gitjob->output().split("\n");
    foreach(QString line, lines) {
        if (line.isEmpty()) {
            // ignore empty lines
            continue;
        } else if (re_file.exactMatch(line)) {
            // line that tell us that a file has been added are OK
            continue;
            // this normaly should be the last line
        } else if (line.contains("No conflicts created by this initialization")) {
            continue;
        } else {
            // any other line must mean that an error occurred
            kDebug(9500) <<"ERR: "<< line;
            error = true;
        }
    }

    if (error) {
        KMessageBox::error(this,
            i18n("Some errors occurred while initializing") + m_url.path(),
            i18n("Initialization Error"));
    } else {
        KDialog::accept();
    }
}

#include "importdialog.moc"
