/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/
#include "qeditor.h"

#include <limits.h>

void GotoLineDialog::init()
{
    m_editor = 0;
    spinLineNumber->setMinValue( 1 );
    spinLineNumber->setMaxValue( INT_MAX );
}

void GotoLineDialog::destroy()
{
}

void GotoLineDialog::setEditor( QEditor* editor )
{
    m_editor = editor;
    
    if( !m_editor )
	return;
    
    int line, col;
    m_editor->getCursorPosition( &line, &col );
    spinLineNumber->setValue( line+1 );
    spinLineNumber->selectAll();
}

void GotoLineDialog::accept()
{
    m_editor->doGotoLine( spinLineNumber->value()-1 );
    QDialog::accept();
}
