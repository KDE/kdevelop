#include <KApplication>
#include <KCmdLineArgs>
#include <QDebug>
#include <KAboutData>
#include <QStringList>

#include "../../../plugins/git/gitexecutor.h"
#include "../../vcsrevision.h"

#include "commitlogmodel.h"
#include "commitView.h"


int main(int argc, char* argv[])
{
    KAboutData aboutData(
                         // The program name used internally.
                         "test",
                         // The message catalog name
                         // If null, program name is used instead.
                         0,
                         // A displayable program name string.
                         ki18n("Test 1"),
                         // The program version string.
                               "1.0",
                         // Short description of what the app does.
                               ki18n("Displays a KMessageBox popup"),
                         // The license this code is released under
                                     KAboutData::License_GPL,
                         // Copyright Statement
                                     ki18n("(c) 2007"),
                         // Optional text shown in the About box.
                         // Can contain any information desired.
                                           ki18n("Some text..."),
                         // The program homepage string.
                                                   "http://kdevelop.org/",
                         // The bug report email address
                                                        "submit@bugs.kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;

    QString repo("/home/kde-devel/kdevelop-projects/git/");
//     QString repo("/workspace/gsoc/kdevplatform/");
    GitExecutor* gitter = new GitExecutor(0);


//     QList<KDevelop::VcsRevision> revs;
//     KDevelop::VcsRevision rev1;
//     rev1.setValue(KDevelop::VcsRevision::Short_log, QString("It's a test commit"));
//     rev1.setValue(KDevelop::VcsRevision::Author, QString("E. Ivanov<qwe@foo.bar>"));
//     rev1.setValue(KDevelop::VcsRevision::DateColumn, QString("2020"));
//     KDevelop::VcsRevision rev2;
//     rev2.setValue(KDevelop::VcsRevision::Short_log, QString("It's a test commit 2"));
//     rev2.setValue(KDevelop::VcsRevision::Author, QString("E. Ivanov<foo@foo.bar>"));
//     rev2.setValue(KDevelop::VcsRevision::DateColumn, QString("2021"));
// 
//     QStringList revsL;
//     QStringList revsL2;
//     revsL  << "gr" <<"It's a test commit" << "E. Ivanov<qwe@foo.bar>" << "2020";
//     revsL2 << "graph" <<"It's a test commit 2" << "E. Ivanov<foo@foo.bar>" << "2021";

//     revs.append(rev1);
//     revs.append(rev2);
//     revs.append(revsL);

    CommitLogModel* model = new CommitLogModel(gitter->getAllCommits(repo));

    CommitView *revTree = new CommitView;
    revTree->setModel(model);

    revTree->show();

    return app.exec();
}
