/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "importdialog.h"

#include <KMessageBox>
#include <KI18n/KLocalizedString>
#include <KDebug>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

#include "cvsplugin.h"
#include "cvsproxy.h"
#include "cvsjob.h"

#include <vcs/vcslocation.h>

#include "importmetadatawidget.h"

ImportDialog::ImportDialog(CvsPlugin* plugin, const KUrl& url, QWidget *parent)
    : KDialog(parent), m_url(url), m_plugin(plugin)
{
    m_widget = new ImportMetadataWidget(this);
    m_widget->setSourceLocation( KDevelop::VcsLocation(m_url) );
    m_widget->setSourceLocationEditable( true );
    setMainWidget(m_widget);
}

ImportDialog::~ImportDialog()
{
}

void ImportDialog::accept()
{
    KDevelop::VcsJob *job = m_plugin->import(m_widget->message(), m_widget->source(),
                                                m_widget->destination());
    if (job) {
        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(jobFinished(KJob*)));
        KDevelop::ICore::self()->runController()->registerJob(job);
    }
}

void ImportDialog::jobFinished(KJob * job)
{
    if (job->error()) {
        KMessageBox::error(this, i18n("Error on importing"), i18n("Import Error"));
        return;
    }

    // The job finished, now let's check the output is everything was OK
    CvsJob* cvsjob = dynamic_cast<CvsJob*>(job);

    static QRegExp re_file("^[IN]\\s(.*)");
    bool error = false;
    QStringList lines = cvsjob->output().split('\n');
    foreach(const QString &line, lines) {
        if (line.isEmpty()) {
            // ignore empty lines
            continue;
        } else if (re_file.exactMatch(line)) {
            // line that tell us that a file has been added are OK
            continue;
            // this normaly should be the last line
        } else if (line.contains("No conflicts created by this import")) {
            continue;
        } else {
            // any other line must mean that an error occurred
            kDebug(9500) <<"ERR: "<< line;
            error = true;
        }
    }

    if (error) {
        KMessageBox::error(this,
            i18n("Some errors occurred while importing %1", m_url.toLocalFile()),
            i18n("Import Error"));
    } else {
        KDialog::accept();
    }
}

