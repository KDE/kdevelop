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
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlistview.h>
#include <qcombobox.h>
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
#include "domutil.h"
#include "ccconfigwidget.h"
#include "cppsupportpart.h"
// c++ includes
#include <iostream>
using namespace std;


CCConfigWidget::CCConfigWidget( CppSupportPart* part, QWidget* parent, const char* name )
    : CCConfigWidgetBase( parent, name )
{
    m_pPart = part;
    m_bChangedCC = m_bChangedCH = m_bChangedCHWindow = m_bChangedPCS
                 = m_bChangedPP = m_bChangedPPPath   = false;

    initCSTab( );
    initCCTab( );
    initFTTab( );
}

void
CCConfigWidget::initCSTab( )
{
    QDomDocument dom = *m_pPart->projectDom();

    cbEnablePCS->setChecked( DomUtil::readBoolEntry( dom, "/cppsupportpart/classstore/enablepcs" ) );
    bool b = DomUtil::readBoolEntry( dom, "/cppsupportpart/classstore/enablepp" );
    cbEnablePP->setChecked( b );
    if( b == false ){
	gbPP->setEnabled( false );
        lvPCSPaths->setEnabled( false );
    }

    lePCSFile->setText( m_pPart->project( )->projectDirectory( ) + "/" +
                        m_pPart->project( )->projectName( ) + ".pcs" );

    lvPCSPaths->setSorting( -1 );

    QDomElement dir = dom.documentElement( )
			 .namedItem( "cppsupportpart" ).toElement( )
			 .namedItem( "classstore"     ).toElement( )
			 .namedItem( "preparsing"     ).toElement( )
			 .firstChild( ).toElement( );
			 
    QListViewItem* lastItem = 0;
    while( !dir.isNull( ) ){
        if( dir.tagName( ) == "directory" ){
            QListViewItem* newItem = new QListViewItem( lvPCSPaths, dir.attribute( "parsesubdir" ),
                                                                    dir.attribute( "dir" ) );
            if( lastItem )
                newItem->moveItem( lastItem );

            lastItem = newItem;
        }
        else
            cerr << "CCConfigWidget::CCConfigWidget unknown tag: '" << dir.tagName( ) << "'" << endl;

        dir = dir.nextSibling( ).toElement( );
    }
}

void
CCConfigWidget::initCCTab( )
{
    QDomDocument dom = *m_pPart->projectDom();

    cbEnableCC->setChecked( DomUtil::readBoolEntry( dom, "/cppsupportpart/codecompletion/enablecc" ) );

    QDomElement chPart = dom.documentElement( )
                            .namedItem( "cppsupportpart" ).toElement( )
    			    .namedItem( "codecompletion" ).toElement( )
			    .namedItem( "codehinting"    ).toElement( );
    
    if( !chPart.isNull( ) ){
	cbEnableCH->setChecked( chPart.attribute( "enablech" ).toInt( ) );
	rbSelectView->setChecked( chPart.attribute( "selectview" ).toInt( ) );
	rbOutputView->setChecked( chPart.attribute( "outputview").toInt( ) );
    }

    if( !cbEnableCH->isChecked( ) )
	bgCodeHinting->setEnabled( false );
    
    // for setting a default value
    if( rbSelectView->isChecked( ) == false )
	rbOutputView->setChecked( true );
}

void CCConfigWidget::initFTTab( )
{
    QDomDocument dom = *m_pPart->projectDom();
    bool files = DomUtil::readBoolEntry(dom, "/cppsupportpart/filetemplates/choosefiles");
    slotEnableChooseFiles(files);
    // read in template groups
    QStringList interface_files = KGlobal::dirs()->findAllResources( "appdata", "templates/*.h", false, true);
    kdDebug() << "** Interface file templates:";
    QStringList::iterator i;
    for (i=interface_files.begin();i!=interface_files.end();i++) {
      QString & file = *i;
      kdDebug() << file << endl;
      QString interface_file = file;
      file.remove(file.length()-2, 2);
      QString impl_filename = file + ".cpp";
      kdDebug() << impl_filename << endl;
      KFileItem impl_file(KFileItem::Unknown, KFileItem::Unknown, impl_filename);
      if (impl_file.isReadable()) {
        template_groups->insertItem(file);
        kdDebug() << "inserted " << file << endl;
      }
    }
    interface_url->setURL(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/interfaceURL"));
    implementation_url->setURL(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/implementationURL"));
    interface_suffix->setText(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/interfacesuffix", ".h"));
    implementation_suffix->setText(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/implementationsuffix", ".cpp"));
    lowercase_filenames->setChecked(DomUtil::readBoolEntry(dom, "/cppsupportpart/filetemplates/lowercasefilenames", true));
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

    bool b = cbEnablePCS->isChecked( );
    if( m_bChangedPCS )
	emit enablePersistantClassStore( b );

    // without persistance we don't allow preparsing
    if( b ){
	if( m_bChangedPP )
	    emit enablePreParsing( cbEnablePP->isChecked( ) );
	if( m_bChangedPPPath )
	    emit changedPreParsingPath( );
    }
    else
        emit enablePreParsing( false );
	
    if( m_bChangedCC )
	emit enableCodeCompletion( cbEnableCC->isChecked( ) );
	
    if( m_bChangedCH || m_bChangedCHWindow )
	emit enableCodeHinting( cbEnableCH->isChecked( ), rbOutputView->isChecked( ) );
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
    
    QDomElement classstore = apPart.namedItem( "classstore" ).toElement( );
    if( classstore.isNull( ) ){
        classstore = dom.createElement( "classstore" );
        apPart.appendChild( classstore );
    }

    DomUtil::writeBoolEntry( dom, "cppsupportpart/classstore/enablepcs", cbEnablePCS->isChecked( ) );
    DomUtil::writeBoolEntry( dom, "cppsupportpart/classstore/enablepp", cbEnablePP->isChecked( ) );

    QDomElement directories = classstore.namedItem( "preparsing" ).toElement( );
    if( directories.isNull( ) ){
        directories = dom.createElement( "preparsing" );
        classstore.appendChild( directories );
    }

    // clear old entries
    while( !directories.firstChild( ).isNull( ) )
        directories.removeChild( directories.firstChild( ) );

    // write content of the listview to dom
    QListViewItemIterator it( lvPCSPaths );
    for( ; it.current( ); ++it ){
        QDomElement dir = dom.createElement( "directory" );
        dir.setAttribute( "parsesubdir", it.current( )->text( 0 ) );
        dir.setAttribute( "dir", it.current( )->text( 1 ) );
        directories.appendChild( dir );
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

    DomUtil::writeBoolEntry( dom, "cppsupportpart/codecompletion/enablecc", cbEnableCC->isChecked( ) );

    QDomElement codehinting = codecompletion.namedItem( "codehinting" ).toElement( );
    if( codehinting.isNull( ) ){
	codehinting = dom.createElement( "codehinting" );
	codecompletion.appendChild( codehinting );
    }

    codehinting.setAttribute( "enablech"  , cbEnableCH->isChecked( ) );
    codehinting.setAttribute( "selectview", rbSelectView->isChecked( ) );
    codehinting.setAttribute( "outputview", rbOutputView->isChecked( ) );

    m_pPart->setEnableCC( cbEnableCC->isChecked( ) );
    emit enableCodeHinting( cbEnableCH->isChecked( ), rbOutputView->isChecked( ) );
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
        delete[] buf;
        f.close();
        interface_file->setText(buf);
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
        delete[] buf;
        f.close();
        implementation_file->setText(text);
    }
}


void
CCConfigWidget::slotEnableCH( bool isChecked )
{
    bgCodeHinting->setEnabled( /*cbEnableCH->isChecked( )*/ isChecked );
    m_bChangedCH = true;    
}


void
CCConfigWidget::slotEnableCC( )
{
    m_bChangedCC = true;
}


void
CCConfigWidget::slotSetCHWindow( )
{
    m_bChangedCHWindow = true;
}


void
CCConfigWidget::slotEnablePCS( bool isChecked )
{
    if( isChecked )
	gbPP->setEnabled( true );
    else
	gbPP->setEnabled( false );

    m_bChangedPCS = true;
}


void
CCConfigWidget::slotEnablePP( bool isChecked )
{
    if( isChecked )
        lvPCSPaths->setEnabled( true );
    else
        lvPCSPaths->setEnabled( false );
	
    m_bChangedPP = true;
}


void
CCConfigWidget::slotAddPPPath( )
{
    QString newDir = KFileDialog::getExistingDirectory( QString::null, 0,
                                                        i18n( "Select a directory to preparse" ) );
    if( newDir.isEmpty( ) )
	return;

    int answer = KMessageBox::questionYesNo( 0, i18n( "Include subdirectories ?" ) );
    if( answer == KMessageBox::Yes )
        ( void ) new QListViewItem( lvPCSPaths, i18n( "Yes" ), newDir );
    else
        ( void ) new QListViewItem( lvPCSPaths, i18n( "No"  ), newDir );
	
    m_bChangedPPPath = true;
}


void
CCConfigWidget::slotRemovePPPath( )
{
    QString text = lvPCSPaths->selectedItem( )->text( 1 );

//    int answer = KMessageBox::warningYesNo( 0, i18n( "Delete entry ?\n" ) + text );
//    if( answer == KMessageBox::Yes )
        delete lvPCSPaths->selectedItem( );
	
    m_bChangedPPPath = true;
}

void CCConfigWidget::slotEnableChooseFiles(bool c)
{
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
