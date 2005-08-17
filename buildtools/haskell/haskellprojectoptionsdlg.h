/***************************************************************************
                          haskellprojectoptionsdlg.h  -  description
                             -------------------
    begin                : Fri Aug 15 2003
    copyright            : (C) 2003 by KDevelop Authors
    email                : kdevelop-devel@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HASKELLPROJECTOPTIONSDLG_H
#define HASKELLPROJECTOPTIONSDLG_H


#include <ktrader.h>

#include "haskellproject_optionsdlgbase.h"
#include "haskellproject_part.h"
//Added by qt3to4:
#include <Q3ValueList>

/**
  *@author KDevelop Authors
  */

class HaskellProjectOptionsDlg : public HaskellProjectOptionsDlgBase
{
   Q_OBJECT
public:
  	HaskellProjectOptionsDlg( HaskellProjectPart *part, QWidget* parent = 0,
  				 							    const char* name = 0, Qt::WFlags fl = 0 );
	~HaskellProjectOptionsDlg();

public slots:
  	virtual void accept();

protected slots:
  	virtual void compiler_box_activated(const QString &s);
  	void configComboTextChanged(const QString &config);
  	void configChanged(const QString &config);
  	void configAdded();
  	void configRemoved();
  	void optionsButtonClicked();
  	void setDirty();
  	void setDefaultOptions();

private:
  	QStringList _allConfigs;
  	QString _currentConfig;
  	bool _dirty;

  	KTrader::OfferList offers;
  	QStringList _serviceNames;
  	QStringList _serviceExecs;
  	HaskellProjectPart *_part;

  	void saveConfig(QString config);
  	void readConfig(QString config);
  	QStringList allBuildConfigs();
  	void insertServicesIntoDlg( const Q3ValueList<KService::Ptr> &list );
	void setCurrentCompBoxText( const QString &str, const QStringList &names);    
  	QString currentCompBoxText( const QStringList &names );
	QString defaultCompiler();
  	int itemForText(const QString &str, const QStringList &names);
};

#endif
