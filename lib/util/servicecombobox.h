/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SERVICECOMBOBOX_H_
#define _SERVICECOMBOBOX_H_

#include <qcombobox.h>
#include <kservice.h>


/**
 * A combo box that allows the user to choose between a set of
 * services (typically obtained from a KTrader query).
 * The user is presented the clear text descriptions of the
 * services obtained from the comment field in the .desktop
 * file file, whereas the programmer is only confronted with
 * the service name, as returned by currentText().
 */
class ServiceComboBox : public QComboBox
{
public:
    /**
     * Constructs a service combo box that presents the
     * comment field of the respective service to the user.
     */
    ServiceComboBox( const QValueList<KService::Ptr> &list,
                     QWidget *parent=0, const char *name=0 );
#if 0
    /**
     * Constructs a service combo box that presents the
     * field given by propertyName to the user.
     */
    ServiceComboBox( const QValueList<KService::Ptr> &list,
                     const QString &propertyName,
                     QWidget *parent=0, const char *name=0 );
#endif
    ~ServiceComboBox();

    QString currentText() const;
    void setCurrentText(const QString &str);

private:
    QStringList names;
};

#endif
