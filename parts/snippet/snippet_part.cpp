/*
 *  File : snippet_part.cpp
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <qwhatsthis.h>

#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <qvbox.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"

#include <kdebug.h>

#include "snippet_widget.h"
#include "snippet_part.h"
#include "snippetitem.h"
#include "snippetsettings.h"

typedef KGenericFactory<SnippetPart> snippetFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsnippet, snippetFactory( "kdevsnippet" ) )

SnippetPart::SnippetPart(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin("CodeSnippet", "snippet", parent, name ? name : "SnippetPart" )
{
  setInstance(snippetFactory::instance());
  //setXMLFile("kdevpart_snippet.rc");

  m_widget = new SnippetWidget(this);
  m_widget->setCaption(i18n("Code Snippets"));
  m_widget->setIcon(SmallIcon("editcut"));

  QWhatsThis::add(m_widget, i18n("<b>Code Snippet</b><p>This is a list of available snippets."));

  mainWindow()->embedSelectViewRight( m_widget, i18n("Code Snippets"), i18n("Insert a code snippet") );

  connect( core(), SIGNAL( configWidget( KDialogBase * ) ), this, SLOT( slotConfigWidget( KDialogBase * ) ) );

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
  QVBox *vbox = dlg->addVBoxPage( i18n("Code Snippets") );

  SnippetSettings * w = new SnippetSettings( m_widget, vbox );

  w->btnGroup->setButton(m_widget->getSnippetConfig()->getInputMethod());
  w->leDelimiter->setText(m_widget->getSnippetConfig()->getDelimiter());
  w->cbToolTip->setChecked(m_widget->getSnippetConfig()->useToolTips());

  connect( dlg, SIGNAL(okClicked()), w, SLOT(slotOKClicked()) );
}


#include "snippet_part.moc"
