if [ ! -d "./build" ];then
echo "Create build folder..."
mkdir build
else
rm -r build
mkdir build
fi

cd build
cmake ..
make
./system_status_loger