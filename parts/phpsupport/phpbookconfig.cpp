#include "domutil.h"
#include <qlineedit.h>
#include <qcstring.h>

#include "phpbookconfig.h"
#include "phpconfigdata.h"

PHPBookConfig::PHPBookConfig(PHPConfigData* data,QWidget* parent,  const char* name, WFlags fl )
  : PHPBookConfigWidget( parent, name, fl )
{
  configData = data;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
PHPBookConfig::~PHPBookConfig()
{
  // no need to delete child widgets, Qt does it all for us
}

void PHPBookConfig::slotOK()
{
  qWarning( "PHPBookConfigBase::slotZendButtonClicked(): Not implemented yet!" );
}

void PHPBookConfig::slotCancel()
{
  qWarning( "PHPBookConfig::slotAboutClicked()" );
}

#include "phpbookconfig.moc"
