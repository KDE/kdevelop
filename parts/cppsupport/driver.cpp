/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   raggi@cli.di.unipi.it                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "driver.h"
#include <kdebug.h>

Driver::Driver()
{

}

Driver::~Driver()
{

}

void Driver::pushScope( const QString& scopeName )
{
    m_currentScope.push_back( scopeName );
}

void Driver::popScope()
{
    m_currentScope.pop_back();
}
