/*
 * $Id:$
 *  Copyright (C) 2003 Ian Reinhart Geiser <geiseri@kde.org>
 */


#ifndef __KDEVPART_BASHSUPPORT_H__
#define __KDEVPART_BASHSUPPORT_H__


#include <qguardedptr.h>

#include <kprocess.h>
#include <kdialogbase.h>

#include "kdevplugin.h"
#include "kdevlanguagesupport.h"


//class BashSupportWidget;


class BashSupportPart : public KDevLanguageSupport
{
  Q_OBJECT

public:

  BashSupportPart(QObject *parent, const char *name, const QStringList &);
  virtual ~BashSupportPart();

protected:
    virtual Features features();
    virtual QStringList fileFilters();

private slots:
    void slotRun();
    void projectConfigWidget(KDialogBase *dlg);
    void projectOpened();
    void projectClosed();
    void savedFile(const QString &fileName);
    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);
    void parse();
private:
    void startApplication(const QString &program);
    QString interpreter();
     void parse(const QString &fileName);

};


#endif
