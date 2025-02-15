cp -r * ../
cd ..
sudo chmod 775 git.sh
setfont ter-132n

sudo mkfs.ext4 /dev/sda
sudo mount /dev/sda /mnt/sda
df -h
