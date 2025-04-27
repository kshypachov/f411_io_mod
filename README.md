# F411 IO Mod Project

![Build Status](https://github.com/kshypachov/f411_io_mod/actions/workflows/c-cpp.yml/badge.svg)

**F411 IO Mod** — прошивка для STM32F411CEUX, обеспечивающая управление цифровыми входами/выходами, сетевые функции и MQTT-коммуникацию с интеграцией в Home Assistant. Устройство поддерживает обновление прошивки и веб-интерфейса "по воздуху", работает под FreeRTOS и предоставляет REST API.

## Основные возможности

- **MQTT**: Публикация/приём данных, совместимость с Home Assistant.
- **REST API**: Управление устройством, конфигурация, обновления, логи.
- **FreeRTOS**: Задачи для IO, сети, настроек и отображения.
- **Файловая система**: LittleFS для хранения настроек и веб-интерфейса.

## Сборка устройства

1. Скачайте из [Releases](https://github.com/kshypachov/f411_io_mod/releases):
   - **`f411_io_mod_bootloader.hex`** — бутлоадер.
   - **`f411_io_mod.hex`** — основная прошивка.

2. Прошивка бутлоадера:
   - Зажмите кнопку **Boot**, подключите к USB.
   - Прошейте **STM32CubeProgrammer**-ом **`f411_io_mod_bootloader.hex`**.

3. Загрузка прошивки:
   - Прошейте **`f411_io_mod.hex`** также через **STM32CubeProgrammer**.

4. Перезагрузите устройство (**Reset**):
   - Светодиод мигнёт 4 раза, версия отобразится на OLED.
   - После подключения Ethernet — IP будет показан на экране.

## Загрузка веб-интерфейса

1. Скачайте:
   - **`web_interface.zip`** — файлы интерфейса.
   - **`web_interface_uploader_win.exe`** или **`web_interface_uploader_mac.app.zip`**.

2. Разархивируйте **web_interface.zip**.

3. Запустите загрузчик, введите IP устройства (`http://<ip>`) и авторизуйтесь (**admin/admin**).

4. Укажите путь к распакованным файлам и нажмите **Загрузить**.

## Быстрый старт

- Проект собирается в **STM32CubeIDE**.
- Конфигурация MQTT доступна через веб-интерфейс.
- Подключите устройство к сети для управления.

## Web API

Полное описание API доступно здесь: **[API.md](API.md)**

---

## Задачи FreeRTOS

- **Ethernet Task**: Сеть и MQTT.
- **IO Task**: Чтение/управление входами и выходами.
- **Settings Task**: Работа с настройками.
- **Display Task**: Отображение данных (опционально).

---

## Лицензия

MIT License — см. [LICENSE](LICENSE).

## Участие

PR и предложения приветствуются. Обратная связь через [Issues](https://github.com/kshypachov/f411_io_mod/issues).

## Благодарности

- [STM32 HAL и CMSIS](https://www.st.com/)
- [FreeRTOS](https://www.freertos.org/)
- [Mongoose](https://www.cesanta.com/)
- [Home Assistant](https://www.home-assistant.io/)