/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/
#include <qcolordialog.h>
#include <qfontdatabase.h>

void RichTextFontDialog::init()
{
    QFontDatabase *db = new QFontDatabase();
    fontCombo->insertStringList( db->families() );
}

void RichTextFontDialog::selectColor()
{
    color = QColorDialog::getColor( "", this );   
    if( color.isValid() )
	colorButton->setPaletteBackgroundColor( color );
}

void RichTextFontDialog::accept()
{
    size = fontSizeCombo->currentText();
    font = fontCombo->currentText();
    done( Accepted );
}

void RichTextFontDialog::reject()
{
    done( Rejected );
}

QString RichTextFontDialog::getSize()
{
    return size;
}

QString RichTextFontDialog::getColor()
{
    return color.name();
}

QString RichTextFontDialog::getFont()
{
    return font;
}
