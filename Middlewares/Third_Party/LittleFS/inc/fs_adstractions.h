#ifndef LFS_MG_FS_H
#define LFS_MG_FS_H

#include <time.h>
#include "lfs.h"


// Определите ваш объект LittleFS и конфигурацию
//lfs_t lfs;

// Структура для хранения данных файловой системы
//struct lfs_file_data {
//    lfs_file_t file;
//};

// Экземпляр файловой системы mg_fs
//extern struct mg_fs my_fs;

// Декларации функций
void set_lfs_config(lfs_t * lfs_conf);

//struct mg_fs get_mongoose_fs_drv(void);


// Получение информации о файле
int mg_fs_lfs_status(const char *path, size_t *size, time_t *mtime);

// Список файлов в директории
void mg_fs_lfs_list(const char *path, void (*fn)(const char *, void *), void *userdata);

// Открытие файла
void *mg_fs_lfs_open(const char *path, int flags);

// Закрытие файла
void mg_fs_lfs_close(void *fd);

// Чтение из файла
size_t mg_fs_lfs_read(void *fd, void *buf, size_t len);

// Запись в файл
size_t mg_fs_lfs_write(void *fd, const void *buf, size_t len);

// Установка позиции в файле
size_t mg_fs_lfs_seek(void *fd, size_t offset);

// Переименование файла
bool mg_fs_lfs_rename(const char *from, const char *to);

// Удаление файла
bool mg_fs_lfs_remove(const char *path);

// Создание директории
bool mg_fs_lfs_mkdir(const char *path);

// Экземпляр файловой системы mg_fs
//struct mg_fs my_fs;
uint32_t get_fs_size(void);

// Erase all flash
void lfs_erase_flash(void);

void set_lfs_config(lfs_t * lfs_conf);

int lfs_fs_ll_init(void (*lock)(void *), void (*unlock)(void *));

int remove_recursive(const char *path);

#endif // LFS_MG_FS_H
