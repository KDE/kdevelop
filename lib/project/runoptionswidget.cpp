/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "runoptionswidget.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kdirselectdialog.h>
#include <urlutil.h>

#include <qlineedit.h>
#include <qlistview.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "domutil.h"
#include "environmentvariableswidget.h"


RunOptionsWidget::RunOptionsWidget(QDomDocument &dom, const QString &configGroup,
                                    const QString &buildDirectory, QWidget *parent, const char *name)
    : RunOptionsWidgetBase(parent, name),
      m_dom(dom), m_configGroup(configGroup)
{
    env_var_group->setColumnLayout( 1, Qt::Vertical );
    m_environmentVariablesWidget = new EnvironmentVariablesWidget( dom, configGroup + "/run/envvars", env_var_group );

    // Store the build directory in a KURL
    if (buildDirectory.right(1) == "/")
      m_buildDirectory = buildDirectory;
    else
      m_buildDirectory = buildDirectory + "/";
    m_buildDirectory.cleanPath();

    buildDirectory_label->setText(m_buildDirectory.directory(false, false));


    QString directoryRadioString = DomUtil::readEntry(dom, configGroup + "/run/directoryradio");
    if ( directoryRadioString == "build" )
        buildDirectory_radio->setChecked(true);
    else
        if ( directoryRadioString == "custom" )
            customDirectory_radio->setChecked(true);
        else
            executableDirectory_radio->setChecked(true);
    directoryRadioChanged();
    
    customRunDirectory_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/customdirectory"));
    mainprogram_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/mainprogram"));
    mainprogram_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/mainprogram"));
    progargs_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/programargs"));
    startinterminal_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/terminal"));
    autocompile_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/autocompile", true));
}


RunOptionsWidget::~RunOptionsWidget()
{}


void RunOptionsWidget::accept()
{
    if ( buildDirectory_radio->isChecked() )
      DomUtil::writeEntry(m_dom, m_configGroup + "/run/directoryradio", "build");
    else
      if ( customDirectory_radio->isChecked() )
        DomUtil::writeEntry(m_dom, m_configGroup + "/run/directoryradio", "custom");
      else
        DomUtil::writeEntry(m_dom, m_configGroup + "/run/directoryradio", "executable");
    
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/customdirectory", customRunDirectory_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/mainprogram", mainprogram_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/programargs", progargs_edit->text());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/terminal", startinterminal_box->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/autocompile", autocompile_box->isChecked());

    m_environmentVariablesWidget->accept();
}


void RunOptionsWidget::directoryRadioChanged()
{
    if ( customDirectory_radio->isChecked() ) {
        customRunDirectory_edit->setEnabled(true);
        browseCustomButton->setEnabled(true);
        mainProgram_relativeness_label->setText("( absolute path )");
        /// FIXME: Adjust mainprogram_edit text
        // mainprogram_edit->setText( absolute path to executable );
    } else {
        customRunDirectory_edit->setEnabled(false);
        browseCustomButton->setEnabled(false);
        mainProgram_relativeness_label->setText("( relative to BUILD directory )");
        /// FIXME: Adjust mainprogram_edit text
        // mainprogram_edit->setText( path to executable relative to BUILD directory );
    }  
}


void RunOptionsWidget::browseCustomDirectory()
{
    QString path = customRunDirectory_edit->text().stripWhiteSpace();
    KDirSelectDialog *dlg = new KDirSelectDialog(path, false, this, QString::null, true);
    dlg->setCaption(i18n("Select a directory"));

    if (dlg->exec()) {
    // if after the dialog execution the OK button was selected:
        path = dlg->url().path();
        if (path.right(1) != "/")
            path = path + "/";   // I find these lines dumb. Why the hell do I always have to add this???
                                 // In *nix whenever a path has a / in the end it is a directory, period!
                                 // Why does the url() return without this? Even if I add .directory(false, false)?
        if (!path.isEmpty()) {
            customRunDirectory_edit->setText(path);
        }
    }
    delete dlg;
}


void RunOptionsWidget::browseMainProgram()
{
    /// FIXME: Adjust mainprogram_edit text in the end of this function
    /// mainprogram_edit->setText( path to executable relative to BUILD directory );
    /// or
    /// mainprogram_edit->setText( absolute path to executable );
    /// depending on the current selected run directory
    
    QString start_directory;
    if ( customDirectory_radio->isChecked() )
        start_directory = mainprogram_edit->text().stripWhiteSpace();
    else
        start_directory = m_buildDirectory.directory(false, false);

    KFileDialog *dlg = new KFileDialog(start_directory, QString::null, this, 0, true);
    QStringList filters;
    filters << "application/x-executable"
    << "application/x-shellscript"
    << "application/x-perl"
    << "application/x-python";
    dlg->setMimeFilter(filters);
    dlg->setCaption(i18n("Select main program executable."));
    QString path = mainprogram_edit->text().stripWhiteSpace();
    if (!path.isEmpty()) {
        // strip initial "./" if necessary
        if ((path.length() > 2) && (path.left(2) == "./"))
            path = path.mid(2);

        // the directory where the executable is
        QString dir;
        int pos = path.findRev("/");
        if (path.left(1) != "/")
            dir = m_buildDirectory.directory(false, false) + path.left(pos);
        else
            dir = path.left(pos);

        // Store it all in a KURL
        KURL target = dir;
        target.addPath(path.mid(pos + 1));
        target.cleanPath();

        // pass it to the dialog
        dlg->setURL(target.directory(false, false));
        dlg->setSelection(target.filename());
    }

    if (dlg->exec()) {
    // if after the dialog execution the OK button was selected:
        path = dlg->selectedFile().stripWhiteSpace();
        if (!path.isEmpty()) {
            QString relative = URLUtil::relativePath(m_buildDirectory.directory(false, false), path, false);

            // if it's relative, uses it (if it's absolute don't touch it)
            if (relative.isEmpty() == false) {
                // add the necessary "./" to make sure it
                // executes the correct (local) file
                path = "./" + relative;
            }
            mainprogram_edit->setText(path);
        }
    }
    delete dlg;
}

#include "runoptionswidget.moc"
