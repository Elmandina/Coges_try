# Coges_try

5. Self-Assessment (5–10 lines) 
Explain: 
o Two real difficulties you encountered 
it was the first time using ipc as communication between  threads, interfacing with GUI was sth hadn't work a long time. 
o What alternative IPC mechanism you considered and why you rejected it 
used posix+semafor as think that would be better to be considered for sake of racing to access the memory
o A design decision you changed during development
on this work i have programmed in c not in c++ since more familiar with c

# Task2
Quality Description (1 paragraph) 
Explain the concrete practices you follow to keep embedded software safe under concurrency, 
resource constraints, and partial hardware failures.
being honest I don't have experience on programming in real embedded software and can't say a lot, but regarding 
resource constraints I know that in parallel computing is important and the division of tasks through threads is crucial,</br>
also is important to handle the read/write accesses to the threads since can happen that u try to readbefore the thread had finished the processing of data and u read wrong information, same for the write cuz u can overwrite the data and the computation is all wrong and is important to inline as much as possible the code instead of doing high-level function calls since they occupy memory.
btw in this task I havent implemented two cases the race and the crush

# Task3
Short justification (5–7 lines) 
Explain why your patch is safer for maintaining legacy embedded codebases. 
I reported the changes in the file task3.c and also wrote comments on the lines i made change, firstly the processing of bytes</br> has to be correct so fixed it is important from the logical computation point of view, and when iterating over the bytes surely would get out of bound cuz it was wrong so it would give an error,  and syntax error when deleting the buffer, regarding startReading and stopReading I put a comment but i am not sure being honest.

# Task 4: Self-Assessment & Process Reflection  
Please write a self-assessment paragraph with your reflection, it must include: 
1. What part of the test required the most debugging, and why 
despite task3, I would say task2 
2. One mistake you made and how you fixed it 
lately have mostly worked on python and was painful going back to ; after each declaration
3. A suggestion to improve the test 
add more bugs in the part of handling the communication ar access of the threads(ps:maybe u did and I wasn't able to understand it)
4. What tool/technique helped you most while developing
chatgpt(ps:I know maybe shouldn't say this but prefer saying things as they are)
