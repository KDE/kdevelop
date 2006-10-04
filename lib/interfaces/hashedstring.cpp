#include "hashedstring.h"

void HashedString::initHash() {
  m_hash = 0;
  int len = m_str.length();
  for( int a = 0; a < len; a++ ) {
    m_hash += m_str[a].unicode() + (a*17);
  }
}
