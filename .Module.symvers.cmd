cmd_/home/pi/Documents/hc-sr04_kernel_module/Module.symvers := sed 's/\.ko$$/\.o/' /home/pi/Documents/hc-sr04_kernel_module/modules.order | scripts/mod/modpost -m -a  -o /home/pi/Documents/hc-sr04_kernel_module/Module.symvers -e -i Module.symvers   -T -
