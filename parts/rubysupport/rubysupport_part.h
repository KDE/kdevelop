/*
 *  Copyright (C) 2003 Marek Janukowicz <marek@janukowicz.net>
 */


#ifndef __KDEVPART_RUBYSUPPORT_H__
#define __KDEVPART_RUBYSUPPORT_H__


#include <qguardedptr.h>

#include <kprocess.h>
#include <kdialogbase.h>

#include "kdevplugin.h"
#include "kdevlanguagesupport.h"


class RubySupportWidget;


class RubySupportPart : public KDevLanguageSupport
{
  Q_OBJECT

public:

  RubySupportPart(QObject *parent, const char *name, const QStringList &);
  virtual ~RubySupportPart();

protected:
    virtual Features features();
    virtual QStringList fileFilters();

private slots:
    void slotRun();
    void projectConfigWidget(KDialogBase *dlg);

private:
    void startApplication(const QString &program);
    QString interpreter();

};


#endif
