/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "qeditor_view.h"
#include "qeditor_part.h"
#include "qsourcecolorizer.h"
#include <private/qrichtext_p.h>
#include <kdebug.h>
#include <kfontdialog.h>

using namespace std;

void HighlightingConfigPage::init()
{
    m_editor = 0;
}

void HighlightingConfigPage::destroy()
{
}

void HighlightingConfigPage::setEditor( QEditorPart* editor )
{
    m_editor = editor;

    if( !m_editor )
	return;

    // setup colorizer
    listElements->clear();
    QSourceColorizer* colorizer = m_editor->colorizer();
    QStringList styleList = colorizer->styleList();
    QStringList::Iterator it = styleList.begin();
    while( it != styleList.end() ){
	QTextFormat* fmt = colorizer->formatFromId( *it );
	m_map[ *it ] = qMakePair( fmt->font(), fmt->color() );
	++it;
    }
    listElements->insertStringList( styleList );
    listElements->setCurrentItem( 0 );
}

void HighlightingConfigPage::accept()
{
    QSourceColorizer* colorizer = m_editor->colorizer();
    colorizer->updateStyles( m_map );
}

void HighlightingConfigPage::slotUpdatePreview()
{
    kdDebug() << "HighlightingConfigPage::slotUpdatePreview()" << endl;

    QFont font( comboFontFamily->currentText(), spinFontSize->value() );
    font.setBold( checkBold->isChecked() );
    font.setItalic( checkItalic->isChecked() );
    font.setUnderline( checkUnderline->isChecked() );
    editPreview->setFont( font );

    QPalette pal = editPreview->palette();
    QColor color = buttonColor->color();
    pal.setColor( QPalette::Active, QColorGroup::Text,  color );
    pal.setColor( QPalette::Active, QColorGroup::Foreground, color );

    m_map[ listElements->currentText() ] = qMakePair( font, color );

    editPreview->setPalette( pal );
}


void HighlightingConfigPage::slotSelectionChanged()
{
    QString id = listElements->currentText();
    QFont font = m_map[ id ].first;
    QColor color = m_map[ id ].second;

    comboFontFamily->setCurrentFont( font.family() );
    spinFontSize->setValue( font.pointSize() );
    checkBold->setChecked( font.bold() );
    checkItalic->setChecked( font.italic() );
    checkUnderline->setChecked( font.underline() );
    buttonColor->setColor( color );
}


void HighlightingConfigPage::slotAdjustAllElements()
{
  QFont changes;
  int diffFlags = 0;
  if (KFontDialog::getFontDiff(changes, diffFlags)) {
      for(uint c=0;c<listElements->count();c++) {
	  QString id = listElements->text(c);
	  QFont font = m_map[ id ].first;
	  QColor color = m_map[ id ].second;
	  if (diffFlags & KFontChooser::FontDiffFamily)
	      font.setFamily( changes.family() );
	  if (diffFlags & KFontChooser::FontDiffStyle) {
 	      font.setWeight( changes.weight() );
	      font.setItalic( changes.italic() );
	      font.setStrikeOut( changes.strikeOut() );
	      font.setUnderline( changes.underline() );
	  }
	  if (diffFlags & KFontChooser::FontDiffSize)
	      font.setPointSize( changes.pointSize() );
	  m_map[ id ] = qMakePair(font, color);
      }
      slotSelectionChanged();
  }
}
