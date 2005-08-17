#include <klocale.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3ValueList>
/**********************************************************************
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

void PixmapCollectionEditor::init()
{
    project = 0;
    setChooserMode( FALSE );
}

void PixmapCollectionEditor::destroy()
{
}

void PixmapCollectionEditor::addPixmap()
{
    if ( !project )
	return;

    QString f;
    QStringList pixmaps = qChoosePixmaps( this );
    if ( pixmaps.isEmpty() )
	return;

    QString lastName;
    for ( QStringList::ConstIterator it = pixmaps.begin(); it != pixmaps.end(); ++it ) {
	QPixmap pm( *it );
	if ( pm.isNull() )
	    continue;
	PixmapCollection::Pixmap pixmap;
	pixmap.pix = pm;
	QFileInfo fi ( *it );
	pixmap.name = fi.fileName();
	pixmap.absname = fi.filePath();
	if ( !project->pixmapCollection()->addPixmap( pixmap, FALSE ) )
	    continue;
	lastName = pixmap.name;
    }

    updateView();
    Q3IconViewItem *item = viewPixmaps->findItem( lastName );
    if ( item ) {
	viewPixmaps->setCurrentItem( item );
	viewPixmaps->ensureItemVisible( item );
    }

}

void PixmapCollectionEditor::removePixmap()
{
    if ( !project || !viewPixmaps->currentItem() )
	return;
    project->pixmapCollection()->removePixmap( viewPixmaps->currentItem()->text() );
    updateView();
}

void PixmapCollectionEditor::updateView()
{
    if ( !project )
	return;

    viewPixmaps->clear();

    Q3ValueList<PixmapCollection::Pixmap> pixmaps = project->pixmapCollection()->pixmaps();
    for ( Q3ValueList<PixmapCollection::Pixmap>::Iterator it = pixmaps.begin(); it != pixmaps.end(); ++it ) {
	// #### might need to scale down the pixmap
	Q3IconViewItem *item = new Q3IconViewItem( viewPixmaps, (*it).name, scaledPixmap( (*it).pix ) );
	//item->setRenameEnabled( TRUE ); // this will be a bit harder to implement
	item->setDragEnabled( FALSE );
	item->setDropEnabled( FALSE );
    }
    viewPixmaps->setCurrentItem( viewPixmaps->firstItem() );
    currentChanged( viewPixmaps->firstItem() );
}

void PixmapCollectionEditor::currentChanged( Q3IconViewItem * i )
{
    buttonOk->setEnabled( !!i );
}

void PixmapCollectionEditor::setChooserMode( bool c )
{
    chooser = c;
    if ( chooser ) {
	buttonClose->hide();
	buttonOk->show();
	buttonCancel->show();
	buttonOk->setEnabled( FALSE );
	buttonOk->setDefault( TRUE );
	connect( viewPixmaps, SIGNAL( doubleClicked( Q3IconViewItem * ) ), buttonOk, SIGNAL( clicked() ) );
	connect( viewPixmaps, SIGNAL( returnPressed( Q3IconViewItem * ) ), buttonOk, SIGNAL( clicked() ) );
	setCaption( i18n( "Choose Image" ) );
    } else {
	buttonClose->show();
	buttonOk->hide();
	buttonCancel->hide();
	buttonClose->setDefault( TRUE );
    }
    updateView();
}

void PixmapCollectionEditor::setCurrentItem( const QString & name )
{
    Q3IconViewItem *i = viewPixmaps->findItem( name );
    if ( i ) {
	viewPixmaps->setCurrentItem( i );
	currentChanged( i );
    }
}

void PixmapCollectionEditor::setProject( Project * pro )
{
    project = pro;
    updateView();
}

QPixmap PixmapCollectionEditor::scaledPixmap( const QPixmap & p )
{
    QPixmap pix( p );
    if ( pix.width() < 50 && pix.height() < 50 )
	return pix;
    QImage img;
    img = pix;
    img = img.smoothScale( 50, 50 );
    pix.convertFromImage( img );
    return pix;
}
