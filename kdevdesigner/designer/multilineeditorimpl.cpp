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

#include "multilineeditorimpl.h"
#include "formwindow.h"
#include "command.h"
#include "mainwindow.h"
#include "richtextfontdialog.h"
#include "syntaxhighlighter_html.h"
#include "widgetfactory.h"

#include <kiconloader.h>
#include "kdevdesigner_part.h"

#include <qtextedit.h>
#include <./private/qrichtext_p.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qapplication.h>
#include <qaction.h>
#include <qmetaobject.h>

#include <klocale.h>

ToolBarItem::ToolBarItem( QWidget *parent, QWidget *toolBar,
			  const QString &label, const QString &tagstr,
			  const QIconSet &icon, const QKeySequence &key )
    : QAction( parent )
{
    setIconSet( icon );
    setText( label );
    setAccel( key );
    addTo( toolBar );
    tag = tagstr;
    connect( this, SIGNAL( activated() ), this, SLOT( wasActivated() ) );
}

ToolBarItem::~ToolBarItem()
{

}

void ToolBarItem::wasActivated()
{
    emit clicked( tag );
}

TextEdit::TextEdit( QWidget *parent, const char *name )
    : QTextEdit( parent, name )
{
    setTextFormat( Qt::PlainText );
}

QTextParagraph* TextEdit::paragraph()
{
    QTextCursor *tc = new QTextCursor( QTextEdit::document() );
    return tc->paragraph();
}


MultiLineEditor::MultiLineEditor( bool call_static, bool richtextMode, QWidget *parent, QWidget *editWidget,
				  FormWindow *fw, const QString &text )
    : MultiLineEditorBase( parent, 0, WType_Dialog | WShowModal ), formwindow( fw ), doWrap( FALSE )
{
    callStatic = call_static;

    if ( callStatic )
	applyButton->hide();

    textEdit = new TextEdit( centralWidget(), "textedit" );
    Layout4->insertWidget( 0, textEdit );

    if ( richtextMode ) {
	QPopupMenu *stylesMenu = new QPopupMenu( this );
	menuBar->insertItem( i18n( "&Styles" ), stylesMenu );

	basicToolBar = new QToolBar( i18n( "Basics" ), this, DockTop );

	ToolBarItem *it = new ToolBarItem( this, basicToolBar, i18n( "Italic" ),
					   "i", BarIcon( "designer_textitalic.png", KDevDesignerPartFactory::instance() ), CTRL+Key_I );
	it->addTo( stylesMenu );
	connect( it, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *b = new ToolBarItem( this, basicToolBar, i18n( "Bold" ),
					  "b", BarIcon( "designer_textbold.png", KDevDesignerPartFactory::instance() ), CTRL+Key_B );
	b->addTo( stylesMenu );
	connect( b, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *ul = new ToolBarItem( this, basicToolBar, i18n( "Underline" ),
					   "u", BarIcon( "designer_textunderline.png" , KDevDesignerPartFactory::instance()), CTRL+Key_U );
	ul->addTo( stylesMenu );
	connect( ul, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *tt = new ToolBarItem( this, basicToolBar, i18n( "Typewriter" ),
					   "tt", BarIcon( "designer_textteletext.png", KDevDesignerPartFactory::instance() ) );
	tt->addTo( stylesMenu );
	connect( tt, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	basicToolBar->addSeparator();

	QPopupMenu *layoutMenu = new QPopupMenu( this );
	menuBar->insertItem( i18n( "&Layout" ), layoutMenu );

	QAction *brAction = new QAction( this );
	brAction->setIconSet( BarIcon( "designer_textlinebreak.png", KDevDesignerPartFactory::instance() ) );
	brAction->setText( i18n("Break" ) );
	brAction->addTo( basicToolBar );
	brAction->addTo( layoutMenu );
	connect( brAction, SIGNAL( activated() ) , this, SLOT( insertBR() ) );

	ToolBarItem *p = new ToolBarItem( this, basicToolBar, i18n( "Paragraph" ),
					  "p", BarIcon( "designer_textparagraph.png", KDevDesignerPartFactory::instance() ) );
	p->addTo( layoutMenu );
	connect( p, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));
	layoutMenu->insertSeparator();
	basicToolBar->addSeparator();

	ToolBarItem *al = new ToolBarItem( this, basicToolBar, i18n( "Align left" ),
					   "p align=\"left\"", BarIcon( "designer_textleft.png", KDevDesignerPartFactory::instance() ) );
	al->addTo( layoutMenu );
	connect( al, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *ac = new ToolBarItem( this, basicToolBar, i18n( "Align center" ),
					   "p align=\"center\"", BarIcon( "designer_textcenter.png", KDevDesignerPartFactory::instance() ) );
	ac->addTo( layoutMenu );
	connect( ac, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *ar = new ToolBarItem( this, basicToolBar, i18n( "Align right" ),
					   "p align=\"right\"", BarIcon( "designer_textright.png", KDevDesignerPartFactory::instance() ) );
	ar->addTo( layoutMenu );
	connect( ar, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *block = new ToolBarItem( this, basicToolBar, i18n( "Blockquote" ),
					      "blockquote", BarIcon( "designer_textjustify.png", KDevDesignerPartFactory::instance() ) );
	block->addTo( layoutMenu );
	connect( block, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));


	QPopupMenu *fontMenu = new QPopupMenu( this );
	menuBar->insertItem( i18n( "&Font" ), fontMenu );

	fontToolBar = new QToolBar( "Fonts", this, DockTop );

	QAction *fontAction = new QAction( this );
	fontAction->setIconSet( BarIcon( "designer_textfont.png", KDevDesignerPartFactory::instance() ) );
	fontAction->setText( i18n("Font" ) );
	fontAction->addTo( fontToolBar );
	fontAction->addTo( fontMenu );
	connect( fontAction, SIGNAL( activated() ) , this, SLOT( showFontDialog() ) );


	ToolBarItem *fp1 = new ToolBarItem( this, fontToolBar, i18n( "Fontsize +1" ),
					    "font size=\"+1\"", BarIcon( "designer_textlarger.png", KDevDesignerPartFactory::instance() ) );
	connect( fp1, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *fm1 = new ToolBarItem( this, fontToolBar, i18n( "Fontsize -1" ),
					    "font size=\"-1\"", BarIcon( "designer_textsmaller.png", KDevDesignerPartFactory::instance() ) );
	connect( fm1, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *h1 = new ToolBarItem( this, fontToolBar, i18n( "Headline 1" ),
					   "h1", BarIcon( "designer_texth1.png", KDevDesignerPartFactory::instance() ) );
	connect( h1, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *h2 = new ToolBarItem( this, fontToolBar, i18n( "Headline 2" ),
					   "h2", BarIcon( "designer_texth2.png", KDevDesignerPartFactory::instance()  ) );
	connect( h2, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	ToolBarItem *h3 = new ToolBarItem( this, fontToolBar, i18n( "Headline 3" ),
					   "h3", BarIcon( "designer_texth3.png", KDevDesignerPartFactory::instance() ) );
	connect( h3, SIGNAL( clicked( const QString& ) ),
		 this, SLOT( insertTags( const QString& )));

	QPopupMenu *optionsMenu = new QPopupMenu( this );
	menuBar->insertItem( i18n( "O&ptions" ), optionsMenu );

	optionsToolBar = new QToolBar( "Options", this, DockTop );
	wrapAction = new QAction( this );
	wrapAction->setToggleAction( TRUE );
	wrapAction->setIconSet( BarIcon( "designer_wordwrap.png", KDevDesignerPartFactory::instance() ) );
	wrapAction->setText( i18n( "Word Wrapping" ) );
	wrapAction->addTo( optionsToolBar );
	wrapAction->addTo( optionsMenu );
	connect( wrapAction, SIGNAL( toggled( bool ) ), this, SLOT( changeWrapMode( bool ) ) );

	oldDoWrap = doWrap;
	wrapAction->setOn( doWrap );

	connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
	textEdit->document()->setFormatter( new QTextFormatterBreakInWords );
	textEdit->document()->setUseFormatCollection( FALSE );
	textEdit->document()->setPreProcessor( new SyntaxHighlighter_HTML );

	if ( !callStatic && ::qt_cast<QTextEdit*>(editWidget) ) {
	    mlined = (QTextEdit*)editWidget;
	    mlined->setReadOnly( TRUE );

	    const QMetaProperty *wordWrap = mlined->metaObject()->property(
					    mlined->metaObject()->findProperty( "wordWrap", TRUE ), TRUE );
	    oldWrapMode = 0;
	    oldWrapString = "NoWrap";
	    if ( wordWrap ) {
		oldWrapMode = mlined->property( "wordWrap" );
		oldWrapString = QString( wordWrap->valueToKey( oldWrapMode.toInt() ) );
		if ( oldWrapString != "NoWrap" )
		    doWrap = TRUE;
	    }
	    textEdit->setAlignment( mlined->alignment() );
	    textEdit->setWordWrap( mlined->wordWrap() );
	    textEdit->setWrapColumnOrWidth( mlined->wrapColumnOrWidth() );
	    textEdit->setWrapPolicy( mlined->wrapPolicy() );
	    textEdit->setText( mlined->text() );
	    if ( !mlined->text().isEmpty() )
		textEdit->selectAll();
	} else {
	    textEdit->setText( text );
	    textEdit->selectAll();
	}
    } else {
	textEdit->setText( text );
	textEdit->selectAll();
    }
    textEdit->setFocus();
}

int MultiLineEditor::exec()
{
    res = 1;
    show();
    qApp->enter_loop();
    return res;
}

void MultiLineEditor::okClicked()
{
    applyClicked();
    close();
}

void MultiLineEditor::applyClicked()
{
    if ( !callStatic ) {
	PopulateMultiLineEditCommand *cmd = new PopulateMultiLineEditCommand( i18n( "Set the text of '%1'" ).arg( mlined->name() ),
						formwindow, mlined, textEdit->text() );
	cmd->execute();
	formwindow->commandHistory()->addCommand( cmd );

	if ( oldDoWrap != doWrap ) {
	    QString pn( i18n( "Set 'wordWrap' of '%2'" ).arg( mlined->name() ) );
	    SetPropertyCommand *propcmd;
	    if ( doWrap )
		propcmd = new SetPropertyCommand( pn, formwindow,
					mlined, MainWindow::self->propertyeditor(),
					"wordWrap", WidgetFactory::property( mlined, "wordWrap" ),
					QVariant( 1 ), "WidgetWidth", oldWrapString );
	    else
		propcmd = new SetPropertyCommand( pn, formwindow,
					mlined, MainWindow::self->propertyeditor(),
					"wordWrap", WidgetFactory::property( mlined, "wordWrap" ),
					QVariant( 0 ), "NoWrap", oldWrapString );

	    propcmd->execute();
	    formwindow->commandHistory()->addCommand( propcmd, TRUE );
	}
	textEdit->setFocus();
    }
    else {
	staticText = textEdit->text();
    }
}

void MultiLineEditor::cancelClicked()
{
    res = 0;
    close();
}

void MultiLineEditor::closeEvent( QCloseEvent *e )
{
    qApp->exit_loop();
    MultiLineEditorBase::closeEvent( e );
}

void MultiLineEditor::insertTags( const QString &tag )
{
    int pfrom, pto, ifrom, ito;
    QString tagend(  tag.simplifyWhiteSpace() );
    tagend.remove( tagend.find( ' ', 0 ), tagend.length() );
    if ( textEdit->hasSelectedText() ) {
	textEdit->getSelection( &pfrom, &ifrom, &pto, &ito );
	QString buf = textEdit->selectedText();
	buf = QString( "<%1>%3</%2>" ).arg( tag ).arg( tagend ).arg( buf );
	textEdit->removeSelectedText();
	textEdit->insertAt( buf, pfrom, ifrom );
	textEdit->setCursorPosition( pto, ito + 2 + tag.length() );
    }
    else {
	int para, index;
	textEdit->getCursorPosition( &para, &index );
	textEdit->insert( QString( "<%1></%2>" ).arg( tag ).arg( tagend ) );
	index += 2 + tag.length();
	textEdit->setCursorPosition( para, index  );
    }
}

void MultiLineEditor::insertBR()
{
    textEdit->insert( "<br>" );
}

void MultiLineEditor::showFontDialog()
{
    bool selText = FALSE;
    int pfrom, pto, ifrom, ito;
    if ( textEdit->hasSelectedText() ) {
	textEdit->getSelection( &pfrom, &ifrom, &pto, &ito );
	selText = TRUE;
    }
    RichTextFontDialog *fd = new RichTextFontDialog( this );
    if ( fd->exec() == QDialog::Accepted ) {
	QString size, font;
	if ( fd->getSize() != "0" )
	    size = "size=\"" + fd->getSize() + "\"";
	QString color;
	if ( !fd->getColor().isEmpty() && fd->getColor() != "#000000" )
	    color = "color=\"" + fd->getColor() + "\"";
	if ( fd->getFont() != "default" )
	    font = "face=\"" + fd->getFont() + "\"";
	QString tag( QString( "font %1 %2 %3" )
	             .arg( color ).arg( size ).arg( font ) );

	if ( selText )
	    textEdit->setSelection( pfrom, ifrom, pto, ito );
	insertTags( tag.simplifyWhiteSpace() );
    }
    else if ( selText )
	textEdit->setSelection( pfrom, ifrom, pto, ito );
}

QString MultiLineEditor::getStaticText()
{
    return staticText.stripWhiteSpace();
}

QString MultiLineEditor::getText( QWidget *parent, const QString &text, bool richtextMode, bool *useWrap )
{
    MultiLineEditor medit( TRUE, richtextMode, parent, 0, 0, text );
    if ( richtextMode )
	medit.setUseWrapping( *useWrap );
    if ( medit.exec() == QDialog::Accepted ) {
	*useWrap = medit.useWrapping();
	return medit.getStaticText();
    }
    return QString::null;
}

void MultiLineEditor::changeWrapMode( bool b )
{
    doWrap = b;
    if ( doWrap && !callStatic ) {
	if ( oldDoWrap )
	    textEdit->setProperty( "wordWrap", oldWrapMode );
	else
	    textEdit->setWordWrap( QTextEdit::WidgetWidth );
    } else {
	textEdit->setWordWrap( QTextEdit::NoWrap );
    }
}

bool MultiLineEditor::useWrapping() const
{
    return doWrap;
}

void MultiLineEditor::setUseWrapping( bool b )
{
    doWrap = b;
    wrapAction->setOn( doWrap );
}
