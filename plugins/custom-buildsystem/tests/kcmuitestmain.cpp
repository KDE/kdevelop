/*
    SPDX-FileCopyrightText: 2012 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#include <KLocalizedString>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QTemporaryDir>
#include <QDir>
#include <QPushButton>
#include <QStandardPaths>

#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <KConfig>

#include "custombuildsystemconfigwidget.h"
#include <debug.h>

static const char version[] = "0.1";

class State : public QObject
{
Q_OBJECT
public:
    explicit State(QDialogButtonBox* buttonBox, CustomBuildSystemConfigWidget* cfgWidget, KConfig* config)
        : buttonBox(buttonBox)
        , configWidget(cfgWidget)
        , cfg(config)
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
        configWidget->saveTo(cfg);
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
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KAboutData aboutData(QStringLiteral("kcm_uitest"), i18n("kcm_uitest"), version,
                         i18n("CustomBuildSystem Config Ui Test App"), KAboutLicense::GPL,
                         i18n("(C) 2012 Andreas Pakulat"));
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

    CustomBuildSystemConfigWidget widget(nullptr);
    widget.loadFrom(&projkcfg);
    mainLayout.addWidget(&widget);

    dlg.setWindowTitle(QStringLiteral("Ui Test App for Config Widget"));

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    buttonBox.button(QDialogButtonBox::Apply)->setEnabled(false);
    buttonBox.button(QDialogButtonBox::Ok)->setEnabled(false);

    const State state(&buttonBox, &widget, &projkcfg);

    dlg.resize(800, 600);

    dlg.show();

    return app.exec();

}

#include "kcmuitestmain.moc"
