/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdialog.h>
#include <qstringlist.h>
#include <qstring.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdevpartcontroller.h>
#include <kparts/part.h>
#include <ktexteditor/editor.h>
#include <kdevproject.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>

#include "closer_part.h"
#include "closer_dialogimpl.h"


typedef KGenericFactory<CloserPart> CloserFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcloser, CloserFactory( "kdevcloser" ) );

CloserPart::CloserPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin("Editor Window Closer", "closer", parent, name ? name : "closerPart" )
{
    setInstance(CloserFactory::instance());
    setXMLFile("kdevpart_closer.rc");

    KAction * action = new KAction( i18n("Close Editor Windows..."), CTRL+ALT+Key_W, this,
        SLOT( openDialog() ), actionCollection(), "closer" );

    action->setStatusText( i18n("Select editor windows to close") );

    core()->insertNewAction( action );
}


CloserPart::~CloserPart()
{}

void CloserPart::openDialog()
{
    CloserDialogImpl d( openFiles() );
    if ( d.exec() == QDialog::Accepted )
    {
        closeFiles( d.getCheckedFiles() );
    }
}

QStringList CloserPart::openFiles()
{
    QStringList openfiles;
    if( const QPtrList<KParts::Part> * partlist = partController()->parts() )
    {
        QPtrListIterator<KParts::Part> it( *partlist );
        while ( KParts::Part* part = it.current() )
        {
            if ( KTextEditor::Editor * ed = dynamic_cast<KTextEditor::Editor *>( part ) )
            {
                openfiles.append( relativeProjectPath( ed->url().path() ) );
            }
            ++it;
        }
    }
    return openfiles;
}

void CloserPart::closeFiles( QStringList const & fileList )
{
    QStringList::ConstIterator it = fileList.begin();
    while ( it != fileList.end() )
    {
        if ( KTextEditor::Editor * ed = getEditorForFile( fullProjectPath( *it ) ) )
        {
            ed->closeURL();
            partController()->removePart( ed );
            delete ed;
        }
        ++it;
    }
}

KTextEditor::Editor * CloserPart::getEditorForFile( QString const & file )
{
    if( const QPtrList<KParts::Part> * partlist = partController()->parts() )
    {
        QPtrListIterator<KParts::Part> it( *partlist );
        while ( KParts::Part* part = it.current() )
        {
            if ( KTextEditor::Editor * ed = dynamic_cast<KTextEditor::Editor *>( part ) )
            {
                if ( file == ed->url().path() )
                {
                    return ed;
                }
            }
            ++it;
        }
    }
    return 0;
}

QString CloserPart::relativeProjectPath( QString path )
{
    QString project = this->project()->projectDirectory() + "/";
    if ( path.left( project.length() ) == project )
    {
        path = path.mid( project.length() );
    }
    return path;
}

QString CloserPart::fullProjectPath( QString path )
{
    QString project = this->project()->projectDirectory() + "/";
    if ( path.left(1) != "/" && path.left( project.length() ) != project )
    {
        path = project + path;
    }
    return path;
}

#include "closer_part.moc"
