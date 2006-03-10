#!/bin/sh
# Helper script that performs a command on all files of a given directory and its subdirectories.
# The files can be filtered so that only the ones ending with a certain suffix are considered.

if [ $# -lt 2 -o $# -gt 3 ]; then
  echo
  echo "Usage: $0 EXEC DESTDIR [SUFFIXFILTER]"
  echo "Performs a command on all files in a directory (recursively)."
  echo "EXEC: the command that is called on the files"
  echo "DESTDIR: the directory containing the files that will be given as argument"
  echo "SUFFIXFILTER: restricts the command to files ending with this suffix ('java')"
  exit $E_WRONG_ARGS
else
  COLOR=$1
fi

  echo "Generating command list..." # 1>&2 # redirect if you want it to be debug output

  echo "PASSED=0" > .rec-check.exec.sh
  echo "FAILED=0" >> .rec-check.exec.sh
  echo >> .rec-check.exec.sh

if [ $# -eq 3 ]; then
  find $2 -type f | grep $3\$ | \
    awk "{ printf( \"echo 'Processing %s...'\nif ( $1 '%s' 2>&1 ); then DUMMY=\$((PASSED++)); else DUMMY=\$((FAILED++)); fi\n\", \$0, \$0 ) }" \
    >> .rec-check.exec.sh
else
  find $2 -type f | \
    awk "{ printf( \"echo 'Processing %s...'\nif ( $1 '%s' 2>&1 ); then DUMMY=\$((PASSED++)); else DUMMY=\$((FAILED++)); fi\n\", \$0, \$0 ) }" \
    >> .rec-check.exec.sh
fi

  echo >> .rec-check.exec.sh
  echo "TOTAL=\$((PASSED + FAILED))" >> .rec-check.exec.sh
  echo "echo" >> .rec-check.exec.sh
  echo "echo \$TOTAL files were processed:" >> .rec-check.exec.sh
  echo "if [ \$TOTAL -ne 0 ]; then" >> .rec-check.exec.sh
  echo "  echo \"\$PASSED passed (\$(( (PASSED*100) / TOTAL))%), \$FAILED failed (\$(( (FAILED*100) / TOTAL))%)\"" >> .rec-check.exec.sh
  echo "fi" >> .rec-check.exec.sh


  echo "...done! Processing files in $2." # 1>&2 # redirect if you want it to be debug output
  echo # 1>&2 # redirect if you want it to be debug output

  chmod o+x .rec-check.exec.sh
  source .rec-check.exec.sh
  rm .rec-check.exec.sh
