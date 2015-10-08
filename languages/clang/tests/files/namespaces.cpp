namespace bar { namespace asdf {} };

/// "toString" : "Import bar as blub",
/// "kind" : "NamespaceAlias"
namespace blub = bar;

/// "toString" : "Import of blub::asdf",
/// "kind" : "NamespaceAlias"
using namespace blub::asdf;
