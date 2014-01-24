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
#include <QApplication>

#include <kdebug.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <KMessageBox>

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>

#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>

#include "core.h"
#include "textdocument.h"
#include <interfaces/isession.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <sublime/area.h>

namespace KDevelop
{

class PartControllerPrivate
{
public:
    PartControllerPrivate() {}

    QString m_editor;
    QStringList m_textTypes;

    Core *m_core;
};

PartController::PartController(Core *core, QWidget *toplevel)
        : IPartController( toplevel ), d(new PartControllerPrivate)

{
    setObjectName("PartController");
    d->m_core = core;
    //Cache this as it is too expensive when creating parts
    //     KConfig * config = Config::standard();
    //     config->setGroup( "General" );
    //
    //     d->m_textTypes = config->readEntry( "TextTypes", QStringList() );
    //
    //     config ->setGroup( "Editor" );
    //     d->m_editor = config->readPathEntry( "EmbeddedKTextEditor", QString() );
}

PartController::~PartController()
{
    delete d;
}

//MOVE BACK TO DOCUMENTCONTROLLER OR MULTIBUFFER EVENTUALLY
bool PartController::isTextType( KMimeType::Ptr mimeType )
{
    bool isTextType = false;
    if ( d->m_textTypes.contains( mimeType->name() ) )
    {
        isTextType = true;
    }

    // is this regular text - open in editor
    return ( isTextType
             || mimeType->is( "text/plain" )
             || mimeType->is( "text/html" )
             || mimeType->is( "application/x-zerosize" ) );
}

KTextEditor::Editor* PartController::editorPart() const
{
    return KTextEditor::Editor::instance();
}

KTextEditor::Document* PartController::createTextPart(const QString &encoding)
{
    KTextEditor::Document* doc = editorPart()->createDocument(this);

    if ( !encoding.isNull() )
    {
        KParts::OpenUrlArguments args = doc->arguments();
        args.setMimeType( QString::fromLatin1( "text/plain;" ) + encoding );
        doc->setArguments( args );
    }
    KTextEditor::Editor::instance()->readConfig();

    return doc;
}

KParts::Part* PartController::createPart( const QString & mimeType,
        const QString & partType,
        const QString & className,
        const QString & preferredName )
{
    KPluginFactory * editorFactory = findPartFactory(
                                          mimeType,
                                          partType,
                                          preferredName );

    if ( !className.isEmpty() && editorFactory )
    {
        return editorFactory->create<KParts::Part>(
                   0,
                   this,
                   className.toLatin1() );
    }

    return 0;
}

bool PartController::canCreatePart(const KUrl& url)
{
    if (!url.isValid()) return false;

    QString mimeType;
    if ( url.isEmpty() )
        mimeType = QString::fromLatin1("text/plain");
    else
        mimeType = KMimeType::findByUrl( url )->name();

    KService::List offers = KMimeTypeTrader::self()->query(
                                mimeType,
                                "KParts/ReadOnlyPart" );

    return offers.count() > 0;
}

KParts::Part* PartController::createPart( const KUrl & url, const QString& preferredPart )
{
    kDebug() << "creating part with url" << url << "and pref part:" << preferredPart;
    QString mimeType;
    if ( url.isEmpty() )
        //create a part for empty text file
        mimeType = QString::fromLatin1("text/plain");
    else if ( !url.isValid() )
        return 0;
    else
        mimeType = KMimeType::findByUrl( url )->name();

    KParts::Part* part = createPart( mimeType, preferredPart );
    if( part )
    {
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

//BEGIN KTextEditor::MdiContainer
void PartController::setActiveView(KTextEditor::View *view)
{
  Q_UNUSED(view)
  // NOTE: not implemented
}

KTextEditor::View *PartController::activeView()
{
    TextView* textView = dynamic_cast<TextView*>(Core::self()->uiController()->activeArea()->activeView());
    if (textView) {
        return textView->textView();
    }
    return 0;
}

KTextEditor::Document *PartController::createDocument()
{
  // NOTE: not implemented
  kWarning() << "WARNING: interface call not implemented";
  return 0;
}

bool PartController::closeDocument(KTextEditor::Document *doc)
{
  Q_UNUSED(doc)
  // NOTE: not implemented
  kWarning() << "WARNING: interface call not implemented";
  return false;
}

KTextEditor::View *PartController::createView(KTextEditor::Document *doc)
{
  Q_UNUSED(doc)
  // NOTE: not implemented
  kWarning() << "WARNING: interface call not implemented";
  return 0;
}

bool PartController::closeView(KTextEditor::View *view)
{
  Q_UNUSED(view)
  // NOTE: not implemented
  kWarning() << "WARNING: interface call not implemented";
  return false;
}
//END KTextEditor::MdiContainer


}
#include "partcontroller.moc"

