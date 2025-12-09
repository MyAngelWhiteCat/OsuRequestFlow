<template>
    <div class="join-channel">
        <div class="notice">
            <div class="notice-title">
                <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="18" height="18" viewBox="0 0 512 512">
                    <path d="M272 0l-48 48 48 48-112 128h-112l88 88-136 180.308v19.692h19.692l180.308-136 88 88v-112l128-112 48 48 48-48-240-240zM224 272l-32-32 112-112 32 32-112 112z"/>
                </svg>
                Настройки подключения       
            </div>
            <div>
                <p><strong>Задержка переподключения:</strong> 30 секунд (10-120 сек). Используется при разрывах соединения. Не нужно перезапускать программу.<br><strong>Пример названия чата:</strong> darkyisoffline</p>
            </div>
        </div>
        <div class="join-channel__menu">
            <div class="join-channel__status">
                <span class="join-channel__status-dot" :class='connectClass'></span>
                <span class="join-channel__status-text">{{ isConnectedText }}</span>
            </div>
            <div class="join-channel__settings">
                <div class="join-channel__fields">
                    <div class="join-channel__connect">
                        <div class="join-channel__title">
                            Подключение к чату 
                        </div>
                        <div class="join-channel__subtitle">
                            Введите название канала для подключения
                        </div>
                        <input type="text" @input="onTypeChannel" placeholder="Введите имя канала" class="join-channel__input" :class="{'join-channel__input--error': inputError}"/>
                    </div>
                    <div class="join-channel__divider"></div>
                    <div class="join-channel__reconnect">
                        <div class="join-channel__title">
                            Автопередподключение 
                        </div>
                        <div class="join-channel__subtitle">
                            Задержка (секунды)
                        </div>
                        <input type="number" max="120" min="10" :value="inputTimeout" @input="onTypeTimeout" placeholder="Введите время задержки" class="join-channel__input" :class="{'join-channel__input--error': inputError}"/>
                    </div>
                </div>
                <button class="join-channel__button" @click="handleClickConnect" :disabled="!inputChannel">
                    Подключиться и продолжить
                </button>
            </div>  
        </div> 
        <!-- <div class="join-channel__lists">
            <div>Список подключенных IRC каналов</div>
            <ul v-for="curr in currentConnetions" :key="curr.id" class="join-channel__ul">
                <li>{{ curr }}
                    <svg @click="removeFromList(curr)" width="24" height="24" viewBox="0 0 24 24" fill="none">
                        <path d="M18 6L6 18M6 6L18 18" stroke="currentColor" stroke-width="2" stroke-linecap="round"></path>
                    </svg>
                </li>
            </ul>
        </div>    -->
    </div>
</template>

<script setup>
import { computed, ref, watch } from 'vue';
import axios from 'axios';

const inputChannel = ref('');
const inputTimeout = ref(30);
const isConnectedClass = ref(false)
const isConnectedText = ref('Не подключено');
const currentConnetions = ref([]);
const inputError = ref(false);
import client from '../../api/client'
import { ENDPOINTS } from '../../api/endpoints';

const handleClickConnect = () => {
    isConnectedText.value = "Идет подключение..."
    setTimeout(async () => {
        reqJoin()
        reqTimeout()
    }, 1000);
}

const connectClass = computed(() => {
    if (isConnectedText.value == 'Идет подключение...') {
        return 'join-channel__status-dot--connecting'
    } else if (isConnectedClass.value) {
        return 'join-channel__status-dot--connected'
    }

})

// const handleClickDisconnect = () => {
//     isConnected.value = "Отключение..."
//     setTimeout(() => {
//         const responseJoin = reqJoin()
//         const responseTimeout = reqTimeout()

//         if (responseJoin || responseTimeout) {
//             isConnectedText.value = "Ошибка подключения"
//         }
//     }, 1000)
// }

// const removeFromList = (name) => {
//     setTimeout(() => {
//         reqPart()
//         currentConnetions.value = currentConnetions.value.filter(channel => channel!== name)
//     }, 1000)
// }


const reqJoin = async () => {
    try {
        const response = await client.post(ENDPOINTS.IRC.JOIN_CHANNEL, {
            Channel: inputChannel.value.trim()
        })
        isConnectedClass.value = true
        isConnectedText.value = "Подключено к чату " + inputChannel.value.trim()

        currentConnetions.value.push(inputChannel.value.trim())

    } catch (error) {
        isConnectedClass.value = false
        isConnectedText.value = "Ошибка подключения к чату"
    }
}

// const reqPart = async () => {
//     try {
//         const response = await axios.post('http://localhost:8181/api/irc_client/part', {
//             Channel: inputChannel.value.trim()
//         })
//         console.log(response);

//         currentConnetions.value = currentConnetions.value.filter(channel => channel!== inputChannel.value.trim())

//         isConnected.value = "Отключено"

//     } catch (error) {
//         console.log(error);
        
//         isConnected.value = "Ошибка отключения"
//     }
// }

const reqTimeout = async () => {
    try {
        const response = await client.put(ENDPOINTS.IRC.TIMEOUT, {
            ReconnectTimeout: Number(inputTimeout.value)
        })

        console.log(response);
        
    } catch (error) {
        console.log(error);
         
    }
}

const onTypeChannel = (event) => {
    inputChannel.value = event.target.value;
}

const onTypeTimeout = (event) => {
    inputTimeout.value = event.target.value;
}

watch(currentConnetions, () => {
    console.log(currentConnetions.value)
})

</script>

<style lang="scss">
.join-channel {
    padding-top: 80px;
    // display: flex; 
}

.join-channel__title {
    font-size: 20px;
    margin-bottom: 12px;
    color: #f5f5f5;
}

.join-channel__subtitle {
    font-size: 16px;
    margin-bottom: 8px;
}

.join-channel__input {
    min-width: 470px;
    padding: 10px;
    border-radius: 5px;
    border: 1px solid #ccc;
    padding: 0 10px;
	line-height: 40px;
	height: 40px;
	display: inline-flex;
	border: 1px solid #ddd;
    background-color: transparent;
    color: #f5f5f5;
    font-size: 15px;
    transition: .4s;

    &:focus {
        box-shadow: 0 1px 10px 4px rgba(45, 91, 255, 0.4);
        outline: none;
    }
}

.join-channel__buttons {
    display: flex;
    margin-top: 20px;
    gap: 25px;
}

.join-channel__button {
    transition: .3s;
    height: 42px;
    width: 100%;
    margin-top: 20px;
    background: linear-gradient(45deg, #202abe 0%, #0960d1 100%);
    color: #ffffff;
    font-weight: 600;
    border: none;
    box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);

    &:hover { 
        box-shadow: 0 6px 20px rgba(45, 91, 255, 0.4);
    }
}

.join-channel__load {
    display: inline-block;
    margin-left: 20px;
    width: 200px;
}

.join-channel__ul li {
    display: inline-flex;
    width: 100%;
    justify-content: space-between;
    position: relative;
    align-items: center;

    &::before {
        content: "";
        width: 5px;
        height: 5px;
        position: absolute;
        top: 45%;
        left: -25px;
        border-radius: 50%;
        background-color: #3ed80f;
    }

    svg {
        width: 16px;
        height: 16px;
        cursor: pointer;
    }
}

.join-channel__status {
    border: 1px solid #36455e;
    background: radial-gradient(circle at center, #2f3f58 0%, #1E293B 100%);
    border-radius: 15px;
    padding: 10px 20px;
    margin-top: 25px;
}

.join-channel__status-dot {
    display: inline-block;
    width: 10px;
    height: 10px;
    border-radius: 50%;
    background: #af1717;
    box-shadow: 0 0 10px #af1717;
    animation: pulse 2s infinite;

    &--connecting {
        background: #b3b608;
        box-shadow: 0 0 10px #b3b608;
    }

    &--connected {
        background: #31a70d;
        box-shadow: 0 0 10px #31a70d;
    }
}

@keyframes pulse {
    0%, 100% {
        opacity: 1;
    }

    50% {
        opacity: 0.5;
    }
        }

.join-channel__fields {
    display: flex;
    flex-direction: row;
    justify-content: space-between;
    // gap: 25px; 
}

.join-channel__status-text {
    margin-left: 7px;
    font-size: 14px;
}

.join-channel__settings {
    margin-top: 25px;
    border: 1px solid #36455e;
    background: radial-gradient(circle at center, #2f3f58 0%, #1E293B 100%);
    border-radius: 15px;
    padding: 20px;
}

.join-channel__divider {
    height: auto;
    width: 1px;
    background-color: #36455e;
}

.join-channel__reconnect {
    // margin-top: 25px; 
}
</style>