#curl -L http://cdimage.ubuntu.com/ubuntu-base/releases/20.04/release/ubuntu-base-20.04.1-base-amd64.tar.gz -o ubuntu-base-20.04.1.tar.gz
util/gem5img.py init ubuntu-x86.img 16384
util/gem5img.py mount ubuntu-x86.img ../mnt
sudo tar -xvzf ubuntu12.tar.gz -C ../mnt
#sudo cp -a ../req/uintr-linux-kernel ../mnt
util/gem5img.py umount ../mnt
