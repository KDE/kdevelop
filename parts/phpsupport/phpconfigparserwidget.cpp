#include "domutil.h"
#include <kprocess.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcstring.h>
#include <iostream>
#include <qradiobutton.h>
#include <kfiledialog.h>
#include <qcheckbox.h>

#include "phpinfodlg.h"
#include "phpconfigparserwidget.h"
#include "phpconfigdata.h"

using namespace std;

PHPConfigParserWidget::PHPConfigParserWidget(PHPConfigData* data,QWidget* parent,  const char* name, WFlags fl )
  : PHPConfigParserWidgetBase( parent, name, fl ){
  m_configData = data;
  codeCompletion_checkbox->setChecked(m_configData->getCodeCompletion());
  codeHinting_checkbox->setChecked(m_configData->getCodeHinting());
  realtimeParsing_checkbox->setChecked(m_configData->getRealtimeParsing());
}
/*  
 *  Destroys the object and frees any allocated resources
 */
PHPConfigParserWidget::~PHPConfigParserWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

void PHPConfigParserWidget::accept()
{
  cerr << endl << "PHPConfigParserwidget::accept()";
  m_configData->setCodeCompletion(codeCompletion_checkbox->isChecked());
  m_configData->setCodeHinting(codeHinting_checkbox->isChecked());
  m_configData->setRealtimeParsing(realtimeParsing_checkbox->isChecked());
  
  m_configData->storeConfig();
    
}


#include "phpconfigparserwidget.moc"
