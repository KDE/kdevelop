/***************************************************************************
 *   Copyright (C) 2002 by Yann Hodique                                    *
 *   Yann.Hodique@lifl.fr                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _GLOBALCVS_H_
#define _GLOBALCVS_H_

#include <qwidget.h>

#include "kdevglobalversioncontrol.h"

class CvsForm;

class GlobalCvs : public KDevGlobalVersionControl {
    Q_OBJECT

public:
    GlobalCvs(QObject *parent, const char *name, const QStringList &);
    ~GlobalCvs();

    QWidget * newProjectWidget(QWidget *parent);
    void createNewProject(QString dir);

public slots:
    void slotImportCvs();

private:
    CvsForm * form;
};

#endif
