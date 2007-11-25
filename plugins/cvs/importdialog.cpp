/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "importdialog.h"

#include <KMessageBox>
#include <KDebug>

#include "cvsplugin.h"
#include "cvsproxy.h"
#include "cvsjob.h"

ImportDialog::ImportDialog(CvsPlugin* plugin, const KUrl& url, QWidget *parent)
    : KDialog(parent), Ui::ImportDialogBase(), m_url(url), m_plugin(plugin)
{
    Ui::ImportDialogBase::setupUi(this);

    labelUrl->setText( url.path() );
}

ImportDialog::~ImportDialog()
{
}

void ImportDialog::accept()
{
    CvsJob *job = m_plugin->proxy()->import(m_url,
                            repository->text(),
                            module->text(),
                            vendorTag->text(),
                            releaseTag->text(),
                            comment->text());
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SLOT( jobFinished(KJob*) ));
        job->start();
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

    static QRegExp re_file("^N\\s(.*)");
    bool error = false;
    QStringList lines = cvsjob->output().split("\n");
    foreach(QString line, lines) {
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
            kDebug(9500) << line;
            error = true;
        }
    }

    if (error) {
        KMessageBox::error(this,
            i18n("Some errors occurred while importing") + m_url.path(),
            i18n("Import Error"));
    } else {
        KDialog::accept();
    }
}

#include "importdialog.moc"
