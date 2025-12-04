<template>
    <div class="join-channel">
        <div class="join-channel__menu">
            <div class="join-channel__title">
                Подключиться к IRC канала 
            </div>
            <input type="text" @input="onTypeChannel" placeholder="Введите имя канала" class="join-channel__input" :class="{'join-channel__input--error': inputError}"/>
            <span class="join-channel__load">{{ isConnected }}</span>
            <div class="join-channel__buttons">
                <button class="join-channel__button" @click="handleClickConnect" :disabled="!input">
                    Подключиться
                </button>
                <button class="join-channel__button join-channel__button--disconnect" @click="handleClickDisconnect" :disabled="!input">
                    Отключиться
                </button>
            </div>
        </div> 
        <div class="join-channel__lists">
            <div>Список подключенных IRC каналов</div>
            <ul v-for="curr in currentConnetions" :key="curr.id" class="join-channel__ul">
                <li>{{ curr }}
                    <svg @click="removeFromList(curr)" width="24" height="24" viewBox="0 0 24 24" fill="none">
                        <path d="M18 6L6 18M6 6L18 18" stroke="currentColor" stroke-width="2" stroke-linecap="round"></path>
                    </svg>
                </li>
            </ul>
        </div>   
    </div>
</template>

<script setup>
import { ref, watch } from 'vue';
import axios from 'axios';

const input = ref('');
const isConnected = ref('')
const currentConnetions = ref([]);
const inputError = ref(false);

const handleClickConnect = () => {
    isConnected.value = "Подключение..."
    setTimeout(() => {
        reqJoin()
    }, 1000)
}

const handleClickDisconnect = () => {
    isConnected.value = "Отключение..."
    setTimeout(() => {
        reqPart()
    }, 1000)
}

const removeFromList = (name) => {
    setTimeout(() => {
        reqPart()
        currentConnetions.value = currentConnetions.value.filter(channel => channel!== name)
    }, 1000)
}

const reqJoin = async () => {
    try {
        const response = await axios.post('http://localhost:8181/api/irc_client/join', {
            Channel: input.value.trim()
        })
        
        currentConnetions.value.push(input.value.trim())

        console.log(response);

        isConnected.value = "Подключено"
    } catch (error) {
        console.log(error);

        isConnected.value = "Ошибка подключения"
    }
}

const reqPart = async () => {
    try {
        const response = await axios.post('http://localhost:8181/api/irc_client/part', {
            Channel: input.value.trim()
        })
        console.log(response);

        currentConnetions.value = currentConnetions.value.filter(channel => channel!== input.value.trim())

        isConnected.value = "Отключено"

    } catch (error) {
        console.log(error);
        
        isConnected.value = "Ошибка отключения"
    }
}

const onTypeChannel = (event) => {
    input.value = event.target.value;
}

watch(currentConnetions, () => {
    console.log(currentConnetions.value)
})

</script>

<style lang="scss">
.join-channel {
    padding-top: 80px;
    display: flex;
}

.join-channel__title {
    font-size: 24px;
    margin-bottom: 20px;
}

.join-channel__input {
    min-width: 350px;
    padding: 10px;
    border-radius: 5px;
    border: 1px solid #ccc;

    &--error {
        border-color: #ff0000;

        &::placeholder {
            color: #ff0000;
        }
    }
}

.join-channel__buttons {
    display: flex;
    margin-top: 20px;
    gap: 25px;
}

.join-channel__button {
    background-color: #1863d4;
    color: #fff;
    transition: .3s;

    &:hover {
        background-color: #0349b1;
    }

    &--disconnect {
        background-color: #ff0000;

        &:hover {
            background-color: #c30000;
        }
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
</style>