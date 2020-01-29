#include <pthread.h> 
#include <semaphore.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h> 

#define max_p 1000 // maximum number of promoters 
#define max_c 1000 // maximum number of clients

struct Queue 
{ 
    int front, rear, size; 
    unsigned capacity; 
    int* array; 
}; 
  
// function to create a queue of given capacity.  
// It initializes size of queue as 0 
struct Queue* createQueue(unsigned capacity) 
{ 
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue)); 
    queue->capacity = capacity; 
    queue->front = queue->size = 0;  
    queue->rear = capacity - 1;  // This is important, see the enqueue 
    queue->array = (int*) malloc(queue->capacity * sizeof(int)); 
    return queue; 
} 
  
// Queue is full when size becomes equal to the capacity  
int isFull(struct Queue* queue) 
{  return (queue->size == queue->capacity);  } 
  
// Queue is empty when size is 0 
int isEmpty(struct Queue* queue) 
{  return (queue->size == 0); } 
  
// Function to add an item to the queue.   
// It changes rear and size 
void enqueue(struct Queue* queue, int item) 
{ 
    if (isFull(queue)) 
        return; 
    queue->rear = (queue->rear + 1)%queue->capacity; 
    queue->array[queue->rear] = item; 
    queue->size = queue->size + 1; 
    //printf("%d enqueued to queue\n", item); 
} 
  
// Function to remove an item from queue.  
// It changes front and size 
int dequeue(struct Queue* queue) 
{ 
    if (isEmpty(queue)) 
        return INT_MIN; 
    int item = queue->array[queue->front]; 
    queue->front = (queue->front + 1)%queue->capacity; 
    queue->size = queue->size - 1; 
    return item; 
} 
  
// Function to get front of queue 
int front(struct Queue* queue) 
{ 
    if (isEmpty(queue)) 
        return INT_MIN; 
    return queue->array[queue->front]; 
} 
  
// Function to get rear of queue 
int rear(struct Queue* queue) 
{ 
    if (isEmpty(queue)) 
        return INT_MIN; 
    return queue->array[queue->rear]; 
} 

struct Queue* wq , *iq ; // waiting queue

int no_p ; // number of promoters
int no_c ; // number of clients
int in_p = 0 ; // number of promoters in the flat
int in_c = 0 ; // number of clients in the flat

int tim = 0 ;
int i ;
int x ; 

sem_t Sc [max_c] ; // semaphore for each client
sem_t mutex, mutex1;
// mutex for mutual exclusion for promoter
// mutex1 for mutual exclusion for client arrival
 
int sizc = 0 ; // size of waiting queue of clients 

// function leave_promoter
void leave_promoter ( int i ) {
	int cnt = 0 ; 
	while ( !isEmpty(wq) && cnt < 10 ) {
		int n = dequeue ( wq ) ;
		sem_post ( &Sc [ n ] ) ; 
		cnt++ ; 
	}
	while ( in_c > 0  ) {

	}
	while ( iq->size)
	dequeue(iq);

	printf("Promoter %d leaves flat at time %d \n" , i , tim);
	tim++;
	in_p = 0 ;
}

// function leave_client
void leave_client( int i ) {
	printf("Client %d leaves flat at time %d \n" , i , tim) ; 
	tim ++ ; 
	in_c--;
	enqueue ( wq , i ) ;
	sem_wait(&Sc[i]);
	sizc++;
}

// function open_flat
void open_flat ( int i ) { //agent
	while ( sizc == 0  ) ;

	in_p = 1 ;
	printf("Promoter %d opens flat at time %d\n", i , tim) ;
	tim++;     
}

// function view_flat
void view_flat ( int i ) { // tenant
      while ( in_p == 0 || iq->size>10) ;
      
      printf("Client %d views flat at time %d \n" , i , tim) ; 
      sizc--;
      enqueue(iq , i ) ;
      in_c++;
      tim++ ;
}

// function promoter_arrives
void* promoter_arrives (void * it ) {
	int i = (long)it;
	sizc++;
	do {
		sem_wait ( &mutex ) ;  
		open_flat(i);
		sleep ( 5 );
		leave_promoter(i) ;
		sem_post ( &mutex ) ; 
	}while ( 1 ) ;
	sleep(0) ; 
}

// function client_arrives
void* client_arrives (void * it ) {
	int i = (long)it ; 
	sizc++;
	do {
		if ( in_p == 0 || in_c > 10 ) {
			enqueue ( wq , i ) ; 
			sem_wait ( &Sc [ i ] ) ;  
		}
		sem_wait ( &mutex1 );
		view_flat( i ) ;
		sleep ( 1 ) ;
		sem_post ( &mutex1 ) ;
		sleep(2);
		leave_client( i );
	} while ( 1 ) ;
}

// main() function 
int main(int argc , char* argv [ ]) {
	wq = createQueue ( 1000 ) ;
	iq = createQueue ( 1000 ) ;
	no_p =  atoi(argv[1]) ; 
	no_c   =  atoi(argv[2]) ;
	
	pthread_t p_id[no_p];
	pthread_t c_id[no_c];

	sem_init(&mutex,0,1);
	sem_init(&mutex1,0,1);

	for (  i = 0 ; i<no_c ; i++ )
		sem_init(&Sc[i],0,0);

	for (i = 0; i < no_p ; i++) { 
		pthread_create(&p_id[i], NULL, 
			      promoter_arrives, (void*)i );
		printf("promoter %d arrives at time %d \n", i, tim );
		tim++ ; 
	} 

	for (i = 0; i < no_c; i++) { 
		pthread_create(&c_id[i], NULL, 
			       client_arrives , (void*)i ); 
		printf("Client %d arrives at time %d \n", i , tim) ;
		tim++ ; 
	}  

	for (i = 0; i < no_c; i++) 
	 	pthread_join(c_id[i], NULL); 
	 
	for (i = 0; i < no_p; i++) 
		pthread_join(p_id[i], NULL); 
}
