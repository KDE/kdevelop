/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IOUTPUTVIEWMODEL_H
#define KDEVPLATFORM_IOUTPUTVIEWMODEL_H

#include "outputviewexport.h"

#include <QObject>

class QModelIndex;

namespace KDevelop
{

class KDEVPLATFORMOUTPUTVIEW_EXPORT IOutputViewModel
{
public:
    virtual ~IOutputViewModel();

    /**
     * Called when the index @arg index was activated in output view.
     */
    virtual void activate( const QModelIndex& index ) = 0;

    /**
     * Called when the user wants to see first item. For example, in makebuilder it would be
     * first error spot.
     *
     * @return First model index that is to be highlighted and activated.
     *  Return invalid index if no appropriate item to highlight exists.
     */
    virtual QModelIndex firstHighlightIndex() = 0;

    /**
     * Called when the user wants to see next item. For example, in makebuilder it would be
     * next error spot. In subversion plugin it would be the next conflicted item.
     *
     * @param currentIndex Currently selected index in active outputview. It can be invalid index
     *  if no item is selected or highlighted.
     * @return Next model index that is to be highlighted and activated.
     *  Return invalid index if no appropriate item to highlight exists.
     */
    virtual QModelIndex nextHighlightIndex( const QModelIndex& currentIndex ) = 0;

    /**
     * Called when the user wants to see previous item. For example, in makebuilder it would be
     * previous error spot. In subversion plugin it would be the previous conflicted item.
     *
     * @param currentIndex Currently selected index in active outputview. It can be invalid index
     *  if no item is selected or highlighted.
     * @return Previous model index that is to be highlighted and activated.
     *  Return invalid index if no appropriate item to highlight exists.
     */
    virtual QModelIndex previousHighlightIndex( const QModelIndex& currentIndex ) = 0;

    /**
     * Called when the user wants to see last item. For example, in makebuilder it would be
     * last error spot.
     *
     * @return Last model index that is to be highlighted and activated.
     *  Return invalid index if no appropriate item to highlight exists.
     */
    virtual QModelIndex lastHighlightIndex() = 0;

};

}

Q_DECLARE_INTERFACE(KDevelop::IOutputViewModel, "org.kdevelop.IOutputViewModel")

#endif

