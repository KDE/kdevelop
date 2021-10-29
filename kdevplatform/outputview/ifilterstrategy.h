/*
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>
    SPDX-FileCopyrightText: 2016 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IFILTERSTRATEGY_H
#define KDEVPLATFORM_IFILTERSTRATEGY_H

#include "outputviewexport.h"

#include <QMetaType>
#include <QString>

namespace KDevelop
{

struct FilteredItem;

/**
* Interface class for filtering output. Filtered output is divided into two catagories: Errors
* and Actions. Use this interface if you want to write a filter for the outputview.
*
* @author Morten Danielsen Volden
*/
class KDEVPLATFORMOUTPUTVIEW_EXPORT IFilterStrategy
{
public:
    IFilterStrategy();
    virtual ~IFilterStrategy();

    struct Progress
    {
        Progress(const QString& status = QString(), int percent = -1)
            : status(status), percent(percent) {}

        QString status;   /// Status message (example: "Building foo.cpp")
        int percent; /// Percentage from 0-100; -1 indicates no progress could be parsed
    };

    /**
     * Examine if a given line contains output that is defined as an error (E.g. from a script or from a compiler, or other).
     * @param line the line to examine
     * @return FilteredItem with associated metadata if an error is found, an item of type InvalidItem otherwise
     **/
    virtual FilteredItem errorInLine(QString const& line) = 0;

    /**
     * Examine if a given line contains output that is defined as an action (E.g. from a script or from a compiler, or other).
     * @param line the line to examine
     * @return Filtered of type ActionItem with associated metadata if an action is found, an item of type InvalidItem otherwise
     **/
    virtual FilteredItem actionInLine(QString const& line) = 0;

    /**
     * Examine if a given line contains output which reports progress information
     *
     * E.g. `make` reports progress like this:
     * @code
     * [   5%] Doing something
     * [   6%] Doing something
     * @endcode
     *
     * @return Processed percent & status of the output, default implementation returns default-constructed value
     */
    virtual Progress progressInLine(const QString& line);

};

} // namespace KDevelop

Q_DECLARE_METATYPE(KDevelop::IFilterStrategy*)
Q_DECLARE_METATYPE(KDevelop::IFilterStrategy::Progress)

#endif // KDEVPLATFORM_IFILTERSTRATEGY_H
