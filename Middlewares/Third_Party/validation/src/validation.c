/*
 * validation.c
 *
 *  Created on: Oct 26, 2024
 *      Author: kiro
 */



#include "validation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PREFIX "mqtt://"
#define MAX_PORT 65535

int has_mqtt_prefix(const char *url) {
    return strncmp(url, PREFIX, strlen(PREFIX)) == 0;
}

int is_valid_ip_or_domain(const char *host) {
    int dot_count = 0;
    int is_ip_format = 1;

    // Проверка, является ли это IP-адресом
    for (int i = 0; host[i] != '\0'; i++) {
        if (host[i] == '.') {
            dot_count++;
        } else if (!isdigit(host[i])) {
            is_ip_format = 0;
            break;
        }
    }

    // Если это может быть IP-адрес
    if (is_ip_format && dot_count == 3) {
        int octet;
        const char *ptr = host;

        for (int i = 0; i < 4; i++) {
            octet = atoi(ptr);
            if (octet < 0 || octet > 255) return 0;  // Неверный диапазон октета

            // Переход к следующему октету
            ptr = strchr(ptr, '.');
            if (ptr != NULL) ptr++;
        }
        return 1;  // Строка - корректный IP-адрес
    }

    // Проверка на корректность доменного имени (буквы, цифры, точки и дефисы)
    dot_count = 0;
    for (int i = 0; host[i] != '\0'; i++) {
        if (host[i] == '.') {
            dot_count++;
        } else if (!isalnum(host[i]) && host[i] != '-') {
            return 0;  // Некорректный символ для доменного имени
        }
    }
    return dot_count > 0; // Доменное имя должно содержать хотя бы одну точку
}

int is_valid_port(const char *port_str) {
    int port = atoi(port_str);
    if (port < 1 || port > MAX_PORT) {
        return 0; // Порт вне диапазона
    }
    // Проверить, что порт состоит только из цифр
    while (*port_str) {
        if (!isdigit(*port_str)) {
            return 0;
        }
        port_str++;
    }
    return 1;
}

int is_valid_mqtt_url(const char *url) {
    if (!has_mqtt_prefix(url)) {
        printf("Error: URL does not start with 'mqtt://'\n");
        return -1;
    }

    // Найти первый символ после "mqtt://"
    const char *host_start = url + strlen(PREFIX);
    const char *colon = strchr(host_start, ':');
    if (!colon) {
        printf("Error: URL does not contain a port separator ':'\n");
        return -2;
    }

    // Извлечь доменное имя или IP-адрес
    char host[256];
    strncpy(host, host_start, colon - host_start);
    host[colon - host_start] = '\0';

    if (!is_valid_ip_or_domain(host)) {
        printf("Error: Invalid domain or IP address\n");
        return -3;
    }

    // Проверить порт
    const char *port_str = colon + 1;
    if (!is_valid_port(port_str)) {
        printf("Error: Invalid port number\n");
        return -4;
    }

    return 0; // URL прошел все проверки
}
