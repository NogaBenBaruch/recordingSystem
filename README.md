# recordingSystem
Principles used: Parallelism, Synchronization, Communication-UDP

I created a recording system that will listen and receive packages from two channels simultaneously, using UDP.
Channel 1 is for transmitting data packages, and channel 2 is for transmitting header packages.(each header package is associated with several data packages)
The system should receive the packages, and keep them in the file in a logical way: each header package with its data packages.

The file "main.cpp" creates the listener threads, and the file "client.cpp" creates the sender threads. 
One should run "main.cpp" first, and then "client.cpp".
The system creates and writes into the file "output.txt".

