#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "pizza.h"

// Δομή που περιέχει πληροφορίες για κάθε παραγγελία
typedef struct {
    int id;						// Αριθμός παραγγελίας
    int num_pizzas;				// Αριθμός πίτσων στην παραγγελία
    int* pizza_types;			// Τύποι πίτσων στην παραγγελία
    struct timespec order_time;	// Χρονική στιγμή καταχώρησης παραγγελίας
} Order;

// Δηλώσεις mutex και condition variables για συγχρονισμό των πόρων
pthread_mutex_t mutex_tel, mutex_cook, mutex_oven, mutex_deliver, mutex_print, mutex_stats;
pthread_cond_t cond_tel, cond_cook, cond_oven, cond_deliver;

// Διαθέσιμοι πόροι
int available_tel = Ntel;
int available_cook = Ncook;
int available_oven = Noven;
int available_deliver = Ndeliverer;

// Μεταβλητές στατιστικών και εσόδων
int total_revenue = 0;
int total_orders = 0;
int failed_orders = 0;
int pizza_sales[3] = {0};

// Χρόνοι εξυπηρέτησης και κρυώματος
time_t max_service_time = 0;
time_t max_cooling_time = 0;
time_t total_service_time = 0;
time_t total_cooling_time = 0;

// Συνάρτηση για ενημέρωση του μέγιστου χρόνου
void update_max_time(time_t *max, time_t current) {
    if (current > *max) {
        *max = current;
    }
}

// Συνάρτηση για άθροισμα δύο timespec
void add_timespec(struct timespec *result, struct timespec *a, struct timespec *b) {
    result->tv_sec = a->tv_sec + b->tv_sec;
    result->tv_nsec = a->tv_nsec + b->tv_nsec;
    if (result->tv_nsec >= 1000000000) {
        result->tv_sec++;
        result->tv_nsec -= 1000000000;
    }
}

// Συνάρτηση για διαφορά δύο timespec
void subtract_timespec(struct timespec *result, struct timespec *a, struct timespec *b) {
    result->tv_sec = a->tv_sec - b->tv_sec;
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (result->tv_nsec < 0) {
        result->tv_sec--;
        result->tv_nsec += 1000000000;
    }
}

// Συνάρτηση που διαχειρίζεται κάθε παραγγελία
void* handle_order(void* arg) {
    Order* order = (Order*)arg;
    int id = order->id;
    int num_pizzas = order->num_pizzas;
    int* pizza_types = order->pizza_types;
    struct timespec start_cooling, end_cooling, end_time;
    struct timespec service_time, cooling_time;

    unsigned int seed = order->id;

    // Τηλεφωνητές
    pthread_mutex_lock(&mutex_tel);
    while (available_tel == 0) {
        pthread_cond_wait(&cond_tel, &mutex_tel);
    }
    available_tel--;
    pthread_mutex_unlock(&mutex_tel);

    // Προσομοίωση χρόνου τηλεφωνητή
    sleep(rand_r(&seed) % (Tpaymenthigh - Tpaymentlow + 1) + Tpaymentlow);

	// Έλεγχος για αποτυχία παραγγελίας
    if ((rand_r(&seed) % 100) < (Pfail * 100)) {
        pthread_mutex_lock(&mutex_print);
        printf("Η παραγγελία με αριθμό %d απέτυχε.\n", id);
        pthread_mutex_unlock(&mutex_print);

        pthread_mutex_lock(&mutex_stats);
        failed_orders++;
        pthread_mutex_unlock(&mutex_stats);

        pthread_mutex_lock(&mutex_tel);
        available_tel++;
        pthread_cond_signal(&cond_tel);
        pthread_mutex_unlock(&mutex_tel);
        free(pizza_types);
        free(order);
        return NULL;
    }

    pthread_mutex_lock(&mutex_print);
    printf("Η παραγγελία με αριθμό %d καταχωρήθηκε.\n", id);
    pthread_mutex_unlock(&mutex_print);

    pthread_mutex_lock(&mutex_tel);
    available_tel++;
    pthread_cond_signal(&cond_tel);
    pthread_mutex_unlock(&mutex_tel);

    // Παρασκευαστές
    pthread_mutex_lock(&mutex_cook);
    while (available_cook == 0) {
        pthread_cond_wait(&cond_cook, &mutex_cook);
    }
    available_cook--;
    pthread_mutex_unlock(&mutex_cook);
	
	// Προσομοίωση χρόνου προετοιμασίας
    sleep(Tprep * num_pizzas);

    pthread_mutex_lock(&mutex_cook);
    available_cook++;
    pthread_cond_signal(&cond_cook);
    pthread_mutex_unlock(&mutex_cook);

    // Φούρνοι
    pthread_mutex_lock(&mutex_oven);
    while (available_oven < num_pizzas) {
        pthread_cond_wait(&cond_oven, &mutex_oven);
    }
    available_oven -= num_pizzas;
    pthread_mutex_unlock(&mutex_oven);
	
	// Προσομοίωση χρόνου ψησίματος
    sleep(Tbake);

    pthread_mutex_lock(&mutex_oven);
    available_oven += num_pizzas;
    pthread_cond_signal(&cond_oven);
    pthread_mutex_unlock(&mutex_oven);

    clock_gettime(CLOCK_REALTIME, &start_cooling);

    // Διανομείς
    pthread_mutex_lock(&mutex_deliver);
    while (available_deliver == 0) {
        pthread_cond_wait(&cond_deliver, &mutex_deliver);
    }
    available_deliver--;
    pthread_mutex_unlock(&mutex_deliver);
	
	// Προσομοίωση χρόνου πακεταρίσματος
    sleep(Tpack);

    clock_gettime(CLOCK_REALTIME, &end_cooling);

    pthread_mutex_lock(&mutex_print);
    struct timespec prep_time;
    subtract_timespec(&prep_time, &end_cooling, &order->order_time);
    printf("Η παραγγελία με αριθμό %d ετοιμάστηκε σε %ld λεπτά.\n", id, prep_time.tv_sec);
    pthread_mutex_unlock(&mutex_print);
	
	// Προσομοίωση χρόνου παράδοσης
    sleep(rand_r(&seed) % (Tdelhigh - Tdellow + 1) + Tdellow);

    clock_gettime(CLOCK_REALTIME, &end_time);
    subtract_timespec(&service_time, &end_time, &order->order_time);
    subtract_timespec(&cooling_time, &end_time, &start_cooling);

    pthread_mutex_lock(&mutex_print);
    printf("Η παραγγελία με αριθμό %d παραδόθηκε σε %ld λεπτά.\n", id, service_time.tv_sec);
    pthread_mutex_unlock(&mutex_print);
	
	// Ενημέρωση στατιστικών δεδομένων
    pthread_mutex_lock(&mutex_stats);
    for (int i = 0; i < num_pizzas; i++) {
        pizza_sales[pizza_types[i]]++;
        if (pizza_types[i] == 0) total_revenue += Cm;
        if (pizza_types[i] == 1) total_revenue += Cp;
        if (pizza_types[i] == 2) total_revenue += Cs;
    }
    total_orders++;
    total_service_time += service_time.tv_sec;
    total_cooling_time += cooling_time.tv_sec;
    update_max_time(&max_service_time, service_time.tv_sec);
    update_max_time(&max_cooling_time, cooling_time.tv_sec);
    pthread_mutex_unlock(&mutex_stats);

    pthread_mutex_lock(&mutex_deliver);
    available_deliver++;
    pthread_cond_signal(&cond_deliver);
    pthread_mutex_unlock(&mutex_deliver);

    free(pizza_types);
    free(order);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Ncust> <Seed>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int Ncust = atoi(argv[1]);
    unsigned int Seed = (unsigned int)atoi(argv[2]);
    srand(Seed);

    pthread_t threads[Ncust];
	
	// Αρχικοποίηση μεταβλητών συγχρονισμού
    pthread_mutex_init(&mutex_tel, NULL);
    pthread_mutex_init(&mutex_cook, NULL);
    pthread_mutex_init(&mutex_oven, NULL);
    pthread_mutex_init(&mutex_deliver, NULL);
    pthread_mutex_init(&mutex_print, NULL);
    pthread_mutex_init(&mutex_stats, NULL);

    pthread_cond_init(&cond_tel, NULL);
    pthread_cond_init(&cond_cook, NULL);
    pthread_cond_init(&cond_oven, NULL);
    pthread_cond_init(&cond_deliver, NULL);

    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
	
	// Δημιουργία και διαχείριση νημάτων παραγγελιών
    for (int i = 0; i < Ncust; i++) {
        Order* order = malloc(sizeof(Order));
        order->id = i + 1;
        order->num_pizzas = rand_r(&Seed) % (Norderhigh - Norderlow + 1) + Norderlow;
        order->pizza_types = malloc(order->num_pizzas * sizeof(int));
        clock_gettime(CLOCK_REALTIME, &order->order_time);

        for (int j = 0; j < order->num_pizzas; j++) {
            double p = (double)rand_r(&Seed) / RAND_MAX;
            if (p < Pm) {
                order->pizza_types[j] = 0; // μαργαρίτα
            } else if (p < Pm + Pp) {
                order->pizza_types[j] = 1; // πεπερόνι
            } else {
                order->pizza_types[j] = 2; // σπέσιαλ
            }
        }
        pthread_create(&threads[i], NULL, handle_order, order);
        sleep(rand_r(&Seed) % (Torderhigh - Torderlow + 1) + Torderlow);
    }
	
	// Αναμονή ολοκλήρωσης όλων των νημάτων
    for (int i = 0; i < Ncust; i++) {
        pthread_join(threads[i], NULL);
    }
	
	// Καταστροφή μεταβλητών συγχρονισμού
    pthread_mutex_destroy(&mutex_tel);
    pthread_mutex_destroy(&mutex_cook);
    pthread_mutex_destroy(&mutex_oven);
    pthread_mutex_destroy(&mutex_deliver);
    pthread_mutex_destroy(&mutex_print);
    pthread_mutex_destroy(&mutex_stats);

    pthread_cond_destroy(&cond_tel);
    pthread_cond_destroy(&cond_cook);
    pthread_cond_destroy(&cond_oven);
    pthread_cond_destroy(&cond_deliver);
	
	// Υπολογισμός μέσων χρόνων
    long avg_service_time = total_service_time / total_orders;
    long avg_cooling_time = total_cooling_time / total_orders;
	
	// Εκτύπωση στατιστικών
    printf("Συνολικά έσοδα: %d ευρώ\n", total_revenue);
    printf("Πωλήσεις μαργαρίτα: %d\n", pizza_sales[0]);
    printf("Πωλήσεις πεπερόνι: %d\n", pizza_sales[1]);
    printf("Πωλήσεις σπέσιαλ: %d\n", pizza_sales[2]);
    printf("Επιτυχημένες παραγγελίες: %d\n", total_orders);
    printf("Αποτυχημένες παραγγελίες: %d\n", failed_orders);
    printf("Μέσος χρόνος εξυπηρέτησης: %ld λεπτά\n", avg_service_time);
    printf("Μέγιστος χρόνος εξυπηρέτησης: %ld λεπτά\n", max_service_time);
    printf("Μέσος χρόνος κρυώματος: %ld λεπτά\n", avg_cooling_time);
    printf("Μέγιστος χρόνος κρυώματος: %ld λεπτά\n", max_cooling_time);

    return EXIT_SUCCESS;
}
