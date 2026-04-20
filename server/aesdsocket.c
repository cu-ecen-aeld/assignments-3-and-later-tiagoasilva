#include "aesdsocket.h"


bool writeToFile(char* content) {
    FILE *fptr;
    fptr = fopen(BUFFER_FILE, "a");
    if (fptr == NULL) {
        printf("Error opening file to write\n");
        syslog(LOG_ERR, "Error opening file to write");
        return false;
    }
    fprintf(fptr, "%s", content);
    fclose(fptr);
    return true;
}

char * readFromFile() {
    FILE *fptr;
    fptr = fopen(BUFFER_FILE, "r");
    if (fptr == NULL) {
        printf("Error opening file to read\n");
        syslog(LOG_ERR, "Error opening file to read");
        return NULL;
    }

    fseek(fptr, 0, SEEK_END);
    long unsigned fileSize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    char *buffer = malloc(fileSize + 1);
    if (buffer == NULL) {
        fclose(fptr);
        return NULL;
    }

    size_t size = fread(buffer, 1, fileSize, fptr);
    buffer[fileSize] = '\0';

    if (size < fileSize) {
        printf("Error reading file. Not able to read all file content\n");
        syslog(LOG_ERR, "Error reading file. Not able to read all file content");
    }

    fclose(fptr);
    return buffer;
}

bool commClient(int conn_fd) {
    char buf[MAXDATASIZE];
    int numbytes;
    bool final = false;
    size_t read_total = 0;

    char *final_buffer = malloc(MAXDATASIZE);
    if (!final_buffer) {
        return false;
    }

    while(!final) {
        numbytes = recv(conn_fd, buf, sizeof(buf), 0);
        if (numbytes <= 0) break;

        if (read_total + numbytes + 1 > MAXDATASIZE) {
            char *new_buffer = realloc(final_buffer, MAXDATASIZE*2);
            if (!new_buffer) {
                free(final_buffer);
                return false;
            }
            final_buffer = new_buffer;
        }

        memcpy(final_buffer + read_total, buf, numbytes);
        read_total += numbytes;
        if (memchr(buf, '\n', numbytes)) {
            final_buffer[read_total] = '\0';
            break;
        }
    }

    writeToFile(final_buffer);
    free(final_buffer);

    char *response = readFromFile();
    if (response == NULL) {
        printf("Error reading file\n");
        syslog(LOG_ERR, "Error reading file");
        return false;
    }

    size_t responseSize = strlen(response);
    if (send(conn_fd, response, responseSize, 0) == -1) {
        printf("send error\n");
        syslog(LOG_ERR, "send error");
        return false;
    }

    free(response);

    return true;
}

static void signal_handler () {
    int errno_saved = errno;

    remove(BUFFER_FILE);

    printf("Caught signal, exiting\n");
	syslog(LOG_INFO,"Caught signal, exiting");
    errno = errno_saved;
    exit(0);
}

bool registerSignals() {
    struct sigaction new_action;
    bool success = true;
    memset(&new_action,0,sizeof(struct sigaction));
    new_action.sa_handler=signal_handler;
    if( sigaction(SIGTERM, &new_action, NULL) != 0 ) {
        printf("Error %d (%s) registering for SIGTERM\n",errno,strerror(errno));
        syslog(LOG_ERR,"Error %d (%s) registering for SIGTERM",errno,strerror(errno));
        success = false;
    }
    if( sigaction(SIGINT, &new_action, NULL) ) {
        printf("Error %d (%s) registering for SIGINT\n",errno,strerror(errno));
        syslog(LOG_ERR,"Error %d (%s) registering for SIGINT",errno,strerror(errno));
        success = false;
    }
    return  success;
}

int main(int argc, char**argv) {
    bool daemon_mode = false;
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        printf("Starting as a deamon\n");
        syslog(LOG_INFO,"Starting as a deamon");
        daemon_mode = true;
    }

    registerSignals();

    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //TODO: change port to 9000
    if ((status = getaddrinfo(NULL/*"localhost"*/, PORT, &hints, &servinfo)) != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(status));
        syslog(LOG_ERR,"getaddrinfo error: %s", gai_strerror(status));
        return 1;
    }
    char host[NI_MAXHOST];
    getnameinfo(servinfo->ai_addr, servinfo->ai_addrlen, host, sizeof host, NULL, 0, NI_NUMERICHOST);
    printf("IP address: %s\n", host);
    syslog(LOG_INFO,"IP address: %s", host);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("socket error %d: %s\n", errno, strerror(errno));
        syslog(LOG_ERR,"socket error %d: %s", errno, strerror(errno));
        freeaddrinfo(servinfo);
        return 1;
    }

    //Allows other sockets to bind() to this port, unless there is an active 
    //listening socket bound to the port already. This enables you to get around
    //those “Address already in use” error messages when you try to restart
    //your server after a crash.
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == -1) {
        printf("SO_REUSEADDR not enabled on sockfd!\n");
        syslog(LOG_ERR,"SO_REUSEADDR not enabled on sockfd!");
    }
    
    socklen_t addrlen = sizeof(struct sockaddr);
    status = bind(sockfd, servinfo->ai_addr, addrlen);
    if (status != 0) {
        printf("bind error %d: %s\n", errno, strerror(errno));
        syslog(LOG_ERR,"bind error %d: %s", errno, strerror(errno));
        freeaddrinfo(servinfo);
        return 1;    
    }
    
    if (daemon_mode)
    {
        pid_t pid = fork();
        if (pid < 0) exit(1);
        if (pid > 0) exit(0);

        if (setsid() < 0) return 1;

        umask(0);
        if (chdir("/") != 0) {
            return 1; 
        }
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    status = listen(sockfd, BACKLOG);
    if (status < 0) {
        printf("listen error %d: %s\n", errno, strerror(errno));
        syslog(LOG_ERR,"listen error %d: %s", errno, strerror(errno));
        freeaddrinfo(servinfo);
        return 1;    
    }

    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    while (true) {
        printf("listen for connection...\n");
        syslog(LOG_INFO,"listen for connection...");
        addr_size = sizeof their_addr;
        int conn_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if (conn_fd < 0) {
            printf("file descriptor for the accepted socket error %d: %s\n", errno, strerror(errno));
            syslog(LOG_ERR,"file descriptor for the accepted socket error %d: %s", errno, strerror(errno));
            freeaddrinfo(servinfo);
            continue;
        }
        socklen_t len;
        struct sockaddr_storage addr;
        char ipstr[INET_ADDRSTRLEN];

        len = sizeof addr;
        getpeername(conn_fd, (struct sockaddr*)&addr, &len);

        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);

        printf("Accepted connection from %s\n",ipstr);
	    syslog(LOG_INFO,"Accepted connection from %s",ipstr);
        commClient(conn_fd);
        printf("Closed connection from %s\n",ipstr);
	    syslog(LOG_INFO,"Closed connection from %s",ipstr);
    }

    freeaddrinfo(servinfo);
    return 0;
}
