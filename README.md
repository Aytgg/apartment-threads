# Apartment Thread Mutex Project via C

Bu proje, çok katlı bir apartmanın inşaat sürecini simüle ederek işletim sistemlerindeki process (süreç), thread (iş parçacığı) ve senkronizasyon kavramlarını modellemektedir.

## Proje Hakkında

Simülasyon, 10 katlı ve her katta 4 daire bulunan bir apartmanın inşaat sürecini gerçekleştirir. Her kat bir process olarak, her daire ise o process içinde çalışan bir thread olarak temsil edilir. Proje, gerçek bir inşaat sürecindeki senkronizasyon ihtiyaçlarını ve kaynak paylaşımını işletim sistemi kavramları ile modellemektedir.

### Temel Özellikler

- **Process (Süreç) Yapısı:**
  - Her kat bağımsız bir process olarak çalışır (toplam 10 process)
  - Katlar sıralı olarak inşa edilir (üst kat, alt kat bitmeden başlayamaz)
  - Process'ler arası senkronizasyon fork() ve wait() sistem çağrıları ile sağlanır

- **Thread (İş Parçacığı) Yapısı:**
  - Her katta 4 daire bulunur ve her daire bir thread olarak çalışır
  - Daireler kendi içlerinde paralel olarak inşa edilebilir
  - Thread'ler POSIX pthread kütüphanesi kullanılarak yönetilir

- **Senkronizasyon Mekanizmaları:**
  - Mutex kilitleri ile ortak kaynakların (elektrik, su, vinç) kullanımı kontrol edilir
  - Yarış koşulları (race condition) önlenir
  - İlerleme durumu shared memory ile takip edilir

## Gereksinimler

- GCC (GNU Compiler Collection)
- POSIX uyumlu bir işletim sistemi (Linux, macOS)
- pthread kütüphanesi

## Kurulum ve Çalıştırma Adımları

### 1. Gerekli Ortamın Hazırlanması

#### Linux için:
```bash
# GCC ve pthread kütüphanesinin kurulumu
sudo apt-get update
sudo apt-get install build-essential
```

#### macOS için:
```bash
# Xcode Command Line Tools kurulumu (GCC içerir)
xcode-select --install
```

### 2. Projeyi İndirme
```bash
# Projeyi bilgisayarınıza kopyalayın
git clone <proje-url>
cd apartman-insaat
```

### 3. Derleme
```bash
# Projeyi derleyin
gcc -o apartman_insaat main.c -pthread

# Derleme başarılı olduysa apartman_insaat adında bir çalıştırılabilir dosya oluşacaktır
```

### 4. Çalıştırma
```bash
# Programı çalıştırın
./apartman_insaat
```

### 5. Çıktıların Takibi

Program çalıştığında:
- Her katın inşaat durumunu terminal ekranında görebilirsiniz
- Her dairenin yapım aşamalarını anlık olarak takip edebilirsiniz
- İnşaat tamamlandığında toplam süre ve sonuç bilgisi görüntülenecektir

### Hata Durumları ve Çözümleri

1. **Derleme Hatası**: 
   - pthread kütüphanesi bulunamadı hatası alırsanız:
     ```bash
     sudo apt-get install libpthread-stubs0-dev  # Linux için
     ```

2. **Çalıştırma Hatası**:
   - "Permission denied" hatası alırsanız:
     ```bash
     chmod +x apartman_insaat
     ```

3. **MacOS Uyarıları**:
   - MacOS'ta bazı uyarı mesajları görülebilir, bunlar programın çalışmasını etkilemez

### Program Sonlandırma

Program normal şartlarda inşaat tamamlandığında kendini sonlandıracaktır. Eğer programı manuel sonlandırmak isterseniz:
```bash
# Terminal üzerinde Ctrl+C tuş kombinasyonunu kullanabilirsiniz
```

## Program Çıktısı

Program çalıştırıldığında:
1. Önce temel atma işlemi başlar
2. Her kat için ayrı bir process oluşturulur
3. Her katta 4 daire için thread'ler başlatılır
4. Her dairede sırasıyla:
   - Sıva işlemi
   - Elektrik tesisatı
   - Su tesisatı
   - Vinç ile malzeme taşıma
   işlemleri gerçekleştirilir
5. Her işlem tamamlandığında ilerleme durumu ekrana yazdırılır

## Kod Yapısı

- `main.c`: Ana program dosyası
  - Process ve thread oluşturma
  - Senkronizasyon mekanizmaları
  - İnşaat simülasyonu mantığı

### Önemli Fonksiyonlar

- `main()`: Ana program akışını kontrol eder
- `kat_insa_et()`: Her kat için process yönetimini sağlar
- `daire_fonksiyonu()`: Her daire için thread işlemlerini yürütür

## Senkronizasyon Detayları

Projede üç temel mutex kullanılmaktadır:
1. `elektrik_mutex`: Elektrik tesisatı işlemlerini senkronize eder
2. `su_mutex`: Su tesisatı işlemlerini senkronize eder
3. `vinc_mutex`: Vinç kullanımını senkronize eder

Ayrıca `print_mutex` ile ekrana yazdırma işlemleri de senkronize edilmiştir.


| Ad - Soyad     | Öğrenci Numarası |
| :------------- | :--------------- |
| Aytuğ DEĞER    | 22360859014      |
| Elif PAZARBAŞI | 22360859009      |

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://choosealicense.com/licenses/mit/)
