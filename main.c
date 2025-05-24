#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

#define KAT_SAYISI 10
#define DAIRE_SAYISI 4

// Shared Memory Pointer
volatile int* progresses;

// Mutex Definitions
pthread_mutex_t elektrik_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t su_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vinc_mutex = PTHREAD_MUTEX_INITIALIZER;
// Print Mutex Definiton
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void* daire_fonksiyonu(void* arg) {
    int daire_no = ((int*)arg)[0];
    int kat_index = ((int*)arg)[1];
    free(arg);

    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: Sıva yapılıyor...\n", daire_no);
    pthread_mutex_unlock(&print_mutex);
    sleep(1);

    pthread_mutex_lock(&elektrik_mutex);
    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: Elektrik tesisatı çekiliyor...\n", daire_no);
    pthread_mutex_unlock(&print_mutex);
    sleep(1);
    pthread_mutex_unlock(&elektrik_mutex);

    pthread_mutex_lock(&su_mutex);
    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: Su tesisatı döşeniyor...\n", daire_no);
    pthread_mutex_unlock(&print_mutex);
    sleep(1);
    pthread_mutex_unlock(&su_mutex);

    pthread_mutex_lock(&vinc_mutex);
    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: Vinç ile malzeme alınıyor...\n", daire_no);
    pthread_mutex_unlock(&print_mutex);
    sleep(1);
    pthread_mutex_unlock(&vinc_mutex);

    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: İnşaat tamamlandı.\n", daire_no);
    pthread_mutex_unlock(&print_mutex);

    // Progress bar increase on end of thread +10 per each (4*10=40)
    __sync_fetch_and_add(&progresses[kat_index], 10);

    return NULL;
}

void kat_insa_et(int kat_no) {
    int kat_index = kat_no - 1;

    pthread_mutex_lock(&print_mutex);
    printf("Kat %d inşaatına başlandı (PID: %d).\n", kat_no, getpid());
    pthread_mutex_unlock(&print_mutex);

    pthread_t threadler[DAIRE_SAYISI];

    for (int i = 0; i < DAIRE_SAYISI; i++) {
        int* arg = malloc(sizeof(int) * 2);
        arg[0] = (kat_index * DAIRE_SAYISI) + i + 1;    // Daire no
        arg[1] = kat_index;
        pthread_create(&threadler[i], NULL, daire_fonksiyonu, arg);
    }

    for (int i = 0; i < DAIRE_SAYISI; i++) {
        pthread_join(threadler[i], NULL);
    }

    // Progress Bar = 100% on end
    progresses[kat_index] = 100;

    pthread_mutex_lock(&print_mutex);
    printf("Kat %d tamamlandı.\n", kat_no);
    pthread_mutex_unlock(&print_mutex);

    exit(0); // End of child process
}

int main() {
    printf("Temel atılıyor...\n");
    sleep(1);

    progresses = mmap(NULL, sizeof(int) * KAT_SAYISI,
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (progresses == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    for (int i = 0; i < KAT_SAYISI; i++) {
        progresses[i] = 0;
    }

    pid_t pids[KAT_SAYISI];

    for (int kat = 1; kat <= KAT_SAYISI; kat++) {
        pid_t pid = fork();
        if (pid == 0) {
            kat_insa_et(kat);   // child process
        } else if (pid > 0) {
            pids[kat - 1] = pid;
            waitpid(pid, NULL, 0);
            printf("Kat %d inşaatı tamamlandı.\n", kat);
        } else {
            perror("fork");
            exit(1);
        }
    }

    printf("Apartman inşaatı tamamlandı.\n");
    munmap((void*)progresses, sizeof(int) * KAT_SAYISI);
    return 0;
}
