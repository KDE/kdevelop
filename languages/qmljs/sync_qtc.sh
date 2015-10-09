#note: run in the same directory where the file is

QTC_PATH=$1

FILES_USED=`find . -name "*.cpp" -o -name "*.h"`

for file in $FILES_USED
do
    cp -v "$QTC_PATH/src/libs/$file" $file
done
