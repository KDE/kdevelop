/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <kdebug.h>

void CvsForm::init()
{
    setWFlags( getWFlags() | WDestructiveClose );
    kdDebug( 9999 ) << " **** CvsForm created" << endl;
}

void CvsForm::destroy()
{
    kdDebug( 9999 ) << " **** CvsForm destroyed" << endl;
}
