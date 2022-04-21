# Asio chat

Chat application, example using network in C++.

- Multiple clients, one server
- Using [Asio](https://github.com/chriskohlhoff/asio)
- Inspired by [this tutorial](https://dens.website/tutorials/cpp-asio)
- Portable C++17
- Variadic-length message

The console parallel read / write is handled in a different thread. There is no way in standard C++ to make it prettier without a GUI.

# Build

```
cd asio-chat
mkdir build && cd build && cmake .. && cmake --build . --target both
```

# Run

Run server:

```
    →  ./server
client accepted!
[127.0.0.1:47496] hello all
[127.0.0.1:47496] nobody?
[127.0.0.1:47496] ok...
client accepted!
[127.0.0.1:47500] hi there
```

Run client:

```
    → ./client 
hello all
send: "hello all"
[127.0.0.1:47496] hello all
nobody?
send: "nobody?"
[127.0.0.1:47496] nobody?
ok...
send: "ok..."
[127.0.0.1:47496] ok...
[127.0.0.1:47500] hi there
```

Run a second client:

```
    → ./client 
hi there
send: "hi there"
[127.0.0.1:47500] hi there
```
