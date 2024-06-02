# HTTP C Server

This project is an implementation of a HTTP server in C. It's a lightweight, high-performance server designed to serve static files or can be integrated with other software to provide a dynamic content generation.

## Features

- **HTTP/1.1 Support**: This server supports the HTTP/1.1 protocol, including methods such as GET, POST, PUT, DELETE, etc.

- **Compression**: One of the key features of this server is its support for compression. It uses gzip compression to reduce the size of the HTTP response, which can significantly improve the performance especially for large files or slow network connections.

## Building

To build the server, you need a C compiler such as gcc. You can build the server using the following command:

```bash
gcc -o server app/server.c app/http.c app/map.c -lcurl -lz
```

## Running

To run the server, use the following command:

```bash
./server
```

## To Host files in a directory

```bash
./server --directory <directory absolute or relative>
```

## Contributing

Contributions are welcome! Please read the contributing guide for details.

## License

This project is licensed under the MIT License. See the LICENSE file for details.
