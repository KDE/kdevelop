#include "domutil.h"
#include <kprocess.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qmultilineedit.h>
#include <qcstring.h>
#include <iostream>
#include <qradiobutton.h>

#include <kfiledialog.h>
#include <kstandarddirs.h>

#include "phpinfodlg.h"
#include "phpconfigwidget.h"
#include "phpconfigdata.h"

using namespace std;

PHPConfigWidget::PHPConfigWidget(PHPConfigData* data,QWidget* parent,  const char* name, WFlags fl )
  : PHPConfigWidgetBase( parent, name, fl )
{
   configData = data;
   m_phpInfo = "";

   PHPConfigData::InvocationMode mode = configData->getInvocationMode();

   // page "Invocation"
   if (mode == PHPConfigData::Shell) {
      callPHPDirectly_radio->setChecked(true);
   }

   if (mode == PHPConfigData::Web) {
      callWebserver_radio->setChecked(true);
   }

   // page webserver
   QString weburl = configData->getWebURL();
   if (weburl.isEmpty())
      weburl = "http://localhost/";
   weburl_edit->setText(weburl); 

   // page shell
   QString exepath = configData->getPHPExecPath();
   if (exepath.isEmpty()) {
      QString fiexepath = KStandardDirs::findExe("php");

      if (exepath.isEmpty())
         exepath = "/usr/local/bin/php";
   }

   exe_edit->setText(exepath);

   // page options
   PHPConfigData::StartupFileMode phpStartupFileMode = configData->getStartupFileMode();
   QString phpStartupFile = configData->getStartupFile();

   useDefaultFile_edit->setText(phpStartupFile);

   if(phpStartupFileMode == PHPConfigData::Current) {
      useCurrentFile_radio->setChecked(true);
   }

   if(phpStartupFileMode == PHPConfigData::Default) {
      useDefaultFile_radio->setChecked(true);
   }

   QString includepath = configData->getPHPIncludePath();
   include_path_edit->setText(includepath);

   codeCompletion_checkbox->setChecked(configData->getCodeCompletion());
   codeHinting_checkbox->setChecked(configData->getCodeHinting());
   realtimeParsing_checkbox->setChecked(configData->getRealtimeParsing());
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
  // invocation
   if (callPHPDirectly_radio->isChecked()){
      configData->setInvocationMode(PHPConfigData::Shell);
   }
   if (callWebserver_radio->isChecked()){
      configData->setInvocationMode(PHPConfigData::Web);
   }

   // webserver
   configData->setWebURL(weburl_edit->text());  

   // shell
   configData->setPHPExePath(exe_edit->text());

   // config
   configData->setPHPIniPath(ini_edit->text());

   // options
   configData->setStartupFile(useDefaultFile_edit->text());

   if (useCurrentFile_radio->isChecked()) {
      configData->setStartupFileMode(PHPConfigData::Current);
   }
   if (useDefaultFile_radio->isChecked()){
      configData->setStartupFileMode(PHPConfigData::Default);
   }

   configData->setPHPIncludePath(include_path_edit->text());
   configData->setCodeCompletion(codeCompletion_checkbox->isChecked());
   configData->setCodeHinting(codeHinting_checkbox->isChecked());
   configData->setRealtimeParsing(realtimeParsing_checkbox->isChecked());

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

   connect( &proc, SIGNAL(receivedStdout (KProcess*, char*, int)),
            this, SLOT(slotReceivedPHPInfo (KProcess*, char*, int)));
   proc.start(KProcess::Block,KProcess::Stdout);
   PHPInfoDlg dlg(this,"phpinfo",true);
   dlg.php_edit->setText(m_phpInfo);
   dlg.exec();
   m_phpInfo = "";
}

void PHPConfigWidget::slotReceivedPHPInfo (KProcess* /*proc*/, char* buffer, int buflen){
   m_phpInfo += QCString(buffer,buflen+1);
}

void PHPConfigWidget::slotPHPExeButtonClicked(){
   QString exe = KFileDialog::getOpenFileName(QFileInfo(exe_edit->text()).filePath());
   if (!exe.isEmpty()){
      exe_edit->setText(exe);
   }
}

void PHPConfigWidget::slotPHPIniButtonClicked()
{
   QString file = KFileDialog::getOpenFileName(QFileInfo(exe_edit->text()).filePath(), QString("*.ini|INI File (*.ini)"));

   if (!file.isEmpty()){
      ini_edit->setText(file);
   }
}

#include "phpconfigwidget.moc"
