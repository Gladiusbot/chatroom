import threading
import readline
import socket
import time
import sys

def listening_worker(conn):
    print("listener working...")
    while True:
        try:
            svr_str = conn.recv(1024)
        except:
            print("connection error in listening thread")
            sys.exit(0)
        # print("listener still working check"
        if svr_str != None:
            print(svr_str)
            if "Chatroom\n### Bye " in svr_str:
                time.sleep(1)
                conn.close()
                sys.exit(0)   

def speaking_worker(conn):
    while True:
        #user type in message
        user_input_str = sys.stdin.readline()
        try:
            #send user input string to server
            conn.send(user_input_str)
        except:
            print("connection error in speaking thread")
            sys.exit(0)
        if user_input_str == "LogOut\n":
            print("speaking thread terminated")
            sys.exit(0)
            break

def main():
    #acquire locel ip and port
    client_ip = socket.gethostname()
    while True:
            client_port = int(input(
                "select a port for local socket in range[58000, 58998]:"
            ))
            if client_port not in range(58000, 58999):
                print("please select a port in range[58000, 58998]")
            else:
                break
    #user input server ip and port
    server_ip = input("input server ip:")
    server_port = int(input("input port:"))
    print("connecting to \"" + server_ip + ":" + str(server_port) + "\"")
    #create new socket connection
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #connect client socket to server socket
    try:
        client_socket.connect((server_ip, server_port))
    except:
        print("Can't connect to server")
        sys.exit()
    #create a listening thread for bradcasting
    listening_thread = threading.Thread(
        target = listening_worker,
        args = (client_socket, )
        )
    speaking_thread = threading.Thread(
        target = speaking_worker,
        args = (client_socket, )
        )
    listening_thread.start()
    speaking_thread.start()

if __name__ == '__main__':
    main()
    sys.exit(0)