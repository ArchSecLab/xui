#curl -L http://cdimage.ubuntu.com/ubuntu-base/releases/20.04/release/ubuntu-base-20.04.1-base-amd64.tar.gz -o ubuntu-base-20.04.1.tar.gz
util/gem5img.py init ubuntu-x86.img 16384
util/gem5img.py mount ubuntu-x86.img ../mnt
sudo tar -xvf ubuntu7.tar -C ../mnt
sudo cp init-safepoint ../mnt/bin/init
sudo cp -r aspen-gem5/safepoint-benchmarks ../mnt/.
sudo chroot ../mnt /bin/bash << EOT
echo "kernel.randomize_va_space = 0" > /etc/sysctl.d/01-disable-aslr.conf
apt update
echo "install clang-11"
apt install -y clang-11
clang-11 --version
echo "install llvm-11"
apt install -y llvm-11
apt install -y libpapi-dev libpfm4-dev
pushd safepoint-benchmarks
make clean
make 
pushd concord/src
pushd cache-line-pass
./setup-pass.sh
popd
pushd cache-line-pass-func
./setup-pass.sh
popd
pushd cache-line-pass-loopfunc
./setup-pass.sh
popd
pushd safepoint
./setup-pass.sh
popd
popd
pushd programs
mkdir build
make
popd
popd
EOT
util/gem5img.py umount ../mnt