/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "qeditor_part.h"
#include "qeditor_view.h"
#include "qeditor_indenter.h"

void IndentConfigPage::init()
{
    part = 0;
}

void IndentConfigPage::destroy()
{
}

void IndentConfigPage::setPart( QEditorPart* p )
{
    part = p;

    if( !part )
	return;

    QEditorIndenter* indenter = part->indenter();

    if( !indenter )
	return;

    values = indenter->values();

    spinTabSize->setValue( values[ "TabSize" ].toInt() );
    spinIndentSize->setValue( values[ "IndentSize" ].toInt() );
    spinContinuationSize->setValue( values[ "ContinuationSize" ].toInt() );
    spinCommentOffset->setValue( values[ "CommentOffset" ].toInt() );
}

void IndentConfigPage::accept()
{
    if( !part )
	return;

    QEditorIndenter* indenter = part->indenter();

    if( !indenter )
	return;

    part->currentView()->setTabStop( spinTabSize->value() );

    values[ "TabSize" ] = spinTabSize->value();
    values[ "IndentSize" ] = spinIndentSize->value();
    values[ "ContinuationSize" ] = spinContinuationSize->value();
    values[ "CommentOffset" ] = spinCommentOffset->value();

    indenter->updateValues( values );
}
