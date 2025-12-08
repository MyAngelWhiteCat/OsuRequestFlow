<template>
    <div class="directory">
        <div class="directory__notice">
            <div class="directory__notice-title">
                <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="18" height="18" viewBox="0 0 512 512">
                    <path d="M272 0l-48 48 48 48-112 128h-112l88 88-136 180.308v19.692h19.692l180.308-136 88 88v-112l128-112 48 48 48-48-240-240zM224 272l-32-32 112-112 32 32-112 112z"/>
                </svg>
                Инструкция
            </div>
            <div>
                <p><b>Путь для сохранения osu! карт</b></p>
                <ul>
                    <li>Укажите полный путь к папке Songs в вашей установке osu!</li>
                    <li>Пример: D:\osu!\Songs или C:\Games\osu!\Songs</li>
                    <li>Папка должна существовать и иметь права на запись</li>
                </ul>
            </div>
        </div>
        <div class="directory__settings">
            <div class="directory__settings-title">
                <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="20" height="20" viewBox="0 0 512 512">
                    <path d="M416 480l96-256h-416l-96 256zM64 192l-64 288v-416h144l64 64h208v64z"/>
                </svg>
                Настройка директории
            </div>
            <p>Укажите путь для сохранения загруженных карт</p>
            <form method="post">
                <label class="input-file">
                    <span class="input-file-btn" @click="choosePath">Выберите файл</span>
                    <span class="input-file-text" type="text">{{ selectedPath }}</span>
                    <input type="file" name="file" ref="pathInput" @change="onPathChange">        
                </label>
            </form>
        </div>
    </div>
</template>

<script setup>
import { ref, watch } from 'vue';

const pathInput = ref(null)
const selectedPath = ref('')

const choosePath = () => {
    pathInput.value.click();
}

const onPathChange = (e) => {
    const file = e.target.files[0]
    if (file) {
        selectedPath.value = file.path
        console.log(selectedPath.value)
    }
}

watch(pathInput, () => {
    console.log(pathInput.value);
})


</script>

<style lang="scss">
.directory {
    padding-top: 50px;
}

.directory__notice {
    padding: 25px;
    background: radial-gradient(circle at center, #1f305a 0%, #16274e 100%);
    border: 2px solid #16274e;
    border-radius: 15px;
    color: rgb(189, 189, 189);
}

.directory__notice-title {
    font-size: 18px;
    color: #93a7ff;
    font-weight: 600;

    svg
    {
        margin-right: 10px;

        path
        {
            fill: #a7a7a7;
        }
    }
}

.directory__settings {
    padding: 25px;
    background: radial-gradient(circle at center, #2f3f58 0%, #1E293B 100%);
    border: 2px solid #36455e;
    border-radius: 15px;
    color: rgb(189, 189, 189);
    margin-top: 25px;
}

.directory__settings-title {
    font-size: 20px;
    color: #f1f1f1;
    font-weight: 600;

    svg
    {
        margin-right: 10px;

        path
        {
            fill: #a7a7a7;
        }
    }
}

.input-file {
	position: relative;
	display: inline-flex;
    width: 100%;
    flex-direction: row;
}
.input-file-text {
	padding: 0 10px;
	line-height: 40px;
	height: 40px;
	display: inline-flex;
	width: 100%;
	border: 1px solid #ddd;
    border-left: none;
    border-top-right-radius: 6px;
    border-bottom-right-radius: 6px;
}

.input-file-btn {
    position: relative;
	display: inline-block;
	cursor: pointer;
	font-size: 14px;
	color: rgb(255 255 255);
	text-align: center;
	background-color: #2267c2;
	line-height: 22px;
	padding: 10px 20px;
	transition: background-color 0.2s;
    margin-left: auto;
    flex-shrink: 0;
    border-top-left-radius: 6px;
    border-bottom-left-radius: 6px;
}
.input-file input[type=file] {
	position: absolute;
	z-index: -1;
	opacity: 0;
	display: block;
	width: 0;
	height: 0;
}
</style>