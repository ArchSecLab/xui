#!/bin/bash

# Update the system
sudo apt update

# Install required dependencies
sudo apt install -y build-essential libpciaccess-dev libaio-dev ninja-build

# Clone the SPDK repository
git clone https://github.com/spdk/spdk.git
cd spdk

# Build SPDK
./configure --with-rdma
make -j$(nproc)

# Install SPDK
sudo make install

# Add SPDK to LD_LIBRARY_PATH
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/local/lib" >> ~/.bashrc
source ~/.bashrc

# Verify the installation
spdk/scripts/setup.sh

echo "SPDK installation completed successfully!"