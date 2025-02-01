#curl -L http://cdimage.ubuntu.com/ubuntu-base/releases/20.04/release/ubuntu-base-20.04.1-base-amd64.tar.gz -o ubuntu-base-20.04.1.tar.gz
util/gem5img.py init ubuntu-x86.img 16384
util/gem5img.py mount ubuntu-x86.img ../mnt
sudo tar -xvf ubuntu7.tar -C ../mnt
# sudo cp -r aspen-gem5 ../mnt/.
sudo cp util/m5/build/x86/out/libm5.a ../mnt/libm5.a
sudo cp init-rocksdb ../mnt/bin/init
sudo chroot ../mnt /bin/bash << EOT
echo "kernel.randomize_va_space = 0" > /etc/sysctl.d/01-disable-aslr.conf
echo "apt update"
apt update -o Acquire::ForceIPv4=true
echo "apt install"
apt install -y git make gcc cmake pkg-config libnl-3-dev libnl-route-3-dev libnuma-dev uuid-dev libssl-dev libaio-dev libcunit1-dev libclang-dev libncurses-dev meson python3-pyelftools
apt install -y strace gdb
apt install -y libjemalloc-dev libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev
git clone https://github.com/LinsongGuo/aspen-gem5.git
mv /libm5.a /aspen-gem5/m5/libm5.a
cd /aspen-gem5
make submodules
mv dpdk dpdk-old
cp -r ../dpdk dpdk
export PKG_CONFIG_PATH=/aspen-gem5/dpdk/build/lib64/pkgconfig
echo "--------PKG_CONFIG_PATH:$PKG_CONFIG_PATH"
./compile.sh
EOT
util/gem5img.py umount ../mnt