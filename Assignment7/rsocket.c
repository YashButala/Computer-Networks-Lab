

#include "rsocket.h"
#define MAXLINE 1024
// Maximum message buffer

int numTransmissions = 0;

const int MAXTABLES = 100;  // The size of the tables


struct sendBuffer *sendBuf;   // Buffer of data packets to be sent
struct recvBuffer *recBuf;   // Buffer of unique packets received
struct unAckTable *uAckTable;   // A table of unACKed packets
struct recvIDs *recID;   // A table of packets whose packets have been received



// A function to generate the drop probability by generating a random number

int socket_fd = -1;   // The socket fd
void mem_init(struct sendBuffer *obj1,struct recvBuffer *obj2,struct unAckTable *obj3,struct recvIDs *obj4)
{
    //initialize send buffer
    (*obj1).p = (struct sendPkt **)malloc(MAXTABLES * sizeof(struct sendPkt *));
    for(int n=0 ;n < MAXTABLES; n++)
    {
        (*obj1).p[n] = NULL;
    }
    obj1->end = -1;
    (*obj1).size = 0;
    obj1->front = -1;


    //initialize receive buffer
    (*obj2).p = (struct rcvPkt **)malloc(MAXTABLES * sizeof(struct rcvPkt *));
    for(int n=0;n<MAXTABLES;n++)
    {
        (*obj2).p[n] = NULL;
    }
    (*recBuf).front = -1;
    (*recBuf).end = -1; 
    (*obj2).size = 0;


    //init unAck table
    (*obj3).p = (struct unAckPacket **)malloc(MAXTABLES * sizeof(struct unAckTable *));
    for(int n=0;n<MAXTABLES;n++)
    {
        (*obj3).p[n] = NULL;
    }
    (*obj3).size = 0;

    //init recvIDs
    int k = 5 * MAXTABLES;
    (*obj4).IDs = (int *)malloc(k * sizeof(int));
    for(int n=0;n<MAXTABLES;n++)
    {
        (*obj4).IDs[n] = -1;
    }
    (*obj4).size = 0;

}

// Initialise the receiving packet
void initRecvPacket(struct rcvPkt *entity, struct sockaddr_in src, struct data buff)
{
    (*entity).d = buff;
    (*entity).from = src;
}
// Initialise element of unACK Table
void initUnAckPacket(struct unAckPacket *unPacket, struct sendPkt packet)
{
    unPacket->t = time(NULL);
    unPacket->p = packet;
}

// Add a packet to the sending buffer
void editSendBuff(int flag,struct sendPkt *pkta,struct sendPkt **packs,struct sendBuffer *entity)
{
    // Case of valid buffer
    if(flag==0)
    {
        if((*sendBuf).front != -1)
        {
            (*sendBuf).end = ((*sendBuf).end + 1) % MAXTABLES;
        }
        // Case of empty buffer
        else
        {
            (*sendBuf).front = 0;
            (*sendBuf).end  = 0;
        }
        sendBuf->p[(*sendBuf).end ] = pkta;
        sendBuf->size += 1;
        return;
    }
    if(flag==1)
    {
        *packs = sendBuf->p[(*sendBuf).front];
        // Only one element is remaining
        if((*sendBuf).front == (*sendBuf).end )
        {
            (*sendBuf).front = -1;
            (*sendBuf).end  = -1;
        }

        // Other cases
        else
        {
            (*sendBuf).front = ((*sendBuf).front + 1) % MAXTABLES;
        }
        sendBuf->size -= 1;
    }
    if(flag==2)
    {
        free((*entity).p);
        free(entity);  
    }

}


void editRecvBuff(int flag,struct rcvPkt *pkta,struct rcvPkt **packet,struct recvBuffer *entity)
{
    if(flag==0)
    {
        if(recBuf->front == -1)
        {
            (*recBuf).end = 0;
            (*recBuf).front = 0;

        }

        // All other case
        else
        {
            (*recBuf).end = ((*recBuf).end + 1) % MAXTABLES;
        }

        (*recBuf).p[(*recBuf).end] = pkta;
        (*recBuf).size += 1;       
    }
    if(flag==1)
    {
        *packet = recBuf->p[recBuf->front];
        // Only one element is remaining
        if((*recBuf).front == (*recBuf).end)
        { 
            (*recBuf).front = -1;
            (*recBuf).end = -1;
        }
        // Other cases
        else
        {
            (*recBuf).front = ((*recBuf).front + 1) % MAXTABLES;
        }

        (*recBuf).size -= 1;
    }
    if(flag==2)
    {
        free(entity->p);
        free(entity);
    }
}




void editUnAckTable(int flag,struct unAckPacket *packet,int seqNo,struct unAckTable *entity)
{
    if(flag==0)
    {
        for(int i=0;i<MAXTABLES;i++)
        {
            if((*uAckTable).p[i] == NULL)
            {
                (*uAckTable).p[i] = packet;
                break;
            }
        }
        ((*uAckTable).size)++;   
    }
    if(flag==1)
    {
        if(!((*uAckTable).size))
            return ;

        for(int n=0;n < MAXTABLES;n++)
        {
            if( (*uAckTable).p[n] == NULL)
                continue;
            if((*(*uAckTable).p[n]).p.seqNo == seqNo)
            {
                free((*uAckTable).p[n]);
                (*uAckTable).size -= 1;
                (*uAckTable).p[n] = NULL;
                return;           
            }
        }
         
    }
    if (flag==2)
    {
        free(entity->p);
        free(entity);    
    }
}


// Search and Add receive ID if not there
int addIDtoRecvIDs(int seqNo)
{
    int n = 0;
    int fl=1;
    while(n<(*recID).size)
    {
        if(recID->IDs[n] == seqNo)
            {
                fl=0;
                break;
            }
            
        n++;
    }

    (*recID).IDs[recID->size] = seqNo;
    (*recID).size += 1;
    if(!fl)
        return -1;
    else
        return 0;
}

// Free receive ID Table
void freeRecvIDs(struct recvIDs *entity)
{
    free(entity);
    free((*entity).IDs);
}

// Encode the sendPkt structure into a character array
void msg_encoder(struct sendPkt *pkt, void *msg, int *rec)
{
    int cur1,cur;
    cur1= 0;
    memcpy(msg+cur1, &((*pkt).to), sizeof((*pkt).to));
    int cur2 = sizeof(pkt->to);
    memcpy(msg+cur2, &((*pkt).type), sizeof((*pkt).type));
    int cur3 = cur2 + sizeof(pkt->type);
    memcpy(msg+cur3, &(pkt->seqNo), sizeof(pkt->seqNo));
    int cur4 = cur3 + sizeof(pkt->seqNo);
    memcpy(msg+cur4, &((*pkt).d.msg_len), sizeof((*pkt).d.msg_len));
    int cur5 = cur4 + sizeof(pkt->d.msg_len);
    if(pkt->d.msg_len !=0 )
    {
        memcpy(msg+cur5, pkt->d.msg, pkt->d.msg_len) ;
    }
    int cur6 = cur5 + pkt->d.msg_len;
    *rec = cur6;
    return ;
}

// Decode the character array back to a sendPkt structure
int msg_decoder(struct sendPkt *p, void *msg, int rec)
{
    int cur1 = 0,cur;
    memcpy(&(p->to), msg+cur1, sizeof(struct sockaddr_in));
    int cur2 = cur1 + sizeof(struct sockaddr_in);
    memcpy(&(p->type), msg+cur2, sizeof(int));
    int cur3 = cur2 + sizeof(int);
    memcpy(&(p->seqNo), msg+cur3, sizeof(int));
    int cur4 = cur3 + sizeof(int);
    memcpy(&(p->d.msg_len), msg+cur4, sizeof(int));
    int cur5 = cur4 + sizeof(int);

    // only if there is data
    if(p->d.msg_len!=0)
    {
        p->d.msg = malloc(p->d.msg_len);
        memcpy(p->d.msg, msg+cur5, p->d.msg_len);
        cur5 += p->d.msg_len;
    }
    if(rec != cur5)
        return -1;  
    return 0;
}

int probability(float p)
{
    srand((unsigned int)time(0));
    float val = (float)rand()/RAND_MAX ;
    if (val < p)
        return 1;
    return 0;
}

// Close the socket and free the memory
int r_close(int socket)
{
	printf("Number of transmissions, %d\n", numTransmissions);
    if(close(socket) < 0)
    {
        return -1;
    }

    editSendBuff(2,NULL,NULL,sendBuf);
    editRecvBuff(2,NULL,NULL,recBuf);
    editUnAckTable(2,NULL,-1,uAckTable);
    freeRecvIDs(recID);
    return 0;
} 

// Function to handle application buf
void handleAppMsgRecv(struct sendPkt *pkt, struct sockaddr_in *con_addr)
{
    // Drop the buf if the receive buffer is full
    if(recBuf->size == MAXTABLES)
    {
        return;
    }

    // First time receiving the buf, add it to receive buffer
    if( addIDtoRecvIDs((*pkt).seqNo) == 0)
    {
        struct rcvPkt *tmp; 
        tmp = (struct rcvPkt *)malloc(sizeof(struct rcvPkt));
        initRecvPacket(tmp, *con_addr, (*pkt).d);
        editRecvBuff(0,tmp,NULL,NULL);
    }

    struct sendPkt *p ;
    p = (struct sendPkt *)malloc(sizeof(struct sendPkt));
    (*p).d.msg_len = 0; (*p).seqNo = (*pkt).seqNo; (*p).type = ackFlag; (*p).to = *con_addr;
    
    // Encode the buf to char array
    void *msg = malloc(MAXLINE);
    int size;
    msg_encoder(p, msg, &size);

    // Send it to the origin untill properly sent
    ssize_t ret =  sendto(socket_fd, msg, size, MSG_DONTWAIT,(struct sockaddr *)con_addr, sizeof(*con_addr));

	// increment the number of transmissions
	numTransmissions++;

    while( ret < 0)
    {
        if(!(errno == EAGAIN && errno == EWOULDBLOCK))
        {
            ret = sendto(socket_fd, msg, size, MSG_DONTWAIT,(struct sockaddr *)con_addr, sizeof(*con_addr));
			// increment the number of transmissions
			numTransmissions++;
        }
        else
        {
            break;
        }
    }
    free(p);
}

// Function to handle ACK buf
void handleACKMsgRecv(struct sendPkt *packet)
{
    editUnAckTable(1,NULL,packet->seqNo,NULL);
}

// Handle received buf, depending on whether they are ACK or not
void handleReceive()
{   
   
    struct sockaddr_in con_addr;
    socklen_t size = sizeof(struct sockaddr_in);
    int rec;
    char buff[MAXLINE];
    rec = recvfrom(socket_fd, buff, MAXLINE, MSG_DONTWAIT, (struct sockaddr *)&con_addr, &size);
    if( rec > 0)
    {   
       
        if( probability(Prob) )
        {
            return;
        }

        struct sendPkt *pkt;
        pkt = (struct sendPkt *)malloc(sizeof(struct sendPkt));
        if( msg_decoder(pkt, buff, rec) <= -1)
        {
            return;
        }         

        if(pkt->type != appFlag)
        {
            handleACKMsgRecv(pkt);
        }
        else
        {
            handleAppMsgRecv(pkt, &con_addr);
        }
        free(pkt);
    }
}

// See if any buf is timed out and resend it
void handleRetransmit()
{
    if( !uAckTable->size )
        return;
    int n=0;
    int cnt = 0;
    while( n<MAXTABLES && cnt<(*uAckTable).size)
    {
        time_t cur_t = time(NULL);
        if((*uAckTable).p[n] != NULL)
        {
            if( (cur_t >= (*(*uAckTable).p[n]).t) + T )   // Timeout condition
            {
                void *msg = (void*)malloc(MAXLINE);
                int cap;
                msg_encoder(&((*(*uAckTable).p[n]).p), msg, &cap);

                // Handle sendto errors
                int param = sizeof((*(*uAckTable).p[n]).p.to);
                int ret = sendto(socket_fd, msg, cap, MSG_DONTWAIT,(const struct sockaddr *)&((*(*uAckTable).p[n]).p.to),param );
				
				// increment the number of transmissions
				numTransmissions++;

                if( ret < 0 )
                {
                    if(errno != EAGAIN )
                    {
                        if(errno != EWOULDBLOCK)
                        {
                            cnt = cnt + 1;
                            continue;
                        }
                    }
                }
                
                uAckTable->p[n]->t = time(NULL);    // Reset time
            }
            cnt = cnt + 1;
        }
        n = n+1;
    }
}

// Send messages from send buffer if possible
void handleTransmit()
{
    // As long as possible
    while(uAckTable->size < MAXTABLES && sendBuf->size >0)
    {
        int size;
        struct sendPkt *packet;
        editSendBuff(1,NULL,&packet,NULL);
        void *msg = malloc(MAXLINE);
        msg_encoder(packet, msg, &size);
        
        // Handle incorrect arguments and interrupts
        ssize_t ret = sendto(socket_fd, msg, size, 0, (const struct sockaddr *)&(packet->to), sizeof(packet->to));
		
		// increment the number of transmissions
		numTransmissions++;

        if(  ret < 0)
        {
            if( errno != EWOULDBLOCK)
                if(errno != EAGAIN )
                {
                    continue;
                }
        }

        // After sending add to unacknowledged packets
        struct unAckPacket *unPkt; 
        unPkt = (struct unAckPacket *)malloc(sizeof(struct unAckPacket));
        initUnAckPacket(unPkt, *packet);

        editUnAckTable(0,unPkt,-1,NULL);
    }
}

// Signal handler for handling  SIGALARM
void signalHandler(int signal)
{
    handleReceive();
    handleRetransmit();
    handleTransmit();
}

// Initialise everything
int init()
{
    // Initialise the data tables and buffers
    sendBuf = (struct sendBuffer *)malloc(sizeof(struct sendBuffer));

    recBuf = (struct recvBuffer *)malloc(sizeof(struct recvBuffer));
    
    uAckTable = (struct unAckTable *)malloc(sizeof(struct unAckTable));

    recID = (struct recvIDs *)malloc(sizeof(struct recvIDs));
 
    mem_init(sendBuf,recBuf,uAckTable,recID);
    // Set up the signal handler
    if( signal(SIGALRM, signalHandler)  == SIG_ERR)
    {
        perror("Unable to catch SIGALRM");
        exit(1);
    }

    // Set up timer
    struct itimerval *timer = (struct itimerval *)malloc(sizeof(struct itimerval));
    timer->it_value.tv_sec = INTERVAL/1000;
    timer->it_value.tv_usec = 0;
    timer->it_interval.tv_sec = INTERVAL/1000;
    timer->it_interval.tv_usec = 0;
    if( setitimer(ITIMER_REAL, timer, NULL) < 0)
    {
        perror("error calling setitimer()");
        exit(1);
    }
    
    return 0;
}

// Initialise the socket
int r_socket(int domain, int type, int protocol)
{
    if(type == SOCK_MRP)
    {
        socket_fd = socket(domain, SOCK_DGRAM, protocol);
        if(socket_fd >= 0 && init() < 0)
        {
            close(socket_fd);
            return (socket_fd = -1);
        }
        return socket_fd;
    }
    else
    {
        errno = EPROTOTYPE;
        return -1;
    }
            
}


// Send function, put the buf into send buffer as long as there is space
int r_sendto(int sockfd, const void *buf, size_t length, int fl, const struct sockaddr *dest_addr, socklen_t dest_len)
{
    int flag=1;
    if(dest_addr == NULL  ||  length == 0)
        flag=0;
    if(sockfd != socket_fd || buf == NULL)
        flag=0;
    if(!flag)
        return -1;        
    // For the sequence number
    static int pkt_no = 0;
    
    struct data *msg = (struct data *)malloc(sizeof(struct data));
    

    //initialize the Data
    
    msg->msg_len = length;
    msg->msg = malloc(length);
    memcpy(msg->msg, buf, length);

    struct sendPkt *packet = (struct sendPkt *)malloc(sizeof(struct sendPkt));
   
   // initialize theSendPacket
    packet->to = *((const struct sockaddr_in *)dest_addr);
    packet->type = appFlag;
    packet->seqNo = pkt_no;
    packet->d = *msg;

    pkt_no += 1;

    // While the buffer is full
    while( !((*sendBuf).size - MAXTABLES ));
    
    editSendBuff(0,packet,NULL,NULL);

    return 0;


}

ssize_t r_recvfrom(int sockfd,  void *  buffer, size_t length, int fl, struct sockaddr *  addr, socklen_t *  addr_size)
{

    int ret=0;
    if(buffer == NULL) 
        ret++; 
    if( sockfd != socket_fd) 
        ret++;
    if( length==0) 
        ret++;
    if(ret)
    {
        return -1;
    }
    

    // While the receive buffer is empty
    while((recBuf->size)==0);
    
    struct rcvPkt *pack;
    pack = (struct rcvPkt *)malloc(sizeof(struct rcvPkt));
    editRecvBuff(1,NULL,&pack,NULL);

    // Check for buf overflow
    int lent = max(pack->d.msg_len, length);
    memcpy(buffer, pack->d.msg, lent);
    
    if(addr != NULL)
    {
        memcpy(addr, &((*pack).from), sizeof((*pack).from));
    }
    if(addr_size != NULL)
    {
        *addr_size = sizeof((*pack).from);
    }

    lent = max(pack->d.msg_len, length);

    return lent;
}

// Bind the socket
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t address_len)
{
    return bind(sockfd, addr, address_len);
}
