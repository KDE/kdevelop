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

#include "paletteeditorimpl.h"
#include "paletteeditoradvancedimpl.h"
#include "previewframe.h"
#include "styledbutton.h"
#include "mainwindow.h"
#include "formwindow.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qpushbutton.h>

PaletteEditor::PaletteEditor( FormWindow *fw, QWidget * parent, const char * name, bool modal, WFlags f )
    : PaletteEditorBase( parent, name, modal, f ), formWindow( fw )
{
    connect( buttonHelp, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );

    editPalette =  QApplication::palette();
    setPreviewPalette( editPalette );

    buttonMainColor->setColor( editPalette.active().color( QColorGroup::Button ) );
    buttonMainColor2->setColor( editPalette.active().color( QColorGroup::Background ) );
}

PaletteEditor::~PaletteEditor()
{
}

void PaletteEditor::onTune()
{
    bool ok;
    QPalette pal = PaletteEditorAdvanced::getPalette( &ok, editPalette, backgroundMode, this, "tune_palette", formWindow);
    if (!ok) return;

    editPalette = pal;
    setPreviewPalette( editPalette );
}

void PaletteEditor::onChooseMainColor()
{
    buildPalette();
}

void PaletteEditor::onChoose2ndMainColor()
{
    buildPalette();
}

void PaletteEditor::paletteSelected(int)
{
    setPreviewPalette(editPalette);
}

QColorGroup::ColorRole PaletteEditor::centralFromItem( int item )
{
    switch( item )
	{
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
	default:
	    return QColorGroup::NColorRoles;
	}
}

QColorGroup::ColorRole PaletteEditor::effectFromItem( int item )
{
    switch( item )
	{
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

void PaletteEditor::buildPalette()
{
    int i;
    QColorGroup cg;
    QColor btn = buttonMainColor->color();
    QColor back = buttonMainColor2->color();
    QPalette automake( btn, back );

    for (i = 0; i<9; i++)
	cg.setColor( centralFromItem(i), automake.active().color( centralFromItem(i) ) );

    editPalette.setActive( cg );
    buildActiveEffect();

    cg = editPalette.inactive();

    QPalette temp( editPalette.active().color( QColorGroup::Button ),
		   editPalette.active().color( QColorGroup::Background ) );

    for (i = 0; i<9; i++)
	cg.setColor( centralFromItem(i), temp.inactive().color( centralFromItem(i) ) );

    editPalette.setInactive( cg );
    buildInactiveEffect();

    cg = editPalette.disabled();

    for (i = 0; i<9; i++)
	cg.setColor( centralFromItem(i), temp.disabled().color( centralFromItem(i) ) );

    editPalette.setDisabled( cg );
    buildDisabledEffect();

    updateStyledButtons();
}

void PaletteEditor::buildActiveEffect()
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

void PaletteEditor::buildInactive()
{
    editPalette.setInactive( editPalette.active() );
    buildInactiveEffect();
}

void PaletteEditor::buildInactiveEffect()
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

void PaletteEditor::buildDisabled()
{
    QColorGroup cg = editPalette.active();
    cg.setColor( QColorGroup::ButtonText, darkGray );
    cg.setColor( QColorGroup::Foreground, darkGray );
    editPalette.setDisabled( cg );

    buildDisabledEffect();
}

void PaletteEditor::buildDisabledEffect()
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

void PaletteEditor::setPreviewPalette( const QPalette& pal )
{
    QColorGroup cg;

    switch (paletteCombo->currentItem()) {
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

    previewFrame->setPreviewPalette(previewPalette);
}

void PaletteEditor::updateStyledButtons()
{
    buttonMainColor->setColor( editPalette.active().color( QColorGroup::Button ));
    buttonMainColor2->setColor( editPalette.active().color( QColorGroup::Background ));
}

void PaletteEditor::setPal( const QPalette& pal )
{
    editPalette = pal;
    setPreviewPalette( pal );
    updateStyledButtons();
}

QPalette PaletteEditor::pal() const
{
    return editPalette;
}

QPalette PaletteEditor::getPalette( bool *ok, const QPalette &init, BackgroundMode mode,
				    QWidget* parent, const char* name, FormWindow *fw )
{
    PaletteEditor* dlg = new PaletteEditor( fw, parent, name, TRUE );
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
