/***************************************************************************
 *   Copyright 2006 Adam Treat  <treat@kde.org>                     *
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "partcontroller.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QFile>
#include <QTimer>
#include <QMutexLocker>

#include <kdebug.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>

#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "core.h"
// #include "kdevmainwindow.h"
// #include "kdevconfig.h"
#include "editorintegrator.h"

namespace KDevelop
{

class PartControllerPrivate
{
public:
    QString m_editor;
    QStringList m_textTypes;

    Core *m_core;

    KParts::Factory *findPartFactory( const QString &mimeType,
                                      const QString &partType,
                                      const QString &preferredName = QString() )

    {
        KService::List offers = KMimeTypeTrader::self() ->query(
                                    mimeType,
                                    "KParts/ReadOnlyPart",
                                    QString( "'%1' in ServiceTypes" ).arg( partType ) );

        if ( offers.count() > 0 )
        {
            KService::Ptr ptr;
            // if there is a preferred plugin we'll take it
            if ( !preferredName.isEmpty() )
            {
                KService::List::ConstIterator it;
                for ( it = offers.begin(); it != offers.end(); ++it )
                {
                    if ( ( *it ) ->desktopEntryName() == preferredName )
                    {
                        ptr = ( *it );
                    }
                }
            }
            // else we just take the first in the list
            if ( !ptr )
            {
                ptr = offers.first();
            }
            KParts::Factory *factory = static_cast<KParts::Factory*>(
                                           KLibLoader::self() ->factory(
                                               QFile::encodeName( ptr->library() ) ) );
            return factory;
        }

        return 0;
    }

};

PartController::PartController(Core *core, QWidget *toplevel)
        : KParts::PartManager( toplevel, 0 ), d(new PartControllerPrivate)

{
    d->m_core = core;
    //Cache this as it is too expensive when creating parts
    //     KConfig * config = Config::standard();
    //     config->setGroup( "General" );
    //
    //     d->m_textTypes = config->readEntry( "TextTypes", QStringList() );
    //
    //     config ->setGroup( "Editor" );
    //     d->m_editor = config->readPathEntry( "EmbeddedKTextEditor" );
}

PartController::~PartController()
{}

//MOVE BACK TO DOCUMENTCONTROLLER OR MULTIBUFFER EVENTUALLY
bool PartController::isTextType( KMimeType::Ptr mimeType )
{
    bool isTextType = false;
    if ( d->m_textTypes.contains( mimeType->name() ) )
    {
        isTextType = true;
    }

    bool isKDEText = false;
    QVariant v = mimeType->property( "X-KDE-text" );
    if ( v.isValid() )
        isKDEText = v.toBool();

    // is this regular text - open in editor
    return ( isTextType || isKDEText
             || mimeType->is( "text/plain" )
             || mimeType->is( "text/html" )
             || mimeType->is( "application/x-zerosize" ) );
}

KTextEditor::Document* PartController::createTextPart(
    const KUrl &url,
    const QString &encoding,
    bool activate )
{
    KTextEditor::Document* doc =
        qobject_cast<KTextEditor::Document *>( createPart(
                                                   "text/plain",
                                                   "KTextEditor/Document",
                                                   "KTextEditor::Editor",
                                                   d->m_editor ) );

    EditorIntegrator::addDocument( doc );

    if ( !encoding.isNull() )
    {
        KParts::BrowserExtension * extension =
            KParts::BrowserExtension::childObject( doc );
        if ( extension )
        {
            KParts::URLArgs args;
            args.serviceType = QString( "text/plain;" )
                               + encoding;
            extension->setUrlArgs( args );
        }
    }

    if ( activate )
    {
        doc->openUrl( url );
    }

    return doc;
}

void PartController::removePart( KParts::Part *part )
{
    if ( KTextEditor::Document * doc = qobject_cast<KTextEditor::Document *>( part ) )
    {
        if ( KTextEditor::SmartInterface * smart = dynamic_cast<KTextEditor::SmartInterface*>( doc ) )
        {
            // FIXME not supposed to delete locked mutexes...
            QMutexLocker lock ( smart->smartMutex() );
            KParts::PartManager::removePart( part );
            return ;
        }

        kWarning() << k_funcinfo << "Deleting text editor " << doc << " which does not have a smart interface." << endl;
    }

    KParts::PartManager::removePart( part );
}

KParts::Part* PartController::createPart( const QString & mimeType,
        const QString & partType,
        const QString & className,
        const QString & preferredName )
{
    KParts::Factory * editorFactory = d->findPartFactory(
                                          mimeType,
                                          partType,
                                          preferredName );

    if ( !className.isEmpty() && editorFactory )
    {
        return editorFactory->createPart(
                   0,
                   this,
                   className.toLatin1() );
    }

    return 0;
}

KParts::Part* PartController::createPart( const KUrl & url )
{
    if ( !url.isValid() )
        return 0;

    KMimeType::Ptr mimeType = KMimeType::findByUrl( url );

    QString className;
    QString services[] =
        {
            "KParts/ReadWritePart", "KParts/ReadOnlyPart"
        };

    QString classNames[] =
        {
            "KParts::ReadWritePart", "KParts::ReadOnlyPart"
        };
    KParts::Factory *editorFactory = 0;
    for ( uint i = 0; i < 2; ++i )
    {
        editorFactory = d->findPartFactory( mimeType->name(), services[ i ] );
        if ( editorFactory )
        {
            className = classNames[ i ];
            break;
        }
    }

    if ( !className.isEmpty() && editorFactory )
    {
        KParts::Part * part = editorFactory->createPart(
                                  0,
                                  this,
                                  className.toLatin1() );
        readOnly( part ) ->openUrl( url );
        return part;
    }

    return 0;
}

KParts::ReadOnlyPart* PartController::activeReadOnly() const
{
    return readOnly( activePart() );
}

KParts::ReadWritePart* PartController::activeReadWrite() const
{
    return readWrite( activePart() );
}

KParts::ReadOnlyPart* PartController::readOnly( KParts::Part * part ) const
{
    return qobject_cast<KParts::ReadOnlyPart*>( part );
}

KParts::ReadWritePart* PartController::readWrite( KParts::Part * part ) const
{
    return qobject_cast<KParts::ReadWritePart*>( part );
}

void PartController::loadSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded );
}

void PartController::saveSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded );
}

void PartController::initialize()
{}

void PartController::cleanup()
{}

}
#include "partcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
