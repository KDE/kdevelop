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

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevconfig.h"
#include "kdeveditorintegrator.h"

#include "kdevpartcontroller.h"

KDevPartController::KDevPartController()
        : KParts::PartManager( KDevCore::mainWindow() ),
        m_editor( QString::null ),
        m_textTypes( QStringList() )
{
    //Cache this as it is too expensive when creating parts
    //     KConfig * config = KDevConfig::standard();
    //     config->setGroup( "General" );
    //
    //     m_textTypes = config->readEntry( "TextTypes", QStringList() );
    //
    //     config ->setGroup( "Editor" );
    //     m_editor = config->readPathEntry( "EmbeddedKTextEditor" );
}

KDevPartController::~KDevPartController()
{}

//MOVE BACK TO DOCUMENTCONTROLLER OR MULTIBUFFER EVENTUALLY
bool KDevPartController::isTextType( KMimeType::Ptr mimeType )
{
    bool isTextType = false;
    if ( m_textTypes.contains( mimeType->name() ) )
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

KTextEditor::Document* KDevPartController::createTextPart(
    const KUrl &url,
    const QString &encoding,
    bool activate )
{
    Q_UNUSED( activate );

    KTextEditor::Document* doc =
        qobject_cast<KTextEditor::Document *>( createPart(
                                                   "text/plain",
                                                   "KTextEditor/Document",
                                                   "KTextEditor::Editor",
                                                   m_editor ) );

    KDevEditorIntegrator::addDocument(doc);

    if ( !encoding.isNull() )
    {
        KParts::BrowserExtension * extension =
                KParts::BrowserExtension::childObject( doc );
        if ( extension )
        {
            KParts::URLArgs args;
            args.serviceType = QString( "text/plain;" )
                    + encoding;
            extension->setURLArgs( args );
        }
    }

    if ( activate )
    {
        doc->openURL( url );
    }

    return doc;
}

void KDevPartController::removePart( KParts::Part *part)
{
    if (KTextEditor::Document* doc = qobject_cast<KTextEditor::Document *>(part)) {
        if (KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>(doc)) {
            // FIXME not supposed to delete locked mutexes...
            QMutexLocker lock(smart->smartMutex());
            KParts::PartManager::removePart(part);
            return;
        }

        kWarning() << k_funcinfo << "Deleting text editor " << doc << " which does not have a smart interface." << endl;
    }

    KParts::PartManager::removePart(part);
}

KParts::Part* KDevPartController::createPart( const QString & mimeType,
        const QString & partType,
        const QString & className,
        const QString & preferredName )
{
    KParts::Factory * editorFactory = findPartFactory(
                                          mimeType,
                                          partType,
                                          preferredName );

    if ( !className.isEmpty() && editorFactory )
    {
        return editorFactory->createPart(
                   /*KDevCore::mainWindow() ->centralWidget()*/0,
                   KDevCore::mainWindow() ->centralWidget(),
                   className.toLatin1() );
    }

    return 0;
}

KParts::Part* KDevPartController::createPart( const KUrl & url )
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
        KParts::Part * part = editorFactory->createPart(
                                  KDevCore::mainWindow() ->centralWidget(),
                                  KDevCore::mainWindow() ->centralWidget(),
                                  className.toLatin1() );
        readOnly( part ) ->openURL( url );
        return part;
    }

    return 0;
}

KParts::Factory *KDevPartController::findPartFactory(
    const QString & mimeType,
    const QString & partType,
    const QString & preferredName )
{
    //Cache the factories as it is too expensive to use KLibLoader for
    //every part.
    //if ( m_factoryCache.contains( mimeType + partType + preferredName ) )
        //return m_factoryCache.value( mimeType + partType + preferredName );

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
        m_factoryCache.insert( mimeType + partType + preferredName, factory );
        return factory;
    }

    return 0;
}

KParts::ReadOnlyPart* KDevPartController::activeReadOnly() const
{
    return readOnly( activePart() );
}

KParts::ReadWritePart* KDevPartController::activeReadWrite() const
{
    return readWrite( activePart() );
}

KParts::ReadOnlyPart* KDevPartController::readOnly( KParts::Part * part ) const
{
    return qobject_cast<KParts::ReadOnlyPart*>( part );
}

KParts::ReadWritePart* KDevPartController::readWrite( KParts::Part * part ) const
{
    return qobject_cast<KParts::ReadWritePart*>( part );
}

#include "kdevpartcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
