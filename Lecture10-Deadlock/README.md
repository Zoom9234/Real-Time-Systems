# Assignment 1 Deadlock

## Part A

1.) Task_Sensor can hold i2cMutex while Task_Logger holds uartMutex. Each then waits for the other mutex, so neither can finish.

2.) Task_Display only uses i2cMutex. It cannot create a circular wait on its own, but it can get stuck waiting behind the deadlocked tasks.

3.) Using the same lock order removes circular wait. The other Coffman conditions can still apply.

## Part B

### Step 1

With DEMO_MODE 0, Task_1 holds A and waits for B, while Task_2 holds B and waits for A. Both workers stay blocked forever. In the HEARTBEAT, the rounds values for Task_1 and Task_2 stay at zero and only t keeps increasing.

Some lines from my Serial Monitor
```
    === Demo 1: Deadlock | DEMO_MODE 0 ===
    [Task_1] taking A ...
    [Task_1] has A, now needs B
    [Task_2] taking B ...
    [Task_2] has B, now needs A
    [HEARTBEAT] t=2 s | rounds: Task_1=0 Task_2=0
    [HEARTBEAT] t=3 s | rounds: Task_1=0 Task_2=0
    [HEARTBEAT] t=4 s | rounds: Task_1=0 Task_2=0
```
### Step 2

I kept DEMO_MODE at 0 and changed plan2 directly so Task_2 also takes A then B. Both round counters increased again. A task waiting for A does not hold B, so the other task can finish and release both mutexes.

Some lines from my Serial Monitor
```
    === Demo 1: Deadlock | DEMO_MODE 0 ===
    [Task_1] taking A ...
    [Task_1] has A, now needs B
    [Task_2] taking A ...
    [Task_1] has BOTH locks -> work done (round 1)
    [Task_2] has A, now needs B
    [Task_2] has BOTH locks -> work done (round 1)
    [HEARTBEAT] t=2 s | rounds: Task_1=4 Task_2=3
```
### Step 3

For this step I set DEMO_MODE to 1 and temporarily changed plan2 back to B then A, so the deadlock situation from Step 1 could happen again. If a task cannot get the second mutex within 500 ms, it prints TIMEOUT and releases the mutex it already holds. It must release the first mutex, because otherwise it would keep holding something the other task needs and the deadlock would stay. The random pause of 100 to 399 ms before the next try makes it less likely that both tasks collide again at the same time.

Some lines from my Serial Monitor

```
=== Demo 1: Deadlock | DEMO_MODE 1 ===
[Task_1] has A, now needs B
[Task_2] has B, now needs A
[Task_1] TIMEOUT waiting for B -> release A, back off
[Task_2] has BOTH locks -> work done (round 1)
[HEARTBEAT] t=2 s | rounds: Task_1=0 Task_2=1
[Task_2] TIMEOUT waiting for A -> release B, back off
[Task_1] has BOTH locks -> work done (round 1)
[HEARTBEAT] t=3 s | rounds: Task_1=2 Task_2=1
```

After the timeout, the other task could take both mutexes and finish its work. The system kept running, but a collision still happened from time to time, so timeouts are a recovery method and not a real fix like the same lock order in Step 2.

## What I learned

I learned that the order of taking mutexes matters. Using the same order stopped the tasks from blocking each other forever.
