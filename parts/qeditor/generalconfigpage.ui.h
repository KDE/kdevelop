/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "qeditor_part.h"

void GeneralConfigPage::init()
{
    part = 0;
    
    checkWordWrap->hide();
    checkParenthesesMatching->hide();
	   
}

void GeneralConfigPage::destroy()
{
}

void GeneralConfigPage::setPart( QEditorPart* p )
{
    part = p;

    if( !part )
	return;

    checkWordWrap->setChecked( part->wordWrap() );
    checkParenthesesMatching->setChecked( part->parenthesesMatching() );
    checkShowMarkers->setChecked( part->showMarkers() );
    checkShowLineNumber->setChecked( part->showLineNumber() );
    checkShowCodeFoldingMarkers->setChecked( part->showCodeFoldingMarkers() );
}

void GeneralConfigPage::accept()
{
    if( !part )
	return;

    part->setWordWrap( checkWordWrap->isChecked() );
    part->setParenthesesMatching( checkParenthesesMatching->isChecked() );
    part->setShowMarkers( checkShowMarkers->isChecked() );
    part->setShowLineNumber( checkShowLineNumber->isChecked() );
    part->setShowCodeFoldingMarkers( checkShowCodeFoldingMarkers->isChecked() );
}
