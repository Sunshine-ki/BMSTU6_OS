#include <linux/module.h> // MODULE_LICENSE, MODULE_AUTHOR
#include <linux/kernel.h> // KERN_INFO
#include <linux/init.h>	  // ​Макросы __init и ​__exit
#include <linux/init_task.h>
#include <linux/interrupt.h>
#include <linux/timex.h>

// Посмотреть инф-ию о обработчике прерывания
// cat /proc/interrupts | head -n 1 && cat /proc/interrupts| grep my_irq_handler
// CPUi - число прерываний, полученных i-ым процессорным ядром.
// TODO: В нашем случае при каждом нажатии клавиши увеличивается на 3, почему...?

/*
enum
{
	// Когда tasklet запланирован, ему выставляется состояние TASKLET_STATE_SCHED,
	TASKLET_STATE_SCHED, 0
	// TASKLET_STATE_RUN блокирует tasklet, что предотвращает исполнение одного и того же tasklet’а на разных CPU.
	TASKLET_STATE_RUN	1
};
*/
// TODO: А вот состояние 2 это какое...?

/*
Обработчики прерываний делятся на быстрые и медленные.
Быстрые выполняются от начала до конца.
(В современных системах остался только один быстрый обработчик - от таймера).
(Таймер ничего не вызывает, а только инициализирует отложенные вызовы).
*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alice");
MODULE_DESCRIPTION("Tasklet");

int my_irq = 1; // Прерывание от клавиатуры.

char my_tasklet_data[] = "my_tasklet_function была вызвана";

void my_tasklet_function(unsigned long data);

// Статическое создание тасклета.
// Создает экземпляр структуры struct tasklet_struct с именем my_tasklet.
DECLARE_TASKLET(my_tasklet, my_tasklet_function, (unsigned long)&my_tasklet_data);

void my_tasklet_function(unsigned long data)
{
	atomic_t count = my_tasklet.count;
	/*
	typedef struct {
		int counter;
	} atomic_t;
	*/
	printk(" + data: %s счетчик ссылок: %u состояние тасклета: %lu\n", (char *)data, count.counter, my_tasklet.state);
	return;
}

irqreturn_t irq_handler(int irq, void *dev, struct pt_regs *regs)
{
	if (irq == my_irq)
	{
		// Запланировать тасклет на выполнение.
		// После того, как он запланирован, он добавляется в очередь (на планирование)
		// (состояние меняется на TASKLET_STATE_SCHED).
		// После того, как он запланирован, он выполняется
		// один раз в некоторый момент вермени в будущем.
		// (тасклет всегда выполняется на том процессоре,
		//  который его запланировал на выполнение)
		printk(" + irq_handler был вызван");
		tasklet_schedule(&my_tasklet);
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

	printk(KERN_INFO " + Module: module loaded!\n");
	return 0;
}

static void __exit md_exit(void)
{
	// Удаляет из очереди тасклетов, ожидающих выполнения.
	tasklet_kill(&my_tasklet);
	// my_irq - номер прерывания.
	// irq_handler - идентификатор устройства.
	free_irq(my_irq, irq_handler); // Отключение обработчика прерывания.
	printk(KERN_INFO " + Module: unloaded!\n");
}

module_init(md_init);
module_exit(md_exit);