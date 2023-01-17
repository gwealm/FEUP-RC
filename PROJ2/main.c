#include "app.h"

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Usage:\tdownload ftp://[<user>:<password>@]<host>/<url-path>\n");
        return -1;
    }

    if (parse_url(argv[1]) != 0){
        printf("Invalid parameters\n");
        printf("Usage:\tdownload ftp://[<user>:<password>@]<host>/<url-path>\n");
        return -1;
    }

    return app();
}
