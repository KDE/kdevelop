project(FooBar)

# an empty project
# this test currently crashes.

# steps to crash:
# 1/ boot kdevelop
# 2/ load cmakemanager_empty.kdev4
# 3/ insert space in this CMakeLists.txt + save
# 4/ insert space + save
# 5/ seg fault

#(gdb) thread apply all bt
#Thread 2 (Thread 0xb2e3fb90 (LWP 12166)):
#0  0xffffe430 in __kernel_vsyscall ()
#1  0xb6789ef1 in select () from /lib/libc.so.6
#2  0xb752e557 in ?? () from /usr/lib/libQtCore.so.4
#3  0xb74709b7 in ?? () from /usr/lib/libQtCore.so.4
#4  0xb7414175 in start_thread () from /lib/libpthread.so.0
#5  0xb6790dce in clone () from /lib/libc.so.6

#Thread 1 (Thread 0xb5e7f6d0 (LWP 12165)):
#0  0xb68f4b9b in __dynamic_cast () from /usr/lib/libstdc++.so.6
#1  0xb2ee3790 in CMakeProjectManager::parse (this=0x865d128, item=0x8877aa0) at /home/nix/KdeDev/kdevelop2/buildtools/managers/cmake/cmakemanager.cpp:337
#2  0xb657b17f in KDevelop::ImportProjectJob::qt_metacall (this=0x8947fa8, _c=QMetaObject::InvokeMetaMethod, _id=0, _a=0xbff32bb8)
#    at /home/nix/KdeDev/kdevplatform2/project/importprojectjob.cpp:53
#3  0xb755c3fa in QMetaObject::activate () from /usr/lib/libQtCore.so.4
#4  0xb755c972 in QMetaObject::activate () from /usr/lib/libQtCore.so.4
#5  0xb75919a7 in QTimer::timeout () from /usr/lib/libQtCore.so.4
#6  0xb7563abe in QTimer::timerEvent () from /usr/lib/libQtCore.so.4
#7  0xb7558d1f in QObject::event () from /usr/lib/libQtCore.so.4
#8  0xb6ca3ecc in QApplicationPrivate::notify_helper () from /usr/lib/libQtGui.so.4
#9  0xb6caa31e in QApplication::notify () from /usr/lib/libQtGui.so.4
#10 0xb7834d1d in KApplication::notify (this=0xbff33454, receiver=0x871ec20, event=0xbff3305c) at /home/nix/KdeDev/kdelibs/kdeui/kernel/kapplication.cpp:311
#11 0xb754ab51 in QCoreApplication::notifyInternal () from /usr/lib/libQtCore.so.4
#12 0xb7573181 in ?? () from /usr/lib/libQtCore.so.4
#13 0xb75733fb in QEventDispatcherUNIX::processEvents () from /usr/lib/libQtCore.so.4
#14 0xb6d2e0a2 in ?? () from /usr/lib/libQtGui.so.4
#15 0xb75492ca in QEventLoop::processEvents () from /usr/lib/libQtCore.so.4
#16 0xb754948a in QEventLoop::exec () from /usr/lib/libQtCore.so.4
#17 0xb754b66d in QCoreApplication::exec () from /usr/lib/libQtCore.so.4
#18 0xb6ca3d47 in QApplication::exec () from /usr/lib/libQtGui.so.4
#19 0x0804e464 in main (argc=1, argv=0xbff338b4) at /home/nix/KdeDev/kdevelop2/src/main.cpp:194
