/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "projecttemplateitem.h"

ProjectTemplateItem::ProjectTemplateItem()
    :QStandardItem()
{
    init();
}

ProjectTemplateItem::ProjectTemplateItem(const QIcon & icon, const QString & text)
    :QStandardItem(icon, text)
{
    init();
}

ProjectTemplateItem::ProjectTemplateItem(const QString & text)
    :QStandardItem(text)
{
    init();
}

ProjectTemplateItem::ProjectTemplateItem(int rows, int columns)
    :QStandardItem(rows, columns)
{
    init();
}

void ProjectTemplateItem::init()
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
