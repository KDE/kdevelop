# This spec file was generated using Kpp
# If you find any problems with this spec file please report
# the error to ian geiser <geiseri@msoe.edu>
Summary:   %%info%%
Name:      %%name%%
Version:   %%version%%
Release:   %%release%%
Copyright: %%license%%
Vendor:    %%vendor%%
Url:       %%url%%
%%icon%%
Packager:  %%author%%
Group:     %%catagory%%
Source:    %%source%%
BuildRoot: %%buildroot%%

%description
%%desc%%

%prep
%setup

%build
export CFLAGS="$RPM_OPT_FLAGS"
export CXXFLAGS="$RPM_OPT_FLAGS"

# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.%%name%%
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.%%name%%
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.%%name%%

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/%%name%%
rm -rf ../file.list.%%name%%


%files -f ../file.list.%%name%%

%post
echo %%name%% has been added to your system

%postun
echo %%name%% has been removed from your system.


%changelog
*Sun Jan 21 2001 Ian Reinhart Geiser <geiseri@linuxppc.com>
- created inital template.
- all changes should go here after this date.
