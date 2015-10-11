/// "useCount" : 1,
/// "EXPECT_FAIL": {"useCount": "friend statements are unexposed in clang-c and thus not visited"}
void myFriendFunction();

/// "useCount" : 1,
/// "EXPECT_FAIL": {"useCount": "friend statements are unexposed in clang-c and thus not visited"}
struct myFriendStruct
{
    /// "useCount" : 1,
    /// "EXPECT_FAIL": {"useCount": "friend statements are unexposed in clang-c and thus not visited"}
    void myFriendMemberFunction();
};

/**
 * "internalContext" : { "localDeclarationCount" : 0 }
 */
struct iHaveFriends
{
    friend struct myFriendStruct;
    friend void myFriendStruct::myFriendMemberFunction();
    friend void myFriendFunction();
};
