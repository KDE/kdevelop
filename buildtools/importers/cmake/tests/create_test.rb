#!/usr/bin/ruby
# Copyright 2006 by Matt Rogers <mattr@kde.org>
# Licensed under the GNU LGPL
# I don't really care what you do with 

className = ARGV[0]

# Takes a class name as the first and only parameter
if className.nil?
	puts "Need a class name to generate the test. aborting"
	exit
end

hBase = "cmake_#{className.downcase}_test.h"
cppBase = hBase.gsub(/.h/, ".cpp")
mocBase = hBase.gsub(/.h/, ".moc")
license = <<LICENSE_EOF
/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
LICENSE_EOF

fileH = <<HEADER_EOF
#{license}
#ifndef CMAKE#{className.upcase}TEST_H
#define CMAKE#{className.upcase}TEST_H

#include <QtTest/QtTest>

class #{className}Test : public QObject
{
    Q_OBJECT
public:
    #{className}Test() {}
    virtual ~#{className}Test() {}

private slots:
    void testGoodParse();
    void testGoodParse_data();

    void testBadParse();
    void testBadParse_data();
};

#endif
HEADER_EOF

fileCpp = <<CPP_EOF
#{license}
#include "#{hBase}"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( #{className}Test )

void #{className}Test::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    #{className}* ast = new #{className}();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void #{className}Test::testGoodParse_data()
{
}

void #{className}Test::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void #{className}Test::testBadParse_data()
{
}

#include "#{mocBase}"
CPP_EOF

puts "Creating test for class #{className}"



sourceVar = "#{className.downcase}test_SRCS"
targetName = "cmake-#{className.downcase}test"
cmakeText = <<CMAKE_EOF
set ( #{sourceVar} #{cppBase} )
kde4_automoc( #{targetName} ${#{sourceVar}} )
kde4_add_executable( #{targetName} NOGUI RUN_UNINSTALLED ${#{sourceVar}} )
target_link_libraries( #{targetName} 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( #{targetName} ${EXECUTABLE_OUTPUT_PATH}/#{targetName} )
endif(KDE4_BUILD_TESTS)

CMAKE_EOF

hOut = File.new(hBase, "w")
cppOut = File.new(cppBase, "w")

hOut.write(fileH)
cppOut.write(fileCpp)

puts "#{hBase} and #{cppBase} written."

puts "Adding build system bits to CMakeLists.txt"
cmakeList = File.new("CMakeLists.txt", "r+")
oldCMakeText = cmakeList.read;
cmakeList.close;
oldCMakeText.gsub!(/endif\(KDE4_BUILD_TESTS\)/, cmakeText)
cmakeList = File.new("CMakeLists.txt", "w")
cmakeList.write(oldCMakeText)

puts "Appending AST definition to cmakeast.h"
cmakeAst = File.new("../parser/cmakeast.h", "r+")
oldAstText = cmakeAst.read;
cmakeAst.close;
astDecl =<<AST_EOF

CMAKE_BEGIN_AST_CLASS( #{className} )
CMAKE_END_AST_CLASS( #{className} )

#endif
AST_EOF
oldAstText.gsub!(/#endif/, astDecl)
cmakeAst = File.new("../parser/cmakeast.h", "w")
cmakeAst.write(oldAstText)
cmakeAst.close

puts "Appending AST implementation to cmakeast.cpp"
cmakeAst = File.new("../parser/cmakeast.cpp", "a")
astDef =<<AST_EOF
#{className}::#{className}()
{
}

#{className}::~#{className}()
{
}

void #{className}::writeBack( QString& )
{
}

bool #{className}::parseFunctionInfo( const CMakeFunctionDesc& func )
{
    return false;
}

AST_EOF
cmakeAst.write(astDef);
cmakeAst.close
