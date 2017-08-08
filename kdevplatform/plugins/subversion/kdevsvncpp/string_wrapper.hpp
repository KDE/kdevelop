/*
 * ====================================================================
 * Copyright (c) 2002-2009 The RapidSvn Group.  All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the file GPL.txt.  
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */

#ifndef _SVNCPP_STRING_WRAPPER_H_
#define _SVNCPP_STRING_WRAPPER_H_

// Ignore MSVC 6 compiler warning
#if defined (_MSC_VER) && _MSC_VER <= 1200
// debug symbol truncated
#pragma warning (disable: 4786)
// C++ exception specification
#pragma warning (disable: 4290)
// conflict between signed and unsigned
#pragma warning (disable: 4018)
#endif

// Ignore MSVC 7,8,9 compiler warnings
#if defined (_MSC_VER) && _MSC_VER > 1200 && _MSC_VER <= 1500
// C++ exception specification
#pragma warning (disable: 4290)
#endif

#include <string>

// re-enable warnings that are us
#if defined (_MSC_VER) && _MSC_VER <= 1200
#pragma warning (default: 4018)
#endif

#if defined (_MSC_VER) && _MSC_VER > 1200 && _MSC_VER <= 1500
#endif



#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
