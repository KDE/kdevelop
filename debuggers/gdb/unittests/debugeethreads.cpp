/*
   Copyright 2009 Niko Sams <niko.sams@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <iostream>
#include <QtCore/QThread>
#include <unistd.h>

class TestThread : public QThread
{
public:
    void run() override {
        sleep(1);
        std::cout << "Hello, world!" << std::endl;
    }
};


int main(int /*argc*/, char **/*argv*/) {
    TestThread t1;
    TestThread t2;
    TestThread t3;
    t1.start();
    t2.start();
    t3.start();
    usleep(500000);
    usleep(600000);
    return 0;
}
