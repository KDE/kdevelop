#include <qlayout.h>
#include <qcheckbox.h>
#include <qregexp.h>

#include <klocale.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

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
  QString vUrl = ve;
  if ( vUrl.isEmpty() ) {
    vUrl = KStandardDirs::findExe( "valgrind" );
  }
  if ( vUrl.isEmpty() ) {
    KMessageBox::sorry( this, i18n( "Could not find valgrind in your $PATH. Please make "
                                    "sure it is installed properly." ),
                        i18n( "valgrind not found" ) );
    w->valExecutableEdit->setURL( "valgrind" );
  } else {
    w->valExecutableEdit->setURL( vUrl );
  }
}

static const QString leakCheckParam( "--leak-check=yes" );
static const QString reachableParam( "--show-reachable=yes" );
static const QString childrenParam( "--trace-children=yes" );

QString ValgrindDialog::valParams() const
{
  QString params = w->valParamEdit->text();
  if ( w->memleakBox->isChecked() )
    params += " " + leakCheckParam;
  if ( w->reachableBox->isChecked() )
    params += " " + reachableParam;
  if ( w->childrenBox->isChecked() )
    params += " " + childrenParam;
  
  return params;
}

void ValgrindDialog::setValParams( const QString& params )
{
  QString myParams = params;
  if ( myParams.contains( leakCheckParam ) )
    w->memleakBox->setChecked( true );
  if ( myParams.contains( reachableParam ) )
    w->reachableBox->setChecked( true );
  if ( myParams.contains( childrenParam ) )
    w->childrenBox->setChecked( true );
  w->init();

  myParams = myParams.replace( QRegExp( leakCheckParam ), "" );
  myParams = myParams.replace( QRegExp( reachableParam ), "" );
  myParams = myParams.replace( QRegExp( childrenParam ), "" );
  myParams = myParams.stripWhiteSpace();
  w->valParamEdit->setText( myParams );
}

