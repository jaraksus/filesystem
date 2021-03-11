rm -r tmp/
mkdir tmp
cd tmp
cmake ../
make
cd ../

if [[ $1 == "--new" ]]
then
    ./tmp/main image --new
else
    ./tmp/main image
fi