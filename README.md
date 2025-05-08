# Laporan Penjelasan dan Penyelesaian Soal Modul 3

بِسْمِ اللهِ الرَّحْمٰنِ الرَّحِيْمِ

Segala puji bagi Allah, Rabb semesta alam. Shalawat dan salam semoga tercurah kepada Nabi Muhammad ﷺ, keluarga, sahabat, serta siapa saja yang mengikuti jalan beliau hingga hari kiamat.

Laporan ini disusun sebagai bentuk tanggung jawab akademik dan ikhtiar dalam menuntut ilmu, sebagaimana sabda Nabi ﷺ:

"Barangsiapa menempuh jalan untuk menuntut ilmu, niscaya Allah akan mudahkan baginya jalan menuju surga."

(HR. Muslim)

Semoga apa yang tertulis dalam laporan ini menjadi sebab keberkahan, ilmu yang bermanfaat, dan amal yang diterima di sisi Allah ﷻ.

Semoga Allah memudahkan langkah kita semua dalam menuntut ilmu, mengamalkannya, serta menjaganya agar tidak sekadar jadi hafalan di otak, tapi bekal untuk akhirat.


## Anggota Kelompok
| Nama                      | NRP        |
|---------------------------|------------|
|Muhammad Hikari Reiziq R.  | 5027241079 |
|Dira Muhammad Ilyas S. A.  | 5027241033 |
|Thariq Kyran Aryunaldi     | 5027241073 |
### Soal 1
Kita disuruh membuat sistem RPC server-client untuk mengubah text file sehingga bisa dilihat dalam bentuk file jpeg.
```bash
wget "https://drive.usercontent.google.com/u/0/uc?id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw&export=download" -O secrets.zip && unzip secrets.zip && rm secrets.zip && mkdir -p client && mv secrets client/ && mkdir -p server/database && touch server/server.log 

```

---

## Bagian: **Header dan Definisi Makro**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

```

Mengimpor semua header penting:

- `socket`, `bind`, `accept` → untuk server TCP (Transmission Control Protocol)
- `time.h`, `unistd.h` → untuk waktu & proses
- `sys/stat.h` → untuk `umask`, file permission
- `pthread.h` di sini **tidak digunakan**, jadi bisa dihapus

```c
#define PORT 8080
#define LOG_PATH "/home/reiziqzip/modul3/soal_1/server/server.log"
#define DB_PATH "/home/reiziqzip/modul3/soal_1/server/database/"
#define MAX_BUF 65536

```

Makro untuk:

- Port TCP yang digunakan (8080)
- Path untuk file log dan folder database
- Buffer maksimum 64KB

---

## Fungsi: `log_message(...)`

```c
void log_message(const char *source, const char *action, const char *info)

```

Menulis log ke `server.log` dalam format:

```
[Source][YYYY-MM-DD HH:MM:SS]: [ACTION] [Info]

```

---

## Fungsi: `hex_to_bytes(...)`

```c
void hex_to_bytes(const char *hex, unsigned char *bytes, size_t *out_len)

```

Mengubah string hex menjadi array byte (binary JPEG):

- Misal `"4a4b"` → byte[0] = 0x4a, byte[1] = 0x4b

---

## Fungsi: `reverse_string(...)`

```c
void reverse_string(char *str)

```

Membalikkan string, digunakan sebelum decoding karena isi text dienkripsi secara terbalik (reverse).

---

## Fungsi Inti: `handle_client(...)`

Fungsi utama yang memproses koneksi masuk dari client:

### a. `recv()`:

```c
int received = recv(client_sock, buffer, sizeof(buffer), 0);

```

Menerima pesan dari client, disimpan ke `buffer`.

---

### b. `DECRYPT`:

```c
if (strncmp(buffer, "DECRYPT", 7) == 0)

```

- Client mengirim data hex terenkripsi
- Server log `DECRYPT`
- Proses:
    1. Reverse string
    2. Decode hex → byte[]
    3. Simpan ke `server/database/<timestamp>.jpeg`
    4. Kirim nama file kembali ke client

---

### c. `DOWNLOAD`:

```c
else if (strncmp(buffer, "DOWNLOAD", 8) == 0)

```

- Client meminta file JPEG dari server
- Server:
    1. Log `DOWNLOAD`
    2. Baca file → kirim datanya ke client
    3. Log `UPLOAD`

Jika file tidak ditemukan:

- Kirim pesan error
- Log `ERROR`

---

### d. `EXIT`:

```c
else if (strncmp(buffer, "EXIT", 4) == 0)

```

- Client keluar dari aplikasi
- Log `EXIT`
- Kirim pesan konfirmasi ke client

---

### e. Default: perintah tidak dikenali

```c
else {
    log_message("Server", "ERROR", "Perintah tidak dikenali");
}

```

---

## Fungsi: `main()`

```c
pid_t pid = fork();
if (pid < 0) exit(EXIT_FAILURE);
if (pid > 0) exit(EXIT_SUCCESS);

```

Membuat proses menjadi **daemon** (background):

- Fork 2x → detach dari terminal
- `setsid()` → sesi baru
- `umask(0)` → atur permission
- `chdir()` → pindah ke direktori kerja

---

### Socket Setup:

```c
int server_sock = socket(...);
bind(...);
listen(...);

```

Membuka socket TCP dan mulai listening di `PORT 8080`.

---

### Loop menerima koneksi:

```c
while (1) {
    int client_sock = accept(...);
    if (fork() == 0) {
        handle_client(client_sock);
        exit(0);
    }
    close(client_sock);
}

```

Untuk setiap client:

- **Fork child process** baru
- Jalankan `handle_client()`
- Tutup socket di parent

---

Berikut adalah **penjelasan lengkap per bagian dan fungsi dari file `image_client.c`**, sesuai permintaan kamu. File ini merupakan bagian **client-side** dari sistem RPC dan bertanggung jawab untuk menghubungi server, mengirim file untuk didekripsi, dan meminta file hasil decode dari server.

---

## HEADERS dan DEFINISI KONSTANTA

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

```

Mengimpor fungsi standar, termasuk:

- `socket`, `connect`, `recv`, `send`
- `fopen`, `fread`, `fwrite`
- `inet_pton` untuk menghubungkan ke IP server

```c
#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define MAX_BUF 65536

```

 Mendefinisikan:

- Port socket
- IP lokal server
- Buffer maksimal untuk file

---

## Fungsi `menu()`

```c
void menu() {
    printf("\n===== MENU =====\n");
    printf("1. Kirim dan decrypt file txt\n");
    printf("2. Download file jpeg dari server\n");
    printf("3. Exit\n");
    printf("Masukkan pilihan Anda: ");
}

```

Menampilkan opsi menu utama untuk user. Ini akan dipanggil setiap iterasi `while` di `main()`.

---

## Fungsi `connect_and_send(...)`

```c
void connect_and_send(const char *message, size_t message_len, const char *save_path, int expect_file)

```

### Tugas fungsi ini:

- Membuka koneksi socket ke server
- Mengirimkan pesan
- Jika `expect_file == 1` → berarti akan menerima file dan simpan ke `save_path`
- Jika `expect_file == 0` → hanya tunggu balasan teks (filename/error)

### Langkah-langkah:

1. **Buat socket** TCP dan hubungkan ke server
2. **Kirim message** (DECRYPT/EXIT/DOWNLOAD ...)
3. Jika `expect_file`:
    - Terima data secara bertahap dengan `recv`
    - Simpan ke file lokal `save_path`
4. Jika bukan file:
    - Terima balasan teks dari server (misal: nama file `.jpeg`)
5. Tutup socket

---

## Fungsi `main()`

```c
int main() {
    while (1) {
        menu();
        ...
    }
}

```

Loop utama program untuk menjalankan CLI interaktif.

---

## Opsi 1: Kirim file untuk DECRYPT

```c
char fname[128];
printf("Masukkan nama file txt (di folder client/secrets/): ");
scanf("%s", fname);
...
FILE *f = fopen(path, "r");

```

1. Minta user memasukkan nama file `.txt` dari folder `client/secrets/`
2. File dibuka dan dibaca
3. Isi file dianggap string **hex terenkripsi terbalik**
4. Dibungkus ke pesan `"DECRYPT <hex>"` lalu dikirim ke server

```c
sprintf(message, "DECRYPT %s", hex);
connect_and_send(message, strlen(message), NULL, 0);

```

Server akan reverse + decode hex, lalu simpan `.jpeg`, dan mengirimkan nama file balasan.

---

## Opsi 2: Download file JPEG dari server

```c
char fname[128];
printf("Masukkan nama file jpeg: ");
scanf("%s", fname);

```

1. Minta nama file `.jpeg`
2. Buat pesan `"DOWNLOAD <nama_file>"`
3. Kirim ke server
4. Simpan file hasil ke `client/<nama_file>`

---

## Opsi 3: Exit

```c
char *exit_message = "EXIT";
connect_and_send(exit_message, strlen(exit_message), NULL, 0);

```

 Kirim sinyal ke server untuk logout, lalu keluar dari program.

---

## Error Handling

- Jika file tidak ditemukan: tampilkan `"Gagal membuka file."`
- Jika server tidak responsif: akan keluar pesan dari `perror(...)`
- Jika `recv()` gagal → tampil `"Gagal menerima balasan dari server."`

---

## Cara menjalankannya

gcc image_client.c -o client/image_client

./client/image_client

gcc image_server.c -o server/image_server

./server/image_server && ./client/image_client

check yang sedang pakai port 8080 —> sudo lsof -i :8080

### Soal 2
## Deskripsi Singkat

RushGo adalah perusahaan ekspedisi fiktif yang memiliki dua jenis pengiriman:

- **Express**: ditangani otomatis oleh 3 agent (`delivery_agent.c`)
- **Reguler**: ditangani manual oleh user (`dispatcher.c`)

Sistem ini dibangun dengan:

- **Shared Memory** (`shmget`, `shmat`, `shmdt`) untuk berbagi data order
- **Multithread** (`pthread`) untuk agent Express
- **Log system** (`delivery.log`) untuk mencatat aktivitas pengiriman

---

### **Download delivery order csv**

```bash
wget "[Link]" -O delivery_order.csv && code .
```

## Penjelasan `delivery_agent.c`

`delivery_agent.c` adalah program otomatisasi untuk pengiriman **pesanan bertipe Express**. Program ini:

- Terhubung ke shared memory (berisi semua pesanan)
- Menjalankan **3 thread agen**: AGENT A, B, dan C
- Masing-masing agen bertugas mencari pesanan Express yang masih “Pending”
- Jika menemukan pesanan, agen akan:
    - Mengubah status menjadi "Delivered"
    - Menulis catatan ke file `delivery.log`
    - Menampilkan informasi pengiriman di terminal

### Struktur Data

- `Order`: struktur untuk satu pesanan (nama, alamat, tipe, status)
- `SharedData`: array dari `Order` yang dibagikan lewat shared memory

Shared memory ini memiliki **key** `1234`, digunakan oleh `dispatcher.c` dan `delivery_agent.c`.

---

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

```

Bagian ini mengimpor library standar C yang dibutuhkan:

- `stdio.h`, `stdlib.h`, `string.h` untuk I/O dan string
- `unistd.h` untuk `sleep()`
- `pthread.h` untuk membuat thread
- `sys/ipc.h` dan `sys/shm.h` untuk shared memory
- `time.h` untuk mencatat waktu pengiriman

---

```c
#define MAX_ORDERS 100
#define SHM_KEY 1234
#define LOG_FILE "delivery.log"

```

Konstanta yang digunakan:

- `MAX_ORDERS`: maksimal jumlah pesanan
- `SHM_KEY`: kunci shared memory (harus sama dengan dispatcher.c)
- `LOG_FILE`: nama file log pengiriman

---

```c
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

```

Struktur data:

- `Order`: mewakili satu pesanan
- `SharedData`: array `orders` dan total pesanan

---

```c
SharedData *shared_data;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

```

Deklarasi variabel global:

- `shared_data`: pointer ke data di shared memory
- `lock`: mutex untuk menghindari race condition antar thread

---

```c
void load_csv_to_shared_memory(SharedData *data) {

```

- Fungsi menerima pointer ke `SharedData` yang berada di shared memory.
- Semua perubahan di `data->orders` akan langsung mengubah isi shared memory.

---

```c
    FILE *file = fopen("delivery_order.csv", "r");
    if (!file) {
        perror("Gagal membuka delivery_order.csv");
        exit(EXIT_FAILURE);
    }

```

- Membuka file `delivery_order.csv` dengan mode `read`.
- Jika gagal dibuka (misal file tidak ada), program langsung keluar (`exit`) dan mencetak error melalui `perror()`.

---

```c
    char line[256];
    fgets(line, sizeof(line), file); // skip header

```

- Membaca baris pertama (header `Nama,Alamat,Tipe`) lalu **mengabaikannya**, karena bukan data.
- Fungsi `fgets` menyimpan satu baris ke dalam buffer `line`.

---

```c
    int idx = 0;
    while (fgets(line, sizeof(line), file) && idx < MAX_ORDERS) {

```

- Mulai membaca baris data satu per satu.
- Loop akan berhenti jika:
    - Sudah tidak ada baris lagi (EOF)
    - Jumlah order mencapai batas `MAX_ORDERS`

---

```c
        char *nama = strtok(line, ",");
        char *alamat = strtok(NULL, ",");
        char *tipe = strtok(NULL, "\n");

```

- Menggunakan `strtok` untuk memecah baris CSV berdasarkan koma `,`
- Hasilnya:
    - `nama` berisi nama pemesan
    - `alamat` berisi alamat tujuan
    - `tipe` berisi "Express" atau "Reguler"

---

```c
        if (nama && alamat && tipe) {

```

- Cek apakah semua data berhasil diambil. Jika ya, lanjut isi struct.

---

```c
            strncpy(data->orders[idx].nama, nama, sizeof(data->orders[idx].nama));
            strncpy(data->orders[idx].alamat, alamat, sizeof(data->orders[idx].alamat));
            strncpy(data->orders[idx].tipe, tipe, sizeof(data->orders[idx].tipe));
            strncpy(data->orders[idx].status, "Pending", sizeof(data->orders[idx].status));
            idx++;
        }

```

- Mengisi elemen `orders[idx]` dengan informasi dari file:
    - `nama`, `alamat`, `tipe` → dari CSV
    - `status` → default `"Pending"` (belum dikirim)
- `idx++` menambah jumlah order yang berhasil dimasukkan

---

```c
    data->total_orders = idx;
    fclose(file);
}

```

- Setelah semua baris dibaca, `total_orders` diisi dengan jumlah order aktual.
- File ditutup dengan `fclose()` untuk membebaskan resource.

---

```c
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
    fflush(log);
    fclose(log);
}

```

Fungsi ini digunakan untuk mencatat log pengiriman ke `delivery.log`. Format timestamp menggunakan `strftime`, dan log ditulis dalam format yang telah ditentukan.

---

```c
int express_orders_done() {
    for (int i = 0; i < shared_data->total_orders; i++) {
        if (strcmp(shared_data->orders[i].tipe, "Express") == 0 &&
            strcmp(shared_data->orders[i].status, "Pending") == 0) {
            return 0;
        }
    }
    return 1;
}

```

Fungsi ini mengecek apakah masih ada pesanan bertipe `"Express"` yang belum dikirim (`status == Pending`). Jika tidak ada, maka return `1`.

---

```c
void *agent_thread(void *arg) {
    char *agent_name = (char *)arg;

```

Setiap thread akan menjalankan fungsi ini. `arg` berisi nama agen: `"A"`, `"B"`, atau `"C"`.

---

```c
    while (1) {
        pthread_mutex_lock(&lock);
        int found = 0;

        for (int i = 0; i < shared_data->total_orders; i++) {
            if (strcmp(shared_data->orders[i].tipe, "Express") == 0 &&
                strcmp(shared_data->orders[i].status, "Pending") == 0) {

```

Loop mencari order Express yang belum dikirim (`status == Pending`). Mutex digunakan agar thread tidak konflik saat mengakses `shared_data`.

---

```c
                char status[64];
                snprintf(status, sizeof(status), "Delivered by %s", agent_name);
                strncpy(shared_data->orders[i].status, status, sizeof(shared_data->orders[i].status));

                save_log(agent_name, &shared_data->orders[i]);
                printf("[AGENT %s] Delivered Express package to %s in %s\n", agent_name, shared_data->orders[i].nama, shared_data->orders[i].alamat);
                fflush(stdout);
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

```

Jika ada pesanan ditemukan:

- Status diperbarui
- Log ditulis
- Informasi dicetak ke terminal

Jika tidak ditemukan dan semua Express sudah terkirim, thread keluar dari loop.

---

```c
int main() {
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget gagal");
        exit(EXIT_FAILURE);
    }

    shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat gagal");
        exit(EXIT_FAILURE);
    }

```

Program utama:

- Membuat atau mengambil shared memory
- `shmat` menghubungkan memori tersebut ke pointer `shared_data`

---

```c
    if (shared_data->total_orders == 0) {
        printf("Tidak ada pesanan dalam shared memory. Harap jalankan dispatcher terlebih dahulu.\n");
        shmdt(shared_data);
        return 0;
    }

```

Kalau belum ada pesanan (`total_orders == 0`), program keluar dan memberi tahu pengguna untuk menjalankan `dispatcher` dulu.

---

```c
    pthread_t agentA, agentB, agentC;
    pthread_create(&agentA, NULL, agent_thread, "A");
    pthread_create(&agentB, NULL, agent_thread, "B");
    pthread_create(&agentC, NULL, agent_thread, "C");

    pthread_join(agentA, NULL);
    pthread_join(agentB, NULL);
    pthread_join(agentC, NULL);

```

Membuat dan menjalankan 3 thread agen. Program akan menunggu ketiganya selesai (`join`).

---

```c
    shmdt(shared_data);
    printf("Semua paket Express telah dikirim. Program selesai.\n");
    return 0;
}

```

Setelah semua agen selesai, shared memory dilepas dan program ditutup.

---

## Penjelasan Lengkap Kode `dispatcher.c`

---

### 1. Import Library

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

```

> Kode ini mengimpor peralatan yang dibutuhkan:
> 
- `stdio.h` untuk baca/tulis data di terminal dan file
- `stdlib.h` untuk fungsi umum seperti `exit()`
- `string.h` untuk mengelola teks (nama, alamat, dst.)
- `sys/ipc.h` dan `sys/shm.h` untuk fitur **shared memory**
- `time.h` untuk mencatat waktu pengiriman
- `unistd.h` untuk dapatkan nama user Linux dan fungsi tambahan

---

### 2. Konstanta & Struktur Data

```c
#define MAX_ORDERS 100
#define SHM_KEY 1234
#define LOG_FILE "delivery.log"

```

> Di sini kita tentukan:
> 
- Maksimal pesanan yang bisa ditampung: 100
- Kunci shared memory yang digunakan: 1234
- Nama file log: "delivery.log"

---

```c
typedef struct {
    char nama[64];
    char alamat[128];
    char tipe[16];
    char status[64];
} Order;

```

> Order adalah struktur/data untuk 1 pesanan:
> 
- `nama`: siapa yang dikirimi
- `alamat`: ke mana dikirim
- `tipe`: "Express" atau "Reguler"
- `status`: misalnya "Pending" atau "Delivered by Agent B"

---

```c
typedef struct {
    Order orders[MAX_ORDERS];
    int total_orders;
} SharedData;

```

> SharedData adalah kumpulan semua pesanan (orders) dan jumlah totalnya.
> 

---

### 3. Fungsi untuk Memuat CSV

```c
void load_csv_to_shared_memory(SharedData *data)

```

> Fungsi ini memuat data pesanan dari file delivery_order.csv dan menyimpannya ke memori bersama (shared memory).
> 

---

```c
FILE *file = fopen("delivery_order.csv", "r");
if (!file) {
    perror("Gagal membuka delivery_order.csv");
    exit(EXIT_FAILURE);
}

```

> Kita coba buka file CSV. Kalau file tidak ditemukan, tampilkan pesan error dan keluar dari program.
> 

---

```c
char line[256];
fgets(line, sizeof(line), file); // skip header

```

> Baris pertama file berisi "Nama,Alamat,Tipe", yang tidak kita perlukan, jadi dilewati.
> 

---

```c
int idx = 0;
while (fgets(line, sizeof(line), file) && idx < MAX_ORDERS)

```

> Kita baca baris demi baris, selama belum mencapai batas maksimal (100 order).
> 

---

```c
char *nama = strtok(line, ",");
char *alamat = strtok(NULL, ",");
char *tipe = strtok(NULL, "\n");

```

> Setiap baris kita pecah berdasarkan koma (,). Kita ambil:
> 
- nama penerima
- alamat tujuan
- tipe pesanan

---

```c
if (nama && alamat && tipe)

```

> Kalau semua bagian berhasil terbaca, baru lanjut isi ke struct pesanan.
> 

---

```c
strncpy(data->orders[idx].nama, nama, ...);
strncpy(data->orders[idx].alamat, alamat, ...);
strncpy(data->orders[idx].tipe, tipe, ...);
strncpy(data->orders[idx].status, "Pending", ...);
idx++;

```

> Masukkan semua informasi ke dalam array orders. Status awal = "Pending".
> 

---

```c
data->total_orders = idx;
fclose(file);

```

> Simpan jumlah total pesanan ke shared memory dan tutup file.
> 

---

### 4. Fungsi Menyimpan ke Log

```c
void save_log(const char *agent, const Order *order, const char *tipe)

```

> Fungsi ini mencatat pengiriman ke dalam file delivery.log.
> 

---

```c
FILE *log = fopen(LOG_FILE, "a");
if (!log) { perror(...); return; }

```

> Buka file untuk ditambahkan (a = append). Kalau gagal, tampilkan error.
> 

---

```c
time_t now = time(NULL);
struct tm *t = localtime(&now);
strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", t);

```

> Ambil waktu sekarang dan ubah ke format jam/tanggal yang mudah dibaca.
> 

---

```c
fprintf(log, "[%s] [AGENT %s] %s package delivered to %s in %s\n", ...);
fflush(log);
fclose(log);

```

> Tulis info ke file log dan tutup file-nya.
> 

---

### 5. Fungsi `main()` (Program Utama)

### Penjelasan `int main(int argc, char *argv[])`

Fungsi `main` adalah pusat dari program `dispatcher.c`. Ia mengatur:

- Penghubung ke shared memory
- Pengisian data dari CSV jika belum ada
- Penanganan perintah pengguna (`deliver`, `status`, `list`)

---

### Bagian 1: Mengakses Shared Memory

```c
int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);

```

**Penjelasan**:

- `shmget` digunakan untuk **mengakses** atau **membuat** shared memory.
- `SHM_KEY` (1234) adalah kunci agar semua program (dispatcher & delivery agent) memakai memory yang sama.
- `sizeof(SharedData)` menentukan ukuran memory yang dialokasikan.
- `IPC_CREAT | 0666` artinya:
    - Jika belum ada, maka buat memory baru.
    - Izin akses diberikan penuh (read & write) ke semua user.

---

```c
if (shmid == -1) {
    perror("shmget gagal");
    exit(EXIT_FAILURE);
}

```

**Penjelasan**:

- Kalau `shmget` gagal (misalnya karena hak akses atau memori penuh), program langsung keluar dan mencetak pesan kesalahan.

---

### Bagian 2: Menghubungkan ke Shared Memory

```c
SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);

```

**Penjelasan**:

- `shmat` adalah fungsi untuk **menghubungkan shared memory** yang tadi dibuat ke variabel `shared_data`.
- Setelah ini, `shared_data->orders` bisa diakses seperti array biasa.

---

```c
if (shared_data == (void *)-1) {
    perror("shmat gagal");
    exit(EXIT_FAILURE);
}

```

**Penjelasan**:

- Kalau gagal menghubungkan, program akan keluar. Ini penting untuk mencegah operasi ke memori yang tidak valid.

---

### Bagian 3: Load dari CSV jika Kosong

```c
if (shared_data->total_orders == 0) {
    printf("Shared memory kosong. Memuat dari CSV...\n");
    load_csv_to_shared_memory(shared_data);
}

```

**Penjelasan**:

- Jika jumlah pesanan masih 0, artinya belum pernah diisi.
- Maka kita muat data dari file `delivery_order.csv` ke dalam shared memory dengan memanggil fungsi `load_csv_to_shared_memory`.

---

### Bagian 4: Menangani Argumen dari Terminal

```c
if (argc >= 2) {

```

**Penjelasan**:

- Mengecek apakah ada **perintah** yang diberikan oleh pengguna.
- `argc` menyimpan jumlah argumen, dan `argv` adalah array dari string perintah, misalnya:
    - `argv[0] = "./dispatcher"`
    - `argv[1] = "-deliver"`
    - `argv[2] = "Novi"`

---

### Sub-perintah 1: `deliver [Nama]`

```c
if (strcmp(argv[1], "-deliver") == 0 && argc == 3)

```

**Penjelasan**:

- Mengecek apakah perintah adalah `deliver` dan ada nama penerima.

---

```c
for (int i = 0; i < shared_data->total_orders; i++) {
    if (strcmp(shared_data->orders[i].nama, target) == 0 &&
        strcmp(shared_data->orders[i].tipe, "Reguler") == 0 &&
        strcmp(shared_data->orders[i].status, "Pending") == 0) {

```

**Penjelasan**:

- Mencari pesanan dengan nama yang sama, bertipe `Reguler`, dan statusnya masih `Pending`.
- Jika ditemukan:

```c
char *agent = getenv("USER");
if (agent == NULL) agent = "UNKNOWN";

```

- Ambil nama user Linux (sebagai pengantar).

```c
snprintf(status, sizeof(status), "Delivered by Agent %s", agent);
strncpy(shared_data->orders[i].status, status, sizeof(shared_data->orders[i].status));
save_log(agent, &shared_data->orders[i], "Reguler");

```

- Update status di shared memory
- Simpan ke log

---

### Sub-perintah 2: `status [Nama]`

```c
else if (strcmp(argv[1], "-status") == 0 && argc == 3)

```

**Penjelasan**:

- Perintah untuk **melihat status pengiriman** dari seseorang.

```c
if (strcmp(shared_data->orders[i].nama, target) == 0)

```

- Jika ditemukan, tampilkan:

```c
printf("Status for %s: %s\n", target, shared_data->orders[i].status);

```

---

### Sub-perintah 3: `list`

```c
else if (strcmp(argv[1], "-list") == 0)

```

**Penjelasan**:

- Menampilkan semua pesanan dalam shared memory, termasuk:
    - Nama penerima
    - Alamat
    - Tipe
    - Status

---

### Penanganan Perintah Tidak Dikenali

```c
else {
    printf("Perintah tidak dikenali.\n");
}

```

- Jika perintah tidak cocok dengan `deliver`, `status`, atau `list`, maka ditolak.

---

### Jika Tidak Ada Argumen

```c
} else {
    printf("Penggunaan:\n");
    printf("./dispatcher -deliver [Nama]\n");
    printf("./dispatcher -status [Nama]\n");
    printf("./dispatcher -list\n");
}

```

- Kalau pengguna tidak mengetikkan perintah apa pun, program akan mencetak panduan penggunaan.

---

### Terakhir: Lepaskan Memory

```c
shmdt(shared_data);
return 0;

```

- `shmdt` digunakan untuk **memutus koneksi dari shared memory**.
- Program selesai.

---

Kalau kamu mau, aku bisa gabungkan semua penjelasan ini jadi satu file Markdown atau Word untuk dokumentasi. Perlu?

### Cara menjalankannya

ipcs -m (check shared memory)

ipcrm -M 1234 (Reset Shared Memory)
rm delivery.log
./delivery_agent

gcc -o delivery_agent delivery_agent.c -lpthread

./delivery_agent

gcc -o dispatcher dispatcher.c
./dispatcher -deliver Novi
./dispatcher -status Novi
./dispatcher -list


### Soal 4
**Soal a**  

**File:** `system.c` `hunter.c` 

**Kode:**  
```system.c
key_t key = ftok(KEY_PATH, PROJ_ID);
shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
data = (SharedData *)shmat(shmid, NULL, 0);
memset(data, 0, sizeof(SharedData));
```
```hunter.c
key_t key = ftok(KEY_PATH, PROJ_ID);
shmid = shmget(key, sizeof(SharedData), 0666);
if (shmid < 0) {
    printf("System not running. Start system.c first.\n");
    exit(1);
}
data = (SharedData *)shmat(shmid, NULL, 0);
```
    
**Penjelasan:**  
Kode di atas menginisialisasi shared memory menggunakan `shmget` dan `shmat`, yang menjadi inti dari `system.c` untuk mengelola data hunter dan dungeon. Ini sesuai dengan soal a yang menyebutkan `system.c` sebagai pengelola shared memory utama. Library yang digunakan yakni <sys/ipc.h> dan <sys/shm.h>

---

**Soal b**  
**File:** `hunter.c` (SharedData struct)  
**Kode:**  
```c
typedef struct {
    char username[USERNAME_LEN];
    int level, exp;
    int atk, hp, def;
    bool banned;
    // ... 
} Hunter;
void register_hunter() {
    char name[USERNAME_LEN];
    printf("Username: "); scanf("%s", name);

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (data->hunters[i].used && strcmp(data->hunters[i].username, name) == 0) {
            printf("Hunter already registered\n");
            printf("Registration failed.\n");
            return; // Langsung keluar jika username sudah ada
        }
    }

    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (!data->hunters[i].used) {
            strcpy(data->hunters[i].username, name);
            data->hunters[i].level = 1;
            data->hunters[i].exp = 0;
            data->hunters[i].atk = 10;
            data->hunters[i].hp = 100;
            data->hunters[i].def = 5;
            data->hunters[i].banned = false;
            data->hunters[i].notif_active = false;
            data->hunters[i].used = true;
            printf("Registration success!\n");
            return;
        }
    }

    // Jika tidak ada slot kosong
    printf("Registration failed.\n");
}
void login_hunter() {
    char name[USERNAME_LEN];
    printf("Username: "); scanf("%s", name);
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (data->hunters[i].used && strcmp(data->hunters[i].username, name) == 0) {
            current_id = i;
            printf("Login success!\n");
            hunter_menu();
            return;
        }
    }
    printf("Login failed.\n");
}
```  
**Penjelasan:**  
Struct `Hunter` menyimpan data stat awal (Level=1, EXP=0, ATK=10, HP=100, DEF=5) sesuai soal b. Meskipun implementasi CLI registrasi tidak ditampilkan, struct ini menjadi basis penyimpanan data hunter di shared memory. Terdapat pula fungsi untuk registrasi hunter dan login hunter yang mana jika ada hunter yang belum regis tidak bisa masuk dan hunter yang sudah regis tidak bisa regis ulang dilengkapi error code

---

**Soal c**  
**File:** `system.c`  
**Kode:**  
```c
void list_hunters() {
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (!data->hunters[i].used) continue;
        Hunter h = data->hunters[i];
        printf("- %s | LVL: %d | EXP: %d | ATK: %d | HP: %d | DEF: %d | %s\n",
            h.username, h.level, h.exp, h.atk, h.hp, h.def, h.banned ? "BANNED" : "ACTIVE");
    }
}
```  
**Penjelasan:**  
Fungsi `list_hunters()` menampilkan informasi lengkap semua hunter, termasuk status banned, sesuai dengan soal c.

---

**Soal d**  
**File:** `system.c`  
**Kode:**  
```c
void generate_dungeon() {
    d->min_level = rand() % 5 + 1;              // Level Minimal 1-5
    d->reward_exp = rand() % 151 + 150;         // EXP: 150-300
    d->reward_atk = rand() % 51 + 100;          // ATK: 100-150
    d->reward_hp  = rand() % 51 + 50;           // HP: 50-100
    d->reward_def = rand() % 26 + 25;           // DEF: 25-50
    d->used = true;
}
```  
**Penjelasan:**  
Fungsi `generate_dungeon()` membuat dungeon dengan stat reward acak sesuai rentang yang ditentukan di soal d.

---

**Soal e**  
**File:** `system.c`  
**Kode:**  
```c
void list_dungeons_info() {
    printf("\n=== DUNGEON INFO ===\n");
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (!data->dungeons[i].used) continue;
        Dungeon *d = &data->dungeons[i];
        printf("[Dungeon %d]\nName: %s\nMinimum Level: %d\nEXP Reward: %d\nATK: %d\nHP: %d\nDef: %d\nKey: %d\n\n",
               i + 1, d->name, d->min_level, d->reward_exp,
               d->reward_atk, d->reward_hp, d->reward_def, (int)(intptr_t)d);
    }
}
```  
**Penjelasan:**  
Fungsi `list_dungeons_info()` menampilkan detail semua dungeon, termasuk reward dan level minimal, sesuai soal e.

---

**Soal f**  
**File:** `hunter.c`  
**Kode:**  
```c
void list_dungeons() {
    printf("\n=== AVAILABLE DUNGEON ===\n");
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        Dungeon d = data->dungeons[i];
        if (d.used && d.min_level <= data->hunters[current_id].level) {
            printf("%d. %s (Level %d+)\n", i + 1, d.name, d.min_level);
        }
    }
    printf("Press enter to continue..."); getchar(); getchar();
}
```  
**Penjelasan:**  
Fungsi `list_dungeons()` menampilkan detail semua dungeon, termasuk reward dan level minimal, sesuai soal f.

---

**Soal g**  
**File:** `system.c`  
**Kode:**  
```c
if (data->hunters[current_id].exp >= 500) {
        data->hunters[current_id].level++;
        data->hunters[current_id].exp = 0;
    }
```  
**Penjelasan:**  
Kode diatas untuk membuat hunter yang sudah memenuhi treshold mengalami level up, sesuai soal g.

---

**Soal h**  
**File:** `hunter.c`  
**Kode:**  
```c
if (my_power >= op_power) {
        data->hunters[current_id].atk += data->hunters[id].atk;
        data->hunters[current_id].hp  += data->hunters[id].hp;
        data->hunters[current_id].def += data->hunters[id].def;
        data->hunters[id].used = false;
        printf("Battle won! You acquired %s's stats\n", target);
    } else {
        data->hunters[id].atk += data->hunters[current_id].atk;
        data->hunters[id].hp  += data->hunters[current_id].hp;
        data->hunters[id].def += data->hunters[current_id].def;
        data->hunters[current_id].used = false;
        printf("You lost and your data is deleted.\n");
        exit(0);
    }
    printf("Press enter to continue..."); getchar(); getchar();
}
```  
**Penjelasan:**  
Bagian if statement pada fungsi `battle_hunter()` akan membuat jika hunter menang, maka hunter tersebut akan mendapatkan semua stats milik lawan dan lawannya akan terhapus dari sistem. Jika kalah, hunter tersebutlah yang akan dihapus dari sistem dan semua statsnya akan diberikan kepada hunter yang dilawannya. sesuai soal h.

---

**Soal i**  
**File:** `system.c`  
**Kode:**  
```c
void ban_unban_hunter(bool ban) {
    data->hunters[i].banned = ban; // Mengubah status banned
}
```  
**Penjelasan:**  
Fungsi `ban_unban_hunter()` mengubah status `banned` hunter, memenuhi soal i tentang fitur ban/unban.

---

**Soal j**  
**File:** `system.c`  
**Kode:**  
```c
void reset_hunter() {
    data->hunters[i].level = 1;
    data->hunters[i].exp = 0;
    data->hunters[i].atk = 10;
    data->hunters[i].hp = 100;
    data->hunters[i].def = 5;
}
```  
**Penjelasan:**  
Fungsi `reset_hunter()` mengembalikan stat hunter ke nilai awal, sesuai soal j.

---

**Soal k**  
**File:** `system.c`  
**Kode:**  
```c
void toggle_notification() {
    data->hunters[current_id].notif_active = !data->hunters[current_id].notif_active;
    if (data->hunters[current_id].notif_active) {
        printf("Notification activated!\n");
        while (data->hunters[current_id].notif_active) {
            system("clear");
            for (int i = 0; i < MAX_DUNGEONS; i++) {
                if (data->dungeons[i].used && data->dungeons[i].min_level <= data->hunters[current_id].level) {
                    printf("D-Rank Dungeon for minimum level %d opened!\n", data->dungeons[i].min_level);
                    break;
                }
            }
            sleep(3);
        }
    } else {
        printf("Notification deactivated!\n");
    }
}
```  
**Penjelasan:**  
kode diatas membuat
Saat toggle_notification() di call, akan muncul informasi tentang semua dungeon yang terbuka dan akan terus berganti setiap 3 detik.
Kendalanya perlu force close atau memulai kembali hunter.c untuk bisa mengakses fitur (data tidak akan dihapus selama system.c masih nyala)

---

**Soal l**  
**File:** `system.c`  
**Kode:**  
```c
void detach_and_cleanup() {
    shmctl(shmid, IPC_RMID, NULL); // Menghapus shared memory
}
```  
**Penjelasan:**  
Fungsi `detach_and_cleanup()` menghapus shared memory saat sistem dimatikan, memenuhi soal l.

---

