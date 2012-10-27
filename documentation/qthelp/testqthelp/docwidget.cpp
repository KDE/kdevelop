/*  This file is part of KDevelop
    Copyright 2010 Andreas Pakulat <apaku@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this program; if not, write to the
    Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "docwidget.h"

#include "ui_docwidget.h"
#include <qlibrary.h>
#include <qlibraryinfo.h>
#include <qhelpengine.h>
#include <qdir.h>


DocWidget::DocWidget()
    : QWidget( 0 ), ui( new Ui::DocWidget )
{
    ui->setupUi( this );

    QString helpcoll = QDir::homePath() + QLatin1String( "/.testqthelp.qhc" );
    ui->debug->append( QString("Setting up QtHelp collection at: %1").arg( helpcoll ) );

    engine = new QHelpEngine( helpcoll, this );
    if( !engine->setupData() ) {
        ui->debug->append( QString( "Could not set up help collection: %1" ).arg( engine->error() ) );
    }

    QString docLoc = QLibraryInfo::location( QLibraryInfo::DocumentationPath );
    ui->debug->append( QString("Found Qt Docs in: %1" ).arg( docLoc ) );
    QDir d( docLoc+QLatin1String("/qch") );
    foreach( const QString& file, d.entryList( QStringList() << "*.qch", QDir::Files ) ) {
        QString absfile = d.filePath( file );
        QString ns = QHelpEngine::namespaceName( absfile );
        if( !engine->registeredDocumentations().contains( ns ) ) {
            ui->debug->append( QString("Loading qch file: %1, ns: %2").arg( absfile, ns ) );
            if( !engine->registerDocumentation( absfile )  ) {
                ui->debug->append( QString("ERROR loading %1: %2").arg( absfile, engine->error() ) );
            }
        } else {
            ui->debug->append( QString("Doc already loaded: %1" ).arg( absfile ) );
        }
    }
    connect( ui->search, SIGNAL(returnPressed()), SLOT(searchForIdentifier()) );
}

DocWidget::~DocWidget()
{
    delete ui;
}

void DocWidget::searchForIdentifier()
{
    QMap<QString,QUrl> links = engine->linksForIdentifier( ui->search->text() );
    QStringList dbg;
    foreach( const QString& k, links.keys() ) {
        dbg << QString("%1=%2").arg( k, links[k].toString() );
    }
    ui->debug->append( QString("Found links for %1: %2").arg( ui->search->text(), dbg.join(", ") ) );
    if( !links.isEmpty() ) {
        ui->help->setContent( engine->fileData( links.begin().value() ) );
    }
}


#include "docwidget.moc"
