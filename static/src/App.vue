<script setup>
import Directory from './components/Directory.vue';
import JoinChannel from './components/JoinChannel.vue';


</script>

<template>
  <div class="container">
    <div class="head">
      <h1>OsuRequestFlow</h1>
      <p style="text-align: center;">Начнем с пошаговой настройки программы</p>
    </div>
    
    <!-- <JoinChannel/>  -->
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
