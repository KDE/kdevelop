/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include "paletteeditoradvancedimpl.h"
#include "styledbutton.h"
#include "mainwindow.h"
#include "formwindow.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <q3listbox.h>
#include <qpainter.h>
#include <q3groupbox.h>
//Added by qt3to4:
#include <QPixmap>

/*!
    Class used by PaletteEditor for bold combobox items
*/

class BoldListBoxText : public Q3ListBoxText
{
public:
    BoldListBoxText( QString text, Q3ListBox* lb = 0 );

protected:
    virtual void paint( QPainter* );
};

BoldListBoxText::BoldListBoxText( QString text, Q3ListBox* lb )
    : Q3ListBoxText( lb )
{
    setText( text );
}

void BoldListBoxText::paint( QPainter* painter )
{
    QFont f = painter->font();
    f.setBold( TRUE );
    painter->setFont( f );

    Q3ListBoxText::paint( painter );
}

PaletteEditorAdvanced::PaletteEditorAdvanced( FormWindow *fw, QWidget * parent,
					      const char * name, bool modal, Qt::WFlags f )
    : PaletteEditorAdvancedBase( parent, name, modal, f ), formWindow( fw ), selectedPalette(0)
{
    connect( buttonHelp, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );

    buttonPixmap->setEditor( StyledButton::PixmapEditor );

    editPalette = QApplication::palette();
    setPreviewPalette( editPalette );

    buttonPixmap->setFormWindow( formWindow );

    // force toggle event
    checkBuildEffect->setChecked(FALSE);
    checkBuildEffect->setChecked(TRUE);
}

PaletteEditorAdvanced::~PaletteEditorAdvanced()
{
}

void PaletteEditorAdvanced::onToggleBuildInactive( bool v )
{
    if (selectedPalette == 1) {
	groupCentral->setDisabled(v);
	groupEffect->setDisabled(v);
    }

    if (v) {
	buildInactive();
	updateStyledButtons();
    }
}

void PaletteEditorAdvanced::onToggleBuildDisabled( bool v )
{
    if (selectedPalette == 2) {
	groupCentral->setDisabled(v);
	groupEffect->setDisabled(v);
    }

    if (v) {
	buildDisabled();
	updateStyledButtons();
    }
}

void PaletteEditorAdvanced::paletteSelected(int p)
{
    selectedPalette = p;

    if(p == 1) { // inactive
	groupCentral->setDisabled(checkBuildInactive->isChecked());
	groupEffect->setDisabled(checkBuildInactive->isChecked());
    }
    else if (p == 2) { // disabled
	groupCentral->setDisabled(checkBuildDisabled->isChecked());
	groupEffect->setDisabled(checkBuildDisabled->isChecked());
    }
    else {
	groupCentral->setEnabled(TRUE);
	groupEffect->setEnabled(TRUE);
    }
    updateStyledButtons();
}

void PaletteEditorAdvanced::onChooseCentralColor()
{
    switch(selectedPalette) {
    case 0:
    default:
	mapToActiveCentralRole( buttonCentral->color() );
	break;
    case 1:
	mapToInactiveCentralRole( buttonCentral->color() );
	break;
    case 2:
	mapToDisabledCentralRole( buttonCentral->color() );
	break;
    }
    updateStyledButtons();
}

void PaletteEditorAdvanced::onChooseEffectColor()
{
    switch(selectedPalette) {
    case 0:
    default:
	mapToActiveEffectRole( buttonEffect->color() );
	break;
    case 1:
	mapToInactiveEffectRole( buttonEffect->color() );
	break;
    case 2:
	mapToDisabledEffectRole( buttonEffect->color() );
	break;
    }
    updateStyledButtons();
}

void PaletteEditorAdvanced::onChoosePixmap()
{
    if (buttonPixmap->pixmap()) {
	switch(selectedPalette) {
	case 0:
	default:
	    mapToActivePixmapRole( *buttonPixmap->pixmap() );
	    break;
	case 1:
	    mapToInactivePixmapRole( *buttonPixmap->pixmap() );
	    break;
	case 2:
	    mapToDisabledPixmapRole( *buttonPixmap->pixmap() );
	    break;
	}
    }
    updateStyledButtons();
}

void PaletteEditorAdvanced::onToggleBuildEffects( bool on )
{
    if (!on) return;
    buildActiveEffect();
    buildInactiveEffect();
    buildDisabledEffect();
}

QColorGroup::ColorRole PaletteEditorAdvanced::centralFromItem( int item )
{
    switch( item ) {
        case 0:
            return QColorGroup::Background;
        case 1:
            return QColorGroup::Foreground;
        case 2:
            return QColorGroup::Button;
        case 3:
            return QColorGroup::Base;
        case 4:
            return QColorGroup::Text;
        case 5:
            return QColorGroup::BrightText;
        case 6:
            return QColorGroup::ButtonText;
        case 7:
            return QColorGroup::Highlight;
        case 8:
            return QColorGroup::HighlightedText;
        case 9:
            return QColorGroup::Link;
        case 10:
            return QColorGroup::LinkVisited;
        default:
            return QColorGroup::NColorRoles;
    }
}

QColorGroup::ColorRole PaletteEditorAdvanced::effectFromItem( int item )
{
    switch( item ) {
    case 0:
	return QColorGroup::Light;
    case 1:
	return QColorGroup::Midlight;
    case 2:
	return QColorGroup::Mid;
    case 3:
	return QColorGroup::Dark;
    case 4:
	return QColorGroup::Shadow;
    default:
	return QColorGroup::NColorRoles;
    }
}

void PaletteEditorAdvanced::onCentral( int item )
{
    QPixmap* p = 0;
    QColor c;

    switch(selectedPalette) {
    case 0:
    default:
	c = editPalette.active().color( centralFromItem(item) );
	p = editPalette.active().brush( centralFromItem(item) ).pixmap();
	break;
    case 1:
	c = editPalette.inactive().color( centralFromItem(item) );
	p = editPalette.inactive().brush( centralFromItem(item) ).pixmap();
	break;
    case 2:
	c =  editPalette.disabled().color( centralFromItem(item) );
	p = editPalette.disabled().brush( centralFromItem(item) ).pixmap();
	break;
    }

    buttonCentral->setColor(c);

    if (p)
	buttonPixmap->setPixmap( *p );
    else
	buttonPixmap->setPixmap( QPixmap() );
}

void PaletteEditorAdvanced::onEffect( int item )
{
    QColor c;
    switch(selectedPalette) {
    case 0:
    default:
	c = editPalette.active().color( effectFromItem(item) );
	break;
    case 1:
	editPalette.inactive().color( effectFromItem(item) );
	break;
    case 2:
	editPalette.disabled().color( effectFromItem(item) );
	break;
    }
    buttonEffect->setColor(c);
}

void PaletteEditorAdvanced::mapToActiveCentralRole( const QColor& c )
{
    QColorGroup cg = editPalette.active();
    cg.setColor( centralFromItem(comboCentral->currentItem()), c );
    editPalette.setActive( cg );

    buildActiveEffect();
    if(checkBuildInactive->isChecked())
	buildInactive();
    if(checkBuildDisabled->isChecked())
	buildDisabled();

    setPreviewPalette( editPalette );
}

void PaletteEditorAdvanced::mapToActiveEffectRole( const QColor& c )
{
    QColorGroup cg = editPalette.active();
    cg.setColor( effectFromItem(comboEffect->currentItem()), c );
    editPalette.setActive( cg );

    if(checkBuildInactive->isChecked())
	buildInactive();
    if(checkBuildDisabled->isChecked())
	buildDisabled();

    setPreviewPalette( editPalette );
}

void PaletteEditorAdvanced::mapToActivePixmapRole( const QPixmap& pm )
{
    QColorGroup::ColorRole role = centralFromItem(comboCentral->currentItem());
    QColorGroup cg = editPalette.active();
    if (  !pm.isNull()  )
	cg.setBrush( role, QBrush( cg.color( role ), pm ) );
    else
	cg.setBrush( role, QBrush( cg.color( role ) ) );
    editPalette.setActive( cg );


    buildActiveEffect();
    if(checkBuildInactive->isChecked())
	buildInactive();
    if(checkBuildDisabled->isChecked())
	buildDisabled();

    setPreviewPalette( editPalette );
}

void PaletteEditorAdvanced::mapToInactiveCentralRole( const QColor& c )
{
    QColorGroup cg = editPalette.inactive();
    cg.setColor( centralFromItem(comboCentral->currentItem()), c );
    editPalette.setInactive( cg );

    buildInactiveEffect();

    setPreviewPalette( editPalette );
}

void PaletteEditorAdvanced::mapToInactiveEffectRole( const QColor& c )
{
    QColorGroup cg = editPalette.inactive();
    cg.setColor( effectFromItem(comboEffect->currentItem()), c );
    editPalette.setInactive( cg );

    setPreviewPalette( editPalette );
}

void PaletteEditorAdvanced::mapToInactivePixmapRole( const QPixmap& pm )
{
    QColorGroup::ColorRole role = centralFromItem(comboCentral->currentItem());
    QColorGroup cg = editPalette.inactive();
    if (  !pm.isNull()  )
	cg.setBrush( role, QBrush( cg.color( role ), pm ) );
    else
	cg.setBrush( role, QBrush( cg.color( role ) ) );
    editPalette.setInactive( cg );

    setPreviewPalette( editPalette );
}

void PaletteEditorAdvanced::mapToDisabledCentralRole( const QColor& c )
{
    QColorGroup cg = editPalette.disabled();
    cg.setColor( centralFromItem(comboCentral->currentItem()), c );
    editPalette.setDisabled( cg );

    buildDisabledEffect();

    setPreviewPalette( editPalette );
}

void PaletteEditorAdvanced::mapToDisabledEffectRole( const QColor& c )
{
    QColorGroup cg = editPalette.disabled();
    cg.setColor( effectFromItem(comboEffect->currentItem()), c );
    editPalette.setDisabled( cg );

    setPreviewPalette( editPalette );
}

void PaletteEditorAdvanced::mapToDisabledPixmapRole( const QPixmap& pm )
{
    QColorGroup::ColorRole role = centralFromItem(comboCentral->currentItem());
    QColorGroup cg = editPalette.disabled();
    if (  !pm.isNull()  )
	cg.setBrush( role, QBrush( cg.color( role ), pm ) );
    else
	cg.setBrush( role, QBrush( cg.color( role ) ) );

    editPalette.setDisabled( cg );

    setPreviewPalette( editPalette );
}

void PaletteEditorAdvanced::buildActiveEffect()
{
    QColorGroup cg = editPalette.active();
    QColor btn = cg.color( QColorGroup::Button );

    QPalette temp( btn, btn );

    for (int i = 0; i<5; i++)
	cg.setColor( effectFromItem(i), temp.active().color( effectFromItem(i) ) );

    editPalette.setActive( cg );
    setPreviewPalette( editPalette );

    updateStyledButtons();
}

void PaletteEditorAdvanced::buildInactive()
{
    editPalette.setInactive( editPalette.active() );
    buildInactiveEffect();
}

void PaletteEditorAdvanced::buildInactiveEffect()
{
    QColorGroup cg = editPalette.inactive();

    QColor light, midlight, mid, dark, shadow;
    QColor btn = cg.color( QColorGroup::Button );

    light = btn.light(150);
    midlight = btn.light(115);
    mid = btn.dark(150);
    dark = btn.dark();
    shadow = black;

    cg.setColor( QColorGroup::Light, light );
    cg.setColor( QColorGroup::Midlight, midlight );
    cg.setColor( QColorGroup::Mid, mid );
    cg.setColor( QColorGroup::Dark, dark );
    cg.setColor( QColorGroup::Shadow, shadow );

    editPalette.setInactive( cg );
    setPreviewPalette( editPalette );
    updateStyledButtons();
}

void PaletteEditorAdvanced::buildDisabled()
{
    QColorGroup cg = editPalette.active();
    cg.setColor( QColorGroup::ButtonText, darkGray );
    cg.setColor( QColorGroup::Foreground, darkGray );
    editPalette.setDisabled( cg );

    buildDisabledEffect();
}

void PaletteEditorAdvanced::buildDisabledEffect()
{
    QColorGroup cg = editPalette.disabled();

    QColor light, midlight, mid, dark, shadow;
    QColor btn = cg.color( QColorGroup::Button );

    light = btn.light(150);
    midlight = btn.light(115);
    mid = btn.dark(150);
    dark = btn.dark();
    shadow = black;

    cg.setColor( QColorGroup::Light, light );
    cg.setColor( QColorGroup::Midlight, midlight );
    cg.setColor( QColorGroup::Mid, mid );
    cg.setColor( QColorGroup::Dark, dark );
    cg.setColor( QColorGroup::Shadow, shadow );

    editPalette.setDisabled( cg );
    setPreviewPalette( editPalette );
    updateStyledButtons();
}

void PaletteEditorAdvanced::setPreviewPalette( const QPalette& pal )
{
    QColorGroup cg;

    switch (selectedPalette) {
    case 0:
    default:
	cg = pal.active();
	break;
    case 1:
	cg = pal.inactive();
	break;
    case 2:
	cg = pal.disabled();
	break;
    }
    previewPalette.setActive( cg );
    previewPalette.setInactive( cg );
    previewPalette.setDisabled( cg );
}

void PaletteEditorAdvanced::updateStyledButtons()
{
    QColor central, effect;
    QPixmap *pm = 0;
    switch (selectedPalette) {
    case 0:
    default:
	central = editPalette.active().color( centralFromItem( comboCentral->currentItem() ) );
	effect = editPalette.active().color( effectFromItem( comboEffect->currentItem() ) );
	pm = editPalette.active().brush( centralFromItem( comboCentral->currentItem() ) ).pixmap();
	break;
    case 1:
	central = editPalette.inactive().color( centralFromItem( comboCentral->currentItem() ) );
	effect = editPalette.inactive().color( effectFromItem( comboEffect->currentItem() ) );
	pm = editPalette.inactive().brush( centralFromItem( comboCentral->currentItem() ) ).pixmap();
	break;
    case 2:
	central = editPalette.disabled().color( centralFromItem( comboCentral->currentItem() ) );
	effect = editPalette.disabled().color( effectFromItem( comboEffect->currentItem() ) );
	pm = editPalette.disabled().brush( centralFromItem( comboCentral->currentItem() ) ).pixmap();
	break;
    }

    buttonCentral->setColor(central);
    buttonEffect->setColor(effect);
    if ( pm && !pm->isNull() )
	buttonPixmap->setPixmap( *pm );
    else
	buttonPixmap->setPixmap(QPixmap());
}

void PaletteEditorAdvanced::setPal( const QPalette& pal )
{
    editPalette = pal;
    setPreviewPalette( pal );
    updateStyledButtons();
}

QPalette PaletteEditorAdvanced::pal() const
{
    return editPalette;
}

void PaletteEditorAdvanced::setupBackgroundMode( Qt::BackgroundMode mode )
{
    int initRole = 0;

    switch( mode ) {
    case Qt::PaletteBackground:
	initRole = 0;
	break;
    case Qt::PaletteForeground:
	initRole = 1;
	break;
    case Qt::PaletteButton:
	initRole = 2;
	break;
    case Qt::PaletteBase:
	initRole = 3;
	break;
    case Qt::PaletteText:
	initRole = 4;
	break;
    case Qt::PaletteBrightText:
	initRole = 5;
	break;
    case Qt::PaletteButtonText:
	initRole = 6;
	break;
    case Qt::PaletteHighlight:
	initRole = 7;
	break;
    case Qt::PaletteHighlightedText:
	initRole = 8;
	break;
    case Qt::PaletteLight:
	initRole = 9;
	break;
    case Qt::PaletteMidlight:
	initRole = 10;
	break;
    case Qt::PaletteDark:
	initRole = 11;
	break;
    case Qt::PaletteMid:
	initRole = 12;
	break;
    case Qt::PaletteShadow:
	initRole = 13;
	break;
    default:
	initRole = -1;
	break;
    }

    if ( initRole <= -1 ) return;

    if (initRole > 8 ) {
	comboEffect->setCurrentItem( initRole - 9 );
	if ( comboEffect->listBox() ) {
	    QString text = comboEffect->currentText();
	    comboEffect->listBox()->changeItem( new BoldListBoxText( text ), initRole - 9 );
	}
    }
    else {
	comboCentral->setCurrentItem( initRole );
	if ( comboCentral->listBox() ) {
	    QString text = comboCentral->currentText();
	    comboCentral->listBox()->changeItem( new BoldListBoxText( text ), initRole );
	}
    }
}

QPalette PaletteEditorAdvanced::getPalette( bool *ok, const QPalette &init,
					    Qt::BackgroundMode mode, QWidget* parent,
					    const char* name, FormWindow *fw )
{
    PaletteEditorAdvanced* dlg = new PaletteEditorAdvanced( fw, parent, name, TRUE );
    dlg->setupBackgroundMode( mode );

    if ( init != QPalette() )
        dlg->setPal( init );
    int resultCode = dlg->exec();

    QPalette result = init;
    if ( resultCode == QDialog::Accepted ) {
	if ( ok )
	    *ok = TRUE;
	result = dlg->pal();
    } else {
	if ( ok )
	    *ok = FALSE;
    }
    delete dlg;
    return result;
}
