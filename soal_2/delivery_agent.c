#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_ORDERS 100
#define SHM_KEY 1234
#define LOG_FILE "delivery.log"

typedef struct {
    char nama[64];
    char alamat[128];
    char tipe[16];
    char status[64];
} Order;


typedef struct {
    Order orders[MAX_ORDERS];
    int total_orders;
} SharedData;

SharedData *shared_data;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void load_csv_to_shared_memory(SharedData *data) {
    FILE *file = fopen("delivery_order.csv", "r");
    if (!file) {
        perror("Gagal membuka delivery_order.csv");
        exit(EXIT_FAILURE);
    }

    char line[256];
    fgets(line, sizeof(line), file); // skip header

    int idx = 0;
    while (fgets(line, sizeof(line), file) && idx < MAX_ORDERS) {
        char *nama = strtok(line, ",");
        char *alamat = strtok(NULL, ",");
        char *tipe = strtok(NULL, "\n");

        if (nama && alamat && tipe) {
            strncpy(data->orders[idx].nama, nama, sizeof(data->orders[idx].nama));
            strncpy(data->orders[idx].alamat, alamat, sizeof(data->orders[idx].alamat));
            strncpy(data->orders[idx].tipe, tipe, sizeof(data->orders[idx].tipe));
            strncpy(data->orders[idx].status, "Pending", sizeof(data->orders[idx].status));
            idx++;
        }
    }

    data->total_orders = idx;
    fclose(file);
}

void save_log(const char *agent, const Order *order) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) {
        perror("Gagal membuka delivery.log");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", t);

    fprintf(log, "[%s] [AGENT %s] Express package delivered to %s in %s\n",
            timestamp, agent, order->nama, order->alamat);
    fflush(log); // pastikan data langsung ditulis ke file

    fclose(log);
}

int express_orders_done() {
    for (int i = 0; i < shared_data->total_orders; i++) {
        if (strcmp(shared_data->orders[i].tipe, "Express") == 0 &&
            strcmp(shared_data->orders[i].status, "Pending") == 0) {
            return 0;
        }
    }
    return 1;
}

void *agent_thread(void *arg) {
    char *agent_name = (char *)arg;

    while (1) {
        pthread_mutex_lock(&lock);
        int found = 0;

        for (int i = 0; i < shared_data->total_orders; i++) {
            if (strcmp(shared_data->orders[i].tipe, "Express") == 0 &&
                strcmp(shared_data->orders[i].status, "Pending") == 0) {

                char status[64];
                snprintf(status, sizeof(status), "Delivered by %s", agent_name);
                strncpy(shared_data->orders[i].status, status, sizeof(shared_data->orders[i].status));

                save_log(agent_name, &shared_data->orders[i]);

                printf("[AGENT %s] Delivered Express package to %s in %s\n", agent_name, shared_data->orders[i].nama, shared_data->orders[i].alamat);
                fflush(stdout); // agar output langsung muncul
                found = 1;
                break;
            }
        }

        pthread_mutex_unlock(&lock);

        if (!found && express_orders_done()) break;
        sleep(1);
    }

    return NULL;
}

int main() {
    // Buat atau ambil shared memory
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget gagal");
        exit(EXIT_FAILURE);
    }

    // Hubungkan ke shared memory
    shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat gagal");
        exit(EXIT_FAILURE);
    }

    // Jika shared memory masih kosong, isi dari CSV
    if (shared_data->total_orders == 0) {
        printf("Shared memory kosong. Memuat data dari CSV...\n");
        load_csv_to_shared_memory(shared_data);
    }

    // Jalankan 3 agent
    pthread_t agentA, agentB, agentC;
    pthread_create(&agentA, NULL, agent_thread, "A");
    pthread_create(&agentB, NULL, agent_thread, "B");
    pthread_create(&agentC, NULL, agent_thread, "C");

    pthread_join(agentA, NULL);
    pthread_join(agentB, NULL);
    pthread_join(agentC, NULL);

    // Lepaskan shared memory
    shmdt(shared_data);
    printf("Semua paket Express telah dikirim. Program selesai.\n");
    return 0;
}

