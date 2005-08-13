/*
 *  File : snippetdlg.ui.h
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <kmessagebox.h>

void SnippetDlg::slotHelp()
{
    KMessageBox::information(this, i18n("To use variables in a snippet, you just have to enclose the variablename with $-characters. When you use the snippet, you will then be asked for a value for this variable. \nExample snippet: This is a $VAR$\nWhen you use this snippet you will be prompted for a value for the variable $VAR$. Any occourences of $VAR$ will then be replaced with whatever you've entered.\nIf you need a single $-character in a snippet, which is not used to enclose a variable, type $$(two dollar characters) instead. They will automatically be replaced with a single $-character when you use the snippet.\nIf you want to change the default delimiter to anything different, please use the settings dialog to do so."), i18n("Snippet help"));
}
