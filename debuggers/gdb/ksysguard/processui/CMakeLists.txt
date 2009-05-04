
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../  ../processcore/)

check_include_files(sys/ptrace.h HAVE_SYS_PTRACE_H)
check_include_files(sys/endian.h HAVE_SYS_ENDIAN_H)
check_include_files(byteswap.h HAVE_BYTESWAP_H)

if (HAVE_SYS_PTRACE_H)
   set(_SUPPORTED_REGS_STRUCT_CHECK "
   #include <sys/user.h>
   #include <sys/syscall.h>

   #if defined(__i386__)
   #  define _regs_struct user_regs_struct
   #  define REG_ORIG_ACCUM orig_eax
   #elif defined( __amd64__)
   #  define _regs_struct user_regs_struct
   #  define REG_ORIG_ACCUM orig_rax
   #elif defined(__ppc__) || defined(__powerpc__) || defined(__powerpc64__) || defined(__PPC__) || defined(powerpc)
   #  define _regs_struct pt_regs;
   #  define REG_ORIG_ACCUM gpr[0]
   #endif

   int main() {
     struct _regs_struct* regs;
     return regs->REG_ORIG_ACCUM == SYS_fork;
   }")

   include(CheckCSourceCompiles)
   check_c_source_compiles("${_SUPPORTED_REGS_STRUCT_CHECK}" HAVE_KSYSGUARD_SUPPORTED_REGS_STRUCT)
else (HAVE_SYS_PTRACE_H)
   # Even if we don't have ptrace, we need KMonitorProcessIO.cpp
   # because the widget is referenced from the .widgets file and
   # from the .ui files.
   set(HAVE_KSYSGUARD_SUPPORTED_REGS_STRUCT "NO")
endif (HAVE_SYS_PTRACE_H)

if (HAVE_KSYSGUARD_SUPPORTED_REGS_STRUCT)
   add_definitions(-DWITH_MONITOR_PROCESS_IO)
   set(processui_ptrace_SRCS KMonitorProcessIO.cpp DisplayProcessDlg.cpp)
   set(ksysguard_WIDGETS "${CMAKE_CURRENT_BINARY_DIR}/ksysguard.generated_widgets")

   # Merge both widget files
   file(READ ksysguard.widgets _ksysguard_buffer)
   file(WRITE ${ksysguard_WIDGETS} "${_ksysguard_buffer}")
   file(READ KMonitorProcessIO.widgets _ksysguard_buffer)
   file(APPEND ${ksysguard_WIDGETS} "${_ksysguard_buffer}")
else (HAVE_KSYSGUARD_SUPPORTED_REGS_STRUCT)
   set(processui_ptrace_SRCS )
   set(ksysguard_WIDGETS "ksysguard.widgets")
endif (HAVE_KSYSGUARD_SUPPORTED_REGS_STRUCT)

set(processui_LIB_SRCS
   ksysguardprocesslist.cpp
   ProcessFilter.cc
   ProcessModel.cc
   ReniceDlg.cc
   KTextEditVT.cpp
   ${processui_ptrace_SRCS}
)

kde4_add_ui_files( processui_LIB_SRCS
   ReniceDlgUi.ui
   ProcessWidgetUI.ui
   DisplayProcessUi.ui
)

 #while it is not in kdelibs, we change its name
kde4_add_library(kdev_processui SHARED ${processui_LIB_SRCS})

target_link_libraries(kdev_processui ${KDE4_KDEUI_LIBS} processcore)
target_link_libraries(kdev_processui LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS} processcore)

set_target_properties(kdev_processui
   PROPERTIES VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

install(TARGETS kdev_processui EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

#----------------------

kde4_add_widget_files(ksysguardwidgets_PART_SRCS ${ksysguard_WIDGETS})

kde4_add_plugin(ksysguardwidgets ${ksysguardwidgets_PART_SRCS})

target_link_libraries(ksysguardwidgets ${KDE4_KDEUI_LIBS} processui)

install(TARGETS ksysguardwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )

#install( FILES ProcessModel.h DisplayProcessDlg.h ProcessFilter.h KTextEditVT.h ksysguardprocesslist.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)
