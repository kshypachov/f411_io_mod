#include "fs_adstractions.h"
#include "mongoose.h"
#include "SPI_flash.h"

#ifndef FLASH_BLOCK_CYCLES
	#define FLASH_BLOCK_CYCLES 10000
#endif

// Определите ваш объект LittleFS и конфигурацию
static lfs_t lfs;
static struct lfs_config       cfg;

// Структура для хранения данных файловой системы
static struct lfs_file_data {
    lfs_file_t file;
};

//struct mg_fs *mg_fs;

void fs_proto(void){}; //function prototype

// Получение информации о файле
int mg_fs_lfs_status(const char *path, size_t *size, time_t *mtime) {
    struct lfs_info info;
    if (lfs_stat(&lfs, path, &info) == 0) {
        if (size) *size = info.size;
        if (mtime) *mtime = 0;  // LittleFS не поддерживает время модификации
        return (info.type == LFS_TYPE_DIR) ? MG_FS_DIR : MG_FS_READ;
    }
    return 0;
}

// Список файлов в директории
void mg_fs_lfs_list(const char *path, void (*fn)(const char *, void *), void *userdata) {
    struct lfs_info info;
    lfs_dir_t dir;
    if (lfs_dir_open(&lfs, &dir, path) == 0) {
        while (lfs_dir_read(&lfs, &dir, &info) > 0) {
            fn(info.name, userdata);
        }
        lfs_dir_close(&lfs, &dir);
    }
}

// Открытие файла
void *mg_fs_lfs_open(const char *path, int flags) {
    struct lfs_file_data *fd = (struct lfs_file_data *) malloc(sizeof(struct lfs_file_data));
    if (fd == NULL) return NULL;

    int lfs_flags = 0;
    if (flags & MG_FS_READ) lfs_flags |= LFS_O_RDONLY;
    if (flags & MG_FS_WRITE) lfs_flags |= LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND;

    if (lfs_file_open(&lfs, &fd->file, path, lfs_flags) < 0) {
        free(fd);
        return NULL;
    }
    return fd;
}

// Закрытие файла
void mg_fs_lfs_close(void *fd) {
    struct lfs_file_data *file = (struct lfs_file_data *) fd;
    lfs_file_close(&lfs, &file->file);
    free(file);
}

// Чтение из файла
size_t mg_fs_lfs_read(void *fd, void *buf, size_t len) {
    struct lfs_file_data *file = (struct lfs_file_data *) fd;
    int res = lfs_file_read(&lfs, &file->file, buf, len);
    return res < 0 ? 0 : res;
}

// Запись в файл
size_t mg_fs_lfs_write(void *fd, const void *buf, size_t len) {
    struct lfs_file_data *file = (struct lfs_file_data *) fd;
    int res = lfs_file_write(&lfs, &file->file, buf, len);
    return res < 0 ? 0 : res;
}

// Установка позиции в файле
size_t mg_fs_lfs_seek(void *fd, size_t offset) {
    struct lfs_file_data *file = (struct lfs_file_data *) fd;

    // Попытка установить позицию в файле
    if (lfs_file_seek(&lfs, &file->file, offset, LFS_SEEK_SET) < 0) {
        return (size_t)-1;  // Ошибка при перемещении
    }

    // Получение текущей позиции в файле
    lfs_soff_t pos = lfs_file_tell(&lfs, &file->file);

    // Проверка ошибки
     if (pos < 0) {
         return (size_t)-1;  // Ошибка при получении позиции
     }

     return (size_t)pos;  // Возвращаем текущую позицию

}

// Переименование файла
bool mg_fs_lfs_rename(const char *from, const char *to) {
    return lfs_rename(&lfs, from, to) == 0;
}

// Удаление файла
bool mg_fs_lfs_remove(const char *path) {
    return lfs_remove(&lfs, path) == 0;
}


int remove_recursive(const char *path) {
    lfs_dir_t dir;
    struct lfs_info info;

    // Попытка открыть директорию
    int res = lfs_dir_open(&lfs, &dir, path);
    if (res < 0) {
        // Если не удалось открыть директорию, пытаемся удалить как файл
        return lfs_remove(&lfs, path);
    }

    // Чтение содержимого директории
    while (true) {
        res = lfs_dir_read(&lfs, &dir, &info);
        if (res < 0) {
            lfs_dir_close(&lfs, &dir);
            return res;
        }

        // Если достигнут конец директории
        if (res == 0) {
            break;
        }

        // Пропуск текущей и родительской директории
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) {
            continue;
        }

        // Формирование полного пути для текущего элемента
        char *full_path = calloc(256, sizeof(char));
        if (full_path == NULL) {
            lfs_dir_close(&lfs, &dir);
            return LFS_ERR_NOMEM;
        }

        snprintf(full_path, 256, "%s/%s", path, info.name);

        // Закрытие текущей директории перед рекурсивным вызовом
        lfs_dir_close(&lfs, &dir);

        // Рекурсивный вызов для удаления содержимого директории
        res = remove_recursive(full_path);
        free(full_path);

        // Проверка результата рекурсивного вызова
        if (res < 0) {
            return res;
        }

        // Переоткрытие директории после рекурсивного вызова
        res = lfs_dir_open(&lfs, &dir, path);
        if (res < 0) {
            return res;
        }
    }

    // Закрытие директории
    res = lfs_dir_close(&lfs, &dir);
    if (res < 0) {
        return res;
    }

    // Удаление пустой директории
    return lfs_remove(&lfs, path);
}

// Создание директории
bool mg_fs_lfs_mkdir(const char *path) {
    return lfs_mkdir(&lfs, path) == 0;
}

// Определите вашу структуру mg_fs
//struct mg_fs mg_fs = {
//    .st = lfs_fs_stat,
//    .ls = lfs_fs_list,
//    .op = lfs_fs_open,
//    .cl = lfs_fs_close,
//    .rd = lfs_fs_read,
//    .wr = lfs_fs_write,
//    .sk = lfs_fs_seek,
//    .mv = lfs_fs_rename,
//    .rm = lfs_fs_remove,
//    .mkd = lfs_fs_mkdir,
//};


//struct mg_fs get_mongoose_fs_drv(void){
//
//	return my_fs;
//}

uint32_t get_fs_size(void){
	return lfs_fs_size(&lfs);
}

void lfs_erase_flash(void){
	sFLASH_EraseBulk();
}

void set_lfs_config(lfs_t * lfs_conf){
//	lfs = lfs_conf;
}

int block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int block_device_erase(const struct lfs_config *c, lfs_block_t block);
int block_device_sync(const struct lfs_config *c);

int lfs_fs_ll_init(void (*lock)(void *), void (*unlock)(void *)){

	struct SPI_flash_info	flash_info;

	flash_info = sFLASH_GetInfo();
	if (flash_info.flash_id == 0x0) return -1;

	uint8_t *read_buff   = calloc(1, flash_info.page_size);
	uint8_t *write_buff  = calloc(1, flash_info.page_size);
	void * lookahead_buff = calloc(1, flash_info.page_size);

	if (!(read_buff || write_buff || lookahead_buff)){
		return -2;
	}

	cfg.read_size         = flash_info.read_size;
	cfg.prog_size         = flash_info.write_size;
    cfg.block_size        = flash_info.erase_size;
    cfg.block_count       = flash_info.block_count;

    cfg.lookahead_size    = flash_info.page_size;
    cfg.cache_size        = flash_info.page_size;

    cfg.read_buffer       = read_buff;
    cfg.prog_buffer       = write_buff;
    cfg.lookahead_buffer  = lookahead_buff;

    cfg.read   = block_device_read;
    cfg.prog   = block_device_prog;
    cfg.erase  = block_device_erase;
    cfg.sync   = block_device_sync;

    cfg.lock 	= lock;
	cfg.unlock	= unlock;

#ifdef LFS_THREADSAFE
#endif

    cfg.block_cycles 	= FLASH_BLOCK_CYCLES;

    if (lfs_mount(&lfs, &cfg)< 0){
    	lfs_format(&lfs, &cfg);
    	lfs_mount(&lfs, &cfg);
    }

	return 0;
}

int block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size){
#ifndef LFS_NO_ASSERT
	assert(block < c->block_count);
#endif
	sFLASH_ReadBuffer((uint8_t*)buffer, ((block) * (c->block_size) + off), size);
	return 0;
}

int block_device_erase(const struct lfs_config *c, lfs_block_t block){
#ifndef LFS_NO_ASSERT
	assert(block < c->block_count);
#endif
	sFLASH_EraseSector(block * (c -> block_size));
	return 0;
}

int block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size){
#ifndef LFS_NO_ASSERT
	assert(block < c->block_count);
#endif
	sFLASH_WriteBuffer((uint8_t*)buffer, ((block) * (c->block_size) + off), size);
	return 0;
}

int block_device_sync(const struct lfs_config *c){
	return 0;
}


