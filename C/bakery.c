#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

// --- Configuration ---
#define BAKERY_CAPACITY 25 // The at-a-time capacity of the bakery
#define NUM_CHEFS 4
#define SOFA_CAPACITY 4

// --- Data Structures ---
typedef enum CustomerState { 
    ARRIVED,
    WAITING_FOR_CAKE,
    WAITING_TO_PAY,
    PAID,
    LEFT
} CustomerState;

typedef struct {
    int id;
    int arrival_time;
    CustomerState state;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} CustomerInfo;

typedef struct {
    int id;
    pthread_t thread;
} ChefInfo;

// A dynamic buffer size for the queues, should be larger than BAKERY_CAPACITY
#define QUEUE_BUFFER_SIZE 100
typedef struct {
    CustomerInfo* buffer[QUEUE_BUFFER_SIZE];
    int head;
    int tail;
    int count;
} CustomerQueue;

// NEW: A linked list to track all created threads for joining later.
typedef struct ThreadNode {
    pthread_t thread_id;
    struct ThreadNode* next;
} ThreadNode;

// --- Shared State & Global Variables ---
typedef struct {
    int customers_in_office;
    int sofa_seats_occupied;
    int all_customers_arrived;
    CustomerQueue waiting_for_cake_q;
    CustomerQueue ready_to_pay_q;
    CustomerQueue sofa_waiting_q;
    pthread_cond_t g_work_available_cond;
    pthread_cond_t g_sofa_seat_free_cond;
} SharedBakeryState;

SharedBakeryState g_bakery;
pthread_mutex_t g_bakery_mutex;
struct timeval g_start_time;
ThreadNode* g_customer_thread_list = NULL;

// --- Utility Functions ---
void queue_init(CustomerQueue* q) { q->head = 0; q->tail = 0; q->count = 0; }
void enqueue(CustomerQueue* q, CustomerInfo* customer) {
    if (q->count < QUEUE_BUFFER_SIZE) {
        q->buffer[q->tail] = customer;
        q->tail = (q->tail + 1) % QUEUE_BUFFER_SIZE;
        q->count++;
    }
}
CustomerInfo* dequeue(CustomerQueue* q) {
    if (q->count > 0) {
        CustomerInfo* customer = q->buffer[q->head];
        q->head = (q->head + 1) % QUEUE_BUFFER_SIZE;
        q->count--;
        return customer;
    }
    return NULL;
}
long get_timestamp() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec - g_start_time.tv_sec);
}

// --- Customer Action Functions ---
int enterOfficeBakery(CustomerInfo *customer) {
    // arrive at specified time
    sleep(customer->arrival_time);

    // attempt to enter
    pthread_mutex_lock(&g_bakery_mutex);
    if (g_bakery.customers_in_office >= BAKERY_CAPACITY) {
        printf("%ld Customer %d cannot enter (full), leaves immediately.\n", get_timestamp(), customer->id);
        pthread_mutex_unlock(&g_bakery_mutex);
        return -1;
    }

    // enter, sleep
    g_bakery.customers_in_office++;
    printf("%ld Customer %d enterofficebakery\n", get_timestamp(), customer->id);
    enqueue(&g_bakery.sofa_waiting_q, customer);    // get in the sofa waiting queue.
    pthread_mutex_unlock(&g_bakery_mutex);
    sleep(1);
    return 0;
}

void sitOnSofa(CustomerInfo *customer) {
    // queue for sofa
    pthread_mutex_lock(&g_bakery_mutex);
    
    while (g_bakery.sofa_seats_occupied >= SOFA_CAPACITY || g_bakery.sofa_waiting_q.buffer[g_bakery.sofa_waiting_q.head] != customer) {
        pthread_cond_wait(&g_bakery.g_sofa_seat_free_cond, &g_bakery_mutex);
    }
    dequeue(&g_bakery.sofa_waiting_q);
    
    // sit as soon as sofa seat available
    g_bakery.sofa_seats_occupied++;
    pthread_mutex_unlock(&g_bakery_mutex);
    sleep(1);
    printf("%ld Customer %d sitOnSofa\n", get_timestamp(), customer->id);
}

/*
void getCake(CustomerInfo *customer) {
    sleep(1);

    pthread_mutex_lock(&g_bakery_mutex);
    enqueue(&g_bakery.waiting_for_cake_q, customer);
    customer->state = WAITING_FOR_CAKE;
    
    // THE FIX: Wake up ALL sleeping chefs, not just one.
    pthread_cond_broadcast(&g_bakery.g_work_available_cond);
    
    printf("%ld Customer %d getcake\n", get_timestamp(), customer->id);
    pthread_mutex_unlock(&g_bakery_mutex);

    pthread_mutex_lock(&customer->mutex);
    while (customer->state != WAITING_TO_PAY) {
        pthread_cond_wait(&customer->cond, &customer->mutex);
    }
    pthread_mutex_unlock(&customer->mutex);
}

void pay(CustomerInfo *customer) {
    sleep(1);

    pthread_mutex_lock(&g_bakery_mutex);
    printf("%ld Customer %d pay\n", get_timestamp(), customer->id);
    enqueue(&g_bakery.ready_to_pay_q, customer);

    // THE FIX: Wake up ALL sleeping chefs, not just one.
    pthread_cond_broadcast(&g_bakery.g_work_available_cond);

    pthread_mutex_unlock(&g_bakery_mutex);
    
    pthread_mutex_lock(&customer->mutex);
    while (customer->state != PAID) {
        pthread_cond_wait(&customer->cond, &customer->mutex);
    }
    pthread_mutex_unlock(&customer->mutex);
}
    */

void getCake(CustomerInfo *customer) {
    sleep(1);

    // queue for cake after sitting
    pthread_mutex_lock(&g_bakery_mutex);
    enqueue(&g_bakery.waiting_for_cake_q, customer);
    customer->state = WAITING_FOR_CAKE;
    pthread_cond_signal(&g_bakery.g_work_available_cond);
    printf("%ld Customer %d getcake\n", get_timestamp(), customer->id);
    pthread_mutex_unlock(&g_bakery_mutex);

    // wait till you get cake
    pthread_mutex_lock(&customer->mutex);
    while (customer->state != WAITING_TO_PAY) {
        pthread_cond_wait(&customer->cond, &customer->mutex);
    }
    pthread_mutex_unlock(&customer->mutex);
}

void pay(CustomerInfo *customer) {
    sleep(1);

    // attempt to pay
    pthread_mutex_lock(&g_bakery_mutex);
    printf("%ld Customer %d pay\n", get_timestamp(), customer->id);
    enqueue(&g_bakery.ready_to_pay_q, customer);
    pthread_cond_signal(&g_bakery.g_work_available_cond);
    pthread_mutex_unlock(&g_bakery_mutex);
    
    // wait till payment accepted
    pthread_mutex_lock(&customer->mutex);
    while (customer->state != PAID) {
        pthread_cond_wait(&customer->cond, &customer->mutex);
    }
    pthread_mutex_unlock(&customer->mutex);
}

void customerLeave(CustomerInfo *customer) {
    pthread_mutex_lock(&g_bakery_mutex);
    g_bakery.customers_in_office--;
    g_bakery.sofa_seats_occupied--; // THIS IS THE FIX: The sofa seat is now free.
    customer->state = LEFT;
    pthread_cond_broadcast(&g_bakery.g_sofa_seat_free_cond);
    printf("%ld Customer %d leaves\n", get_timestamp(), customer->id);
    pthread_mutex_unlock(&g_bakery_mutex);
}

void* customer_thread(void* arg) {
    CustomerInfo* customer = (CustomerInfo*)arg;

    if (enterOfficeBakery(customer) < 0) {
        // If customer fails to enter, free their info struct and exit.
        pthread_mutex_destroy(&customer->mutex);
        pthread_cond_destroy(&customer->cond);
        free(customer);
        return NULL;
    }
    sitOnSofa(customer);
    getCake(customer);
    pay(customer);

    customerLeave(customer);
    // Once done, free the memory.
    pthread_mutex_destroy(&customer->mutex);
    pthread_cond_destroy(&customer->cond);
    free(customer);
    return NULL;
}

// --- Chef Thread Functions ---

void bakecake(ChefInfo* chef, CustomerInfo* customer_to_serve) {
    printf("%ld Chef %d bakecake for Customer %d\n", get_timestamp(), chef->id, customer_to_serve->id);

    // bake the cake
    sleep(2);
    // FIX: A chef is now free. Signal the pool of waiting chefs.
    pthread_mutex_lock(&g_bakery_mutex);
    pthread_cond_signal(&g_bakery.g_work_available_cond);
    pthread_mutex_unlock(&g_bakery_mutex);

    // grant the cake to customer
    pthread_mutex_lock(&customer_to_serve->mutex);
    customer_to_serve->state = WAITING_TO_PAY;
    pthread_cond_signal(&customer_to_serve->cond);
    pthread_mutex_unlock(&customer_to_serve->mutex);
}

void acceptPayment(ChefInfo* chef, CustomerInfo* customer_to_serve) {
    printf("%ld Chef %d acceptPayment for Customer %d\n", get_timestamp(), chef->id, customer_to_serve->id);

    // accpet the payment
    sleep(2);
    pthread_mutex_lock(&g_bakery_mutex);
    pthread_cond_signal(&g_bakery.g_work_available_cond);
    pthread_mutex_unlock(&g_bakery_mutex);

    pthread_mutex_lock(&customer_to_serve->mutex);
    customer_to_serve->state = PAID;
    pthread_cond_signal(&customer_to_serve->cond);
    pthread_mutex_unlock(&customer_to_serve->mutex);
}

// ...existing code...
void* chef_thread(void* arg) {
    ChefInfo* chef = (ChefInfo*)arg;
    CustomerInfo* customer_to_serve = NULL;
    int is_baking_task;
    while (1) {

        pthread_mutex_lock(&g_bakery_mutex);
        // chef idling loop
        while (g_bakery.ready_to_pay_q.count == 0 && g_bakery.waiting_for_cake_q.count == 0) {
            // Only exit if: no more customer threads will be created AND
            // there are no customers currently in the office AND both queues empty.
            if (g_bakery.all_customers_arrived
                && g_bakery.waiting_for_cake_q.count == 0
                && g_bakery.ready_to_pay_q.count == 0
                && g_bakery.customers_in_office == 0) {
                pthread_mutex_unlock(&g_bakery_mutex);
                return NULL;
            }
            pthread_cond_wait(&g_bakery.g_work_available_cond, &g_bakery_mutex);
        }

        if (g_bakery.ready_to_pay_q.count > 0) {
            customer_to_serve = dequeue(&g_bakery.ready_to_pay_q);
            is_baking_task = 0;
        } else if (g_bakery.waiting_for_cake_q.count > 0) {
            customer_to_serve = dequeue(&g_bakery.waiting_for_cake_q);
            is_baking_task = 1;
        } else {
            pthread_mutex_unlock(&g_bakery_mutex);
            continue;
        }
        pthread_mutex_unlock(&g_bakery_mutex);
        if (is_baking_task) {
            bakecake(chef, customer_to_serve);
        } else {
            acceptPayment(chef, customer_to_serve);
        }
    }
    return NULL;
}
// ...existing code...
/*
void* chef_thread(void* arg) {
    ChefInfo* chef = (ChefInfo*)arg;
    CustomerInfo* customer_to_serve = NULL;
    int is_baking_task;
    while (1) {

        pthread_mutex_lock(&g_bakery_mutex);
        // chef idling loop
        while (g_bakery.ready_to_pay_q.count == 0 && g_bakery.waiting_for_cake_q.count == 0) {
            if (g_bakery.all_customers_arrived && g_bakery.waiting_for_cake_q.count == 0 && g_bakery.ready_to_pay_q.count == 0) {
                pthread_mutex_unlock(&g_bakery_mutex);
                return NULL;
            }
            pthread_cond_wait(&g_bakery.g_work_available_cond, &g_bakery_mutex);
        }

        if (g_bakery.ready_to_pay_q.count > 0) {
            customer_to_serve = dequeue(&g_bakery.ready_to_pay_q);
            is_baking_task = 0;
        } else if (g_bakery.waiting_for_cake_q.count > 0) {
            customer_to_serve = dequeue(&g_bakery.waiting_for_cake_q);
            is_baking_task = 1;
        } else {
            pthread_mutex_unlock(&g_bakery_mutex);
            continue;
        }
        pthread_mutex_unlock(&g_bakery_mutex);
        if (is_baking_task) {
            bakecake(chef, customer_to_serve);
        } else {
            acceptPayment(chef, customer_to_serve);
        }
    }
    return NULL;
}
    */

// --- Init, Destruct, and Main ---
void bakeryInit() {
    gettimeofday(&g_start_time, NULL);
    g_bakery.customers_in_office = 0;
    g_bakery.sofa_seats_occupied = 0;
    g_bakery.all_customers_arrived = 0;
    queue_init(&g_bakery.waiting_for_cake_q);
    queue_init(&g_bakery.ready_to_pay_q);
    queue_init(&g_bakery.sofa_waiting_q);
    pthread_mutex_init(&g_bakery_mutex, NULL);
    pthread_cond_init(&g_bakery.g_work_available_cond, NULL);
    pthread_cond_init(&g_bakery.g_sofa_seat_free_cond, NULL);
}

void join_customers() {
    ThreadNode* current = g_customer_thread_list;
    while (current != NULL) {
        pthread_join(current->thread_id, NULL);
        ThreadNode* temp = current;
        current = current->next;
        free(temp);
    }
    g_customer_thread_list = NULL;
}

void bakeryDestruct() {
    // Traverse the linked list to join all threads and free the nodes.
    ThreadNode* current = g_customer_thread_list;
    while (current != NULL) {
        pthread_join(current->thread_id, NULL);
        ThreadNode* temp = current;
        current = current->next;
        free(temp);
    }
    pthread_mutex_destroy(&g_bakery_mutex);
    pthread_cond_destroy(&g_bakery.g_work_available_cond);
    pthread_cond_destroy(&g_bakery.g_sofa_seat_free_cond);
}

int main() {
    char *input_file = "input_file";
    FILE *fptr = fopen(input_file, "r");

    bakeryInit();
    ChefInfo chefs[NUM_CHEFS];
    for (int i = 0; i < NUM_CHEFS; ++i) {
        chefs[i].id = i + 1;
        pthread_create(&chefs[i].thread, NULL, chef_thread, &chefs[i]);
    }
    int time, id;
    while (fscanf(fptr, "%d Customer %d", &time, &id) == 2) {
        CustomerInfo* new_customer = (CustomerInfo*)malloc(sizeof(CustomerInfo));
        new_customer->id = id;
        new_customer->arrival_time = time;
        new_customer->state = ARRIVED;
        pthread_mutex_init(&new_customer->mutex, NULL);
        pthread_cond_init(&new_customer->cond, NULL);
        
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, customer_thread, new_customer);
        
        // Add the new thread to the linked list for later joining.
        ThreadNode* new_node = (ThreadNode*)malloc(sizeof(ThreadNode));
        new_node->thread_id = thread_id;
        new_node->next = g_customer_thread_list;
        g_customer_thread_list = new_node;
    }
    printf("scan loop exited\n");
    
    // let customers finish
    join_customers();
    
    // no more customers coming, finish remaining jobs
    sleep(1);
    pthread_mutex_lock(&g_bakery_mutex);
    g_bakery.all_customers_arrived = 1;
    pthread_cond_broadcast(&g_bakery.g_work_available_cond);
    pthread_mutex_unlock(&g_bakery_mutex);
    
    // Join chef threads
    for (int i = 0; i < NUM_CHEFS; ++i) {
        pthread_join(chefs[i].thread, NULL);
    }

    // Join all customer threads and clean up resources
    bakeryDestruct();
    
    printf("\nAll customers served. Bakery is closing.\n");
    return 0;
}