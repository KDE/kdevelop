#!/usr/bin/env python3
#
# Downloads color schemes from download.kde.org and installs them into the desired prefix
# Usage: install_colorschemes.py [INSTALL_DIR]

import distutils.dir_util
import os
import shutil
import sys
import tarfile
import urllib.request

installDir = sys.argv[1] if len(sys.argv) > 1 else None
ver = "5.13.4" # Plasma version
colorSchemesDir = "color-schemes"

def downloadAndExtract(url, extractPath):
    fname = os.path.basename(url)
    if not os.path.exists(fname):
        print("Downloading {0}".format(url))
        urllib.request.urlretrieve(url, fname)
    else:
        print("Skipping download of {0} (file already exists)".format(url))

    print("Unpacking {0}".format(fname))
    dirname = fname.replace(".tar.xz", "")
    shutil.rmtree(dirname, ignore_errors=True)
    with tarfile.open(fname) as tar:
        for tarinfo in tar.getmembers():
            if tarinfo.name.startswith(extractPath):
                tarinfo.name = os.path.basename(tarinfo.name) # remove the path by reset it
                tar.extract(member=tarinfo, path=colorSchemesDir)

downloadAndExtract('https://download.kde.org/stable/plasma/{0}/breeze-{0}.tar.xz'.format(ver),
                   "breeze-{0}/colors/".format(ver))
downloadAndExtract('https://download.kde.org/stable/plasma/{0}/plasma-desktop-{0}.tar.xz'.format(ver),
                   "plasma-desktop-{0}/kcms/colors/schemes/".format(ver))

print("Available color schemes: {0}".format(", ".join(os.listdir(colorSchemesDir))))

if installDir:
    print("Installing to: {0}".format(installDir))
    distutils.dir_util.copy_tree(colorSchemesDir, os.path.join(installDir, colorSchemesDir))
