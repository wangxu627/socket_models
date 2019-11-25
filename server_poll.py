import select
import socket
import sys
import queue
from queue import Queue


def run_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setblocking(False)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = ('0.0.0.0', 20002)
    server.bind(server_address)
    server.listen(5)
    message_queues = {}

    # 常用的标识  代表你想检查的事件类型
    READ_ONLY = select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR
    READ_WRITE = READ_ONLY | select.POLLOUT

    TIMEOUT = 1000
    poller = select.poll() # 创建一个poll对象，该对象可以注册或注销文件描述符
    poller.register(server,READ_ONLY)
    fd_to_socket = { server.fileno(): server,}

    while True:
        events = poller.poll(TIMEOUT) # 如果timeout为None，将会阻塞，知道有事件发生
        if not events:
            print("poll超时无活动连接，重新轮询......")
            continue
        print("有" , len(events), "个新事件，开始处理......")

        for fd, flag in events:
            s = fd_to_socket[fd]
            if flag & (select.POLLIN | select.POLLPRI): # 有数据可以读取
                if s is server: # 表示有新的连接
                    # 可以读取数据
                    connection, client_address = s.accept()
                    print(sys.stderr, '新的连接来自:', client_address)
                    connection.setblocking(False)
                    fd_to_socket[connection.fileno()] = connection # 往fd字典中添加一个新的 文件描述符
                    poller.register(connection, READ_ONLY)
                    message_queues[connection] = Queue() # 为了防止等待客户端发来数据期间发生阻塞，分配一个队列用于保存数据
                else: # 表示客户端传来了消息
                    data = s.recv(1024)
                    if data: # 表明数据接受成功
                        print(sys.stderr, '接受数据 "%s" 来自 %s' % (data, s.getpeername()))
                        message_queues[s].put(data)
                        # 修改一个已经存在的fd，修改事件为写。这里表示服务器向客户端要发送数据
                        poller.modify(s, READ_WRITE)
                    else:
                        # 如果没有接受到数据，表示要断开连接
                        print(sys.stderr, '关闭', client_address, '并未读取到数据')
                        # 停止监听连接上的输入
                        poller.unregister(s)
                        s.close()

                        # 将此链接从队列中删除
                        del message_queues[s]

            elif flag & select.POLLHUP:
                print(sys.stderr, '关闭', client_address, '收到HUP后')
                poller.unregister(s)
                s.close()

            elif flag & select.POLLOUT:
                try:
                    next_msg = message_queues[s].get_nowait()
                except queue.Empty:
                    print(sys.stderr, '队列', s.getpeername(), '为空')
                    poller.modify(s, READ_ONLY)
                else:
                    print(sys.stderr, '发送 "%s" 到 %s' % (next_msg, s.getpeername()))
                    s.send(next_msg)

            elif flag & select.POLLERR:
                print(sys.stderr, '异常信息:', s.getpeername())
                poller.unregister(s)
                s.close()
                del message_queues[s]

if __name__ == "__main__":
    run_server()

