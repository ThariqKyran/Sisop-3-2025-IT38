#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

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

void save_log(const char *agent, const Order *order, const char *tipe) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) {
        perror("Gagal membuka delivery.log");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", t);

    fprintf(log, "[%s] [AGENT %s] %s package delivered to %s in %s\n",
            timestamp, agent, tipe, order->nama, order->alamat);
    fflush(log);
    fclose(log);
}

int main(int argc, char *argv[]) {
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget gagal");
        exit(EXIT_FAILURE);
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat gagal");
        exit(EXIT_FAILURE);
    }

    if (shared_data->total_orders == 0) {
        printf("Shared memory kosong. Memuat dari CSV...\n");
        load_csv_to_shared_memory(shared_data);
    }

    if (argc >= 2) {
        if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
            char *target = argv[2];
            int found = 0;

            for (int i = 0; i < shared_data->total_orders; i++) {
                if (strcmp(shared_data->orders[i].nama, target) == 0 &&
                    strcmp(shared_data->orders[i].tipe, "Reguler") == 0 &&
                    strcmp(shared_data->orders[i].status, "Pending") == 0) {

                    char *agent = getenv("USER");
                    if (agent == NULL) agent = "UNKNOWN";

                    char status[64];
                    snprintf(status, sizeof(status), "Delivered by Agent %s", agent);
                    strncpy(shared_data->orders[i].status, status, sizeof(shared_data->orders[i].status));

                    save_log(agent, &shared_data->orders[i], "Reguler");

                    printf("Status for %s: Delivered by Agent %s\n", target, agent);
                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf("Status for %s: Pending\n", target);
            }
        }

        else if (strcmp(argv[1], "-status") == 0 && argc == 3) {
            char *target = argv[2];
            int found = 0;

            for (int i = 0; i < shared_data->total_orders; i++) {
                if (strcmp(shared_data->orders[i].nama, target) == 0) {
                    printf("Status for %s: %s\n", target, shared_data->orders[i].status);
                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf("Order untuk %s tidak ditemukan.\n", target);
            }
        }

        else if (strcmp(argv[1], "-list") == 0) {
            printf("Daftar Semua Pesanan:\n");
            for (int i = 0; i < shared_data->total_orders; i++) {
                printf("%s | %s | %s | %s\n",
                       shared_data->orders[i].nama,
                       shared_data->orders[i].alamat,
                       shared_data->orders[i].tipe,
                       shared_data->orders[i].status);
            }
        }

        else {
            printf("Perintah tidak dikenali.\n");
        }
    } else {
        printf("Penggunaan:\n");
        printf("./dispatcher -deliver [Nama]\n");
        printf("./dispatcher -status [Nama]\n");
        printf("./dispatcher -list\n");
    }

    shmdt(shared_data);
    return 0;
}
