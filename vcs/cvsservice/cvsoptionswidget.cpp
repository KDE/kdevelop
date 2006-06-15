/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   kdevelop-devel@kde.org                                                *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kdialog.h>

#include "domutil.h"
#include "cvsoptions.h"
#include "cvsoptionswidget.h"

///////////////////////////////////////////////////////////////////////////////
// class DiffDialog
///////////////////////////////////////////////////////////////////////////////

CvsOptionsWidget::CvsOptionsWidget( QWidget *parent, const char *name )
    : CvsOptionsWidgetBase( parent, name )
{
    readConfig();
}

///////////////////////////////////////////////////////////////////////////////

CvsOptionsWidget::~CvsOptionsWidget()
{
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::readConfig()
{
    CvsOptions *options = CvsOptions::instance();

    this->setCvsRshEnvVar( options->cvsRshEnvVar() );
    this->setServerLocation( options->location() );
    this->setPruneEmptyDirWhenUpdating( options->pruneEmptyDirsWhenUpdate() );
    this->setCreateNewDirWhenUpdating( options->createDirsWhenUpdate() );
    this->setRecursiveWhenUpdating( options->recursiveWhenUpdate() );
    this->setRecursiveWhenCommittingRemoving( options->recursiveWhenCommitRemove() );
    this->setDiffOptions( options->diffOptions() );
    this->setContextLines( options->contextLines() );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::storeConfig()
{
    CvsOptions *options = CvsOptions::instance();

    options->setCvsRshEnvVar( this->cvsRshEnvVar() );
    options->setLocation( this->serverLocation() );
    options->setPruneEmptyDirsWhenUpdate( this->pruneEmptyDirWhenUpdating() );
    options->setCreateDirsWhenUpdate( this->createNewDirWhenUpdating() );
    options->setRecursiveWhenUpdate( this->recursiveWhenUpdating() );
    options->setRecursiveWhenCommitRemove( this->recursiveWhenCommittingRemoving() );
    options->setDiffOptions( this->diffOptions() );
    options->setContextLines( this->contextLines() );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::accept() {
    storeConfig();
//    emit configChange();
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::setPruneEmptyDirWhenUpdating( bool b )
{
    this->pruneEmptyDirWhenUpdateCheck->setChecked( b );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::setCreateNewDirWhenUpdating( bool b )
{
    this->createNewDirWhenUpdateCheck->setChecked( b );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::setRecursiveWhenUpdating( bool b )
{
    this->recursiveWhenUpdateCheck->setChecked( b );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::setRecursiveWhenCommittingRemoving( bool b )
{
    this->recursiveWhenCommitRemoveCheck->setChecked( b );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::setContextLines( unsigned int p )
{
    this->contextLinesInput->setValue( p );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::setDiffOptions( const QString &p )
{
    this->diffOptionsEdit->setText( p );
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptionsWidget::diffOptions() const
{
    return this->diffOptionsEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::setCvsRshEnvVar( const QString &p )
{
    this->cvsRshEnvVarEdit->setText( p );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptionsWidget::setServerLocation( const QString &p )
{
    this->serverLocationEdit->setText( p );
}

///////////////////////////////////////////////////////////////////////////////

bool CvsOptionsWidget::pruneEmptyDirWhenUpdating() const
{
    return pruneEmptyDirWhenUpdateCheck->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

bool CvsOptionsWidget::createNewDirWhenUpdating() const
{
    return createNewDirWhenUpdateCheck->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

bool CvsOptionsWidget::recursiveWhenUpdating() const
{
    return recursiveWhenUpdateCheck->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

bool CvsOptionsWidget::recursiveWhenCommittingRemoving() const
{
    return recursiveWhenCommitRemoveCheck->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

unsigned int CvsOptionsWidget::contextLines() const
{
    return contextLinesInput->value();
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptionsWidget::cvsRshEnvVar() const
{
    return cvsRshEnvVarEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptionsWidget::serverLocation() const
{
    return serverLocationEdit->text();
}

#include "cvsoptionswidget.moc"
