
#include "qeditor_search.h"
#include "qeditor_view.h"
#include "qeditor.h"

#include <private/qrichtext_p.h>
#include <kdebug.h>

QEditorFind::QEditorFind( QEditorView* editor, const QString& pattern, long options, QWidget* parent )
	: KoFind( pattern, options, parent ),
	  m_editor( editor ),
	  m_currentParag( 0 )
{
	connect( this, SIGNAL(highlight(const QString&, int, int, const QRect&)),
                 this, SLOT(slotHighlight(const QString&, int, int, const QRect&)) );
}

QEditorFind::~QEditorFind()
{
}

bool QEditorFind::validateMatch( const QString&, int, int )
{
	return TRUE;
}

void QEditorFind::doFind()
{
	QTextDocument* textDoc = m_editor->editor()->document();
	m_currentParag = textDoc->firstParag();
	while( m_currentParag ){
		QString str = m_currentParag->string()->toString();
		str.truncate( str.length() - 1 );
		if( ! find( str, QRect() ) ){
			return;
		}
		m_currentParag = m_currentParag->next();
	}
}

void QEditorFind::slotHighlight( const QString& text, int index, int matchedLength, const QRect& )
{
	m_editor->editor()->setSelection( m_currentParag->paragId(), index,
		m_currentParag->paragId(), index + matchedLength );
}

