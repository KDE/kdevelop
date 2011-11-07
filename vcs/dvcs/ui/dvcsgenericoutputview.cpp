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

#include "dvcsgenericoutputview.h"

#include "../dvcsplugin.h"
#include "../dvcsjob.h"

DVCSgenericOutputView::DVCSgenericOutputView(KDevelop::DistributedVersionControlPlugin *plugin, KDevelop::DVcsJob* job, QWidget* parent)
    : QWidget(parent), Ui::CvsGenericOutputViewBase(), m_plugin(plugin)
{
    Ui::CvsGenericOutputViewBase::setupUi(this);

    if (job) {
        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(slotJobFinished(KJob*)));
    }
}

DVCSgenericOutputView::~DVCSgenericOutputView()
{
}

void DVCSgenericOutputView::appendText(const QString& text)
{
    textArea->append(text);
}

void DVCSgenericOutputView::slotJobFinished(KJob * job)
{
    KDevelop::DVcsJob * gitjob = dynamic_cast<KDevelop::DVcsJob*>(job);
    if (gitjob) {
        appendText( gitjob->dvcsCommand().join(" ") );
        appendText( gitjob->output() );
        if (job->error() == 0) {
            appendText( i18n("Job exited normally") );
        } else {
            appendText( job->errorText() );
        }
    }
}
