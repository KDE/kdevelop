/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef PROJECTTEMPLATEITEM_H
#define PROJECTTEMPLATEITEM_H

#include <QStandardItem>

class ProjectTemplateItem: public QStandardItem {
public:
    ProjectTemplateItem();
    ProjectTemplateItem(const QString &text);
    ProjectTemplateItem(const QIcon &icon, const QString &text);
    explicit ProjectTemplateItem(int rows, int columns = 1);

private:
    void init();

};

#endif

