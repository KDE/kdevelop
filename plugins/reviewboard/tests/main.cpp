/*************************************************************************************
 *  Copyright (C) 2010 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include <KApplication>
#include <k4aboutdata.h>
#include <KCmdLineArgs>
#include <KMessageBox>
#include <QDebug>
#include "reviewpatchdialog.h"
#include "reviewboardjobs.h"

int main(int argc, char *argv[])
{
    K4AboutData about("reviewboardtest", 0, ki18n(("ReviewBoard Test")), "0.10", ki18n("Test ReviewBoard support"),
            K4AboutData::License_GPL, ki18n("(C) 2010 Aleix Pol Gonzalez"));
    about.addAuthor( ki18n("Aleix Pol Gonzalez"), KLocalizedString(), "aleixpol@kde.org" );

    KCmdLineArgs::init( argc, argv, &about );
    KCmdLineOptions options;
    options.add("+patch", ki18n( "Patch" ));
    options.add("basedir <dir>", ki18n( "Base Directory" ));
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    ReviewPatchDialog d;

    KUrl patch(args->arg(0));
    QString basedir=args->getOption("basedir");

    qDebug() << "patch:" << patch << ", basedir:" << basedir;
    d.setServer(KUrl("https://git.reviewboard.kde.org"));
    d.setBaseDir(basedir);
    d.setRepository("kdevplatform");
    int ret=d.exec();
    if(ret==QDialog::Accepted) {
        KUrl url=d.server();
        ReviewBoard::NewRequest* job=new ReviewBoard::NewRequest(d.server(), d.repository());
        bool corr = job->exec();
        if(corr) {
            url.setUserInfo(QString());
            QString requrl = QString("%1/r/%2/").arg(url.prettyUrl()).arg(job->requestId());

            KMessageBox::information(0, i18n("<qt>You can find the new request at:<br /><a href='%1'>%1</a> </qt>", requrl));
        } else {
            KMessageBox::error(0, job->errorText());
        }
    }

    return ret!=QDialog::Accepted;
}
