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
#include <QKeyEvent>
#include <QApplication>
#include <QUuid>

#include <kurl.h>
#include <kicon.h>
#include <kdebug.h>
#include <KLocalizedString>
#include <klibloader.h>
#include <kde_terminal_interface.h>
#include <kparts/part.h>
#include <kparts/readonlypart.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>

#include "kdevkonsoleviewplugin.h"

class KDevKonsoleViewPrivate
{
public:
    KDevKonsoleViewPlugin* mplugin;
    KDevKonsoleView* m_view;
    KParts::ReadOnlyPart *konsolepart;
    QVBoxLayout *m_vbox;

    void _k_slotTerminalClosed();

    void init( KPluginFactory* factory )
    {
        Q_ASSERT( konsolepart == 0 );

        Q_ASSERT( factory != 0 );

        if ( ( konsolepart = factory->create<KParts::ReadOnlyPart>( m_view ) ) )
        {
            QObject::connect(konsolepart, SIGNAL(destroyed(QObject*)), m_view, SLOT(_k_slotTerminalClosed()));

            konsolepart->widget() ->setFocusPolicy( Qt::WheelFocus );
            konsolepart->widget() ->setFocus();
            konsolepart->widget() ->installEventFilter( m_view );

            if ( QFrame * frame = qobject_cast<QFrame*>( konsolepart->widget() ) )
                frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );

            m_vbox->addWidget( konsolepart->widget() );
            m_view->setFocusProxy( konsolepart->widget() );
            konsolepart->widget() ->show();

            TerminalInterface* interface = qobject_cast<TerminalInterface*>(konsolepart);
            Q_ASSERT(interface);

            interface->showShellInDir( QString() );
            interface->sendInput( "kdevelop! -s \"" + KDevelop::ICore::self()->activeSession()->id().toString() + "\"\n" );
            
        }else
        {
            kDebug() << "Couldn't create KParts::ReadOnlyPart from konsole factory!";
        }
    }

};

void KDevKonsoleViewPrivate::_k_slotTerminalClosed()
{
    konsolepart = 0;
    init( mplugin->konsoleFactory() );
}

KDevKonsoleView::KDevKonsoleView( KDevKonsoleViewPlugin *plugin, QWidget* parent )
        : QWidget( parent ), d(new KDevKonsoleViewPrivate)

{
    d->mplugin = plugin;
    d->m_view = this;
    d->konsolepart = 0;
    setObjectName( i18n( "Konsole" ) );

    setWindowIcon( QIcon::fromTheme( "utilities-terminal" ) );
    setWindowTitle( i18n( "Konsole" ) );

    d->m_vbox = new QVBoxLayout( this );
    d->m_vbox->setMargin( 0 );
    d->m_vbox->setSpacing( 0 );

    d->init( d->mplugin->konsoleFactory() );

    //TODO Make this configurable in the future,
    // but by default the konsole shouldn't
    // automatically switch directories on you.

//     connect( KDevelop::Core::documentController(), SIGNAL(documentActivated(KDevDocument*)),
//              this, SLOT(documentActivated(KDevDocument*)) );
}

KDevKonsoleView::~KDevKonsoleView()
{
    delete d;
}

void KDevKonsoleView::setDirectory( const QUrl &url )
{
    if ( !url.isValid() || !url.isLocalFile() )
        return ;

    if ( d->konsolepart && url != d->konsolepart->url() )
        d->konsolepart->openUrl( url );
}

bool KDevKonsoleView::eventFilter( QObject* obj, QEvent *e )
{
    switch( e->type() ) {
        case QEvent::ShortcutOverride: {
            QKeyEvent *k = static_cast<QKeyEvent *>(e);

            // Don't propagate Esc to the top level, it should be used by konsole
            if (k->key() == Qt::Key_Escape) {
                if (d->konsolepart && d->konsolepart->widget()) {
                    e->accept();
                    return true;
                }
            }
            break;
        }

        default:
            break;
    }

    return QWidget::eventFilter( obj, e );
}

#include "moc_kdevkonsoleview.cpp"
