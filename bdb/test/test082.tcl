# See the file LICENSE for redistribution information.
#
# Copyright (c) 2000
#	Sleepycat Software.  All rights reserved.
#
#	$Id$
#
# Test 82.
# Test of DB_PREV_NODUP
proc test082 { method {dir -prevnodup} {pagesize 512} {nitems 100}\
    {tnum 82} args} {
	source ./include.tcl

	eval {test074 $method $dir $pagesize $nitems $tnum} $args
}
