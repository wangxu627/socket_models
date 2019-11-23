server_epoll: socket_impl.o base64.o
	gcc -o server_epoll server_epoll.c socket_impl.o base64.o
server_poll: socket_impl.o base64.o
	gcc -o server_poll server_poll.c socket_impl.o base64.o
server_select: socket_impl.o base64.o
	gcc -o server_select server_select.c socket_impl.o base64.o
socket_impl.o : socket_impl.c socket_hdr.h
	gcc -c socket_impl.c
base64.o : base64.c
	gcc -c base64.c
clean:
	rm -f *.o a.out server_select server_poll server_epoll