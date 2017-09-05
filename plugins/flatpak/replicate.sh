export $(cat /proc/$(pidof $1)/environ | tr \\0 \\n)
shift
$@
