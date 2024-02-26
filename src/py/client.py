import threading
import readline
import socket
import time
import sys

DEBUG_BIT = False 
args = sys.argv

if DEBUG_BIT:
    import pdb
    pdb.set_trace()

def listening_worker(conn):
    print("listener working...")
    while True:
        try:
            svr_str = conn.recv(1024).decode('utf-8')
        except:
            print("connection error in listening thread")
            break
        # print("listener still working check"
        if svr_str != None:
            print(svr_str)
            if "### Bye " in svr_str:
                print("listening thread terminated")
                time.sleep(1)
                conn.close()
                break

def speaking_worker(conn):
    while True:
        #user type in message
        user_input_str_bytes = sys.stdin.readline().encode("utf-8")
        try:
            #send user input string to server
            conn.send(user_input_str_bytes )
        except:
            print("connection error in speaking thread")
            break
        if user_input_str_bytes  == b"LogOut\n":
            print("speaking thread terminated")
            break
        elif user_input_str_bytes  == b"StickyTest\n":
            for i in range(10):
                tmpstr = ""
                for j in range(10):
                    tmpstr += str(j)
                try:
                    conn.send(tmpstr.encode("utf-8"))
                except:
                    print("connection error in speaking thread")
                    break

def main():
    #acquire locel ip and port
    client_ip = socket.gethostname()
    # while True:
        # if DEBUG_BIT:
        #     client_port = 58001
        #     break
        # client_port = int(input(
        #         "select a port for local socket in range[58000, 58998]:"
        #     ))
        # if client_port not in range(58000, 58999):
        #     print("please select a port in range[58000, 58998]")
        # else:
        #     break
    #user input server ip and port
    if DEBUG_BIT:
        server_ip = "localhost"
        server_port = 58000
    elif len(args) == 3:
        server_ip = str(args[1])
        server_port = int(args[2])
    else:
        server_ip = str(input("input server ip:"))
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
    listening_thread.join()
    speaking_thread.join()

if __name__ == '__main__':
    main()
    sys.exit(0)
