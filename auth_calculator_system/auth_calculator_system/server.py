import socket
import threading
import sqlite3
from hashlib import sha256

HOST = "127.0.0.1"  
PORT = 8080       

DB_FILE = "user_database.db"

def init_database():
    with sqlite3.connect(DB_FILE) as conn:
        cursor = conn.cursor()
        cursor.execute('''CREATE TABLE IF NOT EXISTS users (
            email TEXT PRIMARY KEY,
            password TEXT NOT NULL
        )''')
        conn.commit()

def add_user(email, hashed_password):
    with sqlite3.connect(DB_FILE) as conn:
        cursor = conn.cursor()
        try:
            cursor.execute("INSERT INTO users (email, password) VALUES (?, ?)", (email, hashed_password))
            conn.commit()
            return True
        except sqlite3.IntegrityError:
            return False

def get_user(email):
    with sqlite3.connect(DB_FILE) as conn:
        cursor = conn.cursor()
        cursor.execute("SELECT email, password FROM users WHERE email = ?", (email,))
        return cursor.fetchone()

def load_user_data():
    with sqlite3.connect(DB_FILE) as conn:
        cursor = conn.cursor()
        cursor.execute("SELECT email, password FROM users")
        return {email: password for email, password in cursor.fetchall()}


def evaluate_expression(expression):
    try:
        result = eval(expression)
        return f"{expression} = {result}"
    except Exception as e:
        return f"Error evaluating expression: {e}"


def handle_client(client_socket, addr, user_data):
    print(f"New connection from {addr}")
    authenticated = False

    while True:
        try:
            data = client_socket.recv(1024).decode()
            if not data:
                break

            if not authenticated:
               
                command, *args = data.split()
                if command == "LOGIN":
                    email, password = args
                    hashed_password = sha256(password.encode()).hexdigest()
                    user = get_user(email)
                    if user and user[1] == hashed_password:
                        authenticated = True
                        client_socket.send("You logged in".encode())
                    else:
                        client_socket.send("Email or password not match".encode())

                elif command == "SIGNUP":
                    email, password = args
                    hashed_password = sha256(password.encode()).hexdigest()
                    if add_user(email, hashed_password):
                        user_data[email] = hashed_password  
                        client_socket.send("Signup successful!".encode())
                    else:
                        client_socket.send("Email already exists".encode())

                elif command == "LOGOUT":
                    client_socket.send("Logged out".encode())
                    break
                else:
                    client_socket.send("Invalid command".encode())

            else:
                if data.lower() == "exit":
                    client_socket.send("Connection closed".encode())
                    break
                else:
                    result = evaluate_expression(data)
                    client_socket.send(result.encode())

        except ConnectionResetError:
            break

    print(f"Connection from {addr} closed.")
    client_socket.close()

def main():
    init_database()
    user_data = load_user_data()
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((HOST, PORT))
    server.listen(5)
    print(f"Server is listening on {HOST}:{PORT}")

    while True:
        client_socket, addr = server.accept()
        client_handler = threading.Thread(target=handle_client, args=(client_socket, addr, user_data))
        client_handler.start()

if __name__ == "__main__":
    main()
