/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "qeditor_view.h"
#include "qeditor_part.h"

void SettingsDialog::init()
{
    m_editor = 0;
}

void SettingsDialog::destroy()
{
}

void SettingsDialog::setEditor( QEditorPart* editor )
{
    m_editor = editor;
}

void SettingsDialog::accept()
{
    QDialog::accept();
}
