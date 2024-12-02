#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "lab10.h"


extern char * dishes[];

static int served_customers;

static int customer_waiting = -1;
static int customer_seated = -1;
static int customer_order = -1;
static int service_done = -1;

pthread_mutex_t lock;
pthread_cond_t cond; 

void initialize(void)
{
    // add initializations here

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
}


void customer(int id, int dish)
{
    // signal customer ready to be served
    ready_to_seat_customer(id);

    // TODO 1: try to get ownership of the lock
    customer_waiting = id;
    pthread_mutex_lock(&lock);

    // TODO 2: Signal to the waiter that customer is waiting
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    // TODO: 4 Wait to be seated
    while (customer_seated == -1)
        pthread_cond_wait(&cond, &lock);
    customer_seated = -1;

    pthread_mutex_unlock(&lock);
    
    // TODO: 6 place order and signal waiter
    pthread_mutex_lock(&lock);
    customer_order = dish;

    order_placed_by_customer(id, dish);
    pthread_cond_signal(&cond);

    while (service_done == -1)
        pthread_cond_wait(&cond, &lock);
    service_done = -1; 
    
    pthread_mutex_unlock(&lock);

    service_completed_for_customer(id);

}


void waiter(void)
{
    if (served_customers >= MAX_CUSTOMERS)
        return;

    // seat waiting customer

    // TODO 3: replace busy loop with predicate/cond_wait construct
    while (customer_waiting == -1)
        pthread_cond_wait(&cond, &lock);
    
    int customer = customer_waiting;
    customer_waiting = -1;
    
    pthread_mutex_unlock(&lock);

    // TODO 5: seat the customer and then signal the customer thread
    seat_customer(customer);
    customer_seated = 1;
    pthread_cond_signal(&cond);

    // TODO 6: wait for customer to place order then determine the order number
    while(customer_order == -1)
        pthread_cond_wait(&cond, &lock);

    int dish = customer_order;
    customer_order = -1;

    bring_food_to_customer(customer, dish); // second arg should be zero in starter
    service_done = 1;
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&lock);

    served_customers++;
}
