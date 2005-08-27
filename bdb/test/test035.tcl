# See the file LICENSE for redistribution information.
#
# Copyright (c) 1996, 1997, 1998, 1999, 2000
#	Sleepycat Software.  All rights reserved.
#
#	$Id$
#
# DB Test 35 {access method}
# DB_GET_BOTH functionality with off-page duplicates.
proc test035 { method {nentries 10000} args} {
	# Test with off-page duplicates
	eval {test033 $method $nentries 20 35 -pagesize 512} $args

	# Test with multiple pages of off-page duplicates
	eval {test033 $method [expr $nentries / 10] 100 35 -pagesize 512} $args
}
