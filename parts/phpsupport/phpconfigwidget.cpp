#include "domutil.h"
#include <kprocess.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcstring.h>
#include <iostream>
#include <qradiobutton.h>
#include <kfiledialog.h>

#include "phpinfodlg.h"
#include "phpconfigwidget.h"
#include "phpconfigdata.h"

using namespace std;

PHPConfigWidget::PHPConfigWidget(PHPConfigData* data,QWidget* parent,  const char* name, WFlags fl )
  : PHPConfigWidgetBase( parent, name, fl )
{
  configData = data;
  m_phpInfo="";
 
  PHPConfigData::InvocationMode mode = configData->getInvocationMode();
  // page "Invocation"

  if(mode == PHPConfigData::Shell){
    callPHPDirectly_radio->setChecked(true);
  }
  if(mode == PHPConfigData::Web){
    callWebserver_radio->setChecked(true);
  }
  
  // page webserver
  QString weburl = configData->getWebURL();
  PHPConfigData::WebFileMode webFileMode = configData->getWebFileMode();
  QString webDefaultFile = configData->getWebDefaultFile();
  
  if(weburl.isEmpty()) weburl = "http://localhost/"; 
  weburl_edit->setText(weburl);
  useDefaultFile_edit->setText(webDefaultFile);
  
  if(webFileMode == PHPConfigData::Current){
    useCurrentFile_radio->setChecked(true);
  }
  if(webFileMode == PHPConfigData::Default){
    useDefaultFile_radio->setChecked(true);
  }
 
  // page shell
  // todo,check were the php.exe is located
  QString exepath = configData->getPHPExecPath();
  if(exepath.isEmpty()) exepath = "/usr/local/bin/php"; 
  exe_edit->setText(exepath);
}

/*  
 *  Destroys the object and frees any allocated resources
 */
PHPConfigWidget::~PHPConfigWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

void PHPConfigWidget::accept()
{
  cerr << endl << "PHPConfigWidget::accept()";
  // invocation
  if(callPHPDirectly_radio->isChecked()){
    configData->setInvocationMode(PHPConfigData::Shell);
  }
  if(callWebserver_radio->isChecked()){
    configData->setInvocationMode(PHPConfigData::Web);
  }

  // webserver
  configData->setWebURL(weburl_edit->text());  
  configData->setWebDefaultFile(useDefaultFile_edit->text());  
  
  if(useCurrentFile_radio->isChecked()){
    configData->setWebFileMode(PHPConfigData::Current);
  }
  if(useDefaultFile_radio->isChecked()){
    configData->setWebFileMode(PHPConfigData::Default);
  }
  
  // shell
  configData->setPHPExePath(exe_edit->text());  

  configData->storeConfig();
    
}
void PHPConfigWidget::slotZendButtonClicked()
{
  qWarning( "PHPConfigWidgetBase::slotZendButtonClicked(): Not implemented yet!" );
}

void PHPConfigWidget::slotAboutClicked()
{
  qWarning( "PHPConfigWidget::slotAboutClicked()" );
  KShellProcess proc("/bin/sh");
  proc << exe_edit->text();
  proc << "-m";
  
  connect(&proc, SIGNAL(receivedStdout (KProcess*, char*, int)),
   			this, SLOT(slotReceivedPHPInfo (KProcess*, char*, int)));
  proc.start(KProcess::Block,KProcess::Stdout);
  PHPInfoDlg dlg(this,"phpinfo",true);
  dlg.php_edit->setText(m_phpInfo);
  dlg.exec();
  m_phpInfo="";
}

void PHPConfigWidget::slotReceivedPHPInfo (KProcess* /*proc*/, char* buffer, int buflen){
  m_phpInfo += QCString(buffer,buflen+1);
}
void PHPConfigWidget::slotPHPExeButtonClicked(){
  QString exe = KFileDialog::getOpenFileName(QFileInfo(exe_edit->text()).filePath());
  if(!exe.isEmpty()){
    exe_edit->setText(exe);
  }
}

void PHPConfigWidget::slotPHPIniButtonClicked()
{
    qWarning( "PHPConfigWidget::slotPHPIniButtonClicked(): Not implemented yet!" );
}

#include "phpconfigwidget.moc"
