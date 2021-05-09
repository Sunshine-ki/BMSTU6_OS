// https://www.opennet.ru/base/dev/virtual_fs.txt.html
// +
// http://rus-linux.net/MyLDP/BOOKS/Moduli-yadra-Linux/06/kern-mod-06-05.html

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/time.h>

#define MYFS_MAGIC_NUMBER 0x1313131313;
#define SLABNAME "my_cache"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alice");
MODULE_DESCRIPTION("My first module!");

// Передаем параметры модулю (загружаемому модулю ядра).
// (По сути, это полная аналогия с передачей аргументов в программу (argv[])
// Далее значения параметрам могут быть установлены во время загрузки модуля через insmod (см. пример module_param.c).
static int size = 7;
module_param(size, int, 0); // (имя, тип, права_доступа)
static int number = 31;
module_param(number, int, 0);

static void **line = NULL;
// sco - кол-во вызовов вызовов конструктора
// (сколько раз вызвался конструктор при размещении number объектов)
static int sco = 0;

// Создаем структуру кэша slab
static struct kmem_cache *cache = NULL;

void co(void *p)
{
	// При размещении объекта вызывается вызывается данная функция.
	*(int *)p = (int)p;
	sco++;
}

// myfs_inode нужна для кэширования inode.
struct myfs_inode
{
	int i_mode;
	unsigned long i_ino;
} myfs_inode;

static void myfs_put_super(struct super_block *sb)
{
	printk(KERN_DEBUG "myfs : super block destroyed!\n");
}

// int free_alloc_inodes(struct inode *inode)
// {
// 	kmem_cache_free(cache, inode->i_private);
// 	return 1;
// }

// Суперблок - структура, которая описывает ФС (метаданные).
// inode - метаданные о файле (!не содержит путь!) (содержит тип файла, права доступа и тд...)
// dentry (directory entry - запись каталога) - держит дескрипторы и файлы вместе, связывая номер индексных дескрипторов файлов с именами файлов.

static struct super_operations const myfs_super_ops = {
	// myfs_put_super будет вызываться при размонтировании ФС.
	.put_super = myfs_put_super, // Деструктор нашего суперблока.
	.statfs = simple_statfs,	 // Заглушка из libfs. (предоставляется ядром)
	// .drop_inode = free_alloc_inodes,
	.drop_inode = generic_delete_inode, // Заглушка из libfs. (предоставляется ядром)
};

static struct inode *myfs_make_inode(struct super_block *sb, int mode)
{
	// Размещаем новую структуру. ?
	struct inode *ret = new_inode(sb);

	if (ret)
	{
		// struct myfs_inode *my_inode = kmem_cache_alloc(cache, GFP_KERNEL); // get free pages kernel
		inode_init_owner(ret, NULL, mode);

		// *my_inode = (struct myfs_inode){
		// 	.i_mode = ret->i_mode,
		// 	.i_ino = ret->i_ino};

		ret->i_size = PAGE_SIZE;
		ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
		ret->i_private = &myfs_inode;
	}

	return ret;
}

// myfs_fill_sb - создает корневой каталог нашей ФС.
static int myfs_fill_sb(struct super_block *sb, void *data, int silent)
{
	struct inode *root = NULL;

	// Инициализация суперблока.
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	// Магическое число, по которому драйвер файловой системы
	// может проверить, что на диске хранится именно та самая
	// файловая система, а не что-то еще или прочие данные.
	sb->s_magic = MYFS_MAGIC_NUMBER;
	sb->s_op = &myfs_super_ops; // Операции для суперблока.

	// Строим корневой каталог нашей ФС.
	// S_IFDIR - создаем каталог (регулярный).
	// 0755 - стандартные права доступа для папок (rwx r-x r-x). (владелец группа остальные_пользователи)
	root = myfs_make_inode(sb, S_IFDIR | 0755); // Создаем inode.
	if (!root)
	{
		printk(KERN_ERR "myfs : inode allocation failed\n");
		return -ENOMEM;
	}

	root->i_op = &simple_dir_inode_operations; // предоставляется ядром.
	root->i_fop = &simple_dir_operations;	   // предоставляется ядром.

	// суперблок имеет специальное поле, хранящее указатель на dentry корневого каталога.
	sb->s_root = d_make_root(root);
	if (!sb->s_root)
	{
		printk(KERN_ERR "myfs : root creation failed\n");
		iput(root);
		return -ENOMEM;
	}

	return 0;
}

// For my system:
// https://elixir.bootlin.com/linux/v5.4/source/include/linux/fs.h#L2210
static struct dentry *myfs_mount(struct file_system_type *type,
								 int flags, char const *dev, void *data)
{
	// TODO: Убрать
	printk("myfs Name: %s", type->name);

	// Примонтирует устройство и возвращает структуру, описывающую корневой каталог файловой системы.
	// myfs_fill_sb - функция, которая будет вызвана  из mount_bdev, чтобы проинициализировать суперблок.
	struct dentry *const entry = mount_bdev(type, flags, dev, data, myfs_fill_sb);
	if (IS_ERR(entry))
		printk(KERN_ERR "myfs : mounting failed!\n");
	else
		printk(KERN_DEBUG "myfs : mounted");
	return entry;
}

static struct file_system_type myfs_type = {
	.owner = THIS_MODULE, // Счетчик ссылок на модуль. Необходим для предотвращения выгрузки используемого модуля.
	.name = "myfs",		  // Название файловой системы.
	.mount = myfs_mount,  // Ф-ция, которая вызывается при монтировании ФС.
	// kill_block_super - предоставляет ядро.
	.kill_sb = kill_block_super, // Ф-ция, которая вызывается при размонтировании ФС.
};

static int __init myfs_init(void)
{
	int i;
	int ret;

	if (size < 0)
	{
		printk(KERN_ERR "invalid argument\n");
		return -EINVAL;
	}
	line = kmalloc(sizeof(void *) * number, GFP_KERNEL); // Get Free Pages
	if (!line)
	{
		printk(KERN_ERR "kmalloc error\n");
		kfree(line);
		return -ENOMEM;
	}
	for (i = 0; i < number; i++)
		line[i] = NULL;

	// Создаем новый кэш.
	cache = kmem_cache_create(SLABNAME, size, 0, SLAB_HWCACHE_ALIGN, co);

	if (!cache)
	{
		printk(KERN_ERR "kmem_cache_create error\n");
		kmem_cache_destroy(cache);
		kfree(line);
		return -ENOMEM;
	}

	for (i = 0; i < number; i++)
	{
		if (NULL == (line[i] = kmem_cache_alloc(cache, GFP_KERNEL)))
		{
			printk(KERN_ERR "kmem_cache_alloc error\n");
			for (i = 0; i < number; i++)
				kmem_cache_free(cache, line[i]);
			kmem_cache_destroy(cache);
			kfree(line);
			return -ENOMEM;
		}
	}

	ret = register_filesystem(&myfs_type); // Системный вызов.

	if (ret != 0)
	{
		printk(KERN_ERR "myfs : can't register filesystem\n");
		return ret;
	}

	printk(KERN_INFO "myfs : allocate %d objects into slab: %s\n", number, SLABNAME);
	printk(KERN_INFO "myfs : object size %d bytes, full size %ld bytes\n", size, (long)size * number);
	printk(KERN_INFO "myfs : constructor called %d times\n", sco);
	printk(KERN_INFO "myfs : filesystem registered");

	return 0;
}

static void __exit myfs_exit(void)
{
	int ret = unregister_filesystem(&myfs_type); // Системный вызов.
	int i;

	if (ret != 0)
		printk(KERN_ERR "myfs : can't unregister filesystem!\n");

	for (i = 0; i < number; i++)
		kmem_cache_free(cache, line[i]);
	kmem_cache_destroy(cache);
	kfree(line);

	printk(KERN_INFO "myfs : unregistered %d", sco);
}

module_init(myfs_init);
module_exit(myfs_exit);