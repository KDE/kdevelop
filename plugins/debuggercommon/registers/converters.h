/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONVERTERS_H
#define CONVERTERS_H

#include "registercontroller.h"

namespace KDevMI {

class Converters
{
public:
    static QString formatToString(Format format);
    static Format stringToFormat(const QString& format);

    static Mode stringToMode(const QString& mode);
    static QString modeToString(Mode mode);
};

} // end of namespace KDevMI
#endif // CONVERTERS_H
