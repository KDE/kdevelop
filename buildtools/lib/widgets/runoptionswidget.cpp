/* This file is part of the KDE project
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "runoptionswidget.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kdirselectdialog.h>
#include <klineedit.h>
#include <urlutil.h>

#include <qlistview.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>

#include "domutil.h"
#include "environmentvariableswidget.h"


RunOptionsWidget::RunOptionsWidget(QDomDocument &dom, const QString &configGroup,
                                    const QString &buildDirectory, QWidget *parent, const char *name)
    : RunOptionsWidgetBase(parent, name),
      m_dom(dom), m_configGroup(configGroup)
{
    // Create the "Environment Variables" GUI
    env_var_group->setColumnLayout( 1, Qt::Vertical );
    m_environmentVariablesWidget = new EnvironmentVariablesWidget( dom, configGroup + "/run/envvars", env_var_group );

    // Store the BUILD directory in a KURL
    if (buildDirectory.right(1) == "/")
        m_buildDirectory = buildDirectory;
    else
        m_buildDirectory = buildDirectory + "/";
    m_buildDirectory.cleanPath();

    // Display the BUILD directory in a label
    buildDirectory_label->setText(m_buildDirectory.directory(false, false));

    // Update the "Run Directory" radio buttons
    // "Directory where the executable is" is set by default
    QString directoryRadioString = DomUtil::readEntry(dom, configGroup + "/run/directoryradio");
    if ( directoryRadioString == "build" )
        buildDirectory_radio->setChecked(true);
    else
        if ( directoryRadioString == "custom" )
            customDirectory_radio->setChecked(true);
        else
            executableDirectory_radio->setChecked(true);
    directoryRadioChanged();

    // Read the custom directory, store it in a KURL and update it's edit box
    QString customRunDirectory = DomUtil::readEntry(dom, configGroup + "/run/customdirectory");
    if (customRunDirectory.right(1) == "/")
        m_customRunDirectory = customRunDirectory;
    else
        m_customRunDirectory = customRunDirectory + "/";
    m_customRunDirectory.cleanPath();
    customRunDirectory_edit->setText(m_customRunDirectory.directory(false, false));

    // Read the main program path, store it in a KURL and update it's edit box
    QString mainProgramPath = DomUtil::readEntry(dom, configGroup + "/run/mainprogram");
    if ( customDirectory_radio->isChecked() )
        m_mainProgramAbsolutePath = mainProgramPath;
    else
        m_mainProgramAbsolutePath = m_buildDirectory.directory(false, false) + mainProgramPath;
    m_mainProgramAbsolutePath.cleanPath();
    if ( customDirectory_radio->isChecked() )
        mainprogram_edit->setText(m_mainProgramAbsolutePath.path());
    else
        mainprogram_edit->setText(URLUtil::relativePath(m_buildDirectory.directory(false, false), m_mainProgramAbsolutePath.path(), false));

    if( configGroup == "/kdevautoproject" || configGroup == "/kdevtrollproject" )
    {
        directoryButtonGroup->setCheckable(true);
        directoryButtonGroup->setChecked( DomUtil::readBoolEntry(dom, configGroup+"/run/useglobalprogram", false ) );
    }else
    {
        delete notelabel;
    }

    // Read the main program command line arguments and store them in the edit box
    progargs_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/programargs"));

    startinterminal_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/terminal"));
    autocompile_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/autocompile", true));
    autoinstall_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/autoinstall", false));
    autokdesu_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/autokdsu", false));
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

    QString customDir = customRunDirectory_edit->text();
    if (customDir.right(1) != "/")
        customDir += "/";
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/customdirectory", customDir);
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/mainprogram", mainprogram_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/programargs", progargs_edit->text());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/useglobalprogram", directoryButtonGroup->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/terminal", startinterminal_box->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/autocompile", autocompile_box->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/autoinstall", autoinstall_box->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/autokdesu", autokdesu_box->isChecked());

    m_environmentVariablesWidget->accept();
}


void RunOptionsWidget::directoryRadioChanged()
{
    if ( customDirectory_radio->isChecked() ) {
        customRunDirectory_edit->setEnabled(true);
        browseCustomButton->setEnabled(true);
        mainProgram_relativeness_label->setText("( absolute path )");
        mainprogram_edit->setText( m_mainProgramAbsolutePath.path() );
    } else {
        customRunDirectory_edit->setEnabled(false);
        browseCustomButton->setEnabled(false);
        mainProgram_relativeness_label->setText("( relative to BUILD directory )");
        mainprogram_edit->setText( URLUtil::relativePath(m_buildDirectory.directory(false, false), m_mainProgramAbsolutePath.path(), false) );
    }
}


void RunOptionsWidget::browseCustomDirectory()
{
    QString path = customRunDirectory_edit->text().stripWhiteSpace();
    KDirSelectDialog *dlg = new KDirSelectDialog(path, false, this, 0L, true);
    dlg->setCaption(i18n("Select Directory"));

    if (dlg->exec()) {
    // if after the dialog execution the OK button was selected:
        path = dlg->url().path();
        if (path.right(1) != "/")
            path += "/";   // I find these lines dumb. Why the hell do I always have to add this???
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
    dlg->setCaption(i18n("Select Main Program Executable"));
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
        KURL target ( dir );
        target.addPath(path.mid(pos + 1));
        target.cleanPath();

        // pass it to the dialog
        dlg->setURL(KURL::fromPathOrURL( target.directory(false, false) ));
        dlg->setSelection(target.fileName());
    }

    if (dlg->exec()) {
    // if after the dialog execution the OK button was selected:
        path = dlg->selectedFile().stripWhiteSpace();
        if (!path.isEmpty()) {

            m_mainProgramAbsolutePath = path;

            if ( customDirectory_radio->isChecked() ) {
            // Store the absolute path

                mainprogram_edit->setText(path);

            } else {
            // Store the path relative to BUILD directory

                QString relative = URLUtil::relativePath(m_buildDirectory.directory(false, false), path, false);

                if (relative.isEmpty() == false) {
                    mainprogram_edit->setText(relative);
                }
            }

        }
    }
    delete dlg;
}

void RunOptionsWidget::mainProgramChanged( const QString& newtext )
{

    if( directoryButtonGroup->isChecked() && newtext.isEmpty() )
    {
        mainprogram_label->setPaletteForegroundColor(QColor("#ff0000"));
    }
    else
    {
        mainprogram_label->unsetPalette();
    }
}

#include "runoptionswidget.moc"
