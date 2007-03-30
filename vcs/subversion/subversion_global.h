#ifndef SUBVERSION_GLOBAL_H
#define SUBVERSION_GLOBAL_H

namespace SvnGlobal
{

typedef enum {
    path_to_reposit = 0,
    path_to_path = 1,
    dont_touch = 2
} UrlMode;

/// A structure which describes various system-generated metadata about
/// a working-copy path or URL.
class SvnInfoHolder {
public:
    // the requested path
    KURL path;
    /* Where the item lives in the repository. */
    KURL url;
    // The revision of the object.  If path_or_url is a working-copy
    // path, then this is its current working revnum.  If path_or_url
    // is a URL, then this is the repos revision that path_or_url lives in. */
    int rev;
    int kind;
    /* The root URL of the repository. */
    KURL reposRootUrl;
    QString reposUuid;
};

} // end of namespace SvnGlobal
#endif
