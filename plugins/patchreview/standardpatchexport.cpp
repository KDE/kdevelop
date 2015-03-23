/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "standardpatchexport.h"

#include <QFileDialog>
#include <QIcon>
#include <QMenu>
#include <QStandardPaths>

#include <KIO/CopyJob>
#include <KLocalizedString>
#include <KProcess>
#include <KToolInvocation>

#include <interfaces/icore.h>
#include <interfaces/ipatchexporter.h>
#include <interfaces/iruncontroller.h>
#include "patchreview.h"

class StandardExporter : public KDevelop::IPatchExporter
{
public:
    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;
};

class KIOExport : public StandardExporter
{
    virtual void exportPatch( KDevelop::IPatchSource::Ptr source ) override {
        QUrl dest = QFileDialog::getSaveFileUrl();
        if( !dest.isEmpty() ) { //We let KDE do the rest of the job including the notification
            KIO::CopyJob* job = KIO::copy( source->file(), dest );
            KIO::getJobTracker()->registerJob( job );
        }
    }

    virtual QIcon icon() const override { return QIcon::fromTheme( "document-save" ); }
    virtual QString name() const override { return i18n( "Save As..." ); }
};

class EMailExport : public StandardExporter
{
    virtual void exportPatch( KDevelop::IPatchSource::Ptr source ) override {
        KToolInvocation::invokeMailer( QString(), QString(), QString(), QString(), QString(), QString(), QStringList() << source->file().toLocalFile() );
    }

    virtual QIcon icon() const override { return QIcon::fromTheme( "internet-mail" ); }
    virtual QString name() const override { return i18n( "Send..." ); }
};

class TelepathyExport : public StandardExporter
{
public:
    virtual void exportPatch( KDevelop::IPatchSource::Ptr source ) override {
        KProcess::startDetached( QStringList() << "ktp-send-file" << source->file().toDisplayString(QUrl::PreferLocalFile) );
    }

    static bool isAvailable() { return !QStandardPaths::findExecutable( "ktp-send-file" ).isEmpty(); }
    virtual QIcon icon() const override { return QIcon::fromTheme( "telepathy-kde" ); }
    virtual QString name() const override { return i18n( "Send to contact..." ); }
};

class KompareExport : public StandardExporter
{
public:
    KompareExport() {}

    virtual void exportPatch( KDevelop::IPatchSource::Ptr source ) override {
        KProcess::startDetached( QStringList("kompare") << source->baseDir().toDisplayString(QUrl::PreferLocalFile) << source->file().toDisplayString(QUrl::PreferLocalFile) );
    }

    static bool isAvailable() { return !QStandardPaths::findExecutable( "kompare" ).isEmpty(); }
    virtual QIcon icon() const override { return QIcon::fromTheme( "kompare" ); }
    virtual QString name() const override { return i18n( "Side view (Kompare)..." ); }

};

StandardPatchExport::StandardPatchExport( PatchReviewPlugin* plugin, QObject* parent )
    : QObject( parent )
    , m_plugin( plugin )
{
    m_exporters.append( new KIOExport );
    m_exporters.append( new EMailExport );

    if( KompareExport::isAvailable() ) {
        m_exporters.append( new KompareExport );
    }
    if( TelepathyExport::isAvailable() ) {
        m_exporters.append( new TelepathyExport );
    }
}

StandardPatchExport::~StandardPatchExport() {
    qDeleteAll( m_exporters );
}

Q_DECLARE_METATYPE( StandardExporter* );

void StandardPatchExport::addActions( QMenu* m ) {
    foreach( StandardExporter* exp, m_exporters ) {
        QAction* act = m->addAction( exp->icon(), exp->name(), this, SLOT( runExport() ) );
        act->setData( qVariantFromValue( exp ) );
    }
}

void StandardPatchExport::runExport() {
    QAction* act = qobject_cast< QAction* >( sender() );
    StandardExporter* exp = act->data().value< StandardExporter* >();
    exp->exportPatch( m_plugin->patch() );
}
