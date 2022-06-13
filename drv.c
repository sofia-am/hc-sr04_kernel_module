// Parto de drv3 para realizar las modificaciones a rea y write para ver qué hace
// estoy yendo en la dirección de clipboard.buffer_entrada ..!!

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/delay.h>

//Timer Variable
#define TIMEOUT 1000 //milisegundos

#define mem_size 5
#define GPIO_21 (21)
#define GPIO_16 (16)
#define GPIO_20 (20)

//Estructura para config de timer
static struct timer_list timer;

static dev_t first; 		// Global variable for the first device number
static struct cdev c_dev; 	// Global variable for the character device structure
static struct class *cl; 	// Global variable for the device class

static struct gpio buttons[] = {
    {16, GPIOF_IN, "BUTTON 1"}, 
    {20, GPIOF_IN, "BUTTON 2"}, 
};

char *buffer_entrada;
char *buffer_salida;
int contador1;
int contador2;

static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "SdeC_drv4: open()\n");
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "SdeC_drv4: close()\n");
    //TODO: hacer los free 
    return 0;
}

void timer_callback(struct timer_list *data){
    pr_info("Valor entrada: %c \n",buffer_entrada[0]);

    memset(buffer_salida, '\0', mem_size);
    switch (buffer_entrada[0])
        {
            case 'a':
      
                sprintf(buffer_salida, "%d", contador1);
                pr_info("Valor del contador1: %s \n",buffer_salida);
                
                break;

            case 'b':
                                     
                sprintf(buffer_salida, "%d", contador2);
                pr_info("Valor del contador1: %s \n", buffer_salida);
                break;

            default:
                break;
    }
}

// ssize_t resulta ser una palabra con signo.
// Por lo tanto, puede ocurrir que devuelva un número negativo. Esto sería un error. 
// Pero un valor de retorno no negativo tiene un significado adicional. 
// Para my_read sería el número de bytes leídos

// Cuando hago un $ cat /dev/SdeC_drv3, se convoca a my_read.!!
// my_read lee "len" bytes, los guarda en "buf" y devuelve la cantidad leida, que puede
// ser menor, pero nunca mayor que len.

// En SdeC_drv3, devuelve cero. Dado que es un archivo, esto significa no hay mas datos ó EOF.
// Lo que tendría que ocurrir es que el device escriba sobre buf para que el usuario pueda 
// obtener una lectura no nula.

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    //timer_callback(&timer); //longitud del buff de salida
    if (*off == 0)
    {
        if(gpio_get_value(GPIO_16)){
            contador1++;
            if(contador1 == 30)
                    contador1 = 0;
            gpio_set_value(GPIO_21, 1);
        }
        if(gpio_get_value(GPIO_20)){
            contador2++;
            if(contador2 == 30)
                contador2 = 0;
            gpio_set_value(GPIO_21, 0);
        }

        timer_callback(&timer);

        if (copy_to_user(buf, buffer_salida,mem_size) != 0){                  

            printk(KERN_INFO "Valor del contador1: %s", buffer_salida);
            return -EFAULT;
    }
        else
        {
            printk(KERN_INFO "leyendo contador1 \n");
            (*off)++;
            // if(strlen (buffer_salida)>len) //len debe ser la cant max de bytes leidos 
            //     return len; 
            return strlen (buffer_salida);//este es el n de bytes que se van a leer (desde buf)
        }
    }
    else
        return 0;
    

}
// my_write escribe "len" bytes en "buf" y devuelve la cantidad escrita, que debe ser igual "len".
// Cuando hago un $ echo "bla bla bla..." > /dev/SdeC_drv3, se convoca a my_write.!!

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "SdeC_drv4: write()\n");    

    memset(buffer_entrada, '\0', mem_size);
    // if(mem_size > len)
    //     len = mem_size;

    if (copy_from_user(buffer_entrada, buf , len) != 0 )    
        return -EFAULT;
    else{
        if(buffer_entrada[0] == 'a'){
            //contador1++;
        }
        else if(buffer_entrada[0] == 'b'){
            //contador2+=2;
        }       
        return len;
    }
            
}




static struct file_operations pugs_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static int __init drv1_init(void) /* Constructor */
{
    int ret = 0;
    contador1 = 0;
    contador2 = 0;
    struct device *dev_ret;

    //alocamos memoria para el buffer de entrada
    if ((buffer_entrada = kmalloc(mem_size, GFP_KERNEL)) == 0)
    {
        pr_info(KERN_INFO "Error al alocar memoria para buffer_entrada\n");
        return -1;
    }
   
    if ((buffer_salida = kmalloc(mem_size, GFP_KERNEL)) == 0)
    {
        pr_info(KERN_INFO "Error al alocar memoria para buffer_salida\n");
        return -1;
    }
  
    
    // //CONFGIURACION DEL TIMER
    // // Configurando el timer que llamara a my_timer_callback
    // timer_setup(&timer, timer_callback, 0);

    // // Configurando el intervalo del timer basado en la macro TIMEOUT
    // mod_timer(&timer, jiffies + msecs_to_jiffies(TIMEOUT));

 
    printk(KERN_INFO "SdeC_drv4: Registrado exitosamente..!!\n");

//configuracion del modulo
    if ((ret = alloc_chrdev_region(&first, 0, 1, "SdeC_drv4")) < 0)
    {
        return ret;
    }

    if (IS_ERR(cl = class_create(THIS_MODULE, "SdeC_drive")))
    {
        unregister_chrdev_region(first, 1);
        return PTR_ERR(cl);
    }

    if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "SdeC_drive")))
    {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return PTR_ERR(dev_ret);
    }

    cdev_init(&c_dev, &pugs_fops);
    if ((ret = cdev_add(&c_dev, first, 1)) < 0)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return ret;
    }

    if (gpio_is_valid(GPIO_21) == false)
    {
        pr_err("GPIO %d is not valid\n", GPIO_21);
        goto fail1;
    }

    //Solicitando el GPIO
    if (gpio_request(GPIO_21, "GPIO_21") < 0)
    {
        pr_err("ERROR: GPIO %d request\n", GPIO_21);
        goto fail1;
    }

    //Configurando el GPIO como salida
    gpio_direction_output(GPIO_21, 0);

    //Registro de BUTTON GPIOs
    ret = gpio_request_array(buttons, ARRAY_SIZE(buttons));

    if (ret)
    {
        printk(KERN_ERR "Unable to request GPIOs for BUTTONs: %d\n", ret);
        goto fail2;
    }

    return 0;

fail1:
    gpio_free(GPIO_21);
fail2:
    gpio_free_array(buttons, ARRAY_SIZE(buttons));

    return -1;
}

static void __exit drv1_exit(void) /* Destructor */
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    
    printk(KERN_INFO "SdeC_1: dice Adios mundo cruel..!!\n");
}

module_init(drv1_init);
module_exit(drv1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cátedra Sistemas de Computación");
MODULE_DESCRIPTION("Nuestro cuarto driver de SdeC");
