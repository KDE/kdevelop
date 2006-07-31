/***************************************************************************
*   Copyright (C) 2003 by KDevelop Authors                                *
*   kdevelop-devel@kde.org                                                *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QLayout>
#include <QLabel>
#include <QFrame>
#include <QDir>
#include <QVBoxLayout>

#include <klocale.h>
#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevkonsoleview_part.h"
#include "kdevdocumentcontroller.h"

#include "kdevkonsoleview.h"

#include <QtCore/qdebug.h>

KDevKonsoleView::KDevKonsoleView( QWidget *parent )
        : QWidget( parent ),
        m_part( 0 )
{
    //TODO Make this configurable in the future,
    // but by default the konsole shouldn't
    // automatically switch directories on you.

    //     connect( KDevCore::documentController(),
    //              SIGNAL( activePartChanged( KParts::Part* ) ),
    //              this, SLOT( activePartChanged( KParts::Part* ) ) );
    m_vbox = new QVBoxLayout( this );
}

KDevKonsoleView::~KDevKonsoleView()
{
    delete m_part;
}

void KDevKonsoleView::showEvent( QShowEvent *event )
{
    QWidget::showEvent( event );
    activate();
}

void KDevKonsoleView::activate()
{
    static bool initialized = false;

    if ( initialized )
        return ;

    Q_ASSERT( m_part == 0 );
    initialized = true;

    if ( KLibFactory * factory = KLibLoader::self() ->factory( "libkonsolepart" ) )
    {
        if ( m_part = qobject_cast<KParts::ReadOnlyPart*>( factory->create( this ) ) )
        {
            m_part->widget() ->setFocusPolicy( Qt::WheelFocus );
            setFocusProxy( m_part->widget() );
            m_part->widget() ->setFocus();

            if ( QFrame * frame = qobject_cast<QFrame*>( m_part->widget() ) )
                frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );

            m_vbox->addWidget( m_part->widget() );
            m_part->widget() ->show();

            connect( m_part, SIGNAL( destroyed() ), this, SLOT( partDestroyed() ) );
        }
    }
    else
    {
        m_vbox->addWidget(
            new QLabel( i18n( "Konsole not available or libkonsolepart not in path." ),
                        this ) );
    }
}


void KDevKonsoleView::activePartChanged( KParts::Part *activatedPart )
{
    KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( activatedPart );

    if ( ro_part && !ro_part->url().isLocalFile() )
        return ;

    QString dir;
    if ( ro_part )
        dir = ro_part->url().directory();
    else if ( KDevCore::activeProject() )
        dir = KDevCore::activeProject() ->projectDirectory();

    if ( dir.isEmpty() )
        return ;
    setDirectory( KUrl( dir ) );
}

void KDevKonsoleView::setDirectory( const KUrl &dirUrl )
{
    if ( m_part && dirUrl != m_part->url() )
        m_part->openURL( dirUrl );
}

void KDevKonsoleView::partDestroyed()
{
    m_part = 0;
    activate();
}


#include "kdevkonsoleview.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
