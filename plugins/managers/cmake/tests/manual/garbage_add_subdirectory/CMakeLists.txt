add_subdirectory(foo)

# note: this now ASSERTS out, equal to the tiny_project test.
#
# fooling around with the 'add_subdirectory' line above will crash KDevelop
# This is what I observe:
#
#   1. comment out + save
#       */ project icon vanishes [it shouldn't]
#       */ all project files vanish [they shouldn't]
#   2. de-comment again + save
#       */ KDevelop crashes
#
#

#Thread 1 (Thread 0xb5b116d0 (LWP 25165)):
#0  0xb6931b9b in __dynamic_cast () from /usr/lib/libstdc++.so.6
#1  0xb2338c0f in CMakeProjectManager::parse (this=0x8521a28, item=0x8ba6290) at /home/nix/KdeDev/kdevelop/buildtools/managers/cmake/cmakemanager.cpp:310
#2  0xb668644e in KDevelop::ImportProjectJobPrivate::parseItem (this=0x8b053a8) at /home/nix/KdeDev/kdevplatform/project/importprojectjob.cpp:53
#3  0xb66835b3 in KDevelop::ImportProjectJob::qt_metacall (this=0x8baf478, _c=QMetaObject::InvokeMetaMethod, _id=0, _a=0xbfaca688)
#    at /home/nix/KdeDev/kdevplatform/build/project/importprojectjob.moc:65
#4  0xb75c93fa in QMetaObject::activate (sender=0x8b053b0, from_signal_index=4, to_signal_index=4, argv=0x0) at kernel/qobject.cpp:3007
#5  0xb75c9972 in QMetaObject::activate (sender=0x8b053b0, m=0xb76539e4, local_signal_index=0, argv=0x0) at kernel/qobject.cpp:3080
#6  0xb75fe9a7 in QTimer::timeout (this=0x8b053b0) at .moc/release-shared/moc_qtimer.cpp:126
#7  0xb75d0abe in QTimer::timerEvent (this=0x8b053b0, e=0xbfacab2c) at kernel/qtimer.cpp:263
#8  0xb75c5d1f in QObject::event (this=0x8b053b0, e=0xbfacab2c) at kernel/qobject.cpp:1105
#9  0xb6d10ecc in QApplicationPrivate::notify_helper (this=0x80c42f8, receiver=0x8b053b0, e=0xbfacab2c) at kernel/qapplication.cpp:3772
#10 0xb6d1731e in QApplication::notify (this=0xbfacaef8, receiver=0x8b053b0, e=0xbfacab2c) at kernel/qapplication.cpp:3366
#11 0xb7890a7d in KApplication::notify (this=0xbfacaef8, receiver=0x8b053b0, event=0xbfacab2c) at /home/nix/KdeDev/kdelibs/kdelibs/kdeui/kernel/kapplication.cpp:311
#12 0xb75b7b51 in QCoreApplication::notifyInternal (this=0xbfacaef8, receiver=0x8b053b0, event=0xbfacab2c) at kernel/qcoreapplication.cpp:583
#13 0xb75e0181 in QTimerInfoList::activateTimers (this=0x80c4ad4) at kernel/qcoreapplication.h:215
#14 0xb75e03fb in QEventDispatcherUNIX::processEvents (this=0x80c2360, flags={i = -1079202776}) at kernel/qeventdispatcher_unix.cpp:899
#15 0xb6d9b0a2 in QEventDispatcherX11::processEvents (this=0x80c2360, flags={i = -1079202328}) at kernel/qeventdispatcher_x11.cpp:154
#16 0xb75b62ca in QEventLoop::processEvents (this=0xbfacae50, flags={i = -1079202264}) at kernel/qeventloop.cpp:149
#17 0xb75b648a in QEventLoop::exec (this=0xbfacae50, flags={i = -1079202216}) at kernel/qeventloop.cpp:196
#18 0xb75b866d in QCoreApplication::exec () at kernel/qcoreapplication.cpp:845
#19 0xb6d10d47 in QApplication::exec () at kernel/qapplication.cpp:3304
#20 0x08057407 in main (argc=Cannot access memory at address 0x0) at /home/nix/KdeDev/kdevelop/src/main.cpp:209
