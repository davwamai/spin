import socket
import numpy as np
import scipy.signal as signal
import matplotlib.pyplot as plt

TCP_ADDR = '192.168.1.100'
TCP_PORT = 1001
N = 4194304

def main():

    buffer = []
    buf_count = 0
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((TCP_ADDR, TCP_PORT))
        print("Connected to server.")
        while True and len(buffer) == 0:
            raw_data = sock.recv(2 * N, socket.MSG_WAITALL)
            if len(raw_data) < 2 * N:
                print("Received less data than expected.")
                break
            
            data = np.frombuffer(raw_data, dtype=np.int16)
            buffer.append(data)
            print(f"buf: {buf_count} | data: {data[:100]}")
            buf_count += 1
        sock.close()
        print("Data received.")
        plt.plot(-data[:20000])
        plt.show()

if __name__ == "__main__":
    main()
