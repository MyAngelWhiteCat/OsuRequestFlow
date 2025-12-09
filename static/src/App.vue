<script setup>
import Directory from './components/StartSettings/Directory.vue';
import JoinChannel from './components/StartSettings/JoinChannel.vue';
import AccessLevel from './components/StartSettings/AccessLevel.vue';
import SetupWizard from './components/SetupWizard.vue';


</script>

<template>
  <div class="container">
    <div class="head">
      <h1>OsuRequestFlow</h1>
      <p style="text-align: center;">Начнем с пошаговой настройки программы</p>
    </div>
    
    <AccessLevel/>
    <JoinChannel/> 
    <Directory/>
    </div>
</template>

<style scoped>
h1 {
  text-align: center;
}

.head {
  padding-bottom: 30px;
  border-bottom: 1px solid #334155;
}

</style>
