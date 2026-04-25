#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *items;
    int capacity;
    int count;
    int head;
    int tail;
    int producers_done;
    int producers_expected;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} Buffer;

typedef struct {
    Buffer *buffer;
    int identifier;
    int item_count;
} ProducerArgument;

typedef struct {
    Buffer *buffer;
    int identifier;
    int consumed;
    int sum;
} ConsumerArgument;

static void *producer(void *argument)
{
    ProducerArgument *producer_argument = argument;
    Buffer *buffer = producer_argument->buffer;

    pc_seed();
    for (int index = 0; index < producer_argument->item_count; index++) {
        int value = producer_argument->identifier * 1000 + index;
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->count == buffer->capacity) {
            pthread_cond_wait(&buffer->not_full, &buffer->mutex);
        }
        buffer->items[buffer->tail] = value;
        buffer->tail = (buffer->tail + 1) % buffer->capacity;
        buffer->count++;
        printf("Producteur %d: dépose %d\n", producer_argument->identifier, value);
        pthread_cond_signal(&buffer->not_empty);
        pthread_mutex_unlock(&buffer->mutex);
    }
    pthread_mutex_lock(&buffer->mutex);
    buffer->producers_done++;
    printf("Producteur %d: terminé (%d objets)\n", producer_argument->identifier,
           producer_argument->item_count);
    pthread_cond_broadcast(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
    return NULL;
}

static void *consumer(void *argument)
{
    ConsumerArgument *consumer_argument = argument;
    Buffer *buffer = consumer_argument->buffer;

    pc_seed();
    for (;;) {
        int value;
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->count == 0 && buffer->producers_done < buffer->producers_expected) {
            pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
        }
        if (buffer->count == 0) {
            pthread_mutex_unlock(&buffer->mutex);
            break;
        }
        value = buffer->items[buffer->head];
        buffer->head = (buffer->head + 1) % buffer->capacity;
        buffer->count--;
        printf("Consommateur %d: récupère %d\n", consumer_argument->identifier, value);
        pthread_cond_signal(&buffer->not_full);
        pthread_mutex_unlock(&buffer->mutex);
        consumer_argument->consumed++;
        consumer_argument->sum += value;
    }
    printf("Consommateur %d: terminé (%d objets, somme %d)\n",
           consumer_argument->identifier, consumer_argument->consumed,
           consumer_argument->sum);
    return NULL;
}

int main(int argc, char **argv)
{
    int producer_count = 2;
    int consumer_count = 2;
    int capacity = 3;
    int item_count = 10;
    Buffer buffer = {0};
    pthread_t *producers;
    pthread_t *consumers;
    ProducerArgument *producer_arguments;
    ConsumerArgument *consumer_arguments;
    int total_consumed = 0;

    if (argc >= 2 && (!parse_int(argv[1], &producer_count) || producer_count < 1)) {
        fprintf(stderr, "usage: %s [producers] [consumers] [capacity] [items]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 3 && (!parse_int(argv[2], &consumer_count) || consumer_count < 1)) {
        fprintf(stderr, "usage: %s [producers] [consumers] [capacity] [items]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 4 && (!parse_int(argv[3], &capacity) || capacity < 1)) {
        fprintf(stderr, "usage: %s [producers] [consumers] [capacity] [items]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 5 && (!parse_int(argv[4], &item_count) || item_count < 0)) {
        fprintf(stderr, "usage: %s [producers] [consumers] [capacity] [items]\n", argv[0]);
        return EXIT_FAILURE;
    }

    buffer.items = xmalloc((size_t)capacity * sizeof(*buffer.items));
    buffer.capacity = capacity;
    buffer.producers_expected = producer_count;
    pthread_mutex_init(&buffer.mutex, NULL);
    pthread_cond_init(&buffer.not_empty, NULL);
    pthread_cond_init(&buffer.not_full, NULL);

    producers = xmalloc((size_t)producer_count * sizeof(*producers));
    consumers = xmalloc((size_t)consumer_count * sizeof(*consumers));
    producer_arguments = xmalloc((size_t)producer_count * sizeof(*producer_arguments));
    consumer_arguments = xmalloc((size_t)consumer_count * sizeof(*consumer_arguments));

    for (int index = 0; index < producer_count; index++) {
        producer_arguments[index] = (ProducerArgument){&buffer, index, item_count};
        pthread_create(&producers[index], NULL, producer, &producer_arguments[index]);
    }
    for (int index = 0; index < consumer_count; index++) {
        consumer_arguments[index] = (ConsumerArgument){&buffer, index, 0, 0};
        pthread_create(&consumers[index], NULL, consumer, &consumer_arguments[index]);
    }
    for (int index = 0; index < producer_count; index++) {
        pthread_join(producers[index], NULL);
    }
    for (int index = 0; index < consumer_count; index++) {
        pthread_join(consumers[index], NULL);
        total_consumed += consumer_arguments[index].consumed;
    }
    printf("Total consommé: %d objets (attendu %d)\n", total_consumed,
           producer_count * item_count);

    pthread_cond_destroy(&buffer.not_full);
    pthread_cond_destroy(&buffer.not_empty);
    pthread_mutex_destroy(&buffer.mutex);
    free(consumer_arguments);
    free(producer_arguments);
    free(consumers);
    free(producers);
    free(buffer.items);
    return total_consumed == producer_count * item_count ? EXIT_SUCCESS : EXIT_FAILURE;
}
