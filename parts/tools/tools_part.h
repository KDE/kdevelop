/*
 *  Copyright (C) 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>   
 */


#ifndef __KDEVPART_TOOLS_H__
#define __KDEVPART_TOOLS_H__


#include <qguardedptr.h>
#include <kdevpart.h>


class KDialogBase;


class ToolsWidget;


class ToolsPart : public KDevPart
  {
    Q_OBJECT

public:

    ToolsPart(KDevApi *api, QObject *parent = 0, const char *name = 0);
    ~ToolsPart();


private slots:

    void configWidget(KDialogBase *dlg);

    void updateMenu();

	void slotToolActivated();

  };


#endif
