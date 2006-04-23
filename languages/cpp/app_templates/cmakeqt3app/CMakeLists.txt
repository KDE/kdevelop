
PROJECT(%{APPNAMELC})

# at least cmake version 2.4.0 is required
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0)

# uncomment the following line if you require a special version of Qt
# and adjust it accordingly
# SET(QT_MIN_VERSION "3.0.0")

# try to find Qt 3
FIND_PACKAGE(Qt3 REQUIRED)

ADD_SUBDIRECTORY(src)
