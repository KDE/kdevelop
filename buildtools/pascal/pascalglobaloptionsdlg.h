/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef PASCALGLOBALOPTIONSDLG_H
#define PASCALGLOBALOPTIONSDLG_H

#include <qmap.h>

#include <ktrader.h>

#include "pascalproject_optionsdlgbase.h"

class PascalProjectPart;

class PascalGlobalOptionsDlg : public PascalProjectOptionsDlgBase
{
    Q_OBJECT

public:
    PascalGlobalOptionsDlg(PascalProjectPart *part, QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~PascalGlobalOptionsDlg();

public slots:
    virtual void accept();

protected slots:
    virtual void optionsButtonClicked();
    virtual void compiler_box_activated(const QString& text);

private:
    PascalProjectPart *m_part;
    KTrader::OfferList offers;
    QString currentCompiler;
    QStringList service_names;
    QStringList service_execs;
    QMap<QString, QString> configCache;

    void saveCompilerOpts(QString compiler);
    void readCompilerOpts(QString compiler);

    void readConfigCache();
    void saveConfigCache();
};

#endif
