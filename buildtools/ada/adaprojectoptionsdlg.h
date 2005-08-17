/* Copyright (C) 2003 Oliver Kellogg
 * okellogg@users.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef ADAPROJECTOPTIONSDLG_H
#define ADAPROJECTOPTIONSDLG_H

#include <ktrader.h>

#include "adaproject_optionsdlgbase.h"

class AdaProjectPart;
class KDevCompilerOptions;

class AdaProjectOptionsDlg : public AdaProjectOptionsDlgBase
{
  Q_OBJECT

public:
    AdaProjectOptionsDlg(AdaProjectPart *part, QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~AdaProjectOptionsDlg();

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
    QStringList allConfigs;
    QString currentConfig;
    bool dirty;

    KTrader::OfferList offers;
    QStringList service_names;
    QStringList service_execs;

    AdaProjectPart *m_part;

    void saveConfig(QString config);
    void readConfig(QString config);
    QStringList allBuildConfigs();
};

#endif

