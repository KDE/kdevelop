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
    virtual KMimeType::List mimeTypes();

private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void projectOpened();
    void savedFile(const KURL &fileName);
    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);
    void slotRun();

    void initialParse();

private:
    void startApplication(const QString &program);
    QString interpreter();

    void maybeParse(const QString fileName);
    void parse(const QString &fileName);
};


#endif
