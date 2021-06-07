#include <linux/module.h> // MODULE_LICENSE, MODULE_AUTHOR
#include <linux/kernel.h> // KERN_INFO
#include <linux/init.h>	  // ​Макросы __init и ​__exit
#include <linux/init_task.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

// Посмотреть инф-ию о обработчике прерывания
// cat /proc/interrupts | head -n 1 && cat /proc/interrupts| grep my_irq_handler
// CPUi - число прерываний, полученных i-ым процессорным ядром.

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alice");
MODULE_DESCRIPTION("My module!");

int my_irq = 1; // Прерывание от клавиатуры.
static int irq_cnt = 0;

static struct workqueue_struct *my_wq; //очередь работ

static void my_wq_function(struct work_struct *work) // вызываемая функция
{
	// For kernel 5.4
	atomic64_t data64 = work->data;
	long long data = data64.counter;
	// For kernel 5.4

	printk("Module: my_wq_function data = %lld\n", data);
	// TODO: А тут ошибка "dereferencing pointer to incomplete type ‘struct workqueue_struct’" (разыменование указателя на неполный тип)
	printk("Module: my_wq_function data = %lld\n, WQ:name workqueue: %s, current work color:%d", data, my_wq->name, my_wq->work_color);
	return;
}

// Статическая инициализация структуры.
// Создает (работу) переменную my_work с типом struct work_struct *
DECLARE_WORK(my_work, my_wq_function);
DECLARE_WORK(my_work2, my_wq_function);

irqreturn_t irq_handler(int irq, void *dev, struct pt_regs *regs)
{
	if (irq == my_irq)
	{
		// Помещаем структуру в очередь работ.
		// queue_work назначает работу текущему процессору.
		queue_work(my_wq, &my_work);
		queue_work(my_wq, &my_work2);
		printk(KERN_INFO "Module:  my_irq_handler was called %d time(s)", irq_cnt++);
		return IRQ_HANDLED; // прерывание обработано
	}
	else
		return IRQ_NONE; // прерывание не обработано
}

static int __init md_init(void)
{
	// регистрация обработчика прерывания
	if (request_irq(
			my_irq,						/* номер irq */
			(irq_handler_t)irq_handler, /* наш обработчик */
			IRQF_SHARED,				/* линия может быть раздедена, IRQ
										(разрешено совместное использование)*/
			"my_irq_handler",			/* имя устройства (можно потом посмотреть в /proc/interrupts)*/
			(void *)(irq_handler)))		/* Последний параметр (идентификатор устройства) irq_handler нужен
										для того, чтобы можно отключить с помощью free_irq  */
	{
		printk(" + Error request_irq");
		return -1;
	}

	my_wq = create_workqueue("my_queue"); //создание очереди работ
	if (my_wq)
	{
		printk(KERN_INFO "Module: workqueue created!\n");
	}
	else
	{
		free_irq(my_irq, irq_handler); // Отключение обработчика прерывания.
		printk(KERN_INFO "Module: error create_workqueue()!\n");
		return -1;
	}

	printk(KERN_INFO "Module: module md start!\n");
	return 0;
}

static void __exit md_exit(void)
{
	// Принудительно завершаем все работы в очереди.
	// Вызывающий блок блокируется до тех пор, пока операция не будет завершена.
	flush_workqueue(my_wq);
	destroy_workqueue(my_wq);

	// my_irq - номер прерывания.
	// irq_handler - идентификатор устройства.
	free_irq(my_irq, irq_handler); // Отключение обработчика прерывания.
	printk(KERN_INFO "Module: Goodbye!\n");
}

module_init(md_init);
module_exit(md_exit);