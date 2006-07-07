#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QFile>
#include <QTimer>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>

#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>

#include "toplevel.h"
#include "kdevmainwindow.h"
#include "kmainwindow.h"

#include "partcontroller.h"

PartController::PartController( QWidget *parent )
        : KParts::PartManager( parent )
{}

PartController::~PartController()
{}

KParts::Part* PartController::createPart( const QString &mimeType,
        const QString &partType,
        const QString &className,
        const QString &preferredName )
{
    KParts::Factory * editorFactory = findPartFactory(
                                          mimeType,
                                          partType,
                                          preferredName );

    if ( !className.isEmpty() && editorFactory )
    {
        return editorFactory->createPart(
                   TopLevel::getInstance() ->main() ->centralWidget(),
                   TopLevel::getInstance() ->main() ->centralWidget(),
                   className.toLatin1() );
    }

    return 0;
}

KParts::Part* PartController::createPart( const KUrl &url )
{
    if ( !url.isValid() )
        return 0;

    KMimeType::Ptr mimeType = KMimeType::findByURL( url );

    QString className;
    QString services[] =
        {
            "KDevelop/ReadWritePart", "KDevelop/ReadOnlyPart",
            "KParts/ReadWritePart", "KParts/ReadOnlyPart"
        };

    QString classNames[] =
        {
            "KParts::ReadWritePart", "KParts::ReadOnlyPart",
            "KParts::ReadWritePart", "KParts::ReadOnlyPart"
        };
    KParts::Factory *editorFactory = 0;
    for ( uint i = 0; i < 4; ++i )
    {
        editorFactory = findPartFactory( mimeType->name(), services[ i ] );
        if ( editorFactory )
        {
            className = classNames[ i ];
            break;
        }
    }

    if ( !className.isEmpty() && editorFactory )
    {
        return editorFactory->createPart(
                   TopLevel::getInstance() ->main() ->centralWidget(),
                   TopLevel::getInstance() ->main() ->centralWidget(),
                   className.toLatin1() );
    }

    return 0;
}

KParts::Factory *PartController::findPartFactory(
    const QString &mimeType,
    const QString &partType,
    const QString &preferredName )
{
    KService::List offers = KMimeTypeTrader::self() ->query(
                                mimeType,
                                "KParts/ReadWritePart",
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
        return static_cast<KParts::Factory*>(
                   KLibLoader::self() ->factory(
                       QFile::encodeName( ptr->library() ) ) );
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

KParts::ReadOnlyPart* PartController::readOnly( KParts::Part *part ) const
{
    return qobject_cast<KParts::ReadOnlyPart*>( part );
}

KParts::ReadWritePart* PartController::readWrite( KParts::Part *part ) const
{
    return qobject_cast<KParts::ReadWritePart*>( part );
}

#include "partcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
