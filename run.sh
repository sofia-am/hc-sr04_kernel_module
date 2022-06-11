make clean
make
sudo rm /dev/SdeC_drive
sudo rmmod drv.ko
#sudo rm /dev/SdeC_drv4
#sudo rm /dev/SdeC_1
sudo insmod drv.ko
sudo python3 prueba.py
