/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DRIVER_H
#define DRIVER_H

#include <qvaluestack.h>
#include <qstringlist.h>

class Driver{
public:
    Driver();
    ~Driver();

    // static chain
    void pushScope( const QString& scopeName );
    void popScope();

private:
    QStringList m_currentScope;
};

#endif
