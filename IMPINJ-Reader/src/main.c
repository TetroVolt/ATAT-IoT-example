#include "iri.h"
#include "ipj_util.h"
#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>



#include <arpa/inet.h>

#define PORT "3000" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

static ipj_iri_device* iri_device = { 0 };
static ipj_tag_operation_report* tag_operation_report;


typedef struct Queue
{
        int capacity;
        int size;
        int front;
        int rear;
        int *elements;
}Queue;

Queue * createQueue(int maxElements)
{
        /* Create a Queue */
        Queue *Q;
        Q = (Queue *)malloc(sizeof(Queue));
        /* Initialise its properties */
        Q->elements = (int *)malloc(sizeof(int)*maxElements);
        Q->size = 0;
        Q->capacity = maxElements;
        Q->front = 0;
        Q->rear = -1;
        /* Return the pointer */
        return Q;
}

void Dequeue(Queue *Q)
{
        /* If Queue size is zero then it is empty. So we cannot pop */
        if(Q->size==0)
        {
                printf("Queue is Empty\n");
                return;
        }
        /* Removing an element is equivalent to incrementing index of front by one */
        else
        {
                Q->size--;
                Q->front++;
                /* As we fill elements in circular fashion */
                if(Q->front==Q->capacity)
                {
                        Q->front=0;
                }
        }
        return;
}
int front(Queue *Q)
{
        if(Q->size==0)
        {
                printf("Queue is Empty\n");
                exit(0);
        }
        /* Return the element which is at the front*/
        return Q->elements[Q->front];
}
void Enqueue(Queue *Q,int element)
{
        /* If the Queue is full, we cannot push an element into it as there is no space for it.*/
        if(Q->size == Q->capacity)
        {
                printf("Queue is Full\n");
        }
        else
        {
                Q->size++;
                Q->rear = Q->rear + 1;
                /* As we fill the queue in circular fashion */
                if(Q->rear == Q->capacity)
                {
                        Q->rear = 0;
                }
                /* Insert the element in its rear side */ 
                Q->elements[Q->rear] = element;
        }
        return;
}


int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);



static struct ipj_handler event_handlers[] =
{
  /*
    { E_IPJ_HANDLER_TYPE_PLATFORM_OPEN_PORT,        &platform_open_port_handler },
    { E_IPJ_HANDLER_TYPE_PLATFORM_CLOSE_PORT,       &platform_close_port_handler },
    { E_IPJ_HANDLER_TYPE_PLATFORM_TRANSMIT,         &platform_transmit_handler },
    { E_IPJ_HANDLER_TYPE_PLATFORM_RECEIVE,          &platform_receive_handler },
    { E_IPJ_HANDLER_TYPE_PLATFORM_TIMESTAMP,        &platform_timestamp_ms_handler },
    { E_IPJ_HANDLER_TYPE_PLATFORM_SLEEP_MS,         &platform_sleep_ms_handler },
    { E_IPJ_HANDLER_TYPE_PLATFORM_MODIFY_CONNECTION,&platform_modify_connection_handler },
    { E_IPJ_HANDLER_TYPE_PLATFORM_FLUSH_PORT,       &platform_flush_port_handler },
  */
    { E_IPJ_HANDLER_TYPE_REPORT,                    &ipj_util_report_handler },
    { E_IPJ_HANDLER_TYPE_DIAGNOSTIC,                &ipj_util_diagnostic_handler }

};

ipj_error ipj_util_setup(ipj_iri_device* iri_device, char* reader_identifier)
{
    ipj_error error;

    /* Initialize iri_device structure */
    error = ipj_initialize_iri_device(iri_device);
    IPJ_UTIL_RETURN_ON_ERROR(error, "ipj_initialize_iri_device");

    /* Register all handlers */
    error = ipj_util_register_handlers(iri_device);
    IPJ_UTIL_RETURN_ON_ERROR(error, "ipj_util_register_handlers");

    /* Connect to iri device - open serial port */
    error = ipj_connect(
            iri_device,
            reader_identifier,
            E_IPJ_CONNECTION_TYPE_SERIAL,
            NULL);
    IPJ_UTIL_RETURN_ON_ERROR(error, "ipj_connect");

    return error;
}

ipj_error ipj_util_register_handlers(ipj_iri_device* iri_device)
{
    ipj_error error;
    unsigned int i;
    for (i = 0; i < (sizeof(event_handlers) / sizeof(event_handlers[0])); i++)
    {
        error = ipj_register_handler(
                iri_device,
                event_handlers[i].type,
                event_handlers[i].handler);
        if (error)
        {
            return error;
        }
    }
    return E_IPJ_ERROR_SUCCESS;
}

ipj_error ipj_custom_report_handler(
        ipj_iri_device* iri_device,
        ipj_report_id report_id,
        void* report)
{
    ipj_error error = E_IPJ_ERROR_SUCCESS;
    /* Case statement for each report type */
    switch (report_id)
    {
        case E_IPJ_REPORT_ID_TAG_OPERATION_REPORT:
            error = ipj_util_tag_operation_report_handler(
                    iri_device,
                    (ipj_tag_operation_report*) report);
            break;
        default:
            printf(
                    "%s: REPORT ID: %d NOT HANDLED\n",
                    (char*) iri_device->reader_identifier,
                    report_id);
            error = E_IPJ_ERROR_GENERAL_ERROR;
            break;
    }
    return error;
}

ipj_error ipj_util_tag_operation_report_handler(
        ipj_iri_device* iri_device,
        ipj_tag_operation_report* tag_operation_report)
{
    char* readerID = "";
    uint32_t i;

    if (tag_operation_report->has_error && tag_operation_report->error > 0)
    {
        IPJ_UTIL_PRINT_ERROR(tag_operation_report->error,"tag_operation_report");
    }

    /* If tag report has epc, print epc */
    if (tag_operation_report->tag.has_epc)
    {
        /* Print reader identifier */
        readerID = (char*) iri_device->reader_identifier;

    }
	
    /* If tag report has antenna, print antenna */
	if (tag_operation_report->tag.has_antenna)
	{
		/* Print antenna */
		printf("%s: Antenna = ", (char*)iri_device->reader_identifier);

		printf("%d\r\n", tag_operation_report->tag.antenna);
	}

    if (tag_operation_report->has_diagnostic)
    {
        printf("Diagnostic = 0x%08X\n", tag_operation_report->diagnostic);
    }

    if (tag_operation_report->has_tag_operation_type)
    {
        printf("Tag operation: ");
        switch (tag_operation_report->tag_operation_type)
        {
            case E_IPJ_TAG_OPERATION_TYPE_READ:
            {
                printf("READ\n");
                break;
            }
            case E_IPJ_TAG_OPERATION_TYPE_WRITE:
            {
                printf("WRITE\n");
                break;
            }
            case E_IPJ_TAG_OPERATION_TYPE_LOCK:
            {
                printf("LOCK\n");
                break;
            }
            case E_IPJ_TAG_OPERATION_TYPE_KILL:
            {
                printf("KILL\n");
                break;
            }
            case E_IPJ_TAG_OPERATION_TYPE_BLOCKPERMALOCK:
            {
                printf("BLOCK PERMALOCK\n");
                break;
            }
            case E_IPJ_TAG_OPERATION_TYPE_WRITE_EPC:
            {
                printf("WRITE EPC\n");
                break;
            }
            case E_IPJ_TAG_OPERATION_TYPE_QT:
            {
                printf("QT\n");
                break;
            }
            default:
            {
                printf("TYPE=%d\n", tag_operation_report->tag_operation_type);
            }
        }

        if (tag_operation_report->has_tag_operation_data)
        {
            printf(
                    "Report contains data: %d bytes\n",
                    (int) tag_operation_report->tag_operation_data.size);
            for (i = 0; i < tag_operation_report->tag_operation_data.size; i += 2)
            {
                if (i % (2 * 6) == 0 && i > 0)
                {
                    printf("\n");
                }
                printf(
                        "0x%04x|",
                        tag_operation_report->tag_operation_data.bytes[i] << 8
                                | tag_operation_report->tag_operation_data.bytes[i
                                        + 1]);
            }
        }
        printf("\n");

    }

    ipj_util_print_divider('-', 80);
    return E_IPJ_ERROR_SUCCESS;
}


void dataHandeler()
{
  ipj_util_tag_operation_report_handler(iri_device, tag_operation_report);

  
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);

    return 0;
}




/*
int main(void)
{
  CURL *curl;
  CURLcode res;
 
  // In windows, this will init the winsock stuff 
  curl_global_init(CURL_GLOBAL_ALL);
 
  // get a curl handle  
  curl = curl_easy_init();
  if(curl) {
    // First set the URL that is about to receive our POST. This URL can
      // just as well be a https:// URL if that is what should receive the
       //data. 
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3000/");
    
    // Perform the request, res will get the return code 
    res = curl_easy_perform(curl);
    
    // Check for errors 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    // always cleanup  
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}
*/