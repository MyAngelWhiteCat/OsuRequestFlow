<template>
    <div class="setup-wizard">
        <Transition name="fade" mode="out-in">
            <Directory v-if="currentStep === 1" @next-step="goToNextStep" key="directory"/>
            <Loader v-else-if="currentStep === 2" key="loader"/>
        </Transition>

        <button v-if="currentStep > 1" @click="goToPreviousStep" class="prev-button">Назад</button>
    </div>
</template>

<script setup>
import { ref } from 'vue';
import Directory from './StartSettings/Directory.vue';
import Loader from './Loader.vue';

const currentStep = ref(1);

const goToNextStep = () => {
    currentStep.value++
    console.log(currentStep.value);
    
};

const goToPreviousStep = () => {
    currentStep.value--
}
</script>

<style lang="scss">
.setup-wizard {
    width: 100%;
    height: 100%;
}

.prev-button {
    margin-top: 30px;
    padding: 12px 24px;
    background: transparent;
    color: #e2e8f0;
    border: 2px solid #334155;
    border-radius: 12px;
    font-size: 1rem;
    font-weight: 600;
    cursor: pointer;
    transition: all 0.3s ease;

    &:hover {
        background-color: #344a70;
        border-color: #3953c7;
    }

    &:active {
        transform: translateY(1px);
    }
}

.fade-enter-active,
.fade-leave-active {
    transition: opacity 0.4s ease;
}

.fade-enter-from,
.fade-leave-to {
    opacity: 0;
}
</style>