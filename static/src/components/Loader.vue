<template>
    <div class="loader">
        <div class="notice">
            <div class="notice-title">
                <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="18" height="18" viewBox="0 0 512 512">
                    <path d="M272 0l-48 48 48 48-112 128h-112l88 88-136 180.308v19.692h19.692l180.308-136 88 88v-112l128-112 48 48 48-48-240-240zM224 272l-32-32 112-112 32 32-112 112z"/>
                </svg>
                Рекомендуемые значения
            </div>
            <div>
                <p><b>Максимальный размер файла: 350 MB (диапазон: 50-1024 MB)</b></p>
                <p><b>Источник загрузки (выберите один):</b></p>
                <ul>
                    <li>Сравнить и выбрать лучший: https://catboy.best/compare</li>
                    <li>catboy.best: Вебсайт: "catboy.best", Префикс пути: "/d/"</li>
                    <li>osu.direct: Вебсайт: "osu.direct", Префикс пути: "/api/d/"</li>
                    <li>nerinyan.moe: Вебсайт: "nerinyan.moe", Префикс пути: "/d/"</li>
                </ul>
            </div>
        </div>
    </div>
</template>

<script setup>

</script>

<style lang="scss">
.loader {
    padding-top: 50px;
}


</style>