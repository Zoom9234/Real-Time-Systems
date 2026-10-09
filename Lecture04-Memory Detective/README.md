# Memory Detective Assignment

## 1) Recording of values with 4096 Task Stack Size:

- Free heap before Task A: 359112 bytes
- Free heap after Task: 354384 bytes
- Free heap after Task: 349656 bytes

## 2) Recording of values with 8192 Task Stack Size:

- Free heap before Task A: 359112 bytes
- Free heap after Task: 350032 bytes
- Free heap after Task: 341208 bytes

## 3) Questions a-d: 

### a) 
Yes, the free heap before the tasks was identical in both runs. After creating the tasks, it was clearly lower with the 8192-byte stack than with the 4096-byte stack.

### b)
It decreased. With a 4096-byte stack, each task reduced the free heap by 4728 bytes. With an 8192-byte stack, Task A reduced it by 9080 bytes and Task B by 8824 bytes. After both tasks, the heap was 8448 bytes smaller than in Run 1, which is about two times 4096 bytes. The small remaining part of each drop is overhead for the task control block and heap management.

### c)
Every task runs independently and can be interrupted by the scheduler at any time. It therefore needs its own stack to store local variables, function parameters, return addresses of function calls, and the CPU registers that are saved during a context switch. Without a separate stack per task, the tasks would overwrite each other’s data when the scheduler switches between them.

### d)
When I create a task with xTaskCreate(), FreeRTOS allocates memory from the heap. It reserves the stack in the size I pass as a parameter, and it reserves a task control block (TCB) that stores the task’s name, priority, state, and stack pointer. This memory stays reserved as long as the task exists, even if the task is only sleeping in vTaskDelay(). That is why the free heap drops with every task and drops more with a larger stack. On a microcontroller with limited RAM, the stack size has to be chosen carefully. If it is too small, the task may crash with a stack overflow, if it is too large, RAM is wasted that other parts of the program could use.
