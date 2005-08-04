/*
 *  File : snippet_part.cpp
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <q3whatsthis.h>

#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <q3vbox.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <qcheckbox.h>
#include <q3buttongroup.h>
#include <ktrader.h>

#include "kdevcore.h"
#include <kdevproject.h>
#include "kdevmainwindow.h"
#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>

#include <kdebug.h>

#include "snippet_widget.h"
#include "snippet_part.h"
#include "snippetitem.h"
#include "snippetsettings.h"
#include "snippetconfig.h"

static const KDevPluginInfo data("kdevsnippet");
typedef KDevGenericFactory<SnippetPart> snippetFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsnippet, snippetFactory( data ) )

SnippetPart::SnippetPart(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin(&data, parent, name ? name : "SnippetPart" )
{
  setInstance(snippetFactory::instance());
  //setXMLFile("kdevpart_snippet.rc");

  m_widget = new SnippetWidget(this);
  m_widget->setCaption(i18n("Code Snippets"));
  m_widget->setIcon(SmallIcon( info()->icon() ));

  Q3WhatsThis::add(m_widget, i18n("<b>Code Snippet</b><p>This is a list of available snippets."));

  mainWindow()->embedSelectViewRight( m_widget, i18n("Code Snippets"), i18n("Insert a code snippet") );

  connect( core(), SIGNAL( configWidget( KDialogBase * ) ), this, SLOT( slotConfigWidget( KDialogBase * ) ) );
  
  /*The next two connects are used to check if certain SnippetGroups need to be opened 
    according to the languages supported by this project*/
  connect( core(), SIGNAL( projectOpened() ), m_widget, SLOT( languageChanged() ) );
  connect( core(), SIGNAL( languageChanged() ), m_widget, SLOT( languageChanged() ) );
}

SnippetPart::~SnippetPart()
{
  if ( m_widget )
  {
    mainWindow()->removeView( m_widget );
  }

  delete m_widget;
}

/*!
    \fn SnippetPart::aboutData()
 */
KAboutData* SnippetPart::aboutData()
{
  KAboutData *data = new KAboutData  ("snippetpart", I18N_NOOP("SnippetPart"), "1.1",
                                                           I18N_NOOP( "SnippetPart for KDevelop" ),
                                                           KAboutData::License_LGPL_V2,
                                                           I18N_NOOP( "(c) 2003" ), 0, "http://www.kdevelop.org");
  data->addAuthor ("Robert Gruber", I18N_NOOP("Author"), "rgruber@users.sourceforge.net", "http://kmp3indexer.sf.net");


  return data;
}


/*!
    \fn SnippetPart::slotConfigWidget( KDialogBase *dlg )
 */
void SnippetPart::slotConfigWidget( KDialogBase *dlg )
{
  Q3VBox *vbox = dlg->addVBoxPage( i18n("Code Snippets"), i18n("Code Snippets"), BarIcon( info()->icon(), KIcon::SizeMedium ) );

  SnippetSettings * w = new SnippetSettings( m_widget, vbox );

  w->btnGroup->setButton(m_widget->getSnippetConfig()->getInputMethod());
  w->leDelimiter->setText(m_widget->getSnippetConfig()->getDelimiter());
  w->cbToolTip->setChecked(m_widget->getSnippetConfig()->useToolTips());
  w->btnGroupAutoOpen->setButton(m_widget->getSnippetConfig()->getAutoOpenGroups());
  connect( dlg, SIGNAL(okClicked()), w, SLOT(slotOKClicked()) );
}


QStringList SnippetPart::getAllLanguages()
{
    KTrader::OfferList languageSupportOffers =
        KTrader::self()->query(QString::fromLatin1("KDevelop/LanguageSupport"),
                               QString::fromLatin1("[X-KDevelop-Version] == %1"
                               ).arg( KDEVELOP_PLUGIN_VERSION ));
    
    QStringList languages;

    for (KTrader::OfferList::ConstIterator it = languageSupportOffers.begin(); it != languageSupportOffers.end(); ++it)
    {
        QString language = (*it)->property("X-KDevelop-Language").toString();
        languages.append(language);
        
        kdDebug(9035) << "Found language: " << (*it)->property("X-KDevelop-Language").toString() << endl <<
        "genericName(): " <<(*it)->genericName() << endl << 
        "comment(): " <<(*it)->comment() << endl << endl;
    }
    
    return languages;
}


QStringList SnippetPart::getProjectLanguages()
{
    QStringList languages;
    if (!projectDom())
        return languages;

    QDomDocument m_projectDom = *projectDom();
    
    if (m_widget->getSnippetConfig()->getAutoOpenGroups() == 1)
        languages = DomUtil::readListEntry(m_projectDom, "/general/secondaryLanguages", "language");
    languages.prepend( DomUtil::readEntry(m_projectDom, "/general/primarylanguage") );
    return languages;
}


#include "snippet_part.moc"
