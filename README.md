# Real-Time-Systems
This is a repo for my Real Time Systems Lecture at Savonia 

1) Hardware and Software up to date
2) Arduino setup done
3) Sanity Check of Serial Motor sreenshot:
   <img width="1507" height="814" alt="image" src="https://github.com/user-attachments/assets/89f69fcc-4355-4530-bf3d-72b2efef91b4" />

4) The little LED is blinking on and off with a 1000ms delay after I copied the code into my Arduino IDE and clicked on the upload button.

6) 
- Task A: I deleted the variable "RGB_Brightness" because I wanted to use the number instead. For this reason I just changed the former RGB_Brightness variable at the red-position to zero and moved the green-position number up to 50 and clicked upload. For blue I changed the green-position back to zero and the blue-position to 50. For white I put every position to 50 just like the table explained it.
- Task B: Changing the delay time from 1000ms to 250ms made the blinking of the LED a lot faster, making it difficult to look at.
- Task C: For this task I changed the delay time to 500ms, then copied the code consisting of neopixelWrite(); and delay(); two more times and changed the neopixelWrite(); colour positions accordingly so that the LED would change its colour from red to green to blue to no colour with 500ms delay between each state.
- Task D: For the individual LED pattern I had the idea to use morse code. For that reason I coded the phrase "Hello There!" in morse code and let Claude AI write the individual letter-codes by keeping the colour rotation going throughout the changing dots and dashes. I used AI for that because writing it myself would have taken a lot of time, also I am not as proficient in morse code as I would like to be. The "Hello There!" will repeat infinitely.
