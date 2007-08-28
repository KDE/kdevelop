/***************************************************************************
*   Copyright 2003, 2006 Adam Treat <treat@kde.org>                       *
*   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "kdevkonsoleview.h"

#include <QDir>
#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QVBoxLayout>

#include <kurl.h>
#include <kicon.h>
#include <kdebug.h>
#include <klocale.h>
#include <klibloader.h>
#include <kde_terminal_interface.h>
#include <kparts/part.h>

#include <icore.h>
#include <iprojectcontroller.h>
// #include <kdevdocumentcontroller.h>

#include "kdevkonsoleview_part.h"

class KDevKonsoleViewPrivate
{
public:
    KDevKonsoleViewPart* m_part;
    KDevKonsoleView* m_view;
    KParts::ReadOnlyPart *konsolepart;
    QVBoxLayout *m_vbox;

    void init( )
    {
        Q_ASSERT( konsolepart == 0 );


        if ( KPluginFactory * factory = KPluginLoader("libkonsolepart").factory() )
        {
            if ( ( konsolepart = factory->create<KParts::ReadOnlyPart>( m_view ) ) )
            {
                konsolepart->widget() ->setFocusPolicy( Qt::WheelFocus );
                konsolepart->widget() ->setFocus();

                if ( QFrame * frame = qobject_cast<QFrame*>( konsolepart->widget() ) )
                    frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );

                m_vbox->addWidget( konsolepart->widget() );
                m_view->setFocusProxy( konsolepart->widget() );
                konsolepart->widget() ->show();

                TerminalInterface* interface = qobject_cast<TerminalInterface*>(konsolepart);
                Q_ASSERT(interface);

                interface->showShellInDir( QString() );
            }
        }
        else
        {
            m_vbox->addWidget(
                new QLabel( i18n( "Konsole not available or libkonsolepart not in path." ),
                            m_view ) );
        }
    }

};

KDevKonsoleView::KDevKonsoleView( KDevKonsoleViewPart *part, QWidget* parent )
        : QWidget( parent ), d(new KDevKonsoleViewPrivate)

{
    d->m_part = part;
    d->m_view = this;
    d->konsolepart = 0;
    setObjectName( i18n( "Konsole" ) );

    setWhatsThis( i18n( "<b>Konsole</b><p>"
            "This window contains an embedded console.</p>" ) );
    setWindowIcon( KIcon( "konsole" ) );
    setWindowTitle( i18n( "Konsole" ) );

    d->m_vbox = new QVBoxLayout( this );
    d->m_vbox->setMargin( 0 );
    d->m_vbox->setSpacing( 0 );

    d->init();

    //TODO Make this configurable in the future,
    // but by default the konsole shouldn't
    // automatically switch directories on you.

//     connect( KDevelop::Core::documentController(), SIGNAL( documentActivated( KDevDocument* ) ),
//              this, SLOT( documentActivated( KDevDocument* ) ) );
}

KDevKonsoleView::~KDevKonsoleView()
{
    delete d->konsolepart;
    delete d;
}

void KDevKonsoleView::setDirectory( const KUrl &url )
{
    if ( !url.isValid() || !url.isLocalFile() )
        return ;

    if ( d->konsolepart && url != d->konsolepart->url() )
        d->konsolepart->openUrl( url );
}

#include "kdevkonsoleview.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
