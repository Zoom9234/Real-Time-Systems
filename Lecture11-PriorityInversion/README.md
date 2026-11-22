# Priority Inversion

## Part A

1.) With a binary semaphore, Task_H asks for the lock at about 100 ms and blocks, because Task_L holds it. At about 150 ms Task_M wakes up and preempts Task_L (priority 2 > 1). Task_L cannot finish or release the lock until Task_M is done, so Task_H waits for Task_M too.

2.) Timeline with a binary semaphore:

```
  ~0 ms         L takes the lock and works
  ~100 ms       H asks for the lock, blocks
  100-150 ms    L runs
  ~150 ms       M wakes up and preempts L
  150-1150 ms   M runs, L is ready but cannot run, H waits
  1150-1300 ms  L finishes and releases the lock
  ~1300 ms      H gets the lock (waited ~1200 ms), done at ~1350 ms
  Order of finishing: M, L, H
```

3.) With a mutex, Task_L inherits the priority of Task_H when Task_H blocks. Task_M cannot preempt Task_L anymore.

```
  ~0 ms         L takes the lock and works
  ~100 ms       H asks for the lock, blocks. L inherits priority 3
  ~150 ms       M wakes up, but cannot preempt L (3 > 2)
  100-300 ms    L runs and releases the lock, priority back to 1
  ~300 ms       H gets the lock (waited ~200 ms), done at ~350 ms
  350-1350 ms   M runs
  Order of finishing: L, H, M
```

4.) Task_L inherits a higher priority. It inherits priority 3, the priority of Task_H.

## Part B

### Step 1

```
=== Demo 3: Priority Inversion | USE_MUTEX 0 (binary semaphore, NO inheritance) ===
[    0 ms] L: taking lock
[  100 ms] L: critical section 1/3, my priority = 1
[  100 ms] H: needs the lock
[  149 ms] M: starts 1000 ms of work (does not use the lock)
[ 1150 ms] M: finished
[ 1200 ms] L: critical section 2/3, my priority = 1
[ 1300 ms] L: critical section 3/3, my priority = 1
[ 1300 ms] H: got the lock after waiting 1200 ms
[ 1350 ms] H: done
```

Task_H waited 1200 ms. Task_L always printed priority 1. Order of finishing: M, L, H.

### Step 2

```
=== Demo 3: Priority Inversion | USE_MUTEX 1 (mutex, priority inheritance) ===
[    0 ms] L: taking lock
[  100 ms] L: critical section 1/3, my priority = 1
[  100 ms] H: needs the lock
[  200 ms] L: critical section 2/3, my priority = 3
[  300 ms] L: critical section 3/3, my priority = 3
[  300 ms] H: got the lock after waiting 200 ms
[  350 ms] H: done
[  350 ms] M: starts 1000 ms of work (does not use the lock)
[ 1350 ms] M: finished
```

Task_H waited 200 ms. Task_L printed priority 1 in section 1, then 3. Order of finishing: L, H, M.

| Observation | Binary semaphore (USE_MUTEX 0) | Mutex (USE_MUTEX 1) |
|---|---|---|
| Task_H waiting time | 1200 ms | 200 ms |
| Task_L printed priority | 1 | 1 in section 1, then 3 |
| Does Task_M delay Task_L? | Yes, Task_L only finishes at 1300 ms | No, Task_M starts at 350 ms, after H |
| Does priority inheritance occur? | No | Yes, Task_L inherits priority 3 |

Comparison with my predictions: The measurements match. I predicted about 1200 ms and about 200 ms, and the order of finishing and priority 3 were also right. Small differences: Task_M started at 149 ms, not 150 ms. Section 1 of Task_L printed priority 1 with the mutex too, because it ended at 100 ms, just when Task_H asked for the lock.

### Step 3

Prediction before the change: With a binary semaphore, Task_H waits about 1000 ms longer, because it waits for Task_M. With a mutex, the waiting time stays at about 200 ms, because Task_M cannot preempt Task_L while Task_L has inherited priority 3.

```
USE_MUTEX 0:
[ 2150 ms] M: finished
[ 2300 ms] H: got the lock after waiting 2200 ms
[ 2350 ms] H: done

USE_MUTEX 1:
[  300 ms] H: got the lock after waiting 200 ms
[  350 ms] H: done
[ 2350 ms] M: finished
```

| Task_H waiting time | M = 1000 ms | M = 2000 ms |
|---|---|---|
| Binary semaphore | 1200 ms | 2200 ms |
| Mutex | 200 ms | 200 ms |

My predictions were correct. The text printed by Task_M still says "1000 ms", because I only changed the argument of busyWorkMs(). The real workload was 2000 ms.

Why the two configurations differ: With a binary semaphore, Task_H has to wait until Task_M has finished, so a longer workload means a longer wait. With a mutex, Task_L inherits priority 3 and Task_M cannot preempt it, so Task_H gets the lock at 300 ms whatever Task_M does. Task_M just runs later.

## Part C

1.) Task_H waits longer with a binary semaphore because Task_M preempts Task_L while Task_L holds the lock. Task_L cannot run, so it cannot release the lock. The most important task is delayed by a less important task. This is priority inversion.

2.) When Task_H blocks on the mutex, FreeRTOS sees that the holder, Task_L, has a lower priority. Task_L inherits priority 3 from Task_H while it holds the mutex. My output shows this. After releasing the mutex, Task_L goes back to priority 1.

3.) Task_M has priority 2. While Task_L has inherited priority 3, it has a higher priority than Task_M, so the scheduler keeps running Task_L. Task_M cannot preempt it and has to wait.

4.) A mutex supports priority inheritance and a binary semaphore does not. Here, Task_H waits 200 ms with the mutex, but 1200 ms with the binary semaphore.

## What I learned

I learned that a medium priority task can indirectly delay a high priority task, even if it does not use the lock. With a mutex, Task_L inherits the priority of Task_H, so the waiting time stayed at 200 ms, even when I doubled the workload of Task_M. For a lock shared between tasks with different priorities, I should use a mutex.
