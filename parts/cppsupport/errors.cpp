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

#include "errors.h"
#include <klocale.h>

const Error& Errors::InternalError = Error( 1, -1, i18n("Internal Error") );
const Error& Errors::SyntaxError = Error( 2, -1, i18n("Syntax Error before '%1'") );
const Error& Errors::ParseError = Error( 3, -1, i18n("Parse Error before '%1'") );

