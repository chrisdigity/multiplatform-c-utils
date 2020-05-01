#!/bin/sh
echo
echo "  testUNIX.bat - Multiplatform Utilities compilation and execution script"
echo "                 thread.c, time.c"
echo

################
# Remove old error log

LOG="mputilstest.log"
rm -f $LOG

################
# Build test software

printf "Building Multiplatform Utility tests... "
cc -pthread -o mputilstest test/mputilstest.c 2>>$LOG

if test -s $LOG
then
   echo "Error"
   echo
   cat $LOG
else
   echo "OK"
   echo
   echo "Done."

################
# Run software on success

   ./mputilstest

fi

echo
exit
