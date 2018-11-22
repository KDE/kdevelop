#!/usr/bin/env python3
#
# Downloads color schemes from download.kde.org and installs them into the desired prefix
# Usage: install_colorschemes.py [INSTALL_PREFIX]

import distutils.dir_util
import os
import shutil
import sys
import tarfile
import urllib.request

installPrefix = sys.argv[1] if len(sys.argv) > 1 else None
ver = "5.13.4" # Plasma version
colorSchemesDir = "share/color-schemes"

def downloadAndExtract(url):
    fname = os.path.basename(url)
    if not os.path.exists(fname):
        print("Downloading {0}".format(url))
        urllib.request.urlretrieve(url, fname)
    else:
        print("Skipping download of {0} (file already exists)".format(url))

    print("Unpacking {0}".format(fname))
    dirname = fname.replace(".tar.xz", "")
    shutil.rmtree(dirname)
    with tarfile.open(fname) as tar:
        tar.extractall()

downloadAndExtract('https://download.kde.org/stable/plasma/{0}/breeze-{0}.tar.xz'.format(ver))
downloadAndExtract('https://download.kde.org/stable/plasma/{0}/plasma-desktop-{0}.tar.xz'.format(ver))

distutils.dir_util.copy_tree(os.path.join("breeze-{0}".format(ver), "colors"), colorSchemesDir)
distutils.dir_util.copy_tree(os.path.join("plasma-desktop-{0}".format(ver), "kcms/colors/schemes"), colorSchemesDir)

print("Available color schemes: {0}".format(", ".join(os.listdir(colorSchemesDir))))

if installPrefix:
    print("Installing to: {0}".format(installPrefix))
    distutils.dir_util.copy_tree(colorSchemesDir, os.path.join(installPrefix, colorSchemesDir))
