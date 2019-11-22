server_select_c: server_select_c.o socket_impl.o
	gcc -o server_select_c server_select_c.o socket_impl.o 
server_select_c.o : server_select_c.c socket_hdr.h
	gcc -c server_select_c.c 
socket_impl.o : socket_impl.c socket_hdr.h
	gcc -c socket_impl.c
clean:
	rm -f server_select_c.o socket_impl.o a.out server_select_c