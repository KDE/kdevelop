#include "domutil.h"
#include <kprocess.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcstring.h>
#include <iostream.h>
#include <qradiobutton.h>

#include "phpinfodlg.h"
#include "phpconfigwidget.h"

PHPConfigWidget::PHPConfigWidget( QDomDocument* projectDom,QWidget* parent,  const char* name, WFlags fl )
  : PHPConfigWidgetBase( parent, name, fl )
{
  dom = projectDom;
  m_phpInfo="";
 
  // page "Invocation"
  QString shellOrWeb = DomUtil::readEntry(*dom, "/kdevphpsupport/general/defaultInvocation");
  if(shellOrWeb == "shell"){
    callPHPDirectly_radio->setChecked(true);
  }
  if(shellOrWeb == "web"){
    callWebserver_radio->setChecked(true);
  }
  
  // page webserver
  QString weburl = DomUtil::readEntry(*dom, "/kdevphpsupport/webserver/weburl");
  QString webCurrentOrDefault = DomUtil::readEntry(*dom, "/kdevphpsupport/webserver/fileInvocation");
  QString webDefaultFile = DomUtil::readEntry(*dom, "/kdevphpsupport/webserver/defaultFile");
  
  if(weburl.isEmpty()) weburl = "http://localhost/"; 
  weburl_edit->setText(weburl);
  useDefaultFile_edit->setText(webDefaultFile);
  if(webCurrentOrDefault == "current"){
    useCurrentFile_radio->setChecked(true);
  }
  if(webCurrentOrDefault == "default"){
    useDefaultFile_radio->setChecked(true);
  }
  

  // page shell
  // todo,check were the php.exe is located
  QString exepath = DomUtil::readEntry(*dom, "/kdevphpsupport/shell/phpexe");  
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
    DomUtil::writeEntry(*dom, "/kdevphpsupport/general/defaultInvocation","shell");
  }
  if(callWebserver_radio->isChecked()){
    DomUtil::writeEntry(*dom, "/kdevphpsupport/general/defaultInvocation","web");
  }

  // shell
  DomUtil::writeEntry(*dom, "/kdevphpsupport/shell/phpexe", exe_edit->text());
  // webserver
  DomUtil::writeEntry(*dom, "/kdevphpsupport/webserver/weburl", weburl_edit->text());
  DomUtil::writeEntry(*dom, "/kdevphpsupport/webserver/defaultFile", useDefaultFile_edit->text());
  if(useCurrentFile_radio->isChecked()){
    DomUtil::writeEntry(*dom, "/kdevphpsupport/webserver/fileInvocation","current");
  }
  if(useDefaultFile_radio->isChecked()){
    DomUtil::writeEntry(*dom, "/kdevphpsupport/webserver/fileInvocation","default");
  }
    
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

void PHPConfigWidget::slotReceivedPHPInfo (KProcess* proc, char* buffer, int buflen){
  m_phpInfo += QCString(buffer,buflen+1);
}
void PHPConfigWidget::slotPHPExeButtonClicked()
{
    qWarning( "PHPConfigWidget::slotPHPExeButtonClicked(): Not implemented yet!" );
}

void PHPConfigWidget::slotPHPIniButtonClicked()
{
    qWarning( "PHPConfigWidget::slotPHPIniButtonClicked(): Not implemented yet!" );
}

#include "phpconfigwidget.moc"
