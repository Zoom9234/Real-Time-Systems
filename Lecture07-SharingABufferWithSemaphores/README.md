# 1.) What does emptySlots count?

It counts free buffer places that producers can reserve. It starts at five because the buffer is empty. A producer takes one token before writing, a
consumer returns one after reading. When no free place is available, a producer blocks at xSemaphoreTake(emptySlots, portMAX_DELAY).

# 2.) What does filledSlots count?

It counts ready values that consumers can claim. It starts at zero because nothing has been written. A producer gives one token after writing a consumer takes one before reading. 
When no value is available, a consumer blocks at xSemaphoreTake(filledSlots, portMAX_DELAY). 

# 3.) What does the mutex protect?

The mutex protects the shared buffer, the head and tail positions, and the counters counts, invalidValues, and consumed. Only one task can access this data at a time. The monitor also takes the mutex before checking the final counts. Consumer messages are printed while the mutex is held.
A producer first reserves a free slot and then takes the mutex. It writes its ID into the buffer and moves head to the next position. After releasing the mutex, it signals that a value is ready. A consumer first claims a ready value and then takes the mutex. It reads the value, moves tail, and updates the counters. At the end, it releases the mutex and signals that a slot is free.

# 4.) What did you change in the second test, and what happened?

For the second test, I changed CONSUMER_DELAY_MS from 400 ms to 1000 ms and left the producer delay at 100 ms. I kept the consumer delay outside getValue(), so the consumer releases the mutex before pausing.
Before running the test, I expected the buffer to fill up because the consumers were now slower. The producers would then have to wait at xSemaphoreTake(emptySlots, portMAX_DELAY) until a consumer freed a slot. I expected the same 15 values to be read, just over a longer time.
When I checked the results, both versions finished with "PASS: total and all frequencies correct". Each ID appeared exactly three times, and there were no invalid values. I could see that the reads took longer, but the output did not directly show producers waiting.

AI-Use: I used AI to guide me through the solving of the task by repeatedly giving it my thoughts and solutions and asking it if I am correct. Then I used it to polish my vocabulary so I could hand in a clean repo.
