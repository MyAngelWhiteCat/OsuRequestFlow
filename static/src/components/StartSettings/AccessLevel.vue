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
                <div class="access-level__item" :class="{'active': currentLvlId == index}" v-for="(lvl, index) in levels.cards" :key="index" @click="selectLvlId(index)">
                    <div class="access-level__item-title">{{ lvl.name }}</div>
                    <div class="access-level__item-list">Кто имеет доступ: {{ lvl.text }}</div>
                    <div class="access-level__item-lvl" :style="selectColor(lvl.color).borderColor"><span :style="selectColor(lvl.color).fontColor">{{ index }}</span></div>
                </div>
            </div>
            <div class="access-level__trust">
                <div class="access-level__trust-title">
                    Только доверенные пользователи
                </div>
                <div class="access-level__trust-selected" @click="toggleDropdown">
                    <div class="access-level__trust-text">{{ currentTrustId }}</div>
                    <svg :class="{'rotated': isDropdownOpen}" xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <polyline points="6 9 12 15 18 9"></polyline>
                    </svg>
                </div>
                <transition name="dropdown">
                    <div class="access-level__trust-dropdown" v-if="isDropdownOpen">
                        <div class="dropdown-item" v-for="(item, index) in dropdownItems" :key="index" @click="selectTrustId(index)">
                            {{ item }}
                        </div>
                    </div>
                </transition>
            </div>
        </div>
    </div>
</template>

<script setup>
import { computed, ref, watch } from 'vue';
import {levels, dropdownItems} from '../../context/data';

const currentLvlId = ref('0')
const currentTrustId = ref(dropdownItems[0])
const isDropdownOpen = ref(false)

const selectLvlId = (index) => {
    currentLvlId.value = index
}

const selectTrustId = (index) => {
    currentTrustId.value = dropdownItems[index]
    isDropdownOpen.value = !isDropdownOpen.value
}

const toggleDropdown = () => {
    isDropdownOpen.value = !isDropdownOpen.value
}

const selectColor = (color) => {
    return {
        borderColor: `border-color: ${color};`,
        fontColor: `color: ${color};`
    }
};

watch(isDropdownOpen, () => {
    console.log(isDropdownOpen.value);
})


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
    grid-template-columns: repeat(2, 1fr);
    gap: 8px;
    margin-top: 12px;
}

.access-level__item {
    padding: 20px;
    background-color: #233d66;
    border: 1px solid transparent;
    border-radius: 15px;
    position: relative;
    transition: 0.3s;

    &:hover {
        background-color: #152947;
        border-color: #3862a5;
        cursor: pointer;
    }

    &.active {
        background-color: #0c3369;
        border-color: #3862a5;
    }
 
}

.access-level__item-title {
    font-size: 15px;
    font-weight: 600;
    color: #ffffff;
    margin-bottom: 8px;
}

.access-level__item-list {
    color: #3fb972;
    font-size: 14px;

    ul {
        margin: 10px 0;
        padding-left:17px;
    }
}

.access-level__item-lvl {
    position: absolute;
    top: 50%;
    transform: translateY(-50%);
    right: 25px;
    width: 33px;
    height: 33px;
    border: 4px solid transparent;
    border-radius: 50%;

    span {
        position: absolute;
        top: 3px;
        left: 12px;
        font-size: 18px;
        font-weight: 600;
    }
}

.access-level__trust {
    position: relative;
    margin-top: 20px;
}

.access-level__trust-title {
    font-size: 14px;
    font-weight: 600;
    color: #ffffff;
    margin-bottom: 12px;
}

.access-level__trust-selected {
    border: 1px solid #214981;
    background-color: #162d5c;
    border-radius: 15px;
    position: relative;
    z-index: 1;
    width: 525px;
    cursor: pointer;

    svg {
        position: absolute;
        top: 14px;
        right: 15px;
        transition: 0.3s;

        &.rotated {
            transform: rotate(180deg);
        }
    }
}

.access-level__trust-text {
    padding: 12px;
    color: #fff
}

.access-level__trust-dropdown {
    position: absolute;
    width: 525px;
    top: 62px;
    left: 0;
    background-color: #162d5c;
    padding-top: 20px;
    border-bottom-left-radius: 15px;
    border-bottom-right-radius: 15px;
    border: 1px solid #214981;
    z-index: 0; 
}

.dropdown-item {
    padding: 6px 12px;

    &:last-child {
        border-bottom-left-radius: 15px;
        border-bottom-right-radius: 15px;
    }

    &:hover {
        background-color: #152947;
        cursor: pointer;
    }
}

.dropdown-enter-active,
.dropdown-leave-active {
    transition: opacity 0.2s ease, transform 0.3s ease;
}

.dropdown-enter-from {
    opacity: 0;
    transform: translateY(-10px); 
}

.dropdown-leave-to {
    opacity: 0;
    transform: translateY(-10px); 
}
</style>