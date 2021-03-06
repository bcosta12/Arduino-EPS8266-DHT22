import threading
import socket
import logging
import csv 
import io

ipServer = "10.0.0.102" #server IP
portServer = 5005 #port yo connection

class Broker():

    def __init__(self):
        logging.info('Initializing Broker')
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((ipServer, portServer))
        self.clients_list = []

    def talkToClient(self, ip):
        logging.info("Sending 'ok' to %s", ip)
        self.sock.sendto("ok", ip)

    def listen_clients(self):
        while True:
            data = open("data.csv", 'a') # 'a' to append data without overwrite
            msg, client = self.sock.recvfrom(1024)
            logging.info('Received data from client %s: %s', client, msg)
            data.write(msg+","+client[0]+"\n")
            data.close()
            t = threading.Thread(target=self.talkToClient, args=(client,))
            t.start()

if __name__ == '__main__':
    
    # Make sure all log messages show up
    logging.getLogger().setLevel(logging.DEBUG)

    b = Broker()
    b.listen_clients()