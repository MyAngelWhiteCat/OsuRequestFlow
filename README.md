# OsuRequestFlow

Альфа-версия уже доступна: https://github.com/MyAngelWhiteCat/OsuRequestFlow/releases/tag/RequestFlowAlpha

## Что это такое?

Бот для Osu!, который автоматически скачивает карты, ссылки на которые отправляются в чат твича. Буквально. Ведешь стрим, зритель кидает ссылку — и через момент карта уже в папке с песнями, зашел - вышел в любую карту или нажал f5, и реквест уже выбран и готов к игре. Всё локально на твоём компьютере, никаких сторонних сервисов.

Пока что всё довольно просто — иногда даже работает. Есть фильтры по ролям (можно принимать запросы только от сабов, випов или вообще всех), белые и чёрные списки

## Почему всё так странно устроено?

**Локальный сервер и интерфейс в браузере** — не паранойя, а удобство. Всем понятно, как открыть браузер, не нужно таскать лишние окна по экрану, а ещё браузер сам переводит страницы. Изначально я хотел делать это с другом, который пишет фронтенд, но пока что весь интерфейс нарисовал дипсик. Так что могут быть косяки — как только друг освободится, всё станет красиво и надёжно!

**А зачем вообще это выпускать, если сыро?** — чтобы собрать данные. Нужны логи от людей с разными провайдерами и интернет-подключениями, чтобы понять, как сделать сервис стабильным для всех, даже без использования запрета

**В итоге что будет?** Изначально я хотел сделать это для себя и друзей, чтобы быстрее обмениваться картами на стримах. В планах — собрать в одну программу всё, что нужно стримеру Osu!: чат в OBS, PP-каунтер и прочее, сделав это интуитивно понятным, с минимальной нагрузкой на железо и полностью локально. Есть в этом что-то уютное.

## Как это работает?

**Основа** — зритель кидает ссылку, карта качается. Всё безопасно, проверено. Никаких танцев с бубном. Особенно кайфово для тех, у кого нет Osu!Supporter — система ставит карты сама, пока ты играешь.

**Настроек минимум:**
- Укажи папку с картами Osu! (Кнопка "Выбрать папку" откроет проводник — может не сразу развернуться. Не кликай по ней сто раз, а то устанешь закрывать окна)
- Выбери, чьи реквесты принимать: всех, подписчиков, VIP, модераторов или только свои. Есть и белый, и чёрный список :)

В настройках директории есть возможность удалить дубликаты карт. Это нужно как раз для того случая, когда проверка на наличие карты перед загрузкой отключена, чтобы в конце стрима можно было в 1 клик удалить все лишнее. 

Перед установкой система не проверяет, есть ли карта уже в папке — если да, то качает повторно. Потом можно будет это отключить. Добавлю отдельную настройку

## Головная боль — серверы

Программа использует публичные бесплатные серверы (огромное спасибо ребятам из [catboy.best](https://catboy.best/)!), но в текущих реалиях провайдеры этот трафик не очень любят.  

Чтобы не гадать, будет работать или нет, на главном экране есть индикатор:

![Индикатор статуса серверов](look_here.png)

**Три возможных состояния:**
1. ✅ **Всё отлично** — все сервера доступны, система выбрала самый быстрый
2. ⚠️ **Работает, но неидеально** — хотя бы один сервер отвечает, можно пользоваться
3. ❌ **Недоступно** — ни один сервер не отвечает, загрузка карт не сработает

Если ты из России — будь готов к последнему варианту. Многие сервера у нас не очень доступны. Система пока проверяет osu.direct, европейский сервер catboy.best и пару зеркал.

Буквально у меня всё работало только при подключении к вайфаю пивнухи на первом этаже. Интересно, зачем им гигабитный канал с полным набором прокси.

Как же быть? 

Используй запрет https://github.com/Flowseal/zapret-discord-youtube

В list-general добавь:
osu.direct
catboy.best
nerinyan.moe

После чего попробуй разные конфигурации. У меня работает на ALT1

![3 простых шага](zapret_guide.png)

После чего проблем с загрузкой не будет. 

## Про производительность

Я старался сделать так, чтобы бот не мешал играть. В среднем он занимает:
- **Память:** ~15 МБ (попозже оптимизирую)
- **Процессор:** 1-5% в моменты активности

## Нужна твоя помощь!

Если что-то не работает, странно себя ведёт или просто есть идеи — пиши! Прикрепляй логи — всего их 3 вида:
- `LogRequest.txt` — запросы к серверам и их ответы
- `GeneralLogs.txt` — все мелочи, что происходят внутри
- `LogAccessTestResult` с дополнительной информацией о серверах

Так я смогу быстрее разобраться и починить. Не переживай — логи не содержат **НИКАКОЙ** информации о тебе и твоём ПК.

---

## For English speakers

# OsuRequestFlow

Alpha version is available: https://github.com/MyAngelWhiteCat/OsuRequestFlow/releases/tag/RequestFlowAlpha

## What is this?

An Osu! bot that takes map requests from chat and installs them automatically. Literally. You stream, a viewer drops a link — and in a couple of seconds, the map is in your songs folder. Everything runs locally on your computer, no third-party services.

it's pretty simple for now — but it already works. There are role-based filters (you can accept requests only from subs, VIPs, or everyone), whitelist and blacklist, and most importantly — no manual work.

## Why is everything set up so oddly?

**Local server and browser interface** — not paranoia, just convenience. Everyone knows how to open a browser, no need to drag extra windows around, and the browser can translate pages itself. Initially, I wanted to do this with a friend who writes frontend, but for now, DeepSeek drew the whole interface, haha. So there might be some quirks — once my friend is free, everything will become pretty and reliable!

**Why release it if it's raw?** — to collect data. I need logs from people with different providers and internet connections to understand how to make the service stable for everyone.

**What's the end goal?** Initially, I wanted this for myself and friends to exchange maps faster on streams. The plan is to combine everything an Osu! streamer needs into one program: chat in OBS, PP counter, etc., making it intuitive, with minimal load on hardware, and completely local. There's something cozy about that.

## How does it work?

**The core** — a viewer sends a link, the map downloads. Everything is safe, tested. No complicated steps. Especially cool for those without Osu!Supporter — the system installs maps automatically while you play.

**Minimal setup:**
- Specify your Osu! songs folder (The "Select folder" button will open the explorer — it might not immediately expand to full screen. Don't click it a hundred times, or you'll get tired of closing windows)
- Choose whose requests to accept: everyone, subscribers, VIPs, moderators, or only yourself. There's also a whitelist and blacklist :)

Before installing, the system checks if the map is already in the folder — if yes, it won't download again. You'll be able to disable this later if you want.

## The tricky part — servers

The program uses public free servers (huge thanks to the folks at [catboy.best](https://catboy.best/)!), but some internet providers aren't too fond of this traffic.

To avoid guessing whether it'll work or not, there's an indicator on the main screen:

![Server status indicator](look_here.png)

**Three possible states:**
1. ✅ **Everything's great** — all servers available, system picked the fastest one
2. ⚠️ **Works, but not ideal** — at least one server responds, you can use it
3. ❌ **Unavailable** — no servers respond, map downloads won't work

Literally, everything worked for me only when connected to the Wi-Fi of the pub on the ground floor. I wonder why they need a gigabit channel with a full set of proxies if they sell beer.

## About performance

I tried to make sure the bot doesn't interfere with gameplay. On average, it uses:
- **Memory:** ~15 MB (I'll optimize it later)
- **CPU:** 1-5% during active periods

## I need your help!

If something isn't working, behaving strangely, or if you just have ideas — write! Attach logs — there are 3 types:
- `LogRequest.txt` — server requests and responses
- `GeneralLogs.txt` — all internal events
- `LogAccessTestResult` Another file with additional server info

This helps me figure things out and fix them faster. Don't worry — logs contain **NO** information about you or your PC.

---

*P.S. Если что, пиши в Issues или кидай логи в Discussions. Без них сложно понять, что ломается.*

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
  
  
