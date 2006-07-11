/*
 * KDevelop Multiple Buffer Support
 *
 * Copyright (c) 2005 Adam Treat <treat@kde.org>
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

// Ewww... need this to access KParts::Part::setWidget(), so that kdevelop
// doesn't need to be rearchitected for multiple views before the lazy view
// creation can go in
#define protected public
#include <kparts/part.h>
#undef protected

#include "multibuffer.h"

#include "api.h"
#include "toplevel.h"
#include "editorproxy.h"
#include "partcontroller.h"
#include "kdevlanguagesupport.h"

#include <kdebug.h>
#include <kmimetype.h>
#include <kmainwindow.h>
#include <kapplication.h>

#include <kparts/factory.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/viewcursorinterface.h>

MultiBuffer::MultiBuffer( QWidget *parent )
        : QSplitter( parent, "MultiBuffer" ),
        m_editorFactory( 0 ),
        m_delayActivate( false ),
        m_activated( false ),
        m_activeBuffer( 0 )
{
    EditorProxy::getInstance() ->registerEditor( this );
    if ( KDevLanguageSupport *lang =
         API::getInstance() ->languageSupport() )
    {
        setOrientation( lang->splitOrientation() );
        connect( lang, SIGNAL( splitOrientationChanged( Qt::Orientation ) ),
                 this, SLOT( setOrientation( Qt::Orientation ) ) );
    }
    else
    {
        setOrientation( Qt::Vertical );
    }
}

MultiBuffer::~MultiBuffer()
{
    EditorProxy::getInstance() ->deregisterEditor( this );
}

KParts::Part *MultiBuffer::activeBuffer( ) const
{
    if ( m_activeBuffer )
        return m_activeBuffer;
    // The active buffer might just been deleted...
    else if ( m_buffers.begin().data() )
        return ( m_buffers.begin().data() );
    else
        return 0;
}

bool MultiBuffer::hasURL( const KURL &url ) const
{
    return m_buffers.contains( url );
}

int MultiBuffer::numberOfBuffers() const
{
    return m_buffers.count();
}

bool MultiBuffer::isActivated() const
{
    if ( m_delayActivate )
        return m_activated;
    else
        return true;
}

void MultiBuffer::setDelayedActivation( bool delayed )
{
    m_delayActivate = delayed;
    if ( delayed )
        m_activated = false;
}

KParts::Part* MultiBuffer::openURL( const KURL &url )
{
    KParts::ReadOnlyPart * part =
        dynamic_cast<KParts::ReadOnlyPart*>( createPart( url ) );

    if ( !part )
        return 0;

    if ( !part->openURL( url ) )
        return 0;

    m_buffers.insert( url, part );
    return part;
}

bool MultiBuffer::closeURL( const KURL &url )
{
    if ( !m_buffers.contains( url ) )
        return false;

    bool result = false;
    KParts::ReadOnlyPart * part =
        dynamic_cast<KParts::ReadOnlyPart *>( m_buffers[ url ] );
    if ( part )
        if (part->closeURL())
        {
            m_buffers.remove( url );
            return true;
        }
    return false;
}

void MultiBuffer::registerURL( const KURL &url, KParts::Part *part )
{
    m_buffers.insert( url, part );
}

void MultiBuffer::registerDelayedActivation( KParts::Part *part,
        int line, int col )
{
    m_delayedActivation[ part ] = qMakePair( line, col );
}

KParts::Part* MultiBuffer::createPart( const QString &mimeType,
                                       const QString &partType,
                                       const QString &className,
                                       const QString &preferredName )
{
    m_editorFactory = PartController::getInstance() ->findPartFactory(
                          mimeType, partType, preferredName );

    if ( !className.isEmpty() && m_editorFactory )
    {
        return m_editorFactory->createPart(
                   this, 0, 0, 0, className.latin1() );
    }

    return 0;
}

KParts::Part* MultiBuffer::createPart( const KURL &url )
{
    if ( !url.isValid() )
        return 0;

    KMimeType::Ptr mimeType = KMimeType::findByURL( url );

    QString className;
    QString services[] = { "KParts/ReadWritePart", "KParts/ReadOnlyPart" };
    QString classnames[] = { "KParts::ReadWritePart", "KParts::ReadOnlyPart" };
    for ( uint i = 0; i < 2; ++i )
    {
        m_editorFactory = PartController::getInstance() ->findPartFactory(
                              mimeType->name(), services[ i ] );
        if ( m_editorFactory )
        {
            className = classnames[ i ];
            break;
        }
    }

    if ( !className.isEmpty() && m_editorFactory )
    {
        return m_editorFactory->createPart(
                   this, 0, 0, 0, className.latin1() );
    }

    return 0;
}

void MultiBuffer::show()
{
    if ( !m_delayedActivation.count() || m_activated )
    {
        QSplitter::show();
        return ;
    }

    ActivationMap::Iterator it = m_delayedActivation.begin();
    for ( ; it != m_delayedActivation.end(); ++it )
    {
        KTextEditor::Document *document =
            dynamic_cast<KTextEditor::Document*>( it.key() );

        if ( !document )
            continue;

        int line = it.data().first;
        int column = it.data().second;
        KTextEditor::View *view = document->createView( this );
        document->setWidget( view );

        // We're managing the view deletion by being its parent,
        // don't let the part self-destruct
        disconnect( view, SIGNAL( destroyed() ),
                    document, SLOT( slotWidgetDestroyed() ) );

        document->insertChildClient( view );
        PartController::getInstance() ->integrateTextEditorPart( document );

        KTextEditor::ViewCursorInterface *iface =
            dynamic_cast<KTextEditor::ViewCursorInterface*>(
                static_cast<KTextEditor::View*>( view ) );
        if ( iface )
        {
            iface->setCursorPositionReal( line,
                                          column == -1 ? 0 : column );
        }
        else
        {
            // Shouldn't get here
            Q_ASSERT( false );
        }
        view->show();
        kdDebug( 9000 ) << "Delayed activation of "
        << document->url().fileName() << " is now complete." << endl;
    }

    m_activated = true;
    QSplitter::show();
}

void MultiBuffer::setOrientation( Qt::Orientation orientation )
{
    QSplitter::setOrientation( orientation );
}

void MultiBuffer::activePartChanged( const KURL &url )
{
    if ( !m_buffers.contains( url ) )
        return ;

    m_activeBuffer = m_buffers[ url ];
    TopLevel::getInstance() ->setCurrentDocumentCaption( url.fileName() );
}

void MultiBuffer::focusInEvent( QFocusEvent *ev )
{
    KParts::Part *active = activeBuffer();
    if (active && active->widget())
	active->widget()->setFocus();
    QSplitter::focusInEvent(ev);
}


#include "multibuffer.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
