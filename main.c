#include <stdio.h>    // Giriş-çıkış işlemleri (printf, perror gibi) için gerekli olan standart kütüphane
#include <stdlib.h>   // Bellek işlemleri ve exit gibi yardımcı fonksiyonlar için kullandığımız kütüphane
#include <unistd.h>   // UNIX sistem çağrıları (sleep, fork, getpid vb.) için gerekli kütüphane
#include <pthread.h>  // POSIX thread (pthread) kullanmak için gerekli başlık dosyası
#include <sys/mman.h> // mmap ile shared memory oluşturmak için kullanılıyor
#include <sys/wait.h> // waitpid fonksiyonu ile child process’leri beklemek için gerekli
#include <sys/types.h>// fork ve pid_t gibi veri türlerini tanımlamak için bu başlık dosyasını ekliyoruz

// Apartmanda toplam 10 kat olacağını ve her katta 4 daire olacağını sabit olarak tanımlıyoruz
#define KAT_SAYISI 10
#define DAIRE_SAYISI 4

// Bu değişken, her katın inşaat ilerleme durumunu gösterecek bir paylaşımlı bellek alanı olacak.
// volatile ile tanımlıyoruz çünkü bu değişkene farklı processler tarafından erişilecek.
// Bu yüzden derleyicinin bu değişkeni optimize edip önbelleğe almasını istemiyoruz.
volatile int* progresses;

// Aşağıdaki mutex'ler, thread'lerimizin bazı işleri sıralı yapabilmesini sağlayacak.
// Aynı anda birden fazla dairenin aynı kaynağı (elektrik, su, vinç) kullanmasını engellemek istiyoruz.
pthread_mutex_t elektrik_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t su_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vinc_mutex = PTHREAD_MUTEX_INITIALIZER;

// Yazdırma işlemlerinde çıktılar birbirine karışmasın diye ayrı bir mutex de sadece printf için tanımlıyoruz.
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// Bu fonksiyon, bir dairenin baştan sona tüm inşaat sürecini simüle ediyor.
// Her daire, bu fonksiyonla kendi inşaatını sırayla tamamlayacak.
void* daire_fonksiyonu(void* arg) {
    // Fonksiyona gelen argümanı ayrıştırıyoruz.
    // arg[0] daire numarası, arg[1] ise hangi katta olduğunu gösteriyor.
    int daire_no = ((int*)arg)[0];
    int kat_index = ((int*)arg)[1];
    free(arg);  // malloc ile aldığımız belleği işimiz bittiği için serbest bırakıyoruz.
    
// Önce sıva işlemini yapıyoruz. Bu işlem her daire için bağımsız, yani mutex'e gerek yok.
    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: Sıva yapılıyor...\n", daire_no);
    pthread_mutex_unlock(&print_mutex);
    sleep(1);  // Gerçek zamanlı simülasyon için 1 saniye bekliyoruz.

    // Şimdi elektrik tesisatı yapılıyor. Bu aşamada ortak kaynak kullanımı söz konusu.
    // Aynı anda başka bir daire elektrik çekmesin diye elektrik mutex'ini kilitliyoruz.
    pthread_mutex_lock(&elektrik_mutex);
    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: Elektrik tesisatı çekiliyor...\n", daire_no);
    pthread_mutex_unlock(&print_mutex);
    sleep(1);
    pthread_mutex_unlock(&elektrik_mutex);  // Elektrik işi bitti, diğer thread'lere kaynak serbest.
 
    // Su tesisatı da aynı şekilde ortak kaynak olduğu için mutex kullanıyoruz.
    pthread_mutex_lock(&su_mutex);
    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: Su tesisatı döşeniyor...\n", daire_no);
    pthread_mutex_unlock(&print_mutex);
    sleep(1);
    pthread_mutex_unlock(&su_mutex);

    // Vinç ile malzeme taşınması işlemi de sırayla yapılmalı çünkü tek vinç varmış gibi davranıyoruz.
    pthread_mutex_lock(&vinc_mutex);
    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: Vinç ile malzeme alınıyor...\n", daire_no);
    pthread_mutex_unlock(&print_mutex);
    sleep(1);
    pthread_mutex_unlock(&vinc_mutex);

    // Tüm işlemler tamamlandığında, daire inşaatı bitmiş oluyor.
    pthread_mutex_lock(&print_mutex);
    printf("    Daire %d: İnşaat tamamlandı.\n", daire_no);
    pthread_mutex_unlock(&print_mutex);

    // Katın toplam ilerleme yüzdesini artırıyoruz.
    // __sync_fetch_and_add ile atomik (kesintisiz) olarak %10 artırıyoruz.
    // Çünkü bu işleme birden fazla thread aynı anda erişebilir.
    __sync_fetch_and_add(&progresses[kat_index], 10);

    return NULL;  // Thread işi bitince çıkıyor.
}

// Bu fonksiyon her bir kat için çağrılıyor ve ayrı bir process içinde çalışıyor olacak.
// Kat içinde 4 daire var ve her biri için ayrı thread başlatılıyor.
void kat_insa_et(int kat_no) {
    int kat_index = kat_no - 1;
    
    // Kat inşaatı başlıyor, bunu ekrana yazdırıyoruz.
    pthread_mutex_lock(&print_mutex);
    printf("Kat %d inşaatına başlandı (PID: %d).\n", kat_no, getpid());
    pthread_mutex_unlock(&print_mutex);

     // Daire thread'lerini tutacak bir dizi oluşturuyoruz.
    pthread_t threadler[DAIRE_SAYISI];

    // Şimdi 4 adet thread başlatıyoruz, her biri bir daireyi temsil ediyor.
    for (int i = 0; i < DAIRE_SAYISI; i++) {
        int* arg = malloc(sizeof(int) * 2);  // daire_no ve kat_index bilgisini taşımak için bellekte yer ayırıyoruz.
        arg[0] = (kat_index * DAIRE_SAYISI) + i + 1;  // daire numarası 1'den başlıyor.
        arg[1] = kat_index;
        pthread_create(&threadler[i], NULL, daire_fonksiyonu, arg);  // Thread başlatılıyor.
    }

    // Kat içindeki tüm dairelerin inşaatı bitene kadar bekliyoruz.
    for (int i = 0; i < DAIRE_SAYISI; i++) {
        pthread_join(threadler[i], NULL);  // Her thread'i bekliyoruz.
    }

    // Tüm daireler tamamlandığında kat inşaatı da tamamlanmış oluyor.
    progresses[kat_index] = 100;

    pthread_mutex_lock(&print_mutex);
    printf("Kat %d tamamlandı.\n", kat_no);
    pthread_mutex_unlock(&print_mutex);

    // Bu process’in işi bittiği için çıkış yapıyoruz.
    exit(0); 
}

int main() {
    // İnşaata temel atarak başlıyoruz.
    printf("Temel atılıyor...\n");
    sleep(1);

    // Katlardaki ilerleme durumlarını tutacak shared memory (paylaşımlı bellek) alanı açıyoruz.
    // mmap kullanarak belleği tüm process’ler arasında paylaşılabilir hale getiriyoruz.
    progresses = mmap(NULL, sizeof(int) * KAT_SAYISI,
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (progresses == MAP_FAILED) {
        perror("mmap");
        exit(1);  // mmap başarısızsa çıkış yapıyoruz.
    }

    // Başlangıçta tüm katlar için ilerleme %0
    for (int i = 0; i < KAT_SAYISI; i++) {
        progresses[i] = 0;
    }

    // Her kat için ayrı bir process oluşturuyoruz.
    // Katlar sıralı olarak inşa edilecek, yani önce kat 1 bitmeden kat 2 başlamayacak.
    pid_t pids[KAT_SAYISI];

    for (int kat = 1; kat <= KAT_SAYISI; kat++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Eğer fork sonucu 0 dönerse, bu process çocuktur.
            // O zaman bu çocuk ilgili katın inşaatına başlar.
            kat_insa_et(kat);   // child process
        } else if (pid > 0) {
            // Ebeveyn process ise, çocuğun işini bitirmesini bekler.
            // Böylece katlar sırayla yapılmış olur.
            pids[kat - 1] = pid;
            waitpid(pid, NULL, 0);  // Çocuk işini bitirene kadar bekle
            printf("Kat %d inşaatı tamamlandı.\n", kat);
        } else {
            // Eğer fork başarısız olursa hata mesajı verip programı sonlandırıyoruz.
            perror("fork");
            exit(1);
        }
    }

    // Tüm katlar tamamlandıktan sonra apartmanın tamamı bitmiş oluyor.
    printf("Apartman inşaatı tamamlandı.\n");

    // Kullanılan paylaşımlı bellek serbest bırakılıyor.
    munmap((void*)progresses, sizeof(int) * KAT_SAYISI);
    return 0;
}
