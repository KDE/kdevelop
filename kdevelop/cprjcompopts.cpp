/***************************************************************************
                          cprjcompopts.cpp  -  description
                             -------------------
    begin                : Wed Nov 21 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cprjcompopts.h"
#include "cprjcompopts.moc"
#include "cproject.h"
#include "kdevsession.h"

#include <qlistview.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qdom.h>
#include <qregexp.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kapp.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>

CPrjCompOpts::CPrjCompOpts(CProject* prj,KDevSession* session, const QString& curr, QWidget *parent, const char *name ) :
CPrjCompOptsDlg(parent,name) {   prj_info = prj;
  sess=session;
 	disconnectGUI();
 	initGUI();
  connect( conf_pb, SIGNAL(clicked()), SLOT(slotConfigAdd()) );
  connect( vpathdir_pb, SIGNAL(clicked()), SLOT(slotCompPath()) );

  connect( prefix_pb, SIGNAL(clicked()), SLOT(slotPrefixPath()) );
  connect( qtdir_pb, SIGNAL(clicked()), SLOT(slotQtPath()) );
  connect( xincludes_pb, SIGNAL(clicked()), SLOT(slotXincPath()) );
  connect( xlibraries_pb, SIGNAL(clicked()), SLOT(slotXlibPath()) );

  loadSettings();
  connectGUI();
  // replace this with the currently used configuration in the toolbar.
	int idx=compconfs.findIndex(curr); // set the added config as the current item
	conf_cb->setCurrentItem(idx);
	slotConfigChanged(curr);
	
}
CPrjCompOpts::~CPrjCompOpts(){
}

/** No descriptions */
void CPrjCompOpts::initGUI(){
  vpathdir_pb->setPixmap(SmallIcon("fileopen"));
  prefix_pb->setPixmap(SmallIcon("fileopen"));
  qtdir_pb->setPixmap(SmallIcon("fileopen"));
  xincludes_pb->setPixmap(SmallIcon("fileopen"));
  xlibraries_pb->setPixmap(SmallIcon("fileopen"));
	///////////////////////////////////////////////
  KConfig* config= kapp->config();
	config->setGroup("Compiler");
  compilearchs = config->readListEntry("Compilearchs");
  if(compilearchs.isEmpty())
  	compilearchs <<"i386"<<"powerpc"<<"alpha"<<"sparc"<<"arm"<<"ia64"
  	<<"s390"<<"m68k"<<"mips"<<"hppa"<<"sh";
	target_arch_cb->insertStringList(compilearchs);
	int idx = compilearchs.findIndex(config->readEntry("TargetArchitecture", "i386"));
	target_arch_cb->setCurrentItem(idx);
	
	compileplatforms = config->readListEntry("Compileplatforms");
	if(compileplatforms.isEmpty())
		compileplatforms <<"aix"<<"freebsd"<<"hpux10.20"<<"hpux11"<<"irix5"<<"irix6"<<"linux"
		<<"netbsd"<<"openbsd"<<"solaris"<<"sysv5";
	target_platform_cb->insertStringList(compileplatforms);
	idx = compileplatforms.findIndex(config->readEntry("TargetPlatform", "linux"));
	target_platform_cb->setCurrentItem(idx);
////////////////////////////////////////////////////	
		

	w_= new QCheckListItem(warnings_lv, "-W",
                                 QCheckListItem::CheckBox);
	w_aggregate_return= new QCheckListItem(warnings_lv,"-Waggregate-return" ,
                                 QCheckListItem::CheckBox);
	w_all= new QCheckListItem(warnings_lv, "-Wall",
                                 QCheckListItem::CheckBox);
	w_bad_function_cast= new QCheckListItem(warnings_lv, "-Wbad-function-cast",
                                 QCheckListItem::CheckBox);
	w_cast_align= new QCheckListItem(warnings_lv,"-Wcast-align" ,
                                 QCheckListItem::CheckBox);
	w_cast_qual= new QCheckListItem(warnings_lv, "-Wcast-qual",
                                 QCheckListItem::CheckBox);
	w_conversion= new QCheckListItem(warnings_lv, "-Wconversion",
                                 QCheckListItem::CheckBox);
	w_error= new QCheckListItem(warnings_lv,"-Werror",
                                 QCheckListItem::CheckBox);
	w_inline= new QCheckListItem(warnings_lv, "-Winline",
                                 QCheckListItem::CheckBox);
	w_missing_declarations= new QCheckListItem(warnings_lv, "-Wmissing-declarations",
                                 QCheckListItem::CheckBox);
	w_missing_prototypes= new QCheckListItem(warnings_lv, "-Wmissing-prototypes",
                                 QCheckListItem::CheckBox);
	w_nested_externs= new QCheckListItem(warnings_lv,"-Wnested-externs" ,
                                 QCheckListItem::CheckBox);
	w_old_style_cast= new QCheckListItem(warnings_lv, "-Wold-style-cast",
                                 QCheckListItem::CheckBox);
	w_overloaded_virtual= new QCheckListItem(warnings_lv, "-Woverloaded-virtual",
                                 QCheckListItem::CheckBox);
	w_pointer_arith= new QCheckListItem(warnings_lv, "-Wpointer-arith",
                                 QCheckListItem::CheckBox);
	w_redundant_decls= new QCheckListItem(warnings_lv,"-Wredundant-decls" ,
                                 QCheckListItem::CheckBox);
	w_shadow= new QCheckListItem(warnings_lv, "-Wshadow",
                                 QCheckListItem::CheckBox);
	w_sign_compare= new QCheckListItem(warnings_lv,"-Wsign-compare" ,
                                 QCheckListItem::CheckBox);
	w_strict_prototypes= new QCheckListItem(warnings_lv, "-Wstrict-prototypes",
                                 QCheckListItem::CheckBox);
	w_synth= new QCheckListItem(warnings_lv,"-Wsynth" ,
                                 QCheckListItem::CheckBox);
	w_traditional= new QCheckListItem(warnings_lv, "-Wtraditional",
                                 QCheckListItem::CheckBox);
	w_undef= new QCheckListItem(warnings_lv,"-Wundef" ,
                                 QCheckListItem::CheckBox);
	w_write_strings= new QCheckListItem(warnings_lv,"-Wwrite-strings" ,
                                 QCheckListItem::CheckBox);
	
}
/** Select/create the directory to run configure/make in
for the given configuration */
void CPrjCompOpts::slotCompPath(){
  QString dir;
  dir = KFileDialog::getExistingDirectory(prj_info->getProjectDir());
  if (!dir.isEmpty())
    vpathdir_le->setText(dir);
}
/** No descriptions */
void CPrjCompOpts::slotPrefixPath(){
  QString dir;
  dir = KFileDialog::getExistingDirectory(QDir::homeDirPath());
  if (!dir.isEmpty())
    prefix_le->setText(dir);
}
/** No descriptions */
void CPrjCompOpts::slotQtPath(){
  QString dir;
  dir = KFileDialog::getExistingDirectory(QDir::homeDirPath());
  if (!dir.isEmpty())
    qtdir_le->setText(dir);
}
/** No descriptions */
void CPrjCompOpts::slotXincPath(){
  QString dir;
  dir = KFileDialog::getExistingDirectory(QDir::homeDirPath());
  if (!dir.isEmpty())
    xincludes_le->setText(dir);
}
/** No descriptions */
void CPrjCompOpts::slotXlibPath(){
  QString dir;
  dir = KFileDialog::getExistingDirectory(QDir::homeDirPath());
  if (!dir.isEmpty())
    xlibraries_le->setText(dir);
}

/** invoked by pressing the Add button to add a new 
configuration for compiling to the combobox. */
void CPrjCompOpts::slotConfigAdd(){
	QString conf=conf_cb->currentText();
	if(compconfs.findIndex(conf) > -1){
		KMessageBox::error( NULL, i18n( "Can't add configuration %1.\n\n"
								"The list of available configurations already "
								"contains the one you wanted to add." ).arg(conf),
                                i18n( "Error" ) );
 		conf_cb->clearEdit();
		return;
	}
	sess->addCompileConfig(conf);
	sess->setArchitecture(conf,target_arch_cb->currentText() );
	sess->setPlatform(conf,target_platform_cb->currentText() );
	sess->setVPATHSubdir(conf,prj_info->getProjectDir()+conf);
	conf_cb->clear();
	loadSettings(); // refill the configs combo
	int idx=compconfs.findIndex(conf); // set the added config as the current item
	conf_cb->setCurrentItem(idx);
	slotConfigChanged(conf); // change settings to the new configuration
}


/** Load the settings from either the project file if only the default is used (session configuration for the 
other compiler configurations don't exist) and the according configurations in the session file plus their
respective flags and settings. */
void CPrjCompOpts::loadSettings(){
	// read the kdevsess file xml tree here and insert the correct settings into
	// data fields
	compconfs=sess->getCompileConfigs();
	compconfs.prepend(i18n("(Default)"));
	conf_cb->insertStringList(compconfs);


}
/** No descriptions */
void CPrjCompOpts::slotConfigChanged(const QString& conf){
	QString cxxflags, configureargs, ldflags;
	// prevent configuration changes by connections of GUI items to
	// slotSettingsChanged()
	disconnectGUI();
	if(conf == i18n("(Default)")){
		target_arch_cb->setEnabled(false);
		target_platform_cb->setEnabled(false);
		vpathdir_le->setText("(Source Directory)");
		vpathdir_le->setEnabled(false);
		vpathdir_pb->setEnabled(false);		
		KConfig* config=kapp->config();
		config->setGroup("Compiler");
		int idx = compilearchs.findIndex(config->readEntry("Architecture","i386"));
		target_arch_cb->setCurrentItem(idx);
		idx = compileplatforms.findIndex(config->readEntry("System", "linux"));
		target_platform_cb->setCurrentItem(idx);
    // lineedits
  	cppflags_le->setText(prj_info->getCPPFLAGS());
  	cflags_le->setText(prj_info->getCFLAGS());
  	cxxflags_le->setText(prj_info->getAdditCXXFLAGS());
		// strings to be processed below
   	cxxflags=prj_info->getCXXFLAGS();
  	ldflags=prj_info->getLDFLAGS();
  	configureargs=prj_info->getConfigureArgs();
	}
	else{
		target_arch_cb->setEnabled(true);
		target_platform_cb->setEnabled(true);
		vpathdir_le->setText(sess->getVPATHSubdir(conf) );
		vpathdir_le->setEnabled(true);
		vpathdir_pb->setEnabled(true);		
		int idx = compilearchs.findIndex( sess->getArchitecture(conf) );
		target_arch_cb->setCurrentItem(idx);
		idx=compileplatforms.findIndex( sess->getPlatform(conf) );
		target_platform_cb->setCurrentItem(idx);
		// lineedits
  	cppflags_le->setText(sess->getCPPFLAGS(conf));
  	cflags_le->setText(sess->getCFLAGS(conf));
  	cxxflags_le->setText(sess->getAdditCXXFLAGS(conf));
		// strings to be processed below  	
		cxxflags=sess->getCXXFLAGS(conf);				
		ldflags=sess->getLDFLAGS(conf);
		configureargs=sess->getConfigureArgs(conf);		
	}
	cxxflags=cxxflags.simplifyWhiteSpace();
 	configureargs=configureargs.simplifyWhiteSpace(); //remove too much white space
 	
	// the optimization level setting	
	if (cxxflags.find("-O") != -1){
	  bool a;
	  QString level=cxxflags.mid( cxxflags.find("-O"), 3 );
	  level=level.right(1);
    opt_level_sb->setValue(level.toInt(&a));
		if(level.toInt(&a) != 0)
			optimize_cb->setChecked(true);
		else
			optimize_cb->setChecked(false);		
	}
	else
		optimize_cb->setChecked(false);		
	
  if (cxxflags.find("-g") != -1){
		debug_cb->setChecked(true);
    bool a;
	  QString level=cxxflags.mid( cxxflags.find("-g"), 3 );
		level=level.right(1);
    debuglevel_sb->setValue(level.toInt(&a));
	}
	else
		debug_cb->setChecked(false);
		
	enableprofiling_cb->setChecked(cxxflags.find("-pg") != -1);
	storeintermediate_cb->setChecked(cxxflags.find("-save-temps") != -1);
	
	///////////////////////////////////////////////////////////////////////
	// checklistboxes on page 2
  w_all->setOn(cxxflags.find("-Wall") != -1);
  w_->setOn(cxxflags.find("-W ")  != -1);
  w_traditional->setOn(cxxflags.find("-Wtraditional") != -1);
  w_undef->setOn(cxxflags.find("-Wundef") != -1);
  w_shadow->setOn(cxxflags.find("-Wshadow") != -1);
  w_pointer_arith->setOn(cxxflags.find("-Wpointer-arith") != -1);
  w_bad_function_cast->setOn(cxxflags.find("-Wbad-function-cast") != -1);
  w_cast_qual->setOn(cxxflags.find("-Wcast-qual") != -1);
  w_cast_align->setOn(cxxflags.find("-Wcast-align") != -1);
  w_write_strings->setOn(cxxflags.find("-Wwrite-strings") != -1);
  w_conversion->setOn(cxxflags.find("-Wconversion") != -1);
  w_sign_compare->setOn(cxxflags.find("-Wsign-compare") != -1);
  w_aggregate_return->setOn(cxxflags.find("-Waggregate-return") != -1);
  w_strict_prototypes->setOn(cxxflags.find("-Wstrict-prototypes") != -1);
  w_missing_prototypes->setOn(cxxflags.find("-Wmissing-prototypes") != -1);
  w_missing_declarations->setOn(cxxflags.find("-Wmissing-declarations") != -1);
  w_redundant_decls->setOn(cxxflags.find("-Wredundant-decls") != -1);
  w_nested_externs->setOn(cxxflags.find("-Wnested-externs") != -1);
  w_inline->setOn(cxxflags.find("-Winline") != -1);
  w_old_style_cast->setOn(cxxflags.find("-Wold-style-cast") != -1);
  w_overloaded_virtual->setOn(cxxflags.find("-Woverloaded-virtual") != -1);
  w_synth->setOn(cxxflags.find("-Wsynth") != -1);
  w_error->setOn(cxxflags.find("-Werror") != -1);


 	remove_symb_cb->setChecked(ldflags.find("-s ") != -1);
  ldflags = ldflags.replace( QRegExp("-s "), "" );
 	link_static_cb->setChecked(ldflags.find("-static") != -1);		
  ldflags = ldflags.replace( QRegExp("-static "), "" ); 	
 	ldflags_le->setText(ldflags);
	
	///////////////////////////////////////////////////////////////////////
	// configure arguments parsing
 	// remove --target, --host and --build first
	 	
	// checkboxes first
	cfgargs=configureargs;
	findConfigureOption("--host=");
	findConfigureOption("--build=");
	findConfigureOption("--target=");
	disable_debug_cb->setChecked(cfgargs.find("--disable-debug") != -1);
	findConfigureOption("--disable-debug");
	enable_debug_cb->setChecked(cfgargs.find("--enable-debug") != -1);
	findConfigureOption("--enable-debug");
	enable_final_cb->setChecked(cfgargs.find("--enable-final") != -1);
	findConfigureOption("--enable-final");
	enable_strict_cb->setChecked(cfgargs.find("--enable-strict") != -1);
	findConfigureOption("--enable-strict");
	
	// lineedits filling
	prefix_le->setText(findConfigureOption("--prefix="));
	qtdir_le->setText(findConfigureOption("--with-qt-dir="));
	xincludes_le->setText(findConfigureOption("--x-includes="));
	xlibraries_le->setText(findConfigureOption("--x-libraries="));
	
  cfgargs=cfgargs.simplifyWhiteSpace(); //remove pending white space
	add_configure_opts->setText(cfgargs); // add all remaining options
	// reconnect all connections of the GUI to slotSettingsChanged() to re-establish
	// the monitoring of user changes and saving into the configuration tree/projectfile.
	connectGUI();
}


QString CPrjCompOpts::findConfigureOption( QString option ){
	QString path="";
	int len=option.length();						// length of the option itself
	int pos=cfgargs.find(option);// the option starts at pos
	int start=pos+len;								 // option parameter start
	int end=cfgargs.find(' ',pos);// find the next white space after pos - end of parameter and option
	int length=end-pos;  // complete length
	if(pos != -1){  //option is present
		// compute the path
		path=cfgargs.mid(
					start, //start here
					end-start );
		cfgargs.remove(cfgargs.find(option),length); // remove option+ parameter
	}
	return path;		
}

/** called by all items who change their state to notify saving of settings to the according
     configuration section */
void CPrjCompOpts::slotSettingsChanged(){
	QString ldflags;
	if(remove_symb_cb->isChecked())
		ldflags=" -s";
	if(link_static_cb->isChecked())
		ldflags+=" -static";
	ldflags+=" "+ldflags_le->text();
	QString conf=conf_cb->currentText();
	if(conf == i18n("(Default)")){
		// just the lineedits here
		prj_info->setCFLAGS(cflags_le->text());
		prj_info->setCPPFLAGS(cppflags_le->text());
  	prj_info->setAdditCXXFLAGS(cxxflags_le->text());
  	// complex calculation needed
  	prj_info->setCXXFLAGS(calculateCXXFLAGS());
		prj_info->setLDFLAGS(ldflags);
		prj_info->setConfigureArgs(calculateConfigureArgs());	
	}
	else{
		
		QStringList::Iterator it;
		QStringList vpathlist;
  	for( it = compconfs.begin(); it != compconfs.end(); ++it ){
  		if(sess->getVPATHSubdir(*it)!=vpathdir_le->text() )
    		vpathlist.append(sess->getVPATHSubdir(*it));
    	else
    		vpathlist.append(vpathdir_le->text());
    }
  	for( it = vpathlist.begin(); it != vpathlist.end(); ++it ){
			if(vpathlist.contains(*it)!=1){ //more that 1 time in it
				// the warning code comes here
//				KMessageBox::error( NULL, i18n( "The directory %1.\n"
//						"is used more than once for different configurations.\n\n "
//						"Please correct the builddirectory settings\n"
//						"for the different configurations so that a\n"
//						"builddirectory is only used once.\n\n"
//						"No settings will be saved until the error\n"
//						"is corrected." ).arg(*it),
//                            i18n( "Error" ) );
//				return;
			}
		}
		// here comes the DOM storage
		sess->setVPATHSubdir(conf,vpathdir_le->text() );
		sess->setArchitecture(conf,target_arch_cb->currentText() );
		sess->setPlatform(conf,target_platform_cb->currentText() );
		sess->setCFLAGS(conf,cflags_le->text() );
		sess->setCPPFLAGS(conf,cppflags_le->text() );			
		sess->setCXXFLAGS(conf,calculateCXXFLAGS() );
		sess->setAdditCXXFLAGS(conf, cxxflags_le->text() );
		sess->setLDFLAGS(conf, ldflags );
		sess->setConfigureArgs(conf,calculateConfigureArgs() );						
	}
}

/** Calculates the current configure arguments and returns the according argument string
 */
QString CPrjCompOpts::calculateConfigureArgs(){
 	////////// Configure arguments
	KConfig* config=kapp->config();
	config->setGroup("Compiler");
 	QString args=" --build="+config->readEntry("Architecture","i386")+"-"+
 										config->readEntry("System", "linux");
 	args+=" --host="+target_arch_cb->currentText()+"-"+target_platform_cb->currentText();
 	args+=" --target="+target_arch_cb->currentText()+"-"+target_platform_cb->currentText();
 	if(!prefix_le->text().isEmpty())
 		args+=" --prefix="+prefix_le->text();
 	if(!qtdir_le->text().isEmpty())
 		args+=" --with-qt-dir="+qtdir_le->text();
 	if(!xincludes_le->text().isEmpty())
 		args+=" --x-includes="+xincludes_le->text();
 	if(!xlibraries_le->text().isEmpty())
 		args+=" --x-libraries="+xlibraries_le->text();
 	if(disable_debug_cb->isChecked())
 		args+=" --disable-debug";
 	if(enable_debug_cb->isChecked())
 		args+=" --enable-debug";
 	if(enable_final_cb->isChecked())
			args+=" --enable-final";
 	if(enable_strict_cb->isChecked())
			args+=" --enable-strict";
	args+=" "+add_configure_opts->text();
 	return args;
}
/** calculates the current CXXFLAGS for the configuration */
QString CPrjCompOpts::calculateCXXFLAGS(){
	
	/////////// CXXFLAGS
	QString text;
	if(optimize_cb->isChecked()){
		text+=" -O"+opt_level_sb->text();
  } else
  text+=" -O0";
	if(enableprofiling_cb->isChecked())
		text+=" -pg";
	if(storeintermediate_cb->isChecked())
		text+=" -save-temps";
	if(debug_cb->isChecked())
 		text+=" -g"+debuglevel_sb->text();
  //***********gcc-warnings***********
  if (w_all->isOn())
    text+=" -Wall";
  if (w_->isOn())
    text+=" -W ";
  if (w_traditional->isOn())
    text+=" -Wtraditional";
  if (w_undef->isOn())
    text+=" -Wundef";
  if (w_shadow->isOn())
    text+=" -Wshadow";
  if (w_pointer_arith->isOn())
    text+=" -Wpointer-arith";
  if (w_bad_function_cast->isOn())
    text+=" -Wbad-function-cast";
  if (w_cast_qual->isOn())
    text+=" -Wcast-qual";
  if (w_cast_align->isOn())
    text+=" -Wcast-align";
  if (w_write_strings->isOn())
    text+=" -Wwrite-strings";
  if (w_conversion->isOn())
    text+=" -Wconversion";
  if (w_sign_compare->isOn())
    text+=" -Wsign-compare";
  if (w_aggregate_return->isOn())
    text+=" -Waggregate-return";
  if (w_strict_prototypes->isOn())
    text+=" -Wstrict-prototypes";
  if (w_missing_prototypes->isOn())
    text+=" -Wmissing-prototypes";
  if (w_missing_declarations->isOn())
    text+=" -Wmissing-declarations";
  if (w_redundant_decls->isOn())
    text+=" -Wredundant-decls";
  if (w_nested_externs->isOn())
    text+=" -Wnested-externs";
  if (w_inline->isOn())
    text+=" -Winline";
  if (w_old_style_cast->isOn())
    text+=" -Wold-style-cast";
  if (w_overloaded_virtual->isOn())
    text+=" -Woverloaded-virtual";
  if (w_synth->isOn())
    text+=" -Wsynth";
  if (w_error->isOn())
    text+=" -Werror";	
	return text;
}

/** disconnects all connections to slotSettingsChanged(), used in slotConfigChanged() to prevent
overwriting settings by slotSettingsChanged() */
void CPrjCompOpts::disconnectGUI(){
    disconnect( vpathdir_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( target_arch_cb, SIGNAL( activated(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( target_platform_cb, SIGNAL( activated(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( optimize_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( opt_level_sb, SIGNAL( valueChanged(int) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( debug_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( debuglevel_sb, SIGNAL( valueChanged(int) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( enableprofiling_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( storeintermediate_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( cppflags_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( cflags_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( cxxflags_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( remove_symb_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( link_static_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( ldflags_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( disable_debug_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( enable_debug_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( enable_final_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( enable_strict_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( prefix_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( qtdir_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( xincludes_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( xlibraries_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( add_configure_opts, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    disconnect( warnings_lv, SIGNAL( clicked(QListViewItem*) ), this, SLOT( slotSettingsChanged() ) );

}
/** reconnects all GUI items at the end of slotConfigChanged() to reestablish user
manipulation of configuration settings. */
void CPrjCompOpts::connectGUI(){
    connect( vpathdir_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( target_arch_cb, SIGNAL( activated(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( target_platform_cb, SIGNAL( activated(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( optimize_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( opt_level_sb, SIGNAL( valueChanged(int) ), this, SLOT( slotSettingsChanged() ) );
    connect( debug_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( debuglevel_sb, SIGNAL( valueChanged(int) ), this, SLOT( slotSettingsChanged() ) );
    connect( enableprofiling_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( storeintermediate_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( cppflags_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( cflags_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( cxxflags_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( remove_symb_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( link_static_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( ldflags_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( disable_debug_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( enable_debug_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( enable_final_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( enable_strict_cb, SIGNAL( toggled(bool) ), this, SLOT( slotSettingsChanged() ) );
    connect( prefix_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( qtdir_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( xincludes_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( xlibraries_le, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( add_configure_opts, SIGNAL( textChanged(const QString&) ), this, SLOT( slotSettingsChanged() ) );
    connect( warnings_lv, SIGNAL( clicked(QListViewItem*) ), this, SLOT( slotSettingsChanged() ) );
}

//  QWhatsThis::add(warnings_lv, i18n("<b>-W</b>: sets options not included in -Wall "
//      "which are very specific. Please read "
//      "GCC-Info for more information.<p>"
//
//																			"<b>-Waggregate-return</b>:Warn if any functions that return structures or unions are "
//    "defined or called. (In languages where you can return an "
//    "array, this also elicits a warning.)<p>"	
//																			"<b>-Wall</b>:Compile with -Wall. This option "
//      "includes several different warning "
//      "parameters which are recommended to "
//      "turn on.<p>"		
//																			"<b>-Wbad-function-cast</b>: Warn whenever a function call is "
//          "cast to a non-matching type. For "
//          "example, warn if <i>int malloc()</i> "
//          "is cast to <i>anything *.<p>"		
//																			"<b>-Wcast-align</b>:Warn whenever a pointer is cast such "
//        "that the required alignment of the target "
//        "is increased. For example, warn if a "
//        "<i>char *</i> is cast to an <i>int *</i> on "
//        "machines where integers can only be accessed "
//        "at two- or four-byte boundaries.<p>"		
//																			"<b>-Wcast-qual</b>:Warn whenever a pointer is cast "
//        "so as to remove a type qualifier "
//        "from the target type. For example, "
//        "warn if a <i>const char *</i> is "
//        "cast to an ordinary <i>char *.<p>"		
//																			"<b>-Wconversion</b>:Warn if a prototype causes a type conversion that is different "
//        "from what would happen to the same argument in the absence "
//        "of a prototype. This includes conversions of fixed point to "
//        "floating and vice versa, and conversions changing the width "
//        "or signedness of a fixed point argument except when the same "
//        "as the default promotion.  Also warn if a negative integer "
//        "constant expression is implicitly converted to an unsigned "
//        "type.<p>"		
//																			"<b>-Werror</b>:Make all warnings into errors.<p>"		
//																			"<b>-Winline</b>:Warn if a function can not be inlined, and either "
//    "it was declared as inline, or else the "
//    "<i>-finline-functions</i> option was given.<p>"		
//																			"<b>-Wmissing-declarations</b>:Warn if a global function is defined without a previous "
//    "declaration. Do so even if the definition itself pro- "
//    "vides a prototype. Use this option to detect global "
//    "functions that are not declared in header files.<p>"		
//																			"<b>-Wmissing-prototypes</b>:Warn if a global function is defined without a previous "
//    "prototype declaration. This warning is issued even if "
//    "the definition itself provides a prototype. The aim "
//    "is to detect global functions that fail to be declared "
//    "in header files.<p>"		
//																			"<b>-Wnested-externs</b>:Warn if an <i>extern</i> declaration is "
//    "encountered within a function.<p>"		
//																			"<b>-Wold-style-cast</b>:Warn if an old-style (C-style) cast is used "
//       "within a program<p>"		
//																			"<b>-Woverloaded-virtual</b>:Warn when a derived class function declaration may be an "
//    "error in defining a virtual function (C++ only). In "
//    "a derived class, the definitions of virtual functions "
//    "must match the type signature of a virtual function "
//    "declared in the base class. With this option, the "
//    "compiler warns when you define a function with the same "
//    "as a virtual function, but with a type signature that "
//    "does not match any declarations from the base class.<p>"		
//																			"<b>-Wpointer-arith</b>:Warn about anything that "
//        "depends on the <i>size of</i> a "
//        "function type or of <i>void</i>. "
//        "GNU C assigns these types a size of 1, "
//        "for convenience in calculations with "
//        "<i>void *</i> pointers and pointers "
//        "to functions.<p>"
//																			"<b>-Wredundant-decls</b>:Warn if anything is declared more than once in the same scope "
//    "even in cases where multiple declaration is valid and "
//    "changes nothing.<p>"));
//																			"<b></b>:
//  																		<p>"
//
//  QWhatsThis::add(w_shadow, i18n("Warn whenever a local variable "
//        "shadows another local variable."));
//  QWhatsThis::add(w_sign_compare,
//  i18n("Warn when a comparison between signed and unsigned values "
//       "could produce an incorrect result when the signed value "
//       "is converted to unsigned."));
//  QWhatsThis::add(w_strict_prototypes,
//  i18n("Warn if a function is declared or defined without specifying "
//    "the argument types. (An old-style function definition is "
//    "permitted without a warning if preceded by a declaration "
//    "which specifies the argument types.)"));
//  QWhatsThis::add(w_synth,
//  i18n("Warn when g++'s synthesis behavoir does "
//    "not match that of cfront."));
//  QWhatsThis::add(w_traditional, i18n("Warn about certain constructs "
//        "that behave differently in traditional "
//        "and ANSI C."));
//  QWhatsThis::add(w_undef, i18n("Warn if an undefined identifier is "
//                                "evaluated in an '#if' directive"));
//  QWhatsThis::add(w_write_strings,
//  i18n("Give string constants the type <i>const char[LENGTH]</i> "
//        "so that copying the address of one into a non-<i>const "
//        "char *</i> pointer will get a warning. These warnings "
//        "will help you find at compile time code that can try to "
//        "write into a string constant, but only if you have been "
//        "very careful about using <i>const</i> in declarations "
//        "and prototypes. Otherwise, it will just be a nuisance; "
//        "this is why we did not make <i>-Wall</i> request these "
//        "warnings."));
