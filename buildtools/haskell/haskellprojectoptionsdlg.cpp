/***************************************************************************
                          haskellprojectoptionsdlg.cpp  -  description
                             -------------------
    begin                : Fri Aug 15 2003
    copyright            : (C) 2003 by Peter Robinson
    email                : listener@thaldyron.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kurlrequester.h>
#include <kservice.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qvalidator.h>
//Added by qt3to4:
#include <Q3ValueList>

#include "domutil.h"
#include "kdevcompileroptions.h"

#include "haskellproject_part.h"
#include "haskellprojectoptionsdlg.h"

HaskellProjectOptionsDlg::HaskellProjectOptionsDlg( HaskellProjectPart *part, QWidget* parent,
  				 							   const char* name, Qt::WFlags fl )
    : HaskellProjectOptionsDlgBase( parent, name, fl ),
      _part( part )          
{
	config_combo->setValidator( new QRegExpValidator( QRegExp( "^\\D.*" ), this ) );

 	offers = KTrader::self()->query( "KDevelop/CompilerOptions",
  									 "[X-KDevelop-Language] == 'Haskell'" );
  	insertServicesIntoDlg( offers );

  	if ( offers.isEmpty() ) {
    	options_button->setEnabled(false);
	}
	
  	_allConfigs = allBuildConfigs();
  	config_combo->insertStringList( _allConfigs );

  	_dirty = false;

  	QDomDocument &dom = *( _part->projectDom() );
  	_currentConfig = QString::null;
  	configChanged(DomUtil::readEntry(dom,
  								"/kdevhaskellproject/general/useconfiguration",
          						"default" ) );
}

HaskellProjectOptionsDlg::~HaskellProjectOptionsDlg()
{
}

QStringList HaskellProjectOptionsDlg::allBuildConfigs()
{
    QDomDocument &dom = *(_part->projectDom());

    QStringList allConfigs;
    allConfigs.append("default");

    QDomNode node = dom.documentElement().namedItem("kdevhaskellproject").namedItem("configurations");
    QDomElement childEl = node.firstChild().toElement();

    while (!childEl.isNull()) {
        QString config = childEl.tagName();
        kdDebug() << "Found config " << config << endl;
        if (config != "default")
            allConfigs.append(config);
        childEl = childEl.nextSibling().toElement();
    }

    return allConfigs;
}

void HaskellProjectOptionsDlg::accept()
{
    DomUtil::writeEntry(*_part->projectDom(),
    					"/kdevhaskellproject/general/useconfiguration",
         				_currentConfig);
    if (_dirty)
    {
        saveConfig(_currentConfig);
    }
}

void HaskellProjectOptionsDlg::compiler_box_activated( const QString& )
{
  	QString exec = currentCompBoxText( _serviceExecs );
  	exec_edit->setText( exec );
}

void HaskellProjectOptionsDlg::saveConfig( QString config )
{
    QDomDocument dom = *_part->projectDom();
    QString prefix = "/kdevhaskellproject/configurations/" + config + "/";

    DomUtil::writeEntry( dom, prefix + "compiler",
                         currentCompBoxText( _serviceNames ) );
    DomUtil::writeEntry( dom, prefix + "compileroptions", options_edit->text());
    DomUtil::writeEntry( dom, prefix + "compilerexec", exec_edit->text() );
    DomUtil::writeEntry( dom, prefix + "mainsource",
    					mainSourceUrl->url().replace( QRegExp( _part->projectDirectory() +
         													   QString( "/" ) ),"" ) );
}

void HaskellProjectOptionsDlg::readConfig( QString config )
{
    QDomDocument dom = *_part->projectDom();
    QString prefix = "/kdevhaskellproject/configurations/" + config + "/";

    QString compiler = DomUtil::readEntry(dom, prefix + "compiler", "");

    if (compiler.isEmpty())
    {
        offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Haskell'");
        Q3ValueList<KService::Ptr>::ConstIterator it;
        for (it = offers.begin(); it != offers.end(); ++it) {
            if ((*it)->property("X-KDevelop-Default").toBool()) {
                compiler = (*it)->name();
                kdDebug() << "compiler is " << compiler << endl;
                break;
            }
        }
    }
    setCurrentCompBoxText( compiler, _serviceNames );

    QString exec = DomUtil::readEntry(dom, prefix + "compilerexec", "");
    if (exec.isEmpty())
      exec = currentCompBoxText( _serviceExecs );
    exec_edit->setText(exec);
    options_edit->setText(DomUtil::readEntry(dom, prefix + "compileroptions"));
    mainSourceUrl->setURL(_part->projectDirectory() + "/"
    					  + DomUtil::readEntry(dom, prefix + "mainsource") ); 
}

void HaskellProjectOptionsDlg::configComboTextChanged(const QString &config)
{
    bool canAdd = !_allConfigs.contains(config) && !config.contains("/")
    			  && !config.isEmpty();
    bool canRemove = _allConfigs.contains(config) && config != "default";
    addconfig_button->setEnabled(canAdd);
    removeconfig_button->setEnabled(canRemove);
}


void HaskellProjectOptionsDlg::configChanged(const QString &config)
{
    if (config == _currentConfig || !_allConfigs.contains(config))
        return;

    if (!_currentConfig.isNull() && _dirty)
        saveConfig(_currentConfig);

    _currentConfig = config;
    readConfig(config);
    _dirty = false;

    config_combo->blockSignals(true);
    config_combo->setEditText(config);
    config_combo->blockSignals(false);
}


void HaskellProjectOptionsDlg::configAdded()
{
    QString config = config_combo->currentText();

    _allConfigs.append(config);

    config_combo->clear();
    config_combo->insertStringList(_allConfigs);
    configChanged(config);
    setDirty(); // force saving
}


void HaskellProjectOptionsDlg::configRemoved()
{
    QString config = config_combo->currentText();

    QDomDocument dom = *_part->projectDom();
    QDomNode node = dom.documentElement().namedItem( "kdevhaskellproject" ).namedItem("configurations");
    node.removeChild(node.namedItem(config));
    _allConfigs.remove(config);

    config_combo->clear();
    config_combo->insertStringList( _allConfigs );

    _currentConfig = QString::null;
    configChanged( "default" );
}

void HaskellProjectOptionsDlg::optionsButtonClicked( )
{
    QString name = currentCompBoxText( _serviceNames );
    KDevCompilerOptions *plugin = _part->createCompilerOptions( name );

    if ( plugin ) {
        QString flags = plugin->exec(this, options_edit->text());
        options_edit->setText(flags);
        delete plugin;
    }
}

void HaskellProjectOptionsDlg::setDirty( )
{
    _dirty = true;
}

void HaskellProjectOptionsDlg::setDefaultOptions( )
{
	// @todo FIXME!
    if (!compiler_box->currentText().isEmpty()) {
        options_edit->setText( _part->defaultOptions( compiler_box->currentText() ) );
    }
}

void HaskellProjectOptionsDlg::insertServicesIntoDlg( const Q3ValueList<KService::Ptr> &list )
{
    Q3ValueList<KService::Ptr>::ConstIterator it;
    for( it = list.begin(); it != list.end(); ++it ) {
        compiler_box->insertItem( (*it)->comment() );
        _serviceNames << (*it)->name();
        _serviceExecs << (*it)->exec();
        kdDebug() << "insertStringList item " << (*it)->name() << "," << (*it)->exec() << endl;
    }
}

QString HaskellProjectOptionsDlg::currentCompBoxText( const QStringList &names )
{
    if( compiler_box->currentItem() == -1 ) {
      return QString::null;
    }
    else {
	    return names[compiler_box->currentItem()];
	  }
}


void HaskellProjectOptionsDlg::setCurrentCompBoxText( const QString &str, const QStringList &names)
{
    QStringList::ConstIterator it;
    int i = 0;
    for (it = names.begin(); it != names.end(); ++it) {
        if (*it == str) {
            compiler_box->setCurrentItem(i);
            break;
        }
        ++i;
    }
}

QString HaskellProjectOptionsDlg::defaultCompiler()
{
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Haskell'");
    Q3ValueList<KService::Ptr>::ConstIterator it;
    for (it = offers.begin(); it != offers.end(); ++it) {
        if ((*it)->property("X-KDevelop-Default").toBool()) {
            return (*it)->name();;
        }
    }
    return "";
}

int HaskellProjectOptionsDlg::itemForText(const QString &str, const QStringList &names)
{
    QStringList::ConstIterator it;
    int i = 0;
    for( it = names.begin(); it != names.end(); ++it ) {
        if( *it == str ) {
            return i;
        }
        ++i;
    }
    return 0;
}

#include "haskellprojectoptionsdlg.moc"
