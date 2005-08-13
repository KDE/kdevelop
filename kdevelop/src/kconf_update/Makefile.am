AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT $(all_includes)

update_DATA = kdev-gen-settings.upd 
updatedir = $(kde_datadir)/kconf_update

# The Qt app cannot go into kde_datadir, that is not portable.
# install to kde_bindir/kconf_update_bin instead.
# KDE 3.2 will allow kconf_update scripts to run directly from there,
# but for us that's too late. Use the .sh script as a workaround.
kconf_PROGRAMS = kdev-gen-settings-kconf_update
kconfdir = $(libdir)/kconf_update_bin

kdev_gen_settings_kconf_update_SOURCES = kdev-gen-settings-kconf_update.cpp
kdev_gen_settings_kconf_update_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdev_gen_settings_kconf_update_LDADD   = $(LIB_QT)

# vim: set noet:

