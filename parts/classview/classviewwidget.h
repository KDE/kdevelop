/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CLASSVIEWWIDGET_H_
#define _CLASSVIEWWIDGET_H_

#include "classtreebase.h"


class ClassViewWidget : public ClassTreeBase
{
    Q_OBJECT
    
public:
    ClassViewWidget(ClassViewPart *part);
    ~ClassViewWidget();

protected:
    virtual KPopupMenu *createPopup();

public slots:
    void refresh();

private slots:
    void setLanguageSupport(KDevLanguageSupport *ls);
    void slotTreeModeChanged();
    void slotScopeModeChanged();

private:
    void buildClassFolderHierarchy(ClassTreeItem *organizerItem,
                                   const QStringList &dirNames,
                                   QMap<QString, ClassTreeItem*> *folders);
    void buildClassFolderFlatList(ClassTreeItem *organizerItem,
                                  const QStringList &dirNames,
                                  QMap<QString, ClassTreeItem*> *folders);
    void buildTreeByCategory(bool fromScratch);
    void buildTreeByNamespace(bool fromScratch);
    void buildTree(bool fromScratch);

    static QString determineFolder(QString fileName, QString projectDir, int levels = -1);
};

#endif
