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

