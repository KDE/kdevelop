#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include "duchainexport.h"

#include <language/duchain/types/abstracttype.h>

namespace KDevelop {
class IndexedType;
}

namespace TypeUtils
{
  KDEVCLANGDUCHAIN_EXPORT KDevelop::IndexedType removeConstModifier(const KDevelop::IndexedType& type);
  KDEVCLANGDUCHAIN_EXPORT void removeConstModifier(KDevelop::AbstractType::Ptr& type);
};

#endif // TYPEUTILS_H
