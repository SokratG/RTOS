# RTOS
<h2>This is project sample RTOS for microcontroller.</h2>

<div>
<p> Project consist of some modules. Which include in self implementation - cooperative round-robin scheduler, file system based on FAT and 
  simple memory manager. For test used Tiva C Launchpad board(TM4C123GXL), project have a some modules with set up pins, NVIC and Systick periodic for interrupt and internal timers of this board</p>
 </div>
 
 <div>
  <h4><i>Task scheduler</i></h4>
  <p> Cooperative round-robin scheduler used Thread Control Block for stored task and switch between self. Switching control is carried out by
  in function "SchedulerTask", he just assigns the following task to the current task pointer according to conditions. Time manager also control task, he don't get switch thread if he is still sleep. Task priority is also taken into account. For communication between thread used
   semaphore and FIFO data structured for stored data.</p>
 </div> 
 
 <div>
<h4><i>File system</i></h4>
 <p> File system include in self - directories, file allocation table and sector for stored data. Implementation split of the some abstraction layer:<br>
  1) Low-level in module "FlashProgram" operate directly address into microcontroller. <b>Address in ROM depend from type of board, read datasheet for know them.</b><br>
  2) Mid-level in module "DiskFlash" operate disk sectors.<br>
  3) High-level in module "FileSys" operate on files. Include create new file, check file size, append into file, file read, file flush, and format files.</p>
</div>

<div>
<h4><i>Memory manager</i></h4>
  <p> Memory manager just control array of bytes how heap in system. Include in self simple function for control memory such as memory init, memory allocate, memory free and check status of heap</p>
</div> 

<div>
  <h4><i>Build</i></h4>

<p> For build porject need Keil uvision version 5.0 or more. This project include set up for TM4C123GXL, but it's easy to fix, just change device in Keil and use address appropriate board.</p>
</div> 

