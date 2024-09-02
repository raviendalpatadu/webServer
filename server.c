#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFERSIZE 100096
#define SERVER_DIR "./html"
#define PORT 1234

struct
{
    char *ext;
    char *filetype;
} extensions[] = {
    {"gif", "image/gif"},
    {"jpg", "image/jpg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"ico", "image/ico"},
    {"zip", "image/zip"},
    {"gz", "image/gz"},
    {"tar", "image/tar"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"css", "text/css"},
    {0, 0}};

void web(int fd, int hit)
{
    int j, file_fd, buffer_len;
    long i, ret, len;
    char *fstr;
    static char buffer[BUFFERSIZE + 1]; /* static so zero filled */

    ret = read(fd, buffer, BUFFERSIZE); /* read Web request in one go */

    if (ret == 0 || ret == -1)
    { /* read failure stop now */
        printf("Failed to read request from browser \n");
    }

    if (ret > 0 && ret < BUFFERSIZE) /* return code is valid chars */
        buffer[ret] = 0;             /* terminate the buffer */
    else
        buffer[0] = 0;

    for (i = 0; i < ret; i++)
    { /* remove CF and LF characters */
        if (buffer[i] == '\r' || buffer[i] == '\n')
            buffer[i] = '*';
    }

    

    if (strncmp(buffer, "GET ", 4) && strncmp(buffer, "get ", 4))
    {

        printf("Only simple GET operation supported : %s \n", buffer);
        
        char html_content[] = "HTTP/1.1 403 Forbidden\n"
                              "Content-Length: 185\n" 
                              "Connection: close\n"
                              "Content-Type: text/html\n\n"
                              "<html><head>\n"
                              "<title>403 Forbidden</title>\n"
                              "</head><body>\n"
                              "<h1>Forbidden</h1>\n"
                              "The requested URL, file type or operation is not allowed on this simple static file webserver.\n"
                              "</body></html>\n";
                            
        // Calculate the total buffer size, including null terminator
        size_t buffer_size = strlen(html_content) + 1;

        // Write the entire response in one `write()` call
        (void)write(fd, html_content, buffer_size);

        exit(-1);
    }

    for (i = 4; i < BUFFERSIZE; i++)
    { /* null terminate after the second space to ignore extra stuff */
        if (buffer[i] == ' ')
        { /* string is "GET URL " +lots of other stuff */
            buffer[i] = 0;
            break;
        }
    }

    for (j = 0; j < i - 1; j++) /* check for illegal parent directory use .. */
        if (buffer[j] == '.' && buffer[j + 1] == '.')
        {
            printf("Directory . or .. is unsupoorted : %s \n", buffer);
            // error code 403
            char html_content[] = "HTTP/1.1 403 Forbidden\n"
                                  "Content-Length: 185\n" 
                                  "Connection: close\n"
                                  "Content-Type: text/html\n\n"
                                  "<html><head>\n"
                                  "<title>403 Forbidden</title>\n"
                                  "</head><body>\n"
                                  "<h1>Forbidden</h1>\n"
                                  "The requested URL, file type or operation is not allowed on this simple static file webserver.\n"
                                  "</body></html>\n";


            // Calculate the total buffer size, including null terminator
            size_t buffer_size = strlen(html_content) + 1;

            // Write the entire response in one `write()` call
            (void)write(fd, html_content, buffer_size);


            exit(-1);
        }

    if (!strncmp(&buffer[0], "GET /\0", 6) || !strncmp(&buffer[0], "get /\0", 6))
        (void)strcpy(buffer, "GET /index.html");

    buffer_len = strlen(buffer);
    fstr = (char *)0;
    for (i = 0; extensions[i].ext != 0; i++)
    {
        len = strlen(extensions[i].ext);
        if (!strncmp(&buffer[buffer_len - len], extensions[i].ext, len))
        {
            fstr = extensions[i].filetype;
            break;
        }
    }

    if (fstr == 0)
    {
        printf("File type not supported : %s \n", buffer);
        // error code 415

        char html_content[] = "HTTP/1.1 415 Unsupported Media Type\n"
                              "Content-Length: 264\n" 
                              "Connection: close\n"
                              "Content-Type: text/html\n\n"
                              "<html><head>\n"
                              "<title>415 Unsupported Media Type</title>\n"
                              "</head><body>\n"
                              "<h1>Unsupported media type</h1>\n"
                              "The requested media type or operation is not allowed on this simple static file webserver.\n"
                              "</body></html>\n";

        // Calculate the total buffer size, including null terminator
        size_t buffer_size = strlen(html_content) + 1;

        // Write the entire response in one `write()` call
        (void)write(fd, html_content, buffer_size);

        exit(-1);
    }

    if ((file_fd = open(&buffer[5], O_RDONLY)) == -1)
    { /* open the file for reading */
        printf("failed to open file :");

        char html_content[] = "HTTP/1.1 404 Not Found\n"
                              "Content-Length: 136\n" 
                              "Connection: close\n"
                              "Content-Type: text/html\n\n"
                              "<html><head>\n"
                              "<title>404 Not Found</title>\n"
                              "</head><body>\n"
                              "<h1>Not Found</h1>\n"
                              "The requested URL was not found on this server.\n"
                              "</body></html>\n";
        
        // Calculate the total buffer size, including null terminator
        size_t buffer_size = strlen(html_content) + 1;

        // Write the entire response in one `write()` call
        (void)write(fd, html_content, buffer_size);

        exit(-1);
    }

    len = (long)lseek(file_fd, (off_t)0, SEEK_END); /* lseek to the file end to find the length */
    (void)lseek(file_fd, (off_t)0, SEEK_SET);       /* lseek back to the file start ready for reading */

    (void)sprintf(buffer, "HTTP/1.1 200 OK\nServer: webserver/1.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", len, fstr); /* Header + a blank line */

    (void)write(fd, buffer, strlen(buffer));

    /* send file in 8KB block - last block may be smaller */
    while ((ret = read(file_fd, buffer, BUFFERSIZE)) > 0)
    {
        (void)write(fd, buffer, ret);
    }
    sleep(1); /* allow socket to drain before signalling the socket is closed */
    close(fd);
    exit(1);
}

int main()
{
    int port, listenfd, socketfd, hit;
    // int i, pid;
    socklen_t length;
    static struct sockaddr_in cli_addr;  /* static = initialised to zeros */
    static struct sockaddr_in serv_addr; /* static = initialised to zeros */

    listenfd = socket(AF_INET, SOCK_STREAM, 0); // initialize socket

    if (listenfd < 0)
        printf("Failed to inizialize socket");

    port = PORT;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Failed to bind socket\n");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 64) < 0)
        printf("Failed listen socket\n");
    else
        printf("Server started at port %d\n", port);

    if (chdir(SERVER_DIR) == -1)
        (void)printf("ERROR: Can't Change to directory %s\n", SERVER_DIR);

    for (hit = 1;; hit++)
    {
        length = sizeof(cli_addr);
        if ((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
            printf("Failed to accept socket\n");

        // Fork to handle the incoming connection in a separate process
        pid_t pid = fork();

        if (pid < 0)
        {
            printf("Failed to fork process\n");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Child process
            close(listenfd);    // Close listening socket in the child process
            web(socketfd, hit); // Handle the request in the child process
            close(socketfd);    // Close the socket in the child process
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Parent process
            close(socketfd); // Close the socket in the parent process
        }
    }
}
