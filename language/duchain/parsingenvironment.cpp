/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

namespace KDevelop
{

class IdentifiedFilePrivate
{
public:
  IdentifiedFilePrivate() : m_identity(0) {
  }
  HashedString m_url;
  uint m_identity;
};

IdentifiedFile::IdentifiedFile()
  : d(new IdentifiedFilePrivate)
{
}

IdentifiedFile::IdentifiedFile( const HashedString& url , uint identity )
  : d(new IdentifiedFilePrivate)
{
  d->m_url = url;
  d->m_identity = identity;
}

IdentifiedFile::~IdentifiedFile() {
 delete d;
}

HashedString IdentifiedFile::url() const {
  return d->m_url;
}

QString IdentifiedFile::toString() const {
  return QString("%1 %2").arg(url().str()).arg(identity());
}

uint IdentifiedFile::identity() const {
  return d->m_identity;
}

bool IdentifiedFile::operator<( const IdentifiedFile& rhs ) const {
  return d->m_url < rhs.url() || (d->m_url == rhs.url() && d->m_identity < rhs.identity() );
}

IdentifiedFile::IdentifiedFile(const IdentifiedFile& rhs) : d(new IdentifiedFilePrivate) {
  d->m_url = rhs.url();
  d->m_identity = rhs.identity();
}

IdentifiedFile& IdentifiedFile::operator=( const IdentifiedFile& rhs ) {
  d->m_url = rhs.url();
  d->m_identity = rhs.identity();
  return *this;
}

bool IdentifiedFile::isEmpty() const {
  return d->m_url.str().isEmpty();
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

ParsingEnvironmentFile* ParsingEnvironmentManager::find( const HashedString& /*url*/, const ParsingEnvironment* /*environment*/, ParsingEnvironmentFileAcceptor* ) {
  return 0;
}

} //KDevelop
