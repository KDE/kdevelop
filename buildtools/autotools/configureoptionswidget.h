/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef _CONFIGUREOPTIONSWIDGET_H_
#define _CONFIGUREOPTIONSWIDGET_H_

#include "configureoptionswidgetbase.h"

#include <ktrader.h>
#include "domutil.h"


class KDevCompilerOptions;
class AutoProjectPart;
class EnvironmentVariablesWidget;

class ConfigureOptionsWidget : public ConfigureOptionsWidgetBase
{
	Q_OBJECT

public:
	ConfigureOptionsWidget( AutoProjectPart *part, QWidget *parent = 0, const char *name = 0 );
	~ConfigureOptionsWidget();

public slots:
	void accept();

private:
	virtual void builddirClicked();
	virtual void topsourcedirClicked();
	virtual void setDirty();
	virtual void configChanged( const QString &config );
	virtual void configComboTextChanged( const QString &config );
	virtual void configAdded();
	virtual void configRemoved();
	virtual void cflagsClicked();
	virtual void cxxflagsClicked();
	virtual void f77flagsClicked();
	virtual void cserviceChanged();
	virtual void cxxserviceChanged();
	virtual void f77serviceChanged();

	void fixLayout();
	void readSettings( const QString &config );
	void saveSettings( const QString &config );

	KDevCompilerOptions *createCompilerOptions( const QString &lang );
	KTrader::OfferList coffers, cxxoffers, f77offers;
	QStringList cservice_names, cservice_execs;
	QStringList cxxservice_names, cxxservice_execs;
	QStringList f77service_names, f77service_execs;
	QStringList allConfigs;
	QString currentConfig;
	bool dirty;

	AutoProjectPart *m_part;
	EnvironmentVariablesWidget* m_environmentVariablesWidget;
};

#endif 
// kate: indent-mode csands; tab-width 4;
