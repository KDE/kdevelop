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
#include <urlutil.h>

#include <qlineedit.h>
#include <qlistview.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

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

    mainprogram_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/mainprogram"));
    progargs_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/programargs"));
    startinterminal_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/terminal"));
    autocompile_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/autocompile", true));
}


RunOptionsWidget::~RunOptionsWidget()
{}


void RunOptionsWidget::accept()
{
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/mainprogram", mainprogram_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/programargs", progargs_edit->text());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/terminal", startinterminal_box->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/autocompile", autocompile_box->isChecked());

    m_environmentVariablesWidget->accept();
}


void RunOptionsWidget::browseMainProgram()
{
    KFileDialog *dlg = new KFileDialog(m_buildDirectory.directory(false, false), QString::null, this, 0, true);
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
