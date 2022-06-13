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


MODULE_LICENSE("GPL");
MODULE_AUTHOR("ASMAN");
MODULE_DESCRIPTION("Driver para gestión de pulsadores");
MODULE_VERSION("18949174918471.4");

//Timer Variable
#define TIMEOUT 1000 //milisegundos
#define GPIO_21 (21)

//Tamaño maximo de memoria a recibir/transmitir
#define mem_size 128

//Acumulado de presiones del boton 1 considerando escala1
int boton1 = 0;
//Acumulado de presiones del boton 2 considerando escala2
int boton2 = 0;

//Buffers
//Buffer para la transmision de datos
uint8_t *buffer_entrada;
//Buffer para lectura de datos
uint8_t *buffer_salida;

//Estructura para config de timer
static struct timer_list timer;

/* Define GPIOs para BUTTONS */
static struct gpio buttons[] = {
    {16, GPIOF_IN, "BUTTON 1"}, 
    {20, GPIOF_IN, "BUTTON 2"}, 
};

// Los numeros IRQ asociados a los botones seran asignados aqui
static int button_irqs[] = {-1, -1};

//Variables usadas para el registro/config de CDD
dev_t dev = 0;
static struct class *dev_class;
static struct cdev driverCDev;

//Funciones de inicio y exit del modulo
static int __init driverInit(void);
static void __exit driverExit(void);

/*************** Funciones del Driver **********************/
static int driverOpen(struct inode *inode, struct file *file);
static int driverRelease(struct inode *inode, struct file *file);
static ssize_t driverRead(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t driverWrite(struct file *filp, const char *buf, size_t len, loff_t *off);
/******************************************************/

//Estructura de operaciones sobre ficheros
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = driverRead,
    .write = driverWrite,
    .open = driverOpen,
    .release = driverRelease,
};

//Esta funcion sera llamada cuando el timer termine
void timer_callback(struct timer_list *data)
{
 /*   
    if (gpio_get_value(GPIO_21))
    {
        gpio_set_value(GPIO_21, 0);
        pr_info("LED encendido\n");
    }
    else
    {
        gpio_set_value(GPIO_21, 1);
        pr_info("LED apagado\n");
    }*/
    pr_info("Valor entrada: %c \n",buffer_salida[0]);

    memset(buffer_entrada, '\0', mem_size);
    switch (buffer_salida[0])
        {
            case 'a':
      
                sprintf(buffer_entrada, "%d", boton1);
                pr_info("Valor del contador: %s \n",buffer_entrada);
                
                break;

            case 'b':
                                     
                sprintf(buffer_entrada, "%d", boton2);
                pr_info("Valor del contador: %s \n", buffer_entrada);
                break;

            default:
                break;
    }

    /*
       Rehabilita el timer y asigna un nuevo tiempo.
    */
    mod_timer(&timer, jiffies + msecs_to_jiffies(TIMEOUT));
}

/*
 * El servicio de interrupcion llamada cuando se apriete un boton 
 */
static irqreturn_t button_isr(int irq, void *data)
{
    if (irq == button_irqs[0])
    {
        pr_info("Boton 1\n");
        //Sist 2
        boton1++;
        if (boton1 == 30)
            boton1 = 0;
        
        gpio_set_value(GPIO_21, 1);
        pr_info("LED encendido\n");
    }
    else if (irq == button_irqs[1])
    {
        pr_info("Boton 2\n");
        //Sist 2
        boton2++;
        if (boton2 == 30)
            boton2 = 0;

        gpio_set_value(GPIO_21, 0);
        pr_info("LED apagado\n");
        
    }
    mdelay(125);
    return IRQ_HANDLED;
}

/*
** Esta funcion sera llamada cuando abramos el fichero de dispositivo
*/
static int driverOpen(struct inode *inode, struct file *file)
{
    //pr_info("Device File Opened...!!!\n");
    return 0;
}

/*
** Esta funcion sera llamada cuando cerremos el fichero de dispositivo
*/
static int driverRelease(struct inode *inode, struct file *file)
{
    //pr_info("Device File Closed...!!!\n");
    return 0;
}

/*
** Esta funcion sera llamada cuando leamos el fichero de dispositivo
*/
static ssize_t driverRead(struct file *filp, char __user *buf, size_t len, loff_t *off)
{

    if (*off == 0)
    {
        if (copy_to_user(buf, buffer_entrada, mem_size) != 0){                  

            printk(KERN_INFO "Valor: %s", buffer_entrada);
            return -EFAULT;
        }
        else
        {
            printk(KERN_INFO "Leyendo\n");
            (*off)++;
            // if(strlen (buffer_entrada)>len) //len debe ser la cant max de bytes leidos 
            //     return len; 
            return strlen (buffer_entrada);//este es el n de bytes que se van a leer (desde buf)
        }
    }
    else
        return 0;
}

/*
** Esta funcion sera llamada cuando escribamos el fichero de dispositivo
*/
static ssize_t driverWrite(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    if (copy_from_user(buffer_salida, buf, len) > 0)
    {
        pr_info("Error while writing data\n");
        goto fail;
    }

    return len;

fail:
    memset(buffer_salida, '\0', mem_size);
    return len;
}

/*
** Funcion de inicio del modulo
*/
static int __init driverInit(void)
{
    int ret = 0;
    boton1 = 0;
    boton2 = 0;

    /*Alocando el numero mayor*/
    if ((alloc_chrdev_region(&dev, 0, 1, "ASMAN_driver")) < 0)
    {
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /*Creando estructura cdev*/
    cdev_init(&driverCDev, &fops);

    /*Agregando el CDD al sistema*/
    if ((cdev_add(&driverCDev, dev, 1)) < 0)
    {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creando estrucutra de clase*/
    if ((dev_class = class_create(THIS_MODULE, "ASMAN_class")) == NULL)
    {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creando dispositivo*/
    if ((device_create(dev_class, NULL, dev, NULL, "ASMAN_driver")) == NULL)
    {
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }

    //CONFIGURACION DEL PIN DE SALIDA
    //Comprobando si el GPIO es valido o no
    if (gpio_is_valid(GPIO_21) == false)
    {
        pr_err("GPIO %d is not valid\n", GPIO_21);
        goto r_device;
    }

    //Solicitando el GPIO
    if (gpio_request(GPIO_21, "GPIO_21") < 0)
    {
        pr_err("ERROR: GPIO %d request\n", GPIO_21);
        goto r_gpio;
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

    printk(KERN_INFO "Current button1 value: %d\n", gpio_get_value(buttons[0].gpio));

    ret = gpio_to_irq(buttons[0].gpio);

    if (ret < 0)
    {
        printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
        goto fail2;
    }

    button_irqs[0] = ret;

    printk(KERN_INFO "Successfully requested BUTTON1 IRQ # %d\n", button_irqs[0]);

    ret = request_irq(button_irqs[0], button_isr, IRQF_TRIGGER_RISING /* | IRQF_DISABLED */, "gpiomod#button1", NULL);

    if (ret)
    {
        printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
        goto fail2;
    }

    ret = gpio_to_irq(buttons[1].gpio);

    if (ret < 0)
    {
        printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
        goto fail2;
    }

    button_irqs[1] = ret;

    printk(KERN_INFO "Successfully requested BUTTON2 IRQ # %d\n", button_irqs[1]);

    ret = request_irq(button_irqs[1], button_isr, IRQF_TRIGGER_HIGH /* | IRQF_DISABLED */, "gpiomod#button2", NULL);

    if (ret)
    {
        printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
        goto fail3;
    }

    //######################################################################################################//
    //Aloco memoria para el String buffer_entrada
    if ((buffer_entrada = kmalloc(mem_size, GFP_KERNEL)) == 0)
    {
        pr_info(KERN_INFO "Cannot allocate the memory to the kernel\n");
        return -1;
    }
    if ((buffer_salida = kmalloc(mem_size, GFP_KERNEL)) == 0)
    {
        pr_info(KERN_INFO "Cannot allocate the memory to the kernel\n");
        return -1;
    }
    //######################################################################################################//
    
    //CONFGIURACION DEL TIMER
    // Configurando el timer que llamara a my_timer_callback
    timer_setup(&timer, timer_callback, 0);

    // Configurando el intervalo del timer basado en la macro TIMEOUT
    mod_timer(&timer, jiffies + msecs_to_jiffies(TIMEOUT));

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

fail3:
    free_irq(button_irqs[0], NULL);
fail2:
    gpio_free_array(buttons, ARRAY_SIZE(buttons) / 2);
r_gpio:
    gpio_free(GPIO_21);
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    return -1;
}

/*
** Funcion de salida del modulo
*/
static void __exit driverExit(void)
{
    //lIBERO Buffer
    kfree(buffer_entrada); 
    kfree(buffer_salida);
    //Libero interrupciones y gpio
    free_irq(button_irqs[0], NULL);
    free_irq(button_irqs[1], NULL);
    gpio_free_array(buttons, ARRAY_SIZE(buttons));
    //Seteamos el valor de la salida a 0
    gpio_set_value(GPIO_21, 0);
    //Remueve GPIO del kernel   
    gpio_unexport(GPIO_21);
    gpio_free(GPIO_21);
    //Remueve el timer del kernel al descargar el modulo
    del_timer(&timer);
    //Remueve el resto de clases
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&driverCDev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!!\n");
}

module_init(driverInit);
module_exit(driverExit);

