# FriendNet
FriendNet is a simple Facebook clone run on the command line that allows users to see each other, make friends, post to and view each others' profiles. It was my final CSC209 assignment, and I earned a final score of 100%.

In the project, I was responsible for completing the ```friend_server.c``` and ```friends.c``` files.

## Running the Program
From the command line, in the a4 directory, run ```make``` to compile the project files for you. Then, run ```./friend_server``` to start the server.

From there, you can connect to your server via the ```netcat -C [host] [port]``` command, where you will then be prompted to enter a username.

Once successfully signed on, you have access to all of the available commands:
| Command | Description |
| ------- | ----------- |
| list_users | Display the names of all users that have ever connected to the server |
| make_friends username | Add a friend with name username. Friendship is symmetrical |
| post target msg ... | Post a message msg from the current client to the target specified |
| quit | Disconnect from the server |
