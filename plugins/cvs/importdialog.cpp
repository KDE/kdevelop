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

#include <KLocalizedString>
#include <KMessageBox>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

#include "cvsplugin.h"
#include "cvsproxy.h"
#include "cvsjob.h"
#include "debug.h"

#include <vcs/vcslocation.h>

#include "importmetadatawidget.h"

ImportDialog::ImportDialog(CvsPlugin* plugin, const QUrl &url, QWidget *parent)
    : QDialog(parent), m_url(url), m_plugin(plugin)
{
    m_widget = new ImportMetadataWidget(this);
    m_widget->setSourceLocation( KDevelop::VcsLocation(m_url) );
    m_widget->setSourceLocationEditable( true );
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(m_widget);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ImportDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ImportDialog::reject);
}

ImportDialog::~ImportDialog()
{
}

void ImportDialog::accept()
{
    KDevelop::VcsJob *job = m_plugin->import(m_widget->message(), m_widget->source(),
                                                m_widget->destination());
    if (job) {
        connect(job, &KDevelop::VcsJob::result,
                this, &ImportDialog::jobFinished);
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
    auto* cvsjob = static_cast<CvsJob*>(job);

    static QRegExp re_file(QStringLiteral("^[IN]\\s(.*)"));
    bool error = false;
    const QStringList lines = cvsjob->output().split(QLatin1Char('\n'));
    for (const QString& line : lines) {
        if (line.isEmpty()) {
            // ignore empty lines
            continue;
        } else if (re_file.exactMatch(line)) {
            // line that tell us that a file has been added are OK
            continue;
            // this normaly should be the last line
        } else if (line.contains(QStringLiteral("No conflicts created by this import"))) {
            continue;
        } else {
            // any other line must mean that an error occurred
            qCDebug(PLUGIN_CVS) <<"ERR: "<< line;
            error = true;
        }
    }

    if (error) {
        KMessageBox::error(this,
            i18n("Some errors occurred while importing %1", m_url.toLocalFile()),
            i18n("Import Error"));
    } else {
        QDialog::accept();
    }
}

