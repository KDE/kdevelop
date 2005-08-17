/***************************************************************************
                          haskellsupport_part.h  -  description
                             -------------------
    begin                : Mon Aug 11 2003
    copyright            : (C) 2003 Peter Robinson
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

#ifndef KDEVPART_HASKELLSUPPORT_PART_H
#define KDEVPART_HASKELLSUPPORT_PART_H

#include <qwidget.h>
#include <qpointer.h>
//Added by qt3to4:
#include <Q3PopupMenu>

#include "kdevlanguagesupport.h"

class HaskellSupportWidget; // @todo remove this
class KDialogBase;
class Q3PopupMenu;
class Context;

class HaskellSupportPart : public KDevLanguageSupport
{
   Q_OBJECT

public:
	HaskellSupportPart(QObject *parent, const char *name, const QStringList &);
	~HaskellSupportPart();

  	virtual Features features();
  	virtual KMimeType::List mimeTypes();

private slots:
	void projectOpened();
	void projectClosed();
  	void savedFile(const KURL &fileName);
  	void configWidget(KDialogBase *dlg);
	void projectConfigWidget(KDialogBase *dlg);
  	void contextMenu(Q3PopupMenu *popup, const Context *context);

  	void addedFilesToProject(const QStringList &fileList);
  	void removedFilesFromProject(const QStringList &fileList);
  	void slotProjectCompiled();

	void slotInitialParse();

private:
  	void maybeParse(const QString &fileName);
  	void parse(const QString &fileName);

  	QPointer<HaskellSupportWidget> m_widget;
  	bool m_projectClosed;
  	QStringList m_projectFileList;
};

#endif
