/* 
   Copyright (C) 2007 David Nolden <user@host.de>
   (where user = david.nolden.kdevelop, host = art-master)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "parsingenvironment.h"

using namespace KDevelop;

IdentifiedFile::IdentifiedFile() : m_identity(0) {
}

IdentifiedFile::IdentifiedFile( const KUrl& url , uint identity ) : m_url(url ), m_identity(identity) {
}

KUrl IdentifiedFile::url() const {
  return m_url;
}

QString IdentifiedFile::toString() const {
  return QString("%1 %2").arg(url().prettyUrl()).arg(identity());
}

uint IdentifiedFile::identity() const {
  return m_identity;
}

bool IdentifiedFile::operator<( const IdentifiedFile& rhs ) const {
  return m_url < rhs.m_url || (m_identity != 0 && m_identity < rhs.m_identity );
}

bool IdentifiedFile::isEmpty() const {
  return m_url.isEmpty();
}

IdentifiedFile::operator bool() const {
  return !isEmpty();
}

ParsingEnvironment::~ParsingEnvironment() {
}

ParsingEnvironmentManager::~ParsingEnvironmentManager() {
}

void ParsingEnvironmentManager::clear() {
}

ParsingEnvironmentFile::~ParsingEnvironmentFile() {
}

int ParsingEnvironment::type() const {
  return StandardParsingEnvironment;
}

int ParsingEnvironmentManager::type() const {
  return StandardParsingEnvironment;
}

int ParsingEnvironmentFile::type() const {
  return StandardParsingEnvironment;
}

void ParsingEnvironmentManager::addFile( ParsingEnvironmentFile* /*file*/ ) {
}

    ///Remove a file from the manager
void ParsingEnvironmentManager::removeFile( ParsingEnvironmentFile* /*file*/ ) {
}

ParsingEnvironmentFile* ParsingEnvironmentManager::find( const KUrl& /*url*/, const ParsingEnvironment* /*environment*/ ) {
  return 0;
}
