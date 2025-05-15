sudo pacman -S gcc make binutils gawk m4 autoconf automake patch tar gzip bzip2 xz
mkdir -p /mnt/lfs
sudo useradd -m -g lfs -s /bin/bash lfs
sudo passwd lfs
su - lfs
ssss