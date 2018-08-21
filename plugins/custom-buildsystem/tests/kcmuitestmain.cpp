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
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QTemporaryDir>
#include <QDir>
#include <QPushButton>
#include <QStandardPaths>

#include <tests/testproject.h>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <KConfig>

#include "custombuildsystemconfigwidget.h"
#include <debug.h>

static const char description[] =
    I18N_NOOP("CustomBuildSystem Config Ui Test App");

static const char version[] = "0.1";

class State : public QObject
{
Q_OBJECT
public:
    State( QDialogButtonBox* buttonBox, CustomBuildSystemConfigWidget* cfgWidget, KConfig* config, KDevelop::IProject* proj )
        : buttonBox(buttonBox), configWidget(cfgWidget), cfg(config), project(proj)
    {
        connect(buttonBox, &QDialogButtonBox::clicked, this, &State::buttonClicked);
        connect(configWidget, &CustomBuildSystemConfigWidget::changed, this, &State::configChanged);
    }
public Q_SLOTS:
    void buttonClicked(QAbstractButton* button)
    {
        if (button == buttonBox->button(QDialogButtonBox::Apply)) {
            apply();
        } else if (button == buttonBox->button(QDialogButtonBox::Ok)) {
            ok();
        } else if (button == buttonBox->button(QDialogButtonBox::Cancel)) {
            qApp->quit();
        }
    }

    void apply() {
        configWidget->saveTo(cfg, project);
        buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    void ok() {
        apply();
        qApp->quit();
    }
    void configChanged() {
        buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
private:
    QDialogButtonBox* buttonBox;
    CustomBuildSystemConfigWidget* configWidget;
    KConfig* cfg;
    KDevelop::IProject* project;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KAboutData aboutData(QStringLiteral("kcm_uitest"), i18n("kcm_uitest"), version, i18n(description),
                     KAboutLicense::GPL, i18n("(C) 2012 Andreas Pakulat"));
    aboutData.addAuthor( i18n("Andreas Pakulat"), QString(), QStringLiteral("apaku@gmx.de") );
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    QTemporaryDir tempdir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/kdev-custom-uitest");

    qCDebug(CUSTOMBUILDSYSTEM) << "created tempdir:" << tempdir.path();

    KConfig projkcfg( tempdir.path() + "/kdev-custom-uitest.kdev4" );

    QDir projdir(tempdir.path());
    projdir.mkdir(QStringLiteral("includedir"));
    projdir.mkdir(QStringLiteral("subtree"));
    projdir.mkpath(QStringLiteral("subtree/includedir"));
    projdir.mkpath(QStringLiteral("subtree/deeptree"));
    projdir.mkpath(QStringLiteral("subtree/deeptree/includedir"));

    qCDebug(CUSTOMBUILDSYSTEM) << "project config:" << projkcfg.name();

    QDialog dlg;

    QVBoxLayout mainLayout;
    dlg.setLayout(&mainLayout);

    KDevelop::TestProject proj;
    proj.setPath( KDevelop::Path(projkcfg.name()));

    CustomBuildSystemConfigWidget widget(nullptr);
    widget.loadFrom(&projkcfg);
    mainLayout.addWidget(&widget);

    dlg.setWindowTitle(QStringLiteral("Ui Test App for Config Widget"));

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    buttonBox.button(QDialogButtonBox::Apply)->setEnabled(false);
    buttonBox.button(QDialogButtonBox::Ok)->setEnabled(false);

    State state(&buttonBox, &widget, &projkcfg, &proj );

    dlg.resize(800, 600);

    dlg.show();

    return app.exec();

}

#include "kcmuitestmain.moc"
