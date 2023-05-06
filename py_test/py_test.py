import socket, pickle

from enum import Enum

HOST = "127.0.0.1"
PORT = 33003

from enum import Enum
import math

class COMMAND_TYPES(Enum):
    upload = 1
    set_operation = 2
    start = 3
    status = 4

DATA_TYPES = {
        type(int()) : (1,4), # its int32
        type(float()) : (2,8) # its float64
    }

class OPTION_TYPES(Enum):
    eoo = 0
    operation = 1
    min = 2
    max = 3

class OPERATION_TYPES(Enum):
    randomize = 1
    fill_diag_with_row_sums = 2

class request:
    command = COMMAND_TYPES.set_operation
    options = {
            OPTION_TYPES.operation:OPERATION_TYPES.randomize.value,
            OPTION_TYPES.min:0,
            OPTION_TYPES.max:10
            }
    data = [[1,2],[3,4]]

    def __bytes__(self):
        dtype = DATA_TYPES[type(self.data[0][0])]
        rows = len(self.data)
        columns = len(self.data[0])
        
        options_bytes = bytearray(b'')
        for key, value in self.options.items():
            option_length = 0
            if key == OPTION_TYPES.eoo:
                continue
            elif key == OPTION_TYPES.operation:
                option_length = 1
            elif key == OPTION_TYPES.min or key == OPTION_TYPES.max:
                option_length = dtype[1]
        
            option_in_bytes = key.value.to_bytes(1,'big') + option_length.to_bytes(1,'big') + value.to_bytes(option_length,'big')
            options_bytes.extend(option_in_bytes)
        options_bytes.extend(OPTION_TYPES.eoo.value.to_bytes(1,'big') + int(0).to_bytes(1,'big'))
        
        data_bytes = bytearray(b'')
        for row in self.data:
            for cell in row:
                data_bytes.extend(cell.to_bytes(dtype[1],'big'))
        
        offset = 4 + math.ceil(len(options_bytes)/4)
        
        message = bytearray(int(0).to_bytes(4,'big') + self.command.value.to_bytes(1,'big') + dtype[0].to_bytes(1,'big') + offset.to_bytes(2,'big') + rows.to_bytes(4,'big') + columns.to_bytes(4,'big') + options_bytes)
        while len(message)%4 != 0:
            message.append(0)
        message.extend(data_bytes)
        
        message[0:4] = bytearray(len(message).to_bytes(4,'big'))

        return bytes(message)
        

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST,PORT))
    arr = [[1,2],[3,4]]
    data_string = pickle.dumps(arr)
    print("sending data",arr,"as",data_string)
    s.send(data_string)
