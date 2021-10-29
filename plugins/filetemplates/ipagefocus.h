/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_IPAGEFOCUS_H
#define KDEVPLATFORM_PLUGIN_IPAGEFOCUS_H

namespace KDevelop
{

class IPageFocus
{
public:
    virtual ~IPageFocus();

    /**
     * Set the keyboard input focus to the first edit widget.
     */
    virtual void setFocusToFirstEditWidget() = 0;
};

}

#endif
