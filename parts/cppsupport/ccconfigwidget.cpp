/***************************************************************************
 *   Copyright (C) 2001 by Daniel Engelschalt                              *
 *   daniel.engelschalt@gmx.net                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// qt includes
#include <qtabwidget.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qmultilineedit.h>

// kde includes
#include <kdevproject.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kfileitem.h>
#include <kurlrequester.h>

// gideon includes
#include <domutil.h>

#include "ccconfigwidget.h"
#include "cppsupportpart.h"

using namespace std;


CCConfigWidget::CCConfigWidget( CppSupportPart* part, QWidget* parent, const char* name )
    : CCConfigWidgetBase( parent, name )
{
    m_pPart = part;
    m_bChangedCC = false;
    
    codecompletion_tab->removePage( pageCS );
    codecompletion_tab->removePage( pageCC );
    
    // initCCTab( );
    initFTTab( );
}

void
CCConfigWidget::initCCTab( )
{
    QDomDocument dom = *m_pPart->projectDom();

    // cbEnableCC->setChecked( DomUtil::readBoolEntry( dom, "/cppsupportpart/codecompletion/enablecc" ) );

    QDomElement chPart = dom.documentElement( )
                            .namedItem( "cppsupportpart" ).toElement( )
    			    .namedItem( "codecompletion" ).toElement( );    
}

void CCConfigWidget::initFTTab( )
{
    QDomDocument dom = *m_pPart->projectDom();
    bool files = DomUtil::readBoolEntry(dom, "/cppsupportpart/filetemplates/choosefiles");
    slotEnableChooseFiles(files);
    // read in template groups
    QStringList interface_files = KGlobal::dirs()->findAllResources( "data", "kdevcppsupport/templates/*.h", false, true);
    QStringList::iterator i;
    for (i=interface_files.begin();i!=interface_files.end();i++) {
      QString & file = *i;
      QString interface_file = file;
      file.remove(file.length()-2, 2);
      QString impl_filename = file + ".cpp";
      KFileItem impl_file(KFileItem::Unknown, KFileItem::Unknown, impl_filename);
      if (impl_file.isReadable()) {
        template_groups->insertItem(file);
      }
    }
    interface_url->setURL(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/interfaceURL"));
    implementation_url->setURL(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/implementationURL"));
    interface_suffix->setText(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/interfacesuffix", ".h"));
    implementation_suffix->setText(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/implementationsuffix", ".cpp"));
    lowercase_filenames->setChecked(DomUtil::readBoolEntry(dom, "/cppsupportpart/filetemplates/lowercasefilenames", true));
    interface_url->fileDialog()->setURL(KURL(*m_pPart->project()->projectDirectory()));
    implementation_url->fileDialog()->setURL(KURL(*m_pPart->project()->projectDirectory()));
}
    
CCConfigWidget::~CCConfigWidget( )
{
}


void
CCConfigWidget::accept( )
{
    saveCCTab( );
    saveCSTab( );    
    saveFTTab( );    

//    if( m_bChangedCC )
//	emit enableCodeCompletion( cbEnableCC->isChecked( ) );
}


void
CCConfigWidget::saveCSTab( )
{
    QDomDocument dom     = *m_pPart->projectDom( );
    QDomElement  element = dom.documentElement( );
    QDomElement  apPart  = element.namedItem( "cppsupportpart" ).toElement( );
			 
    if( apPart.isNull( ) ){
	apPart = dom.createElement( "cppsupportpart" );
	element.appendChild( apPart );
    }    
}


void
CCConfigWidget::saveCCTab( )
{

    QDomDocument dom     = *m_pPart->projectDom( );
    QDomElement  element = dom.documentElement( );
    QDomElement  apPart  = element.namedItem( "cppsupportpart" ).toElement( );
			 
    if( apPart.isNull( ) ){
	apPart = dom.createElement( "cppsupportpart" );
	element.appendChild( apPart );
    }

    QDomElement codecompletion = apPart.namedItem( "codecompletion" ).toElement( );
    if( codecompletion.isNull( ) ){
	codecompletion = dom.createElement( "codecompletion" );
	apPart.appendChild( codecompletion );
    }

    // DomUtil::writeBoolEntry( dom, "cppsupportpart/codecompletion/enablecc", cbEnableCC->isChecked( ) );
}


void CCConfigWidget::saveFTTab( )
{
    QDomDocument dom = *m_pPart->projectDom();
    DomUtil::writeBoolEntry(dom, "/cppsupportpart/filetemplates/choosefiles", choose_files->isChecked());
    DomUtil::writeEntry(dom, "/cppsupportpart/filetemplates/interfaceURL", interface_url->url());
    DomUtil::writeEntry(dom, "/cppsupportpart/filetemplates/implementationURL", implementation_url->url());
    DomUtil::writeEntry(dom, "/cppsupportpart/filetemplates/interfacesuffix",interface_suffix->text());
    DomUtil::writeEntry(dom, "/cppsupportpart/filetemplates/implementationsuffix",implementation_suffix->text());
    DomUtil::writeBoolEntry(dom, "/cppsupportpart/filetemplates/lowercasefilenames",lowercase_filenames->isChecked());
}

void CCConfigWidget::interfaceFile()
{
    QFile f(interface_url->url());
    if ( f.open(IO_ReadOnly) ) { 
        int size = f.size();
        char *buf = new char[size];
        f.readBlock(&buf[0], size);
        QString text = QString::fromLocal8Bit(buf, size);
	f.close();
        interface_file->setText(text);
	delete[] buf;
    }
}

void CCConfigWidget::implementationFile()
{
    QFile f(implementation_url->url());
    if ( f.open(IO_ReadOnly) ) { 
        int size = f.size();
        char *buf = new char[size];
        f.readBlock(&buf[0], size);
        QString text = QString::fromLocal8Bit(buf, size);
        f.close();
        implementation_file->setText(text);
	delete[] buf;
    }
}

void
CCConfigWidget::slotEnableCC( )
{
    kdDebug( 9007 ) << "slot EnableCC" << endl;
    m_bChangedCC = true;
}

void CCConfigWidget::slotEnableChooseFiles(bool c)
{
    choose_files->setChecked(c);
    template_groups->setEnabled(!c);
    interface_url->setEnabled(c);
    implementation_url->setEnabled(c);
}

void CCConfigWidget::slotSelectTemplateGroup( const QString & str)
{
    interface_url->setURL(str + ".h");
    interfaceFile();
    implementation_url->setURL(str + ".cpp");
    implementationFile();
}

#include "ccconfigwidget.moc"
