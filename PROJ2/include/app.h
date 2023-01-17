#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>


#define FTP_CTRL 21
#define MAX_MSG_SIZE 256

#define OPEN_CONNECTION 150
#define READY_USER 220
#define TRANFER_COMPLETE 226
#define PASSIVE_MODE 227
#define LOGGED_IN 230
#define READY_PASSWORD 331

/**
 * @brief Parameters used for connection
 * 
 */
typedef struct {
    char * user;
    char * password;
    char * host;
    char * url_path;
} connection_parameters;

/**
 * @brief Parse information from URL (RFC1738)
 * 
 * @param url URL to parse
 * @return int 
 */
int parse_url(char * url);

/**
 * @brief Initialize connection_parameters struct according to parsed arguments 
 * 
 * @param user Username for login (NULL if anonymous connection)
 * @param password Password for login (NULL if anonymous connection)
 * @param host Hostname
 * @param url_path Path to the file to be downloaded
 * @return int 
 */
int initialize_connection_parameters(const char * user, const char * password, const char * host, const char * url_path);

/**
 * @brief Establish a TCP connection
 * 
 * @param address Connection address
 * @param port Connection port
 * @return int 
 */
int open_connection(char *address, int port);

/**
 * @brief Get the response code of a server response
 * 
 * @param file File descriptor to read from commands from the server
 * @return int 
 */
int get_response_code(FILE* file);

/**
 * @brief Login to the server (can be anonymous)
 * 
 * @param fd File descriptor to read/send commands from/to the server
 * @return int 
 */
int login(int fd);

/**
 * @brief Ask the server retrive the file and transfer it
 * 
 * @param fd File descriptor to read data/commands from the server
 * @return int 
 */
int download(int fd);

/**
 * @brief FTP implementation, calls the right functions to download a file
 * 
 * @return int 
 */
int app();