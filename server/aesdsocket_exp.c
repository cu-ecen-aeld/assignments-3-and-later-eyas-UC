#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    printf("starting aesd socket server . . .");
    int socket_fd = socket(AF_INET,SOCK_STREAM,0);
    // before binding we need to set up sockaddr struct

    int status;
    struct addrinfo hints;
    struct addrinfo *result, *p;  // will point to the results

    // printf("sin address is %i",sa.sin_addr.s_addr);
    printf("size of hints (addrinfo struct) is %zu\n", sizeof(hints));
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    
    if((status = getaddrinfo("www.example.com","9000", &hints, &result)) !=0)
    {
        fprintf(stderr,"get address error: %s\n", gai_strerror(status));
        exit(1);
    }
    FILE *fd = fopen("myfile.txt", "w+");
    fprintf(fd,"result address = %s ",result->ai_addr->sa_data);
    char ipstr[INET6_ADDRSTRLEN];
    int x =0;
    for ( p = result; p != NULL; p = p->ai_next)
    {
        struct sockaddr_in *add_v4;
        if (p->ai_family == AF_INET)
        {
            add_v4 = (struct sockaddr_in *)p->ai_addr;

            inet_ntop(p->ai_family,add_v4, ipstr,sizeof ipstr);
            // if (ret <1)
            // {
            //     error(ret,ret,"something went wrong return is %i",ret);
            // }
            printf("  %s: %s\n", "IPv4\n", ipstr);
        
            // fprintf(stdout,"result address = %s ",(struct sockaddr_in *)p->ai_addr);
        }
        else {
        
            printf("ipv6\n");
        }
        x ++;
        printf("iteration no (%i)",x);

    }



    // finally we need to free the server 
    freeaddrinfo(result); 
    exit(0);
}