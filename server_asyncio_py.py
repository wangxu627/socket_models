import asyncio
import base64

PORT = 20000

async def handle_read_data(reader, writer):
    loop = asyncio.get_event_loop()
    # data = await reader.read()
    while True:
        data = await reader.read(64)
        print(data)
        if(not data):
            break
        r = base64.b64encode(data)
        r = base64.b64decode(r)
        writer.write(r)
        await writer.drain()

async def create_server(address, port, loop):
    server = await asyncio.start_server(handle_read_data, address, port, loop = loop)
    return server

def run_server():
    loop = asyncio.get_event_loop()
    server = loop.run_until_complete(create_server("0.0.0.0", PORT, loop))
    host = server.sockets[0].getsockname()  
    print('Serving on {}. Hit CTRL-C to stop.'.format(host))  
    try:
        loop.run_forever()  
    except KeyboardInterrupt:  
        pass
    print('Server shutting down.')
    server.close()
    loop.run_until_complete(server.wait_closed())  
    loop.close()  

if __name__ == "__main__":
    run_server()