/*
* This file is part of KDevelop
*
* Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef KDEVPLATFORM_URLPARSELOCK_H
#define KDEVPLATFORM_URLPARSELOCK_H

#include "../duchain/indexedstring.h"

namespace KDevelop
{

/**
 * This is used to prevent the background parser from updating the duchain for a specific file.
 * It can be used to prevent changes while working on the duchain.
 *
 * Every language-specific parse-job has to lock this before updating a TopDUContext.
 *
 * @warning No other mutex must be locked when this lock is acquired, to prevent deadlocks
 */
class KDEVPLATFORMLANGUAGE_EXPORT UrlParseLock
{
public:
  UrlParseLock(const IndexedString& url);
  ~UrlParseLock();

private:
  IndexedString m_url;
};

}

#endif
