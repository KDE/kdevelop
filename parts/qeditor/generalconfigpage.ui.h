/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "qeditor_part.h"
#include "qeditor_settings.h"

void GeneralConfigPage::init()
{
    part = 0;
 
    checkWordWrap->setChecked( QEditorSettings::self()->wordWrap() );
    checkParenthesesMatching->setChecked( QEditorSettings::self()->parenthesesMatching() );
    checkShowMarkers->setChecked( QEditorSettings::self()->showMarkers() );
    checkShowLineNumber->setChecked( QEditorSettings::self()->showLineNumber() );
    checkShowCodeFoldingMarkers->setChecked( QEditorSettings::self()->showCodeFoldingMarkers() );
    
    //checkParenthesesMatching->hide();
}

void GeneralConfigPage::destroy()
{
}

void GeneralConfigPage::setPart( QEditorPart* p )
{
    part = p;
}

void GeneralConfigPage::accept()
{
    if( !part )
	return;

    QEditorSettings::self()->setWordWrap( checkWordWrap->isChecked() );
    QEditorSettings::self()->setParenthesesMatching( checkParenthesesMatching->isChecked() );
    QEditorSettings::self()->setShowMarkers( checkShowMarkers->isChecked() );
    QEditorSettings::self()->setShowLineNumber( checkShowLineNumber->isChecked() );
    QEditorSettings::self()->setShowCodeFoldingMarkers( checkShowCodeFoldingMarkers->isChecked() );    
}
