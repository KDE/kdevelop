#include <qlayout.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kurlrequester.h>
#include <klineedit.h>

#include "dialog_widget.h"
#include "valgrind_dialog.h"


ValgrindDialog::ValgrindDialog( QWidget* parent )
  : KDialogBase( parent, "valgrind dialog", true, i18n("Valgrind memory check"), Ok|Cancel )
{
  w = new DialogWidget( this );
  w->valExecutableEdit->setURL( "valgrind" );
  w->executableEdit->setFocus();
  setMainWidget( w );
}


ValgrindDialog::~ValgrindDialog()
{
}

QString ValgrindDialog::executableName() const
{
  return w->executableEdit->url();
}

void ValgrindDialog::setExecutable( const QString& url )
{
  w->executableEdit->setURL( url );
}

QString ValgrindDialog::valExecutable() const
{
  return w->valExecutableEdit->url();
}

QString ValgrindDialog::parameters() const
{
  return w->paramEdit->text();
}

void ValgrindDialog::setParameters( const QString& params )
{
  w->paramEdit->setText( params );
}

void ValgrindDialog::setValExecutable( const QString& ve )
{
  w->valExecutableEdit->setURL( ve );
}

QString ValgrindDialog::valParams() const
{
  QString params = w->valParamEdit->text();
  if ( w->memleakBox->isChecked() )
    params += " --leak-check=yes";
  if ( w->reachableBox->isChecked() )
    params += " --show-reachable=yes";
  if ( w->childrenBox->isChecked() )
    params += " --trace-children=yes";
  
  return params;
}

void ValgrindDialog::setValParame( const QString& params )
{
  w->valParamEdit->setText( params );
}

