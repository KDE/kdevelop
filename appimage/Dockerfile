FROM centos:6.10

RUN yum -y update && yum -y install wget && yum -y install centos-release-scl epel-release && yum -y update && yum -y install devtoolset-6-gcc devtoolset-6-gcc-c++ devtoolset-6-binutils wget tar bzip2 git libtool which fuse fuse-devel libjpeg-devel libpng-devel automake cppunit-devel cmake glibc-headers libstdc++-devel gcc-c++ freetype-devel fontconfig-devel libxml2-devel libstdc++-devel libXrender-devel libXcomposite libXcomposite-devel patch xcb-util-keysyms-devel libXi-devel libudev-devel.x86_64 openssl-devel sqlite-devel.x86_64 gperftools.x86_64 gperf.x86_64 libicu-devel.x86_64 boost-devel.x86_64 libxslt-devel.x86_64 docbook-style-xsl.noarch python27.x86_64 cmake3.x86_64 bison flex bison-devel flex-devel xz xz-devel pcre-devel pcre2-devel pcre pcre2 mesa-libEGL-devel mesa-libGL-devel glib-devel gettext perl-URI.noarch bzip2-devel.x86_64 subversion-devel.x86_64 subversion.x86_64 sqlite2-devel.x86_64 hunspell-devel aspell-devel hspell-devel vim sudo unzip xkeyboard-config libffi libffi-devel hyphen hyphen-devel
RUN cd /tmp && wget http://opensource.wandisco.com/rhel/6/svn-1.9/RPMS/x86_64/subversion-1.9.4-3.x86_64.rpm http://opensource.wandisco.com/rhel/6/svn-1.9/RPMS/x86_64/subversion-devel-1.9.4-3.x86_64.rpm http://opensource.wandisco.com/rhel/6/svn-1.9/RPMS/x86_64/serf-1.3.7-1.x86_64.rpm && yum -y install subversion* serf*; rm subversion* serf*

RUN echo ". /opt/rh/devtoolset-6/enable && chmod +x /opt/rh/python27/enable && . /opt/rh/python27/enable" >> /root/.bashrc

ENV LANG=en_US.UTF-8 PYTHON_VERSION=3.6.7 QTVERSION=5.12.8 QTVERSION_SHORT=5.12 LLVM_VERSION=10.0.0 LLVM_ROOT=/opt/llvm/ RUBY_VERSION=1.9.3-p484 GLIB_VERSION_SHORT=2.61 GLIB_VERSION=2.61.2 LD_LIBRARY_PATH=$QTDIR/lib/
ENV QTDIR=/opt/qt5

RUN bash -c "ln -sf /opt/rh/devtoolset-6/root/usr/bin/g++ /usr/bin/g++ && ln -sf /opt/rh/devtoolset-6/root/usr/bin/c++ /usr/bin/c++"

# Build Python
RUN bash -c "mkdir -p /python && cd /python && wget https://www.python.org/ftp/python/$PYTHON_VERSION/Python-$PYTHON_VERSION.tar.xz && tar xvf Python-$PYTHON_VERSION.tar.xz && cd /python/Python-$PYTHON_VERSION && mkdir -p /usr/lib/pkgconfig && ./configure --prefix=/usr --enable-shared && make -j$(nproc) install && rm -Rf /python"

# Install ninja (1.8.2 latest working with libc of centos:6.10)
RUN bash -c "yum install unzip && mkdir -p /tmp/deploy && cd /tmp/deploy && wget https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip && unzip ninja-linux.zip && mv -f ninja /usr/local/bin && cd .. && rm -Rf deploy"

# Install meson, required to build xkbcommon and recent versions of glib
RUN bash -c "ldconfig && pip3 install meson"

# Build xkbcommon, required for Qt >= 5.12.1
RUN bash -c "mkdir -p /xkbcommon && cd /xkbcommon && wget https://github.com/xkbcommon/libxkbcommon/archive/xkbcommon-0.8.4.tar.gz"
RUN bash -c "cd /xkbcommon/ && tar xvf xkbcommon-0.8.4.tar.gz"
RUN bash -c ". /opt/rh/devtoolset-6/enable && cd /xkbcommon/libxkbcommon-xkbcommon-0.8.4 && meson setup -Denable-wayland=false -Denable-docs=false -Dprefix=/usr build && ninja -C build && ninja -C build install && rm -Rf /xkbcommon"

# Build Qt5
RUN bash -c "mkdir -p /qt && cd /qt && wget https://download.qt.io/archive/qt/${QTVERSION_SHORT}/${QTVERSION}/single/qt-everywhere-src-${QTVERSION}.tar.xz"
RUN bash -c "cd /qt && tar xvf qt-everywhere-src-${QTVERSION}.tar.xz"
RUN bash -c "export MAKEFLAGS=-j$(nproc) && cd /qt/qt-everywhere-src-${QTVERSION} && ./configure -v -skip qt3d -skip qtconnectivity -skip qtgamepad -skip qtlocation -skip qtcharts -skip qtremoteobjects -skip qtscxml -skip qtsensors -skip qtpurchasing -platform linux-g++ -qt-pcre -qt-xcb -no-pch -nomake tests -nomake examples -confirm-license -opensource -prefix $QTDIR && make -j$(nproc) || make -j 1 install; make -j$(nproc) install && rm -Rf /qt"

RUN ln -sf $QTDIR/bin/qmake /usr/bin/qmake-qt5

# Build Ruby >= 1.9, required for qtwebkit
RUN bash -c "mkdir -p /ruby && cd /ruby && wget http://ftp.ruby-lang.org/pub/ruby/1.9/ruby-${RUBY_VERSION}.tar.gz"
RUN bash -c "cd /ruby/ && tar zxvf ruby-${RUBY_VERSION}.tar.gz"
RUN bash -c "cd /ruby/ruby-${RUBY_VERSION} && ./configure && make -j$(nproc) && make -j$(nproc) install && rm -Rf /ruby"

# Build glib >= 2.36, required for qtwebkit
RUN bash -c "mkdir -p /glib && cd /glib && wget http://ftp.gnome.org/pub/GNOME/sources/glib/${GLIB_VERSION_SHORT}/glib-${GLIB_VERSION}.tar.xz"
RUN bash -c "cd /glib/ && tar xvf glib-${GLIB_VERSION}.tar.xz"
RUN bash -c ". /opt/rh/devtoolset-6/enable && cd /glib/glib-${GLIB_VERSION} && meson -D libmount=false _build && ninja -C _build && ninja -C _build install && rm -Rf /glib"

# Build ICU >= 52.1, required for qtwebkit
RUN bash -c "mkdir -p /icu && cd /icu && wget https://github.com/unicode-org/icu/releases/download/release-52-2/icu4c-52_2-src.tgz"
RUN bash -c "cd /icu/ && tar xvf icu4c-52_2-src.tgz"
RUN bash -c ". /opt/rh/devtoolset-6/enable && cd /icu/icu/source && ./configure --prefix /opt/icu4c-52_2 && make -j$(nproc) && make -j$(nproc) install && rm -Rf /icu && echo "/opt/icu4c-52_2/lib/" > /etc/ld.so.conf.d/icu4c-52_2.conf && ldconfig"

# Build qtwebkit
RUN bash -c "mkdir -p /qtwk && cd /qtwk && wget https://github.com/qt/qtwebkit/archive/v5.212.0-alpha4.tar.gz"
RUN bash -c "cd /qtwk/ && tar xvf v5.212.0-alpha4.tar.gz"
# libxml2 2.6.0 "will probably work" according to https://github.com/qtwebkit/qtwebkit/wiki/Building-QtWebKit-on-Linux
RUN bash -c ". /opt/rh/python27/enable && . /opt/rh/devtoolset-6/enable && sed -i 's/find_package(LibXml2 2.8.0 REQUIRED)/find_package(LibXml2 2.6.0 REQUIRED)/g' /qtwk/qtwebkit-5.212.0-alpha4/Source/cmake/OptionsQt.cmake && mkdir -p /qtwk/qtwebkit-5.212.0-alpha4/WebKitBuild/Release && cd /qtwk/qtwebkit-5.212.0-alpha4/WebKitBuild/Release && PKG_CONFIG_PATH=/opt/icu4c-52_2/lib/pkgconfig/:${PKG_CONFIG_PATH} cmake3 -DPORT=Qt -DCMAKE_BUILD_TYPE=Release -DQt5_DIR=${QTDIR}/lib/cmake/Qt5 -DENABLE_GEOLOCATION=0 -DENABLE_DEVICE_ORIENTATION=0 -DENABLE_WEB_AUDIO=0 -DENABLE_VIDEO=0 ../.. && make -j$(nproc) && make -j$(nproc) install && rm -Rf /qtwk"

# Build Clang/LLVM
RUN bash -c "mkdir -p /llvm && cd /llvm && wget https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/llvm-${LLVM_VERSION}.src.tar.xz"
RUN bash -c "cd /llvm && tar xvf llvm-${LLVM_VERSION}.src.tar.xz && cd llvm-${LLVM_VERSION}.src"
RUN bash -c "cd /llvm/llvm-${LLVM_VERSION}.src/tools && wget https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/clang-${LLVM_VERSION}.src.tar.xz"
RUN bash -c "cd /llvm/llvm-${LLVM_VERSION}.src/tools && tar xvf clang-${LLVM_VERSION}.src.tar.xz"
RUN bash -c ". /opt/rh/python27/enable && . /opt/rh/devtoolset-6/enable && python --version && cd /llvm/llvm-${LLVM_VERSION}.src && mkdir -p build && cd build && cmake3 -G Ninja .. -DCMAKE_INSTALL_PREFIX=/opt/llvm/ -DCMAKE_BUILD_TYPE=Release -DLLVM_INCLUDE_TESTS=OFF -DLLVM_TARGETS_TO_BUILD=X86 && ninja install && rm -Rf /llvm"


CMD /bin/bash
