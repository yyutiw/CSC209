PORT=58678
CFLAGS= -DPORT=\$(PORT) -g -std=gnu99 -Wall -Werror

# friendme: friendme.o friends.o
# 	gcc $(CFLAGS) -o friendme friendme.o friends.o

# friendme.o: friendme.c friends.h
# 	gcc $(CFLAGS) -c friendme.c

# friends.o: friends.c friends.h
# 	gcc $(CFLAGS) -c friends.c

friend_server: friend_server.c friends.h
	gcc $(CFLAGS) -o friend_server friend_server.c

clean:
	rm -f friend_server