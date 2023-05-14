import socket
import time
import random

HOST = "127.0.0.1"
PORT = 33003

import request

def upload(socket, data, min, max):
    req = request.upload_request(data, min, max)
    socket.send(bytes(req))

def start(socket):
    req = request.start_request()
    socket.send(bytes(req))

def status(socket):
    req = request.status_request()
    socket.send(bytes(req))

def recieve(sock):
    length = sock.recv(4, socket.MSG_PEEK)
    data = sock.recv(int.from_bytes(length, byteorder ='big'))
    return request.read_request(data)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST,PORT))
    
    matrix = [[0] * 100 for _ in range(100)]
    min = 0
    max = 10



    print("Initial data:\nmin:",min,"max:",max,"\nmatrix:\n")
    for row in matrix:
        row_str = ''
        for cell in row:
            row_str += str(cell).ljust(10)[:10]
        print(row_str)
    print('')
            
    print("sending data...")
    upload(s, matrix, min, max)


    res = recieve(s)
    if res.command == request.COMMAND_TYPES.r_data_accepted:
        print("recieved.\n")
        print("starting calculations...")

        start(s)
        res = recieve(s)
        if res.command == request.COMMAND_TYPES.r_processing:
            print("started.\n")
            
            status(s)
            res = recieve(s)
            while res.command == request.COMMAND_TYPES.r_processing:
                print('.', end=" ")
                time.sleep(1)
                status(s)
                res = recieve(s)
            if res.command == request.COMMAND_TYPES.r_success:
                print("Calculations completed")
                print("Result:")
                for row in res.data:
                    row_str = ''
                    for cell in row:
                        row_str += str(cell).ljust(10)[:10]
                    print(row_str)
