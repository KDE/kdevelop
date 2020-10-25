### %{APPNAME} template

To install tis template you can use cmake:  
```
# create a build directory called build and configure cmake
# with the custom install prefix we install it in the user-specific folder
cmake --source . --build build -DCMAKE_INSTALL_PREFIX=~/.local
# create and install the template
cmake --build build --target install
```
After this the template shows up in the dialog under File -> New From Template.

If you think that the template can be useful to other people
you can upload it to the KDE store https://store.kde.org/browse/cat/327/.

In case you have an older version of the CMake macro that generates the archive
you can run `rm build/%{APPNAMELC}.tar.bz2` to make sure the file gets recreated.

