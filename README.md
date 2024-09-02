# Simple Static File Web Server

This project implements a simple static file web server written in C. It serves files from a specified directory and supports basic HTTP GET requests. The server can handle multiple file types (HTML, images, CSS) and responds with appropriate error messages for unsupported operations or file types.

## Features

- Serves static files such as HTML, images, and CSS.
- Supports basic `GET` HTTP requests.
- Handles multiple connections using `fork` to create child processes.
- Responds with appropriate HTTP status codes for common errors:
  - `404 Not Found` for non-existent files.
  - `403 Forbidden` for unsupported operations.
  - `415 Unsupported Media Type` for unsupported file types.

## Supported File Types

- `gif`
- `jpg`
- `jpeg`
- `png`
- `ico`
- `zip`
- `gz`
- `tar`
- `html`
- `css`

## Directory Structure

Here is an overview of the project's directory structure:

```plaintext
WEB_SERVER_PROJECT/
├── .vscode/          # VSCode-specific configurations (if any)
├── html/             # Directory for static files (e.g., index.html, images)
├── README.md         # The README file explaining the project
├── server            # The compiled server executable
└── server.c          # The C source code for the web server

```
## Setup Instructions
Follow these steps to set up and run the web server on your local machine:

1. Clone the Repository
Clone this repository to your local machine using Git:
`git clone <repository-url>`
`cd WEBSERVER`

2. Compile the Code
Make sure you have a C compiler installed (such as gcc). To compile the server.c file, run the following command:
`gcc -o server server.c`
This command will compile the server.c file and create an executable named server.

3. Run the Server
Start the server by executing the compiled server binary:
`./server`
By default, the server listens on localhost:1234 and serves files from the html/ directory.

4. Access the Server
You can access the server through a web browser. Open your browser and navigate to:
``http://localhost:1234``
The server will return the index.html file from the html/ directory if it exists. You can also request other files in the directory by specifying their path in the URL. For example:
`http://localhost:1234/image.jpg`
5. Stopping the Server
To stop the server, press `Ctrl + C` in the terminal where the server is running. This will terminate the server process.

7. Testing the Error Responses
The server handles the following error cases:

`403 Forbidden:` Returned when trying to access unsupported file types or attempting directory traversal.
`404 Not Found:` Returned when the requested file does not exist in the html/ directory.
`415 Unsupported Media Type:` Returned when the requested file type is not supported by the server.
To test these error cases, try requesting a non-existent file or an unsupported file type in your browser.

## Customization
Change the Port
By default, the server listens on port 1234. If you want to change this, modify the PORT constant in the server.c file:


`#define PORT 1234  // Change this value to set a different port`
After making this change, recompile the server and run it again with the new port.

## Change the Directory
The server serves files from the html/ directory by default. You can change this by modifying the SERVER_DIR constant in the server.c file:


`#define SERVER_DIR "./html"  // Change this to a different directory if needed`
If you change the directory, make sure the new directory exists and contains the files you want to serve. After making this change, recompile the server.