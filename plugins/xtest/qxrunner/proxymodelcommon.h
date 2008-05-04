/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/*!
 * \file  proxymodelcommon.h
 *
 * \brief Declares class ProxyModelCommon.
 */

#ifndef PROXYMODELCOMMON_H
#define PROXYMODELCOMMON_H

#include <QBitArray>

namespace QxRunner
{

/*!
 * \brief The ProxyModelCommon class defines a set of common methods
 *        for sorting filter proxy models.
 *
 * This class should be seen as a 'static decorator' for the proxy
 * model classes. It is not intended to be used directly, but must
 * be subclassed.
 *
 * \sa \ref views
 */

class ProxyModelCommon
{
public: // Operations

    /*!
     * Constructs a proxy model common. The active flag is set to true.
     */
    ProxyModelCommon();

    /*!
     * Destroys this proxy model common.
     */
    virtual ~ProxyModelCommon();

    /*!
     * Returns the active flag.
     */
    bool isActive() const;

    /*!
     * Sets the \a active flag.
     */
    void setActive(bool active);

    /*!
     * Returns the array with the flags for columns.
     */
    QBitArray enabledColumns() const;

    /*!
     * Sets the enabled flags for columns. True at the corresponding
     * column position in \a enabledColumns enables, false disables
     * that column.
     */
    void setEnabledColumns(const QBitArray& enabledColumns);

    /*!
     * Returns the enabled flag for \a column.
     */
    bool isColumnEnabled(int column) const;

private: // Operations

    // Copy and assignment not supported.
    ProxyModelCommon(const ProxyModelCommon&);
    ProxyModelCommon& operator=(const ProxyModelCommon&);

private: // Attributes

    bool m_active;

    QBitArray m_enabledColumns;
};

} // namespace

#endif // PROXYMODELCOMMON_H
