/***************************************************************************
   Copyright 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "localpatchsource.h"

#include <QIcon>

#include <ktemporaryfile.h>
#include <kdebug.h>
#include <KLocalizedString>
#include <kprocess.h>
#include <KLineEdit>
#include <kshell.h>
#include <QWidget>

#include "ui_localpatchwidget.h"

LocalPatchSource::LocalPatchSource()
    : m_applied(false)
    , m_depth(0)
    , m_widget(new LocalPatchWidget(this, 0))
{
}

LocalPatchSource::~LocalPatchSource()
{
    if ( !m_command.isEmpty() && !m_filename.isEmpty() ) {
        QFile::remove( m_filename.toLocalFile() );
    }
}

QString LocalPatchSource::name() const
{
    return i18n( "Custom Patch" );
}

QIcon LocalPatchSource::icon() const
{
    return QIcon::fromTheme("text-x-patch");
}

void LocalPatchSource::update()
{
    if( !m_command.isEmpty() ) {
        KTemporaryFile temp;
        temp.setSuffix( ".diff" );
        if( temp.open() ) {
            temp.setAutoRemove( false );
            QString filename = temp.fileName();
            kDebug() << "temp file: " << filename;
            temp.close();
            KProcess proc;
            proc.setWorkingDirectory( m_baseDir.toLocalFile() );
            proc.setOutputChannelMode( KProcess::OnlyStdoutChannel );
            proc.setStandardOutputFile( filename );
            ///Try to apply, if it works, the patch is not applied
            proc << KShell::splitArgs( m_command );

            kDebug() << "calling " << m_command;

            if ( proc.execute() ) {
                kWarning() << "returned with bad exit code";
                return;
            }
            if ( !m_filename.isEmpty() ) {
                QFile::remove( m_filename.toLocalFile() );
            }
            m_filename = KUrl::fromLocalFile( filename );
            kDebug() << "success, diff: " << m_filename;
        }else{
            kWarning() << "PROBLEM";
        }
        emit patchChanged();
    }
}

QWidget* LocalPatchSource::customWidget() const
{
    return m_widget;
}

LocalPatchWidget::LocalPatchWidget(LocalPatchSource* lpatch, QWidget* parent)
    : QWidget(parent)
    , m_lpatch(lpatch)
    , m_ui(new Ui::LocalPatchWidget)
{
    m_ui->setupUi(this);
    connect( m_ui->applied, SIGNAL( stateChanged( int ) ), SLOT( updatePatchFromEdit() ) );
    connect( m_ui->filename, SIGNAL( textChanged( QString ) ), SLOT( updatePatchFromEdit() ) );

    m_ui->baseDir->setMode( KFile::Directory );

    connect( m_ui->command, SIGNAL( textChanged( QString ) ), this, SLOT( updatePatchFromEdit() ) );
    //   connect( commandToFile, SIGNAL(clicked(bool)), this, SLOT(slotToFile()) );

    connect( m_ui->filename->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( updatePatchFromEdit() ) );
    connect( m_ui->filename->lineEdit(), SIGNAL( editingFinished() ), this, SLOT( updatePatchFromEdit() ) );
    connect( m_ui->filename, SIGNAL( urlSelected( QUrl ) ), this, SLOT( updatePatchFromEdit() ) );
    connect( m_ui->command, SIGNAL( textChanged( QString ) ), this, SLOT( updatePatchFromEdit() ) );
    //     connect( commandToFile, SIGNAL(clicked(bool)), m_plugin, SLOT(commandToFile()) );

    connect(m_lpatch, SIGNAL(patchChanged()), SLOT(syncPatch()));
}

void LocalPatchWidget::syncPatch()
{
    m_ui->command->setText( m_lpatch->command());
    m_ui->filename->setUrl( m_lpatch->file() );
    m_ui->baseDir->setUrl( m_lpatch->baseDir() );
    m_ui->applied->setCheckState( m_lpatch->isAlreadyApplied() ? Qt::Checked : Qt::Unchecked );

    if ( m_lpatch->command().isEmpty() )
        m_ui->tabWidget->setCurrentIndex( m_ui->tabWidget->indexOf( m_ui->fileTab ) );
    else
        m_ui->tabWidget->setCurrentIndex( m_ui->tabWidget->indexOf( m_ui->commandTab ) );
}

void LocalPatchWidget::updatePatchFromEdit()
{
    m_lpatch->setCommand(m_ui->command->text());
    m_lpatch->setFilename(m_ui->filename->url());
    m_lpatch->setBaseDir(m_ui->baseDir->url());
    m_lpatch->setAlreadyApplied(m_ui->applied->checkState() == Qt::Checked );

    emit m_lpatch->patchChanged();
}
