macro_optional_add_subdirectory(duchainviewer)
macro_optional_add_subdirectory(konsole)
add_subdirectory(filemanager)
add_subdirectory(projectmanagerview)
add_subdirectory(genericprojectmanager)
add_subdirectory(standardoutputview)
macro_optional_add_subdirectory(snippet)
#macro_optional_add_subdirectory(cvs)

############################################################################
# Teamwork is disabled until somebody figures out why it doesn't build with
# boost 1.33.1:
#/usr/src/packages/BUILD/kdevplatform-673194/plugins/teamwork/lib/network/serverconfig.cpp:76:   instantiated from here
#/usr/include/boost/archive/detail/oserializer.hpp:131: error: invalid use of undefined type 'struct boost::serialization::extended_type_info_null<Teamwork::User>'
#/usr/include/boost/serialization/type_info_implementation.hpp:26: error: declaration of 'struct boost::serialization::extended_type_info_null<Teamwork::User>'
############################################################################
#find_package(Commoncpp)
#find_package(BoostLibraries)
#if(COMMONCPP2_FOUND AND Boost_FOUND AND Boost_LIBS_FOUND)
#macro_optional_add_subdirectory(teamwork)
#endif(COMMONCPP2_FOUND AND Boost_FOUND  AND Boost_LIBS_FOUND)

find_package(SubversionLibrary)
if(SUBVERSION_FOUND)
    macro_optional_add_subdirectory(subversion)
endif(SUBVERSION_FOUND)


