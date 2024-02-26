#!/usr/bin/python3
# -*- coding:utf-8 -*-

import threading
import socket
import time
import sys

MAX_USER_COUNT = 5
DEBUG_BIT = True 
PDB_BIT = False 
FRIEND_REQUEST = [
    "#friendme",    #send request
    "#unfriend",    #unfriend
    "#friends"      #accept
    ]
args = sys.argv

if PDB_BIT:
    import pdb
    pdb.set_trace()

class Server:
    #multi-threading broadcast chat room
    def __init__(self):
        #init user info
        self.max_user_count = MAX_USER_COUNT
        self.user_conn_lock = threading.Lock()
        self.user_conn = {}
        self.friend_list_lock = threading.Lock()
        self.friend_list = {}
        self.friend_requests = FRIEND_REQUEST
        self.pending_request_lock = threading.Lock()
        self.pending_request = {}
        #build server socket
        while True:
            if len(args) == 2:
                self.server_port = int(args[1])
                break
            if DEBUG_BIT:
                self.server_port = 58000
                break
            self.server_port = int(input(
                "input port for server socket in range[58000, 58998]:"
                ))
            if self.server_port not in range(58000, 58999):
                print("please select a port in range[58000, 58998]")
            else:
                break
        self.server_socket = socket.socket()
        self.host = socket.gethostname()
        #bind server socket to port
        self.server_socket.bind(('', self.server_port))
        #server port listening
        self.server_socket.listen(self.max_user_count)
        self.run()

    #broadcast server
    def run(self):
        print(self.host, "running")
        accept_id = 0
        while True:
            #accept a request
            if len(self.user_conn) < self.max_user_count:
                conn, addr = self.server_socket.accept()
                accept_id += 1
                new_t = threading.Thread(
                    target = self.worker, 
                    args = (conn, addr, accept_id)
                )
                self.user_conn[accept_id] = conn
                new_t.start()
                if DEBUG_BIT:
                    print("got new thread, curr dict is:", self.user_conn)
            else:
                pass

    #worker for threading
    def worker(self, conn, addr, user_id):
        conn.send("Enter Your Name:".encode("utf-8"))
        user_nickname = str(conn.recv(1024), "utf-8").strip('\n')
        while user_nickname in self.user_conn.keys() or user_nickname.startswith("@"):
            conn.send(
                "This name is illegal or it's being used by someone else, please try another one:".encode("utf-8")
                )
            user_nickname = str(conn.recv(1024), "utf-8").strip('\n')
        #acquire locker for sync
        self.user_conn_lock.acquire()
        self.user_conn[user_nickname] = self.user_conn[user_id]
        self.user_conn.pop(user_id)
        self.user_conn_lock.release()
        user_id = user_nickname
        broadcast_str = user_nickname + " Entered Chatroom\n"
        broadcast_str = broadcast_str.encode("utf-8")
        self.broadcast(broadcast_str)
        reply_str = "Welcome to chat room, type LogOut to exit\n".encode("utf-8")
        conn.send(reply_str)
        while True:
            unicast_bit = False
            friend_req_bit = False
            #acquire user string from client socket
            client_str = str(conn.recv(1024), 'utf-8').strip('\n')
            #print(server log
            print("got message from" + str(addr) + ":" + client_str)
            #close connection of logout
            if client_str == "LogOut":
                broadcast_str = user_nickname + "Left Chatroom\n"
                broadcast_str = broadcast_str.encode("utf-8")
                self.broadcast(broadcast_str)
                conn.send(("### Bye " + user_nickname + " ###").encode("utf-8"))
                self.user_conn_lock.acquire()
                self.terminate_user(user_nickname)
                self.user_conn_lock.release()
                if DEBUG_BIT:
                    print("removing user thread:", user_nickname, " users remains in the room ars:")
                    self.user_conn_lock.acquire()
                    print(self.user_conn)
                    self.user_conn_lock.release()
                break
            client_split = client_str.split(" ")
            #check for friend request
            for request_type in self.friend_requests:
                if request_type in client_split:
                    friend_req_bit = True
                    #request type is send request
                    if request_type == "#friendme":
                        #search for target user
                        for word in client_split:
                            if word[0] == '@':
                                target_user = word[1:]
                        if target_user in self.user_conn.keys():
                            #send friend request
                            request_str = '<System>: ' + user_nickname \
                            + " send you a friend request, type @" \
                            + user_nickname + " #friends to accept"
                            request_str = request_str.encode('utf-8')
                            print("#TODO:", request_str.__class__)
                            self.user_conn[target_user].send(request_str)
                            echo_str = '<System>:request sent'
                            echo_str = echo_str.encode('utf-8')
                            self.user_conn[user_nickname].send(echo_str)
                            self.pending_request_lock.acquire()
                            self.pending_request[target_user] = self.pending_request.get(target_user, []) + [user_nickname]
                            self.pending_request_lock.release()
                        else:
                            self.user_conn[user_nickname].send(
                                b"<System>: user do not exist"
                                )
                    #request type is accept invite
                    if request_type == "#friends":
                        #search for target user
                        for word in client_split:
                            if word[0] == '@':
                                target_user = word[1:]
                        self.pending_request_lock.acquire()
                        if target_user not in self.pending_request.get(user_nickname, []):
                            self.user_conn[user_nickname].send(
                                b"<System>: that user didn't send you a request, maybe send a request to he/her first?"
                                )
                            self.pending_request_lock.release()
                            break
                        self.pending_request_lock.release()
                        #accept request
                        if target_user in self.user_conn.keys() and target_user in self.pending_request.get(user_nickname, []):
                            #update friend list
                            self.friend_list_lock.acquire()
                            self.friend_list[user_nickname] = self.friend_list.get(
                                user_nickname, []
                                ) + [target_user]
                            self.friend_list[target_user] = self.friend_list.get(
                                target_user, []) + [user_nickname]
                            self.friend_list_lock.release()
                            #send reply message 
                            request_str = '<System>: ' + user_nickname \
                            + " and " \
                            + target_user + " are friends now"
                            request_str = request_str.encode('utf-8')
                            self.user_conn[user_nickname].send(request_str)
                            self.user_conn[target_user].send(request_str)
                            self.pending_request_lock.acquire()
                            self.pending_request[user_nickname].remove(target_user)
                            self.pending_request_lock.release()
                        #user not exists
                        else:
                            self.user_conn[user_nickname].send(
                                b"<System>: user do not exist"
                                )
                    #unfriend
                    if request_type == "#unfriend":
                        #search for target user
                        for word in client_split:
                            if word[0] == '@':
                                target_user = word[1:]
                        if target_user not in self.friend_list.get(user_nickname, []):
                            self.user_conn[user_nickname].send(b"<System>:this user is not your friend")
                            break
                        #remove friend
                        self.friend_list_lock.acquire()
                        curr_user_friend_list = self.friend_list.get(user_nickname, [])
                        if target_user in curr_user_friend_list:
                            curr_user_friend_list.remove(target_user)
                        target_user_friend_list = self.friend_list.get(target_user, [])
                        if user_nickname in curr_user_friend_list:
                            curr_user_friend_list.remove(user_nickname)
                        self.friend_list_lock.release()
                        #send reply message
                        reply_str = "<System>: " + target_user \
                        + " and " + user_nickname \
                        + "are no longer friends"
                        reply_str = reply_str.encode('utf-8')
                        if target_user in self.user_conn.keys():
                            self.user_conn[target_user].send(reply_str)
                        self.user_conn[user_nickname].send(reply_str)
            if friend_req_bit:
                continue
            #check for unicast
            if len(client_split) > 0:
                for word in client_split:
                    self.user_conn_lock.acquire()
                    if len(word) > 1 and word[0] ==  '@' and word[1:] in self.user_conn.keys():
                        #send unicast
                        unicast_bit = True
                        self.unicast(user_nickname, word[1:], client_str)
                        self.user_conn_lock.release()
                        break
                    self.user_conn_lock.release()
            #broadcast
            if not unicast_bit: 
                broadcast_str = "<"  \
                + user_nickname + "> " \
                + client_str
                broadcast_str = broadcast_str.encode("utf-8")
                self.user_conn_lock.acquire()
                self.broadcast(broadcast_str)
                self.user_conn_lock.release()
        if DEBUG_BIT:
            print("thread for user", user_nickname, "terminated")

    def broadcast(self, output_str):
        #send broadcastsu
        if(output_str.__class__ == "str"):
            output_str = output_str.encode('utf-8')
        for client_id in self.user_conn:
            self.user_conn[client_id].send(output_str)
    
    def unicast(self, source, target_user, message_str):
        #check friend relationship for part4
        self.friend_list_lock.acquire()
        if target_user not in self.friend_list.get(source, []) or source not in self.friend_list.get(target_user, []):
            message_str = "<System>: " + target_user \
            + " is not your friend, send friend request before wispers."
            message_str = message_str.encode('utf-8')
            self.user_conn[source].send(message_str)
            self.friend_list_lock.release()
            return
        self.friend_list_lock.release()
        target_str = "<" + source + "> whispers: " + message_str
        echo_str = "whispers to " + "<" + target_user + ">:"+ message_str
        echo_str = echo_str.encode('utf-8')
        target_str = target_str.encode('utf-8')
        self.user_conn[source].send(echo_str)
        self.user_conn[target_user].send(target_str)

    def terminate_user(self, user_nickname):
        self.user_conn[user_nickname].close()
        self.user_conn.pop(user_nickname)

if __name__ == '__main__':
    new_server = Server()
