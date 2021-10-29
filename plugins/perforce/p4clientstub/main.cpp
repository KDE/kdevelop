/*
    SPDX-FileCopyrightText: 2013 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <iostream>
#include <QString>
#include <QByteArray>

bool validateNumberOfArguments(int argc, char** /*argv*/)
{
    if (argc < 3) {
        return false;
    }

    return true;
}


int fakeFstatOutput(QString const& filepath)
{
    QByteArray tmp = filepath.toUtf8();
    std::string fileText(tmp.constData(), tmp.size());
    std::cout << "... depotFile /" << fileText << std::endl;// /tools/test/new_file.txt"
    std::cout << "... clientFile /home/projects" << fileText << std::endl;
    std::cout << "... isMapped" << std::endl;
    std::cout << "... headAction add" << std::endl;
    std::cout << "... headType text" << std::endl;
    std::cout << "... headTime 1285014691" << std::endl;
    std::cout << "... headRev 1" << std::endl;
    std::cout << "... headChange 759253" << std::endl;
    std::cout << "... headModTime 1285014680" << std::endl;
    std::cout << "... haveRev 1" << std::endl;
    return 0;
}

int fakeRevertOutput()
{
    return 0;
}

int fakeSyncOutput()
{
    return 0;
}

int fakeSubmitOutput()
{
    return 0;
}

int fakeDiff2Output()
{
    return 0;
}

int fakeDiffOutput()
{
    return 0;
}

int fakeFileLogOutput(QString const& filepath)
{
	QByteArray tmp = filepath.toUtf8();
    std::string fileText(tmp.constData(), tmp.size());
    std::cout << "//depot/" << fileText << std::endl;
    std::cout << "... #14 change 72 edit on 2013/02/04 15:38:35 by mvo@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Log a little more out" << std::endl;
    std::cout << std::endl;
    std::cout << "... #13 change 67 edit on 2013/02/04 15:31:48 by mov@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Rename rand -> random_number" << std::endl;
    std::cout << std::endl;
    std::cout << "... #12 change 79 edit on 2013/01/29 18:02:33 by mvo@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Prefer the use of std::string" << std::endl;
    std::cout << std::endl;
    std::cout << "... #11 change 33 edit on 2013/01/29 15:48:41 by mvo@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Correct Test Class to use QString" << std::endl;
    std::cout << std::endl;
    std::cout << "... #10 change 76 edit on 2013/01/28 19:21:33 by vom@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Minor changes. Changed the log level of a couple of log lines." << std::endl;
    std::cout << std::endl;
    std::cout << "... #9 change 17 edit on 2013/01/22 15:41:16 by vom@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Just added a comment" << std::endl;
    std::cout << std::endl;
    std::cout << "... #8 change 97 edit on 2013/01/17 13:54:26 by mvo@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Do not cleanup before we are absolutely done" << std::endl;
    std::cout << std::endl;
    std::cout << "... #7 change 12 edit on 2013/01/17 11:39:25 by mvo@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Insert a little debugging in the test class. Be more defensive when dealing with iterators" << std::endl;
    std::cout << std::endl;
    std::cout << "... #6 change 60 edit on 2013/01/09 13:19:07 by vom@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	error and abort handling added to RingBuffer." << std::endl;
    std::cout << std::endl;
    std::cout << "... #5 change 30 edit on 2013/01/08 19:21:42 by mvo@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Fix two things in cleanup. Fix item on the todo list, cleanup a map" << std::endl;
    std::cout << std::endl;
    std::cout << "... #4 change 27 edit on 2013/01/08 13:26:05 by vom@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Clean-up of persisted data on disk has been added" << std::endl;
    std::cout << std::endl;
    std::cout << "... #3 change 5 edit on 2012/12/28 13:04:16 by vom@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Support for multiple things added." << std::endl;
    std::cout << std::endl;
    std::cout << "... #2 change 6 edit on 2012/12/04 14:05:10 by mvo@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Make Other test compile with new test class" << std::endl;
    std::cout << std::endl;
    std::cout << "... #1 change 1 add on 2012/12/04 13:48:25 by mvo@testbed (ktext)" << std::endl;
    std::cout << std::endl;
    std::cout << "	Add RingBuffer" << std::endl;
    std::cout << std::endl;
    return 0;
}

int fakeAnnotateOutput()
{
    std::cout << "1: #include \\\"RingBuffer.h\\\"" << std::endl;
    std::cout << "1: #include \\\"inc/test/someClass.h\\\"" << std::endl;
    std::cout << "1: //#include \\\"hejsa.h\\\"" << std::endl;
    std::cout << "1: " << std::endl;
    std::cout << "1: #include <iostream>" << std::endl;
    std::cout << "1: " << std::endl;
    std::cout << "1: #include <vector>" << std::endl;
    std::cout << "1: #include <algorithm>" << std::endl;
    std::cout << "1: " << std::endl;
    std::cout << "1: #include <limits>" << std::endl;
    std::cout << "1: " << std::endl;
    std::cout << "1: #include <cstdlib>" << std::endl;
    std::cout << "1: #include <ctime>" << std::endl;
    std::cout << "1: " << std::endl;
    std::cout << "1: " << std::endl;
    std::cout << "5: void fillBuffer( RingBuffer& myBuffer, std::vector<uint64_t>& testVector )" << std::endl;
    std::cout << "1: {" << std::endl;
    std::cout << "1:     uint64_t random_range( std::numeric_limits<unsigned long>::max() );" << std::endl;
    std::cout << "12:     uint64_t random_number ( 0 );" << std::endl;
    std::cout << "79:     /// First fill the buffer" << std::endl;
    std::cout << "1:     for ( size_t i =0; i < RINGBUFFER_SIZE; ++i )" << std::endl;
    std::cout << "1:     {" << std::endl;
    std::cout << "5:         random_number = std::rand() % random_range;" << std::endl;
    std::cout << "5:         myBuffer.push_back ( random_number );" << std::endl;
    std::cout << "5:         testVector.push_back ( random_number );" << std::endl;
    std::cout << "6:     }" << std::endl;
    std::cout << "76: }" << std::endl;
    std::cout << "76: " << std::endl;
    std::cout << "17: void testWithOnlyknown( RingBuffer& myBuffer, std::vector<uint64_t>& testVector )" << std::endl;
    std::cout << "17: {" << std::endl;
    std::cout << "17:     uint64_t random_range ( RINGBUFFER_SIZE );" << std::endl;
    std::cout << "97:     uint64_t random_number ( 0 );" << std::endl;
    std::cout << "12:     /// Perform the test run on values we have already inserted." << std::endl;
    std::cout << "1:     for ( size_t j =0; j < 1000000; ++j )" << std::endl;
    std::cout << "1:     {" << std::endl;
    std::cout << "1:         random_number = std::rand() % random_range;" << std::endl;
    std::cout << "1:         myBuffer.push_back ( testVector[random_number] );" << std::endl;
    std::cout << "1:     }" << std::endl;
    std::cout << "67: " << std::endl;
    std::cout << "67: }" << std::endl;
    std::cout << "67: " << std::endl;
    std::cout << "60: int main( int argc, char **argv )" << std::endl;
    std::cout << "60: {" << std::endl;
    std::cout << "27:     RingBuffer myBuffer;" << std::endl;
    std::cout << "1:     std::vector<uint64_t> testVector;" << std::endl;
    std::cout << "1: " << std::endl;
    std::cout << "1:     std::srand( std::time ( 0 ) );" << std::endl;
    std::cout << "1: " << std::endl;
    std::cout << "1:     fillBuffer( myBuffer, testVector );" << std::endl;
    std::cout << "27:     testWithOnlyknown( myBuffer, testVector );" << std::endl;
    std::cout << "27: 	" << std::endl;
    std::cout << "27:     std::cout << \\\"Done Testing\\\" << std::endl;" << std::endl;
    std::cout << "1: " << std::endl;
    std::cout << "1:     return 0;" << std::endl;
    std::cout << "1: }" << std::endl;
    return 0;
}

int fakeEditOutput(QString const& filepath)
{
    QByteArray tmp = filepath.toUtf8();
    std::string fileText(tmp.constData(), tmp.size());
    std::cout << fileText << "#1 - opened for edit" << std::endl;
    return 0;
}

int fakeAddOutput(QString const& filepath)
{
    QByteArray tmp = filepath.toUtf8();
    std::string fileText(tmp.constData(), tmp.size());
    std::cout << fileText << "#1 - opened for add" << std::endl;
    return 0;
}

int main(int argc, char** argv)
{
    if (!validateNumberOfArguments(argc, argv)) {
        std::cout << "Was not able to validate number of arguments: " << argc << std::endl;
        return -1;
    }

    if (qstrcmp(argv[1], "revert") == 0)  {
        return fakeRevertOutput();
    } else if (qstrcmp(argv[1], "sync") == 0) {
        return fakeSyncOutput();
    } else if (qstrcmp(argv[1], "submit") == 0) {
        return fakeSubmitOutput();
    } else if (qstrcmp(argv[1], "diff2") == 0) {
        return fakeDiff2Output();
    } else if (qstrcmp(argv[1], "diff") == 0) {
        return fakeDiffOutput();
    } else if (qstrcmp(argv[1], "filelog") == 0) {
        return fakeFileLogOutput(argv[3]);
    } else if (qstrcmp(argv[1], "annotate") == 0) {
        return fakeAnnotateOutput();
    } else if (qstrcmp(argv[1], "edit") == 0) {
        return fakeEditOutput(QString(argv[2]));
    } else if (qstrcmp(argv[1], "fstat") == 0) {
        return fakeFstatOutput(QString(argv[2]));
    } else if (qstrcmp(argv[1], "add") == 0) {
        return fakeAddOutput(QString(argv[2]));
    }
    return -1;
}
