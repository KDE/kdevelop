/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "outputviewsfactory.h"

static const KDevPluginInfo data("kdevoutputviews", I18N_NOOP("Messages Output"), "1.0");

K_EXPORT_COMPONENT_FACTORY( libkdevoutputviews, OutputViewsFactory( &data ) )

const KDevPluginInfo *outputViewsInfo()
{
    return data;
}
