
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<fcntl.h>

#include<arpa/inet.h>
#include<netinet/in.h>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<sys/wait.h>
#include<signal.h>

#include<iostream>
#include<fstream>
#include<string>
#include<cstdlib>

#include "server_util.h"

#define RRQ 1
#define DATA 3
#define ACK 4
#define ERROR 5
#define OCTET "octet"
#define DATALENGTH 512

// Thanks BEEJ.US for tutorial on how to use SELECT and pack and unpack functions
// Some parts of the code have been taken from BEEJ.US

using namespace std;
int main(int argc, char *argv[]){
    struct sigaction sa;
<<<<<<< HEAD
    struct sockaddr_storage client_addr_storage, tmp_addr_storage;
    socklen_t addr_len, tmp_len;
=======
    //struct sockaddr_storage client_addr, tmp_addr;
>>>>>>> fork
    char ipstr[INET_ADDRSTRLEN];        //INET6_ADDRSTRLEN for IPv6
    connection_info server_conn_info;
    connection_info *clients_conn_info =(connection_info*)malloc(MAX_CLIENTS*sizeof(connection_info));
    initialize_server(argc, argv, server_conn_info);

    // Initializing the required variables
    char buf[1024];
    int num_bytes;

    struct sockaddr_in new_addr_in, client_addr_in;
    socklen_t addr_len;
    new_addr_in = server_conn_info.address;
    new_addr_in.sin_port = htons(0); // so can have random new available port
    struct tftp *p_rec_tftpR;
    //Bowei, do not understand here
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
<<<<<<< HEAD

    fd_set file_descriptors;  // temp file descriptor list for select()
    int fdmax = 0;        // maximum file descriptor number



=======
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    FD_ZERO(&read_fds);
>>>>>>> fork
    printf("Server Running ...\n");

    while(1){
<<<<<<< HEAD
        fdmax = construct_fd_set(&file_descriptors, &server_conn_info, clients_conn_info);
        if (select (fdmax+1), &file_descriptors, NULL, NULL, NULL)<0){
            perror("Select failed.");
            exit(1);
        }
        if(FD_ISSET(STDIN_FILENO, &file_descriptors)) { handle_user_input(clients_conn_info , server_conn_info.sockfd);}

        if (FD_ISSET(server_conn_info.sockfd, &file_descriptors )){
            handle_new_connection(server_conn_info, clients_conn_info, client_addr_storage,buf);
        }
        for(i = 0; i < MAX_CLIENTS; i++)
        {
              if(clients_conn_info[i].sockfd > 0 && FD_ISSET(clients_conn_info[i].sockfd, &file_descriptors))
              {
                handle_client_message(clients_conn_info, i);
              }
        }


    }// while (1) loop
    close(sockfd);
    freeaddrinfo(&server_conn_info.address_info);  // i need put this at stop server
=======
        addr_len = sizeof client_addr_in;
        if ((num_bytes = recvfrom(server_conn_info.sockfd, buf, 1023 , 0,(struct sockaddr *)&client_addr_in, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        printf("Server: got packet from %s on port %hu\n",
            inet_ntop(client_addr_in.sin_family,get_in_addr((struct sockaddr *)&client_addr_in),ipstr, sizeof ipstr),
            ntohs(client_addr_in.sin_port)
        );
        //printf("Server: packet is %d bytes long\n", num_bytes);
        p_rec_tftpR = decode(buf);  //decode received message
        cout <<"Opcode = "<< p_rec_tftpR->opcode <<", Filename = "<< p_rec_tftpR->filename <<", Mode = "<< p_rec_tftpR->mode << endl;
        if(p_rec_tftpR->opcode != RRQ){
            cout << "Invalid Opcode Received" << endl;
            continue;
        }
        //getting new socket, according beej 38, I do not need bind this sockfd
        int sockfd = 0;
        if((sockfd = socket(server_conn_info.address_info.ai_family, server_conn_info.address_info.ai_socktype, server_conn_info.address_info.ai_protocol))== -1){   // create the new server socket
            perror("server: socket");
        }
        // ifstream is class
        ifstream myfile;
        myfile.open(p_rec_tftpR->filename,ios::in | ios::binary);
        if(myfile.is_open()==false){
            cout << "Could not open requested file " << p_rec_tftpR->filename << endl;
            printf("------------------------------------------------------------\n");
            string message = "**Could not open requested file**";
            char *emsg= (char *)malloc(1024*sizeof(char));
            emsg = strcpy(emsg,message.c_str());
            int emsg_len = strlen(emsg);
            char *epacket = encode(ERROR,1,emsg,emsg_len);
            free(emsg);
            if( (num_bytes = sendto(sockfd,epacket,emsg_len+5,0,(struct sockaddr *)&client_addr_in,addr_len)) == -1 ){
                perror("server: sendto");
                exit(1);
            }
            exit(0);
        }
        // Calculate the size of the file
        streampos first,last;
        // seekg: Set position in input sequence
        // tellg: Get position in input sequence
        first = myfile.tellg();
        myfile.seekg(0,ios::end);
        last = myfile.tellg();
        int num_packets = ((last - first) /512) + 1;           // Calculate the number of packets to be sent
        myfile.seekg(0,ios::beg); // set the position of input sequence back

        cout << "File Size = " << last - first<< ", Packets = " << num_packets << endl;
        char file_buf[513];
        int blocknumber = 0, last_ack = 0, resend = 0, m_len;

        FD_SET(sockfd,&read_fds);
        fdmax = sockfd;
        struct timeval t;
        t.tv_sec = 0;       // Set timeout to 100 ms
        t.tv_usec = 100000;

        do{
            if(last_ack==blocknumber){
                myfile.read(file_buf,512);                  // Read next 512 bytes
                m_len = myfile.gcount();
                blocknumber++;
                resend = 0;
                //cout << "Blocknumber : " << blocknumber << endl;
            }
            if(resend > 0){
                if(resend == 51){
                    cout << "Time Out !! " << endl;
                    break;
                }
                cout << "Resending Attempt " << resend << " for Blocknumber " << blocknumber << endl;
            }
            char *packet = encode(DATA,blocknumber,file_buf,m_len);                       // Encode the data into the TFTP packet structure
            if( (num_bytes = sendto(sockfd,packet,m_len+4,0,(struct sockaddr *)&client_addr_in,addr_len)) == -1 ){ // Send Data Packet
                perror("server: sendto");
                exit(1);
            }
            free(packet);

            t.tv_usec = 100000;                                         // reset the timeout counter

            if( select(fdmax+1, &read_fds, NULL, NULL, &t) == -1){      // Select between client socket to wait for ACK or timeout after 100 ms
                perror("server: select");
                exit(4);
            }
            if(FD_ISSET(sockfd,&read_fds)){
                struct sockaddr_in tmp_addr_in;
                addr_len = sizeof tmp_addr_in;
                if ( (num_bytes = recvfrom(sockfd, buf, 1023 , 0,(struct sockaddr *)&tmp_addr_in, &addr_len)) == -1 ) {        // receive ACK
                    perror("recvfrom");
                    exit(1);
                }
                if(tmp_addr_in.sin_addr.s_addr == client_addr_in.sin_addr.s_addr){
                    p_rec_tftpR = decode(buf);                                      // decode ACK message
                    // need some conditional block to know whether this is an ACK or ERROR

                    if (p_rec_tftpR->opcode == ACK){
                        last_ack = p_rec_tftpR->blocknumber;
                    } else if (p_rec_tftpR->opcode == ERROR){
                        printf("ERROR: BLOCKNUMBER: %d\n", p_rec_tftpR->blocknumber);
                    }

                }
            }
            resend++;
        }while(last_ack<num_packets);
        /***********************************************************************************/
        myfile.close();
        close(sockfd);
        printf("Request Complete\n");
        printf("------------------------------------------------------------\n");
    }// while (1) loop
    close(server_conn_info.sockfd);
    freeaddrinfo(&server_conn_info.address_info);
>>>>>>> fork
    return 0;
}
