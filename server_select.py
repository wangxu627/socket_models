import select
import socket
import sys
import base64
from queue import Queue
import queue

PORT = 20000

def run_server():
    server = socket.socket()
    server.setblocking(False)
    server.bind(('0.0.0.0', PORT))
    server.listen(5)

    """
    第一个是要检查要读取的传入数据的对象列表，
    第二个包含在缓冲区中有空间时将接收传出数据的对象，
    第三个包含可能有错误的对象（通常是错误的组合输入和输出通道对象）。
    """
    input_fds = [server] # 从中读取数据
    output_fds = [] # 将数据发送出去
    message_queue = {} # 消息队列

    while input_fds:
        readable, writable, exceptional = select.select(input_fds, output_fds, input_fds)
        for s in readable:
            if s is server: # 第一种情况，表示有新的连接进来
                connection,add = s.accept()
                connection.setblocking(0)
                input_fds.append(connection)
                message_queue[connection] = Queue()

            else: # 第2种情况就是，客户端把数据发送了过来
                data = s.recv(64)
                if data:
                    print(data)
                    r = base64.b64encode(data)
                    r = base64.b64decode(r)
                    message_queue[s].put(r) 
                    if s not in output_fds:
                        output_fds.append(s)
                else: # 第3种情况 就是客户端断开了连接，这个时候recv()数据就是空，这个时候就可以跟客户端断开连接
                    if s in output_fds:
                        output_fds.remove(s)
                    input_fds.remove(s) # 在input列表中也删除掉
                    # 关闭连接，在队列中也删除
                    s.close()
                    del message_queue[s]

        for s in writable:
            try:
                next_msg = message_queue[s].get_nowait()
            except queue.Empty as e:
                output_fds.remove(s)
            else:
                s.send(next_msg)

        for s in exceptional:
            input_fds.remove(s)
            for s in output_fds:
                output_fds.remove(s)
            s.close()
            del message_queue[s]

if __name__ == "__main__":
    run_server()