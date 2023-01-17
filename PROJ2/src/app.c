#include "app.h"

connection_parameters parameters;

int parse_url(char * url){

    if(strstr(url,"ftp://") != url) 
        return -1;

    char * user = NULL;
    char * password = NULL;
    char * hostname = NULL;
    char * url_path = NULL;

    char * no_header = url + 6;  // exclude ftp header

    unsigned int url_size = strlen(no_header); // needed before strtok 

    char * host = strtok(no_header, "/");

    if (strlen(host) == url_size){
        url_path = malloc(1);
        url_path = "";

    } else url_path = no_header + strlen(host) + 1;

    if(strstr(host, "@") != NULL) {

        char * auth = strtok(host, "@");
        hostname = strtok(NULL, "@");

        user = strtok(auth, ":");
        password = strtok(NULL, ":");

    } else hostname = host;

    if(hostname == NULL || url_path == NULL) 
        return -1;

    if((user == NULL && password != NULL) ||(user != NULL && password == NULL)) 
        return -1;

    return initialize_connection_parameters(user, password, hostname, url_path);
}

int initialize_connection_parameters(const char * user, const char * password, const char * host, const char * url_path){
    if(user != NULL) {

        parameters.user = (char*) malloc(strlen(user) * sizeof(char));
        strcpy(parameters.user, user);
        parameters.password = (char*) malloc(strlen(password) * sizeof(char));
        strcpy(parameters.password, password);

    } else {
        
        parameters.user = (char*) malloc(strlen("anonymous"));
        parameters.user = "anonymous";
        parameters.password = (char *) malloc(1);
        parameters.password = "";

    }

    struct hostent *h;

    if ((h = gethostbyname(host)) == NULL) {
        printf("gethostbyname() failed");
        return -1;
    }

    char* hostip = inet_ntoa(*((struct in_addr *) h->h_addr));

    parameters.host = (char*) malloc(strlen(hostip) * sizeof(char));
    strcpy(parameters.host, hostip);
    parameters.url_path = (char*) malloc(strlen(url_path) * sizeof(char));
    strcpy(parameters.url_path, url_path);

    return 0;

}

int open_connection(char *adress, int port){
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(adress);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    return sockfd;
}

int get_response_code(FILE* file){
    char *line = malloc(MAX_MSG_SIZE);
    int code = -1;
    size_t n;
    
    while (getline(&line, &n, file) != -1) {
        printf("%s\n",line);
        if (line[3] == ' ') {
            code = atoi(line); // integer value of string = response code
            break;
        }
    }

    free(line);

    return code;
}

int login (int fd){
    FILE *file = fdopen(fd, "r");

    char message[MAX_MSG_SIZE];
    int res = -1;

    while(res != LOGGED_IN){
        res = get_response_code(file);
        switch(res){
            case READY_USER:
                sprintf(message, "user %s\n", parameters.user);
                break;
            case READY_PASSWORD:
                sprintf(message, "pass %s\n", parameters.password);
                break;
            case LOGGED_IN:
                sprintf(message, "pasv\n");
                break;
            default:
                return -1;
        }

        printf("%s\n", message);
        write(fd, message, strlen(message));
    }

    return res;

}

int download(int fd){
    FILE *file = fdopen(fd, "r");
    char *line = malloc(MAX_MSG_SIZE);
    size_t n;

    getline(&line, &n, file);
    if (atoi(line) != PASSIVE_MODE)
        return -1;

    printf("%s\n", line);

    int address[4];
    int port[2];
    sscanf(line, "227 Entering Passive Mode (%d, %d, %d, %d, %d, %d).\n", &address[0], &address[1], &address[2], &address[3], &port[0], &port[1]);

    free(line);

    char address_str[16];
    sprintf(address_str, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
    int port_number = 256*port[0] + port[1];

    int download_fd = open_connection(address_str, port_number);

    char message[MAX_MSG_SIZE];
    sprintf(message, "retr %s\n", parameters.url_path);
    write(fd, message, strlen(message));

    if (get_response_code(file) != OPEN_CONNECTION)
        return -1;    

    char *filename = basename(parameters.url_path);    

    int file_fd = open(filename, O_WRONLY | O_CREAT, 0666);

    char download_bytes[256];
    int bytes;

    while((bytes = read(download_fd, download_bytes, 256)) != 0)
        write(file_fd, download_bytes, bytes);

    if(get_response_code(file) != TRANFER_COMPLETE)
        return -1;

    if (close(download_fd)<0) {
        perror("close()");
        return -1;
    }    
    
    return 0;

}

int app(){

    int sockfd = open_connection(parameters.host, FTP_CTRL);


    if (login(sockfd) == LOGGED_IN){
        download(sockfd);
    }


    if (close(sockfd)<0) {
        perror("close()");
        return -1;
    }

    return 0;
}