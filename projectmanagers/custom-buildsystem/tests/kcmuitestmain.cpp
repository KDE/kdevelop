/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2012 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include <KLocalizedString>
#include <kdialog.h>
#include <QTemporaryDir>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <QPushButton>
#include <QStandardPaths>

#include <tests/testproject.h>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>

#include "custombuildsystemconfigwidget.h"
#include "../debug.h"

static const char description[] =
    I18N_NOOP("CustomBuildSystem Config Ui Test App");

static const char version[] = "0.1";

class State : public QObject
{
Q_OBJECT
public:
    State( KDialog* dlg, CustomBuildSystemConfigWidget* cfgWidget, KConfig* config, KDevelop::IProject* proj )
        : dialog(dlg), configWidget(cfgWidget), cfg(config), project(proj)
    {
        connect(dlg, SIGNAL(applyClicked()), SLOT(apply()));
        connect(dlg, SIGNAL(okClicked()), SLOT(ok()));
        connect(dlg, SIGNAL(cancelClicked()), qApp, SLOT(quit()));
        connect(configWidget, SIGNAL(changed()), SLOT(configChanged()));
    }
public slots:
    void apply() {
        configWidget->saveTo(cfg, project);
        dialog->button( KDialog::Apply )->setEnabled(false);
        dialog->button( KDialog::Ok )->setEnabled(false);
    }
    void ok() {
        apply();
        qApp->quit();
    }
    void configChanged() {
        dialog->button( KDialog::Apply )->setEnabled(true);
        dialog->button( KDialog::Ok )->setEnabled(true);
    }
private:
    KDialog* dialog;
    CustomBuildSystemConfigWidget* configWidget;
    KConfig* cfg;
    KDevelop::IProject* project;
};

int main(int argc, char **argv)
{
    KAboutData aboutData("kcm_uitest", i18n("kcm_uitest"), version, i18n(description),
                     KAboutLicense::GPL, i18n("(C) 2012 Andreas Pakulat"));
    aboutData.addAuthor( i18n("Andreas Pakulat"), QString(), "apaku@gmx.de" );
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QTemporaryDir tempdir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/kdev-custom-uitest");

    qCDebug(CUSTOMBUILDSYSTEM) << "created tempdir:" << tempdir.path();

    KConfig projkcfg( tempdir.path() + "/kdev-custom-uitest.kdev4" );

    QDir projdir(tempdir.path());
    projdir.mkdir("includedir");
    projdir.mkdir("subtree");
    projdir.mkpath("subtree/includedir");
    projdir.mkpath("subtree/deeptree");
    projdir.mkpath("subtree/deeptree/includedir");

    qCDebug(CUSTOMBUILDSYSTEM) << "project config:" << projkcfg.name();

    KDialog dlg;
    dlg.setButtons( KDialog::Ok | KDialog::Apply | KDialog::Cancel );

    KDevelop::TestProject proj;
    proj.setPath( KDevelop::Path(projkcfg.name()));

    CustomBuildSystemConfigWidget widget(nullptr);
    widget.loadFrom(&projkcfg);
    dlg.setMainWidget(&widget);

    dlg.setCaption("Ui Test App for Config Widget");

    dlg.button( KDialog::Apply )->setEnabled(false);
    dlg.button( KDialog::Ok )->setEnabled(false);

    State state(&dlg, &widget, &projkcfg, &proj );

    dlg.resize(800, 600);

    dlg.show();

    return app.exec();

}

#include "kcmuitestmain.moc"
