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

#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtWidgets/QApplication>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageBox>

#include "reviewpatchdialog.h"
#include "reviewboardjobs.h"

int main(int argc, char *argv[])
{
    KAboutData about("reviewboardtest", i18n(("ReviewBoard Test")), "0.10", i18n("Test ReviewBoard support"),
        KAboutLicense::GPL, i18n("(C) 2010 Aleix Pol Gonzalez"));
    about.addAuthor( i18n("Aleix Pol Gonzalez"), QString(), "aleixpol@kde.org" );

    QApplication app(argc, argv);
    KAboutData::setApplicationData(about);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addPositionalArgument("patch", i18n( "Patch" ));
    parser.addOption(QCommandLineOption(QLatin1String("basedir <dir>"), i18n( "Base Directory" )));
    parser.addOption(QCommandLineOption(QLatin1String("id <id>"), i18n( "Review request ID" )));
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    ReviewPatchDialog d(QDir::currentPath());
    int ret=d.exec();
    if(ret==QDialog::Accepted) {
        KUrl url=d.server();
        ReviewBoard::ReviewRequest* job;
        if (parser.positionalArguments().count() == 0) {
            job = new ReviewBoard::NewRequest(d.server(), d.repository());
        } else {
            QUrl patch = QUrl::fromUserInput(parser.positionalArguments()[0]);
            QString basedir=parser.value("basedir");
            QString id=parser.value("id");
            qDebug() << "patch:" << patch << ", basedir:" << basedir;
            job = new ReviewBoard::SubmitPatchRequest(d.server(), patch, basedir, id);
        }
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
