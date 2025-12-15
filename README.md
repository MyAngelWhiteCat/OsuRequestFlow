# OsuRequestFlow

Альфа-версия уже доступна: https://github.com/MyAngelWhiteCat/OsuRequestFlow/releases/tag/RequestFlowAlpha

## Что это такое?

Представь: ты стримишь Osu!, зрители кидают реквесты и карты начинают устанавливаться сами собой. Именно для этого я и делаю OsuRequestFlow. Пока что название временное, но суть уже понятна.

Чат-бот запускается прямо на твоём компьютере, так что зрители могут влиять на происходящее в реальном времени. Основные команды и механики для подписок, донатов и канальных баллов уже в работе. Но главное, что уже работает — это мгновенная установка карт по запросам.

## Как это работает?

**Основная фишка** — зритель кидает ссылку, и карта начинает качаться. Не бойся, это безопастно, все предусмотренно. Без лишних кликов, без танцев с бубном. Особенно круто для тех, у кого нет Osu!Supporter — система ставит карты сама, пока ты играешь.

**Настроек минимум:**
- Укажи папку с картами Osu! (При выборе через проводник учти, что при нажатии кнопки он откроется, но может не развернуться, не нужно кликать много раз, а то устанешь потом закрывать)
- Выбери, чьи реквесты принимать: всех, подписчиков, VIP, модераторов или только свои. А так же есть белый и черный список :)

Перед установкой система проверяет — если карта уже есть, она не будет качаться заново. Потом можно будет это отключить, если захочется.

## Самая сложная часть — серверы

Программа использует публичные бесплатные серверы (огромное спасибо ребятам из [catboy.best](https://catboy.best/)!), но некоторые провайдеры этот трафик не очень любят.

Чтобы не гадать, будет работать или нет, на главном экране есть специальный индикатор:

![Индикатор статуса серверов](look_here.png)

**Три возможных состояния:**
1. ✅ **Всё отлично** — все сервера доступны, система выбрала самый быстрый
2. ⚠️ **Работает, но неидеально** — хотя бы один сервер отвечает, можно пользоваться
3. ❌ **Недоступно** — ни один сервер не отвечает, загрузка карт не сработает

Если ты из России — будь готов к последнему варианту. Многие сервера у нас не очень доступны. Система пока проверяет osu.direct, европейский сервер catboy.best и пару зеркал.
Буквально у меня всё работало только при подключении к вайфаю пивнухи на первом этаже. Интересно, зачем им гигабитный канал с полным набором прокси.
## Про производительность

Я старался сделать так, чтобы бот не мешал играть. В среднем он занимает:
- **Память:** ~15 МБ (попозже оптимизирую)
- **Процессор:** 1-5% в моменты активности

## Нужна твоя помощь!

Если что-то не работает, странно себя ведёт или просто есть идеи — пиши! Прикрепляй логи, всего их 3 вида LogRequest (Записи запросов к серверам и их ответы). GeneralLogs (Все мелочи что происходят внутри) и (Показывает немного больше информации о серверах) — так я смогу быстрее разобраться и починить.
Не переживай. Логи не содержат НИКАКОЙ информации о тебе и твоем ПК.

---

## For English speakers. 

# OsuRequestFlow

Alpha version is available: https://github.com/MyAngelWhiteCat/OsuRequestFlow/releases/tag/RequestFlowAlpha

## What is this?

Imagine this: you're streaming Osu!, viewers send requests, and maps start installing automatically. That's exactly what I'm building with OsuRequestFlow. The name is temporary for now, but the concept is clear.

The chat bot runs directly on your computer, allowing viewers to influence what happens in real time. Core commands and mechanics for subscriptions, donations, and channel points are in development. But the main feature that's already working is instant map installation from requests.

## How does it work?

**The main feature** — a viewer sends a link, and the map starts downloading. Don't worry, it's safe, everything is accounted for. No extra clicks, no complicated steps. Especially great for those without Osu!Supporter — the system installs maps automatically while you play.

**Minimal setup:**
- Specify your Osu! songs folder (When selecting through the explorer, keep in mind that when you click the button, it will open, but it may not expand, you do not need to click many times, otherwise you will get tired of closing it later.)
- Choose whose requests to accept: everyone, subscribers, VIPs, moderators, or only yourself. Plus, there are whitelist and blacklist options :)

Before installing, the system checks if the map already exists — it won't re-download duplicates. You'll be able to disable this check later if you want.

## The tricky part — servers

The program uses public free servers (huge thanks to the folks at [catboy.best](https://catboy.best/)!), but some internet providers aren't too fond of this traffic.

To avoid guessing whether it'll work or not, there's a special indicator on the main screen:

![Server status indicator](look_here.png)

**Three possible states:**
1. ✅ **Everything's great** — all servers available, system picked the fastest one
2. ⚠️ **Works, but not ideal** — at least one server responds, you can use it
3. ❌ **Unavailable** — no servers respond, map downloads won't work

Literally, everything worked for me only when connected to the Wi-Fi of the pub on the ground floor. I wonder why they need a gigabit channel with a full set of proxies.

## About performance

I tried to make sure the bot doesn't interfere with gameplay. On average, it uses:
- **Memory:** ~15 MB (I'll optimize it later)
- **CPU:** 1-5% during active periods

## I need your help!

If something isn't working, behaving strangely, or if you just have ideas — write! Attach logs — there are 3 types: LogRequest (server requests and responses), GeneralLogs (all internal events), and additional server info logs — this helps me figure things out and fix them faster.
Don't worry. Logs contain NO information about you or your PC.

---

## For Developers


[Общая архитектура модулей системы (В какой то момент потеряла актуальность, но для понимания еще пойдет)](Design.drawio.png)

В коде применяется большое количество не очень хороших практик, присутсвуют довольно жесткие архитектурные ошибки и моменты, когда швабры держат полок, так что разобраться в нем будет тяжеловато.
The code uses a large number of not very good practices, there are quite severe architectural errors and moments when mops hold shelves, so it will be difficult to figure it out.
# TODO


# In work
- Downloader
- HTTP Client

# Pre tested

 
# Ready
- HTTP-loopback server
- GUI
- IRC Client
- Core

# Tested
- FileManager

# Prod ver


---

## Current Progress

85-90%

---

## Current Task 

Выделить 2 типа сервров - HTTP и WebSocket
Добавить мониторинг получения заголовков и добавить таймаут отсутсвия данных заголовков
Разобраться в причинах блокировок трафика к серверам.

---


## Task Board

### Main
- Разбить все файлы на заголовки и реализации

### IRC Client
 - Сделать авторизацию OAuth 2.0 flow взаимодействующую с GUI

### ChatBot
-

### Connection
- полный переход на ассинхронные операции.
- сделать ассинхронную запись
- асинхронное подключение


### Downloader

### HTTP Client
- Обработка редиректов
 
### FileManager

### HTTP-loopback server
-

### RequestValidator
- 

### FileRequestHandler

### ApiRequestHandler
   - авторизация через твич
   - скачивать сразу / просить подтверждение / игнорировать - все чаттеры / белый список / бан лист
   - включение / отключение

### Collection Manager
- Создать парсер карт, проходимся по всем картам, записываем большой JSON array JSON dict и отправляем на фронт
- На фронте галочками выбираем нужные карты и нажимаем добавить в коллекцию либо создать новую.
- Карты создаем считая MD5 самостоятельно. В osu.db не лезем.
- запись в collection.db SQLite

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

- **POST** `/api/settings/load`
  - Body: `empty`
  - Response: `ok` / `error`
  - Description: Загрузить настройки системы

- **POST** `/api/settings/save`
  - Body: `empty`
  - Response: `ok` / `error`
  - Description: Сохранить текущие настройки

### Downloader Settings

- **POST** `/api/downloader/remove_dublicates`
  - Body: `empty` 
  - Response: `ok` / `error`
  - Description: Удалить дубликаты из корневой папки
  
- **PUT** `/api/downloader/settings/max_file_size`
  - Body: `{"FileSize": number}` (unsigned int)
  - Response: `ok` / `error`
  - Description: Установить максимальный размер файла для загрузки
  
- **GET** `/api/downloader/settings/max_file_size` 
  - Body: `{"FileSize": number}` (unsigned int)
  - Response: `ok` / `error`
  - Description: Узнать максимальный размер файла для загрузки

- **PUT** `/api/downloader/settings/folder`
  - Body: `{"Path": "string"}` (путь к папке)
  - Response: `ok` / `error` (проверка существования пути)
  - Description: Установить папку для загрузок

- **POST** `/api/downloader/settings/folder`
  - Body: `empty`
  - Response: `ok` / `error` (проверка существования пути)
  - Description: Установить папку для загрузок через проводник
  
- **GET** `/api/downloader/settings/folder`
  - Body: `{"Path": "string"}` (путь к папке)
  - Response: `ok` / `error` (200 ok {"Path": "not setted"} если путь не задан)
  - Description: Узнать папку для загрузок

- **PUT** `/api/downloader/settings/resource_and_prefix`
  - Body: `{"Resource": "string", "Prefix": "string"}`
  - Response: `ok` / `error` (ресурс недоступен | не найден)
  - Description: Настроить ресурс и префикс для загрузки

- **GET** `/api/downloader/settings/resource_and_prefix`
  - Body: `{"Resource": "string", "Prefix": "string"}`
  - Response: `ok` / `error`
  - Description: Посмотреть ресурс и префикс для загрузки

- **POST** `/api/downloader/mesure_speed`
  - Body: `empty`
  - Response: `ok` / `error` (ошибка на стороне сервера)
  - Description: Запустить замер скорости загрузки к добавленным серверам.

- **GET** `/api/downloader/dl_server_status`
  - Body: `{"Status": status}` (string) {Processing, Available, Unavailable}
  - Response: `ok` / `error`
  - Description: Посмотреть есть ли хоть один достпуный ресурс

- WIP **GET** `/api/downloader/base_servers`
  - Body: `[{"Resource": "string", "Prefix": "string"}]`
  - Response: `ok` / `error`
  - Description: Посмотреть список базовых серверов

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

- **GET** `/api/validator/settings/role_filter_level`
  - Body: `{"RoleFilterLevel": number}` (0-4)
  - EMPTY = 0,
  - SUBSCRIBER = 1,
  - VIP = 2,
  - MODERATOR = 3,
  - BROADCASTER = 4
  - Response: `ok` / `error` (уровень вне диапазона)
  - Description: Узнать уровень фильтрации по ролям

- **PUT** `/api/validator/settings/whitelist_only`
  - Body: `{"Enabled": boolean}`
  - Response: `ok` / `error`
  - Description: Включить/выключить режим только белого списка

- **GET** `/api/validator/settings/whitelist_only`
  - Body: `{"Enabled": boolean}`
  - Response: `ok` / `error`
  - Description: Узнать статус режима только белого списка

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
- лепим наугад

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
conan install .. --build=missing --output-folder=. -s build_type=Release -s compiler.runtime=static
(при первом запуске может занять больше часа. Сборка библиотек Boost)
cmake .. --preset conan-default
cmake --build . --config Release
(Может быть очень много ворнингов. Это ок)
```
  
  
