curl -L http://cdimage.ubuntu.com/ubuntu-base/releases/20.04/release/ubuntu-base-20.04.1-base-amd64.tar.gz -o ubuntu-base-20.04.1.tar.gz
util/gem5img.py init ubuntu-x86.img 8192
util/gem5img.py mount ubuntu-x86.img ../mnt
sudo tar -xvzf ubuntu-base-20.04.1.tar.gz -C ../mnt
sudo cp gem5-fs/disk_files/fstab ../mnt/etc/fstab
sudo cp gem5-fs/disk_files/init ../mnt/bin/init
sudo cp gem5-fs/disk_files/m5 ../mnt/sbin/m5
sudo cp /etc/resolv.conf ../mnt/etc/resolv.conf
sudo chmod +x ../mnt/sbin/m5
sudo chmod +x ../mnt/bin/init
sudo cp -r uintr-measure ../mnt/.
sudo chroot ../mnt /bin/bash << EOT
echo "kernel.randomize_va_space = 0" > /etc/sysctl.d/01-disable-aslr.conf
apt update
apt install -y debconf-utils
echo "Europe/Zurich" > /etc/timezone
dpkg-reconfigure -f noninteractive tzdata
apt install -y software-properties-common wget tar cmake || 1
add-apt-repository -y ppa:ubuntu-toolchain-r/test || 1
apt update || 1
apt install -y gcc-11 g++-11 || 1
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 11 || 1
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 11 || 1
#echo gcc
wget https://ftp.gnu.org/gnu/binutils/binutils-2.36.1.tar.gz
tar xzf binutils-2.36.1.tar.gz
cd binutils-2.36.1
./configure
make
make install
echo binutils
apt install -y pkg-config
cd /uintr-measure/micro/uintr-ipc-bench/source/pmc_bench
python3 build.py
EOT
util/gem5img.py umount ../mnt
