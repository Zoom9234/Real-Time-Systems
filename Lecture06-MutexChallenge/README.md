## 1) Why can passing the address of a local variable to a task be unsafe?

Because delay_arg only lives on the stack of setup(). When I pass &delay_arg
to the task, I'm basically handing out a pointer to something temporary. As soon
as setup() finishes, that stack space gets reused, so the task could end up
reading something else instead of my value. I actually ran into this, without the mutex
I got a crash, which makes sense once you realize the pointer was dangling.


## 2) When does blinkLED copy the parameter?

Right at the start of the task, before it does anything else with it. It takes the
mutex, copies the value into its own local int num, and then gives the mutex
back. So the copy happens while the mutex is held.


## 3) How does the mutex make setup() wait?

The task keeps the mutex while it's copying and only releases it when it's done.
Meanwhile setup() tries to take the mutex too, and xSemaphoreTake just blocks
there until it's free again. So setup() is stuck until the copy is finished,
and only then does it print Done!. That's exactly why in the Serial output
Done! always shows up after Received: and never before.


## 4) Why is this described as a demonstration hack? What would be a better design in a real application?

Honestly it feels a bit like abusing the mutex. A mutex is meant to protect shared
data that two tasks keep using, but here I'm only using it as a one-time "wait
until I copied it" signal at startup. On top of that the ordering also depends on
a short vTaskDelay so the task grabs the mutex first, which is kind of fragile.
For a real project I'd rather pass the value by value instead of the address of a short-lived local, or
use a queue, which copies the data cleanly and handles the synchronization.
