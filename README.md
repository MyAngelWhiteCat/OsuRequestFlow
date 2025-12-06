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

Выделить 2 типа сервров - HTTP и WebSocket
Выбор пути открытием папки по запросу -> Добавить статический метод в FileManager fs::path GetAnyPath() -> Открывает проводник для выбора папки / файла. принимает в себя параметр для проверки (файл \ папка \ неважно)
Добавить мониторинг получения заголовков и добавить таймаут отсутсвия данных заголовков

---


## Task Board

### Main
- Разбить все файлы на заголовки и реализации

### IRC Client
 - [Always] Рефакторинг. Анализ багов. Написать тесты.
 - Сделать авторизацию OAuth 2.0 flow взаимодействующую с GUI

### ChatBot
-

### Connection
- полный переход на ассинхронные операции.
- сделать ассинхронную запись
- асинхронное подключение


### Downloader
- Обработать ошибку записи на диск. 

### HTTP Client
- Сделать найти как сделать через file body не лишая возможностей текущей реализации.
- Обработка редиректов
- ConnectionPool, RequestBuilder, ResponseParser, ErrorHandler
 
### FileManager
- Просто добавлять в историю без дальнейшей записи на диск
- Реализовать отдачу итераторов для произвольного удаления. Придумать как их хранить чтобы взаимодействовать с фронтом.
- создать защиту для дубликатов.

### HTTP-loopback server
-

### RequestValidator
- 

### FileRequestHandler

### ApiRequestHandler
- Добавить метод для отправки json (обертка над response_maker.MakeStringResponse)
- Эндпоинты:
- Основа:
   - авторизация через твич
   - скачивать сразу / просить подтверждение / игнорировать - все чаттеры / белый список / бан лист
   - включение / отключение

### Collection Manager
- Понять как создаются коллекции (считается MD5 osu файла сложности)
- Создать парсер карт, проходимся по всем картам, записываем большой JSON array JSON dict и отправляем на фронт
- На фронте галочками выбираем нужные карты и нажимаем добавить в коллекцию либо создать новую.
- Карты создаем считая MD5 самостоятельно. В osu.db не лезем.
- запись в collection.db SQLite
- разобраться в SQLite

### ChatWidget
- отправлять полученные из чата сообщения.
- Найти связь как связать получение сообщений и их передачу WebSocket серверу.

### WebSocketServer
- Написать WebSocket сервер.
- Создание сессии.

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

## Settings

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
  
- WIP **GET** `/api/downloader/settings/max_file_size` 
  - Body: `{"FileSize": number}` (unsigned int)
  - Response: `ok` / `error`
  - Description: Узнать максимальный размер файла для загрузки

- **PUT** `/api/downloader/settings/folder`
  - Body: `{"Path": "string"}` (путь к папке)
  - Response: `ok` / `error` (проверка существования пути)
  - Description: Установить папку для загрузок

- WIP **GET** `/api/downloader/settings/folder`
  - Body: `{"Path": "string"}` (путь к папке)
  - Response: `ok` / `error` 
  - Description: Узнать папку для загрузок

- **PUT** `/api/downloader/settings/resourse_and_prefix`
  - Body: `{"Resourse": "string", "Prefix": "string"}`
  - Response: `ok` / `error` (ресурс недоступен | не найден)
  - Description: Настроить ресурс и префикс для загрузки

- WIP **GET** `/api/downloader/settings/resourse_and_prefix`
  - Body: `{"Resourse": "string", "Prefix": "string"}`
  - Response: `ok` / `error`
  - Description: Посмотреть ресурс и префикс для загрузки

### User Lists Management
- **PUT** `/api/white_list/users`
  - Body: `{"UserName": "string"}`
  - Response: `ok` / `error` (пользователь уже в белом списке | пользователь в черном списке, требуется подтверждение)
  - Description: Добавить пользователя в белый список

- **GET** `/api/white_list/users`
  - Body: `[{"UserName": "string"}]`
  - Response: `ok` / `error` 
  - Description: Посмотреть белый список
  
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

- **GET** `/api/black_list/users`
  - Body: `[{"UserName": "string"}]`
  - Response: `ok` / `error` 
  - Description: Посмотреть черный список

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

- **PUT** `/api/validator/settings/whitelist_only`
  - Body: `{"IsOn": boolean}`
  - Response: `ok` / `error`
  - Description: Включить/выключить режим только белого списка

### IRC Client Settings
- **PUT** `/api/irc_client/settings/reconnect_timeout`
  - Body: `{"ReconnectTimeout": number}` (секунды)
  - Response: `ok` / `error`
  - Description: Установить таймаут переподключения IRC клиента

- **POST** `/api/irc_client/join`
  - Body: `{"Channel": "string"}` (название канала)
  - Response: `ok` / `error` (ошибка сети | неверное имя канала)
  - Description: Подключиться к IRC каналу

- **GET** `/api/irc_client/join`
  - Body: `[{"Channel": "string"}]` (название каналов)
  - Response: `ok` / `error` (ошибка сети | неверное имя канала)
  - Description: Получить список подключенных каналов

- **POST** `/api/irc_client/part`
  - Body: `{"Channel": "string"}` (название канала)
  - Response: `ok` / `error` (ошибка сети | не подключен к каналу)
  - Description: Отключиться от IRC канала

- WIP **PUT** `/api/vidget/chat/show`
  - Body: `{"Enabled": bool}` 
  - Response: `ok` / `error` (ошибка формата JSON)
  - Description: Включить / Выключить виджет чата


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
  
  
