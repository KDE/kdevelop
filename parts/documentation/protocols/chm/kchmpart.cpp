/*  This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kchmpart.h"
#include <qstring.h>

#include <kinstance.h>
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

extern "C"
{
   void* init_libkchmpart()
   {
      return new KChmPartFactory;
   }
}

KInstance* KChmPartFactory::s_instance = 0L;
KAboutData* KChmPartFactory::s_about = 0L;

KChmPartFactory::KChmPartFactory( QObject* parent, const char* name )
   : KParts::Factory( parent, name )
{
}

KChmPartFactory::~KChmPartFactory()
{
   delete s_instance;
   s_instance = 0L;
   delete s_about;
}

KParts::Part* KChmPartFactory::createPartObject( QWidget *parentWidget, const char *, QObject *,
                                 const char *name, const char *, const QStringList & )
{
   KChmPart* part = new KChmPart( parentWidget, name );
   return part;
}

KInstance* KChmPartFactory::instance()
{
   if( !s_instance )
   {
      s_about = new KAboutData( "kchmpart",
                                I18N_NOOP( "KChm" ), "1.0pre" );
      s_instance = new KInstance( s_about );
   }
   return s_instance;
}


KChmPart::KChmPart( QWidget * parent, const char * name )
	: KDevHTMLPart(  ), m_job(0)
{
   KInstance * instance = new KInstance( "kchmpart" );
   setInstance( instance );
   m_extension=new KParts::BrowserExtension(this);
   setOptions(-1);
}

bool KChmPart::openURL( const KURL &url )
{
   KURL chmURL = url;
   chmURL.setProtocol("ms-its");
   chmURL.addPath("/");
   return KDevHTMLPart::openURL(chmURL);
}

void KChmPart::slotDuplicate()
{
}

void KChmPart::slotOpenInNewWindow(const KURL &url)
{
}


/*
bool KChmPart::openFile()
{
   if (m_job!=0)
      m_job->kill();

   m_htmlData.truncate(0);

   m_job = KIO::get( QString("chm:")+m_file+"/", true, false );
   connect( m_job, SIGNAL( data( KIO::Job *, const QByteArray &) ), SLOT( readData( KIO::Job *, const QByteArray &) ) );
   connect( m_job, SIGNAL( result( KIO::Job * ) ), SLOT( jobDone( KIO::Job * ) ) );
   return true;
}

void KChmPart::readData(KIO::Job * , const QByteArray & data)
{
    m_htmlData += data;
}

void KChmPart::jobDone( KIO::Job *)
{
   m_job=0;
   begin();
   write(QString::fromLocal8Bit(m_htmlData));
   end();
}
*/
#include "kchmpart.moc"

