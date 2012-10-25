set( test_filetemplate_DIRS
    test_yaml
    test_cpp
)

include(KDevPlatformMacros)

foreach(_templateName ${test_filetemplate_DIRS})
    kdevplatform_create_template_archive(${_templateName})
endforeach(_templateName ${test_filetemplate_DIRS})
