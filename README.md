[Общая архитектура модулей системы](Design.drawio.png)

# TODO
- HTTP-loopback server
- GUI

# In work
- Downloader
- Core

# Pre tested
- IRC Client
- HTTP Client
 
# Ready


# Tested
- FileManager

# Prod ver


---

## Current Progress

75-80%

---

## Current Task 


---


## Task Board

### Main
- Разбить все файлы на заголовки и реализации

### IRC Client
 - [Always] Рефакторинг. Анализ багов. Написать тесты.
 - Сделать авторизацию OAuth 2.0 flow взаимодействующую с GUI
 - полный переход Connetion на ассинхронные операции.
 - сделать ассинхронное подключение
 - Проверка соединения.
 - Тестирование.
 - вынести реализацию Connection за его объявление
 - сделать ассинхронную запись
 - доделать парсинг бейджей пользователя. 


### Downloader
- Сделать загрузчик через file body
 
- Обработать ошибку записи на диск. 
- Обработка редиректов
- ConnectionPool, RequestBuilder, ResponseParser, ErrorHandler


### HTTP-loopback server
- Проектирование класса сервера, продумыть вспомогательные классы.
  - Какие нужны эндпоинты?
- Эндпоинты:
- Основа:
   - ник для подключения к чату.
   - авторизация через твич
   - путь к директории osu
   - скачивать сразу / просить подтверждение / игнорировать - все чаттеры / белый список / бан лист
   - включение / отключение
 
### FileManager
- Реализовать отдачу итераторов для произвольного удаления. Придумать как их хранить чтобы взаимодействовать с фронтом.
- создать защиту для дубликатов.

### Core
- Сериализация настроек всей системы

### GUI
- Андрюха разберется.
- Придумать еще эндпоинты. 
- Меню настроек
- Панель со списком всех карт пользователя.
   - название мапсета, список сложностей.
   - миниатюра бэкграуда (локальный файл)
   - mp3 / wav плеер локального файла
   - кпонка выбора карты
   - отдельная вкладка с уже выбранными картами
   - кнопка создать коллекцию
- меню коллекций
   - каждую коллекцию можно развернуть в аналогичную панель.
   - кнопка поделиться коллекцией


Эндпоинты: 

- **POST** `/api/setting/load`
  - Body: `empty`
  - Response: `ok` / `error`
  - Description: Загрузить настройки системы

- **POST** `/api/settings/save`
  - Body: `empty`
  - Response: `ok` / `error`
  - Description: Сохранить текущие настройки

### Downloader Settings
- **PUT** `/api/downloader/settings/max_file_size`
  - Body: `{"FileSize": number}` (unsigned int)
  - Response: `ok` / `error`
  - Description: Установить максимальный размер файла для загрузки

- **PUT** `/api/downloader/settings/folder`
  - Body: `{"Path": "string"}` (путь к папке)
  - Response: `ok` / `error` (проверка существования пути)
  - Description: Установить папку для загрузок

- **PUT** `/api/downloader/settings/resourse_and_prefix`
  - Body: `{"Resourse": "string", "Prefix": "string"}`
  - Response: `ok` / `error` (ресурс недоступен | не найден)
  - Description: Настроить ресурс и префикс для загрузки

### User Lists Management
- **PUT** `/api/white_list/users`
  - Body: `{"UserName": "string"}`
  - Response: `ok` / `error` (пользователь уже в белом списке | пользователь в черном списке, требуется подтверждение)
  - Description: Добавить пользователя в белый список

- **DELETE** `/api/white_list/users`
  - Body: `{"UserName": "string"}`
  - Response: `ok` / `error` (пользователь не найден в белом списке)
  - Description: Удалить пользователя из белого списка

- **PUT** `/api/black_list/users`
  - Body: `{"UserName": "string"}`
  - Response: `ok` / `error` (пользователь уже в черном списке | пользователь в белом списке, требуется подтверждение)
  - Description: Добавить пользователя в черный список

- **DELETE** `/api/black_list/users`
  - Body: `{"UserName": "string"}`
  - Response: `ok` / `error` (пользователь не найден в черном списке)
  - Description: Удалить пользователя из черного списка

### Validator Settings
- **PUT** `/api/validator/settings/role_filter_level`
  - Body: `{"RoleFilterLevel": number}` (0-4)
  - EMPTY = 0,
  - SUBSCRIBER = 1,
  - VIP = 2,
  - MODERATOR = 3,
  - BROADCASTER = 4
  - Response: `ok` / `error` (уровень вне диапазона)
  - Description: Установить уровень фильтрации по ролям

- **PUT** `/api/validator/settings/set_whitelist_only`
  - Body: `{"IsOn": boolean}`
  - Response: `ok` / `error`
  - Description: Включить/выключить режим только белого списка

### IRC Client Settings
- **PUT** `/api/irc_client/settings/set_reconnect_timeout`
  - Body: `{"ReconnectTimeout": number}` (секунды)
  - Response: `ok` / `error`
  - Description: Установить таймаут переподключения IRC клиента

- **POST** `/api/irc_client/join`
  - Body: `{"Channel": "string"}` (название канала)
  - Response: `ok` / `error` (ошибка сети | неверное имя канала)
  - Description: Подключиться к IRC каналу

- **POST** `/api/irc_client/part`
  - Body: `{"Channel": "string"}` (название канала)
  - Response: `ok` / `error` (ошибка сети | не подключен к каналу)
  - Description: Отключиться от IRC канала

- **PUT** `/api/vidget/chat/show`
  - Body: `{"Enabled": bool}` 
  - Response: `ok` / `error` (ошибка формата JSON)
  - Description: Включить сохранение истории чата. Обязательно считывать!

- **GET** `/api/vidget/chat/last_messages`
  - Body: (без тела)
  - Response: `ok` / `error` (ошибка формата JSON)
  - Description: Отключиться от IRC канала

---

## Тестирование
- Наверное Макс разберется

---


## Required
- conan2.*
- CMake

Если конан не настроен или его нет:

```
pip install conan
conan profile detect --force
```

Добавь conan в Path (Если путь скриптов pip еще туда не добавлен).

```
mkdir build && cd build
conan install .. --build=missing --output-folder=. -s build_type=Debug -s compiler.runtime=static
(при первом запуске может занять больше часа. Сборка библиотек Boost)
cmake .. --preset conan-default
cmake --build .
(Может быть очень много ворнингов. Это ок)
```
  
  
