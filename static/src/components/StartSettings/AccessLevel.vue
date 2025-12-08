<template>
    <div class="access-level">
        <div class="directory__notice">
            <div class="directory__notice-title">
                <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="18" height="18" viewBox="0 0 512 512">
                    <path d="M272 0l-48 48 48 48-112 128h-112l88 88-136 180.308v19.692h19.692l180.308-136 88 88v-112l128-112 48 48 48-48-240-240zM224 272l-32-32 112-112 32 32-112 112z"/>
                </svg>
                Управление доступом       
            </div>
            <div>
                <p><strong>Выберите уровень доступа:</strong></p>
                <ul>
                    <li><strong>0 - Все пользователи:</strong> Любой пользователь чата</li>
                    <li><strong>1 - Подписчики и выше:</strong> Подписчики, VIP, модераторы, стример</li>
                    <li><strong>2 - VIP и выше:</strong> VIP, модераторы, стример (рекомендуется)</li>
                    <li><strong>3 - Модераторы и стример:</strong> Для ограниченного доступа</li>
                    <li><strong>4 - Только стример:</strong> Для тестирования или приватных стримов</li>
                </ul>
                <p><strong>Режим "Только доверенные":</strong></p>
                <ul>
                    <li>Выключено: доступ по ролям (рекомендуется)</li>
                    <li>Включено: только пользователи из списка Allowed Users</li>
                </ul>
            </div>
        </div>
        <div class="access-level__menu">
            <div class="access-level__menu-title">Уровень доступа</div>
            <div class="access-level__menu-subtitle">Выберите минимальную роль для доступа к командам</div>
            <div class="access-level__choose">Выберите уровень:</div>
            <div class="access-level__items">
                <div class="access-level__item" v-for="lvl in levels.cards" :key="lvl.id">
                    <div class="access-level__item-title">{{ lvl.name }}</div>
                </div>
            </div>
        </div>
    </div>
</template>

<script setup>

const levels = {
    cards: {
        '0': {
            name: 'Все пользователи',
            text: 'Любой пользователь чата',
            color: '#ffffff'
        },
        '1': {
            name: 'Подписчики и выше',
            text: 'Подписчики, VIP, модераторы, стример',
            color: '#F4F4F4'
        },
        '2': {
            name: 'VIP и выше',
            text: 'VIP, модераторы, стример',
            color: '#F4F4F4'
        },
        '3': {
            name: 'Модераторы и стример',
            text: 'Только модераторы и стримеры',
            color: '#F4F4F4'
        },
        '4': {
            name: 'Только стример',
            text: 'Только стример',
            color: '#F4F4F4'
        }
    }
}

</script>

<style lang="scss">
.access-level {
    padding-top: 80px;
}

.access-level__menu {
    margin-top: 25px;
    border: 1px solid #36455e;
    background: radial-gradient(circle at center, #2f3f58 0%, #1E293B 100%);
    border-radius: 15px;
    padding: 20px;
}

.access-level__menu-title {
    font-size: 18px;
    font-weight: 600;
    color: #ffffff;
    margin-bottom: 12px;
}

.access-level__menu-subtitle {
    margin-bottom: 20px;
}

.access-level__choose {
    font-size: 14px;
    font-weight: 600;
    color: #ffffff;
}

.access-level__items {
    display: grid;
    grid-template-columns: repeat(5, 1fr);
    gap: 8px;
    margin-top: 12px;
}

.access-level__item {
    padding: 10px;
    background-color: #233d66;
    border-radius: 15px;
}

.access-level__item-title {
    font-size: 14px;
    font-weight: 600;
    color: #ffffff;
    margin-bottom: 8px;
}
</style>