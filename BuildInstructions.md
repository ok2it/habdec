# Building HABDEC from source

To build habdec from source code, you need to build it's dependecies first:
- FFTW
- boost 1.68
- CMake version 3.8.2
- optionally: FLTK

Below are instructions how to obtain and compile each package. Keep in mind, these are WIP and some adjustment to your build env may be needed.

For windows, you need to start 64bit build env, ie: `C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat`

## CMake

#### Windows
Just download and install binaries from https://cmake.org/download/

#### Linux
    wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz
    tar -xf ./cmake-3.12.3.tar.gz
    cd cmake-3.12.3
    ./configure --prefix=`pwd`/install
    make -j4
    sudo make install
    export PATH=$PATH:`pwd`/install


## boost 1.68

#### Linux
    wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz
    tar -xf ./boost_1_68_0.tar.gz
    cd boost_1_68_0
    ./bootstrap.sh
    ./b2 -j 4 --layout=tagged --build-type=complete stage

#### Windows
Download from https://www.boost.org/users/download/ and unpack.

    cd boost
    bootstrap.bat
    b2.exe  --layout=tagged --build-type=complete --build-dir=build/static toolset=msvc address-model=64 link=static  stage


## FFTW
You need to build FFTW with float (single precission) support. Also, enable neon optimizations for ARM.

#### Linux x86
    wget http://fftw.org/fftw-3.3.8.tar.gz
    tar -xf ./fftw-3.3.8.tar.gz
    cd fftw-3.3.8
    ./configure --with-slow-timer --enable-single CFLAGS="-O4 -pipe -march=native -Wall" --prefix=`pwd`/install
    make
    make install

#### Linux ARM
    wget http://fftw.org/fftw-3.3.8.tar.gz
    tar -xf ./fftw-3.3.8.tar.gz
    cd fftw-3.3.8
    ./configure --with-slow-timer --enable-single --enable-neon CFLAGS="-O4 -pipe -march=native -mfpu=neon -Wall" --prefix=`pwd`/install
    make
    make install

#### Windows
Download http://fftw.org/fftw-3.3.8.tar.gz and unpack.

    cmake -D BUILD_SHARED_LIBS=0 -D ENABLE_FLOAT=1 -D CMAKE_INSTALL_PREFIX=d:\dev\c_libs\fftw\3.3.8\install -D CMAKE_BUILD_TYPE=RELEASE -G "NMake Makefiles" ..\src
    nmake
    nmake install


## SoapySDR

#### Windows
Just go and install binaries from https://github.com/pothosware/PothosCore/wiki/Downloads

#### RaspberryPI
SoapySDR packages are available with apt on Raspbian, but I had problems with these.
You are encouraged to build from source, otherwise performance problems may arise.

tip: while building osmocom-rtlsdr driver, use this command:

    cmake ../ -DINSTALL_UDEV_RULES=ON -DDETACH_KERNEL_DRIVER=ON

#### Linux

    git clone https://github.com/pothosware/SoapySDR.git
    cd SoapySDR
    git pull origin master

    mkdir build
    cd build
    cmake ..
    make -j
    sudo make install
    sudo ldconfig
    SoapySDRUtil --info

### SoapySDR AirSpy Support

    git clone https://github.com/pothosware/SoapyAirspy.git
    mkdir build
    cd build
    cmake ../SoapySDR
    make
    sudo make install
    sudo ldconfig

### SoapySDR RTL-SDR Support
Refer to https://github.com/pothosware/SoapyRTLSDR


## HABDEC Build

If you managed to build or install dependencies, you're ready do build habdec

    git clone habdec
    cd habdec
    mkdir build
    cd build
    cmake cmake -D BOOST_ROOT=/path/to/boost_1.68 -D FFTW_ROOT=/path/to/fftwf/install -DCMAKE_BUILD_TYPE=RELEASE ../code
    make
    make install

You should end up with `install` dir that contains executable and a shell script that runs in loop (for cases where habdec should crash and you wan't continous run).