/***************************************************************************
                          ccompconf.cpp  -  description
                             -------------------
    begin                : Mon Nov 19 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ccompconf.h"
#include "ccompconf.moc"

#include <qtoolbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qstringlist.h>
#include <qpushbutton.h>

#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kapp.h>
#include <klocale.h>

CCompConf::CCompConf(QWidget *parent, const char *name ) : CCompConfDlg(parent,name)
 ,config(kapp->config())
 {
  // connect the add buttons clicked signal to add the strings that the user added
  // to the stringlists of the according comboboxes; clear combo and put in new list
  connect( host_arch_pb, SIGNAL(clicked()), SLOT(slotHostArchAdd()) );
  connect( unix_ver_pb, SIGNAL(clicked()), SLOT(slotUnixVerAdd()) );
  connect( target_arch_pb, SIGNAL(clicked()), SLOT(slotTargetArchAdd()) );
  connect( target_platform_pb, SIGNAL(clicked()), SLOT(slotTargetPlatformAdd()) );

	// initialize the fields by inserting the stringlists
	// start with the host machine
	config->setGroup("Compiler");
	hostarchs= config->readListEntry("Hostarchs");
	if(hostarchs.isEmpty())
		hostarchs <<"i386"<<"powerpc"<<"alpha"<<"sparc"<<"arm"<<"ia64"
  	<<"s390"<<"m68k"<<"mips"<<"hppa"<<"sh";
	host_arch_cb->insertStringList(hostarchs);
	int idx = hostarchs.findIndex(config->readEntry("Architecture","i386"));
	host_arch_cb->setCurrentItem(idx);
	
	unixvers = config->readListEntry("Unixversions");
	if(unixvers.isEmpty())
		unixvers <<"aix"<<"freebsd"<<"hpux10.20"<<"hpux11"<<"irix5"<<"irix6"<<"linux"
		<<"netbsd"<<"openbsd"<<"solaris"<<"sysv5";
	unix_ver_cb->insertStringList(unixvers);
	idx = unixvers.findIndex(config->readEntry("System", "linux"));
	unix_ver_cb->setCurrentItem(idx);
	
  compilearchs = config->readListEntry("Compilearchs");
  if(compilearchs.isEmpty())
  	compilearchs <<"i386"<<"powerpc"<<"alpha"<<"sparc"<<"arm"<<"ia64"
  	<<"s390"<<"m68k"<<"mips"<<"hppa"<<"sh";
	target_arch_cb->insertStringList(compilearchs);
	idx = compilearchs.findIndex(config->readEntry("TargetArchitecture", "i386"));
	target_arch_cb->setCurrentItem(idx);
	
	compileplatforms = config->readListEntry("Compileplatforms");
	if(compileplatforms.isEmpty())
		compileplatforms <<"aix"<<"freebsd"<<"hpux10.20"<<"hpux11"<<"irix5"<<"irix6"<<"linux"
		<<"netbsd"<<"openbsd"<<"solaris"<<"sysv5";
	target_platform_cb->insertStringList(compileplatforms);
	idx = compileplatforms.findIndex(config->readEntry("TargetPlatform", "linux"));
	target_platform_cb->setCurrentItem(idx);

	slotArchChanged(target_arch_cb->text(target_arch_cb->currentItem()));
	slotPlatformChanged(target_platform_cb->text(target_platform_cb->currentItem()));
  // signals and slots connections
  connect( cpp_comp_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotCPPChanged(const QString&) ) );
  connect( c_comp_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotCCChanged(const QString&) ) );
  connect( cxx_comp_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotCXXChanged(const QString&) ) );
  connect( target_arch_cb, SIGNAL( activated(const QString&) ), this, SLOT( slotArchChanged(const QString&) ) );
  connect( target_platform_cb, SIGNAL( activated(const QString&) ), this, SLOT( slotPlatformChanged(const QString&) ) );
	
}
CCompConf::~CCompConf(){
	config->setGroup("Compiler");
	//comboboxes lists
	config->writeEntry("Hostarchs",hostarchs );
	config->writeEntry("Unixversions", unixvers);
	config->writeEntry("Compilearchs",compilearchs);
	config->writeEntry("Compileplatforms",compileplatforms);
	// currently set texts
	config->writeEntry("Architecture",host_arch_cb->currentText());
	config->writeEntry("System", unix_ver_cb->currentText());
	config->writeEntry("TargetArchitecture", target_arch_cb->currentText());
	config->writeEntry("TargetPlatform", target_platform_cb->currentText());
	
}
/** No descriptions */
void CCompConf::slotHostArchAdd(){

		
	if((hostarchs.findIndex(host_arch_cb->currentText())!=-1) ){
		KMessageBox::error( NULL, i18n( "Can't add Architecture %1.\n\n"
								"The list of available architectures already "
								"contains the given architecture." ).arg(host_arch_cb->currentText()),
                                i18n( "Error" ) );
 		host_arch_cb->clearEdit();
		return;
	}
	hostarchs.append(host_arch_cb->currentText());
	host_arch_cb->clear();
	host_arch_cb->insertStringList(hostarchs);
}
/** No descriptions */
void CCompConf::slotUnixVerAdd(){
	if((unixvers.findIndex(unix_ver_cb->currentText())!=-1) ){
		KMessageBox::error( NULL, i18n( "Can't add System %1.\n\n"
								"The list of available systems already "
								"contains the given system." ).arg(unix_ver_cb->currentText()),
                                i18n( "Error" ) );
 		unix_ver_cb->clearEdit();
		return;
	}
	unixvers.append(unix_ver_cb->currentText());
	unix_ver_cb->clear();
	unix_ver_cb->insertStringList(unixvers);
}
/** No descriptions */
void CCompConf::slotTargetArchAdd(){
	if((compilearchs.findIndex(target_arch_cb->currentText())!=-1) ){
		KMessageBox::error( NULL, i18n( "Can't add Target Architecture %1.\n\n"
								"The list of available architectures already "
								"contains the given architecture." ).arg(target_arch_cb->currentText()),
                                i18n( "Error" ) );
 		target_arch_cb->clearEdit();
		return;
	}
	compilearchs.append(target_arch_cb->currentText());
	target_arch_cb->clear();
	target_arch_cb->insertStringList(compilearchs);
}
/** No descriptions */
void CCompConf::slotTargetPlatformAdd(){
	if((compileplatforms.findIndex(target_platform_cb->currentText())!=-1) ){
		KMessageBox::error( NULL, i18n( "Can't add Target System %1.\n\n"
								"The list of available systems already "
								"contains the given system." ).arg(target_platform_cb->currentText()),
                                i18n( "Error" ) );
 		target_platform_cb->clearEdit();
		return;
	}
	compileplatforms.append(target_platform_cb->currentText());
	target_platform_cb->clear();
	target_platform_cb->insertStringList(compileplatforms);
}
/** Architecture changed, updates the line edit contents
 */
void CCompConf::slotArchChanged(const QString& arch){
	config->setGroup("Compilearch "+ arch+"-"+target_platform_cb->currentText());
 	cpp_comp_le->setText(config->readEntry("CPP",arch+"-"+target_platform_cb->currentText()+"-cpp"));	
 	c_comp_le->setText(config->readEntry("CC",arch+"-"+target_platform_cb->currentText()+"-gcc"));	
 	cxx_comp_le->setText(config->readEntry("CXX",arch+"-"+target_platform_cb->currentText()+"-g++"));	

}
/** No descriptions */
void CCompConf::slotPlatformChanged(const QString& platform){
	config->setGroup("Compilearch "+target_arch_cb->currentText()+"-"+platform);
 	cpp_comp_le->setText(config->readEntry("CPP",target_arch_cb->currentText()+"-"+platform+"-cpp"));	
 	c_comp_le->setText(config->readEntry("CC",target_arch_cb->currentText()+"-"+platform+"-gcc"));	
 	cxx_comp_le->setText(config->readEntry("CXX",target_arch_cb->currentText()+"-"+platform+"-g++"));	

}
/** the path changed. Save the new path to the accoring
configuration section of the architecture set in the combo above */
void CCompConf::slotCPPChanged(const QString& cpp){
	config->setGroup("Compilearch "+ target_arch_cb->currentText()+"-"+target_platform_cb->currentText());
  config->writeEntry("CPP",cpp);
}
/** the CC for the set architecture changed, save it
into the section of the current architecture */
void CCompConf::slotCCChanged(const QString& cc){
	config->setGroup("Compilearch "+ target_arch_cb->currentText()+"-"+target_platform_cb->currentText());
  config->writeEntry("CC",cc);
}
/** the current CXX has changed for the architecture
set in the combo above; write it into the according section of
the config file. */
void CCompConf::slotCXXChanged(const QString& cxx){
	config->setGroup("Compilearch "+ target_arch_cb->currentText()+"-"+target_platform_cb->currentText());
  config->writeEntry("CXX",cxx);
}
