/***************************************************************************
Copyright 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "localpatchsource.h"

string LocalPatchSource::stateAsString() {
    switch ( state ) {
    case Applied:
        return "Applied";
    case  NotApplied:
        return "NotApplied";
    default:
        return "Unknown";
    };
}

