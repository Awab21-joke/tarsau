# TARSAU - Basit Arşivleme Programı

## Sistem Programlama 2025-2026 Bahar Dönemi Projesi

---

## 1. Proje Hakkında

Bu projede, `tar`, `rar` veya `zip` gibi çalışan ancak sıkıştırma yapmayan **"tarsau"** adlı bir arşivleme programı geliştirilmiştir. Program, Linux/Unix işletim sisteminde C dilinde yazılmış olup komut satırından çalıştırılmaktadır.

### 1.1 Özellikler

| Özellik | Açıklama |
|---------|----------|
| Arşivleme (`-b`) | Metin dosyalarını `.sau` formatında birleştirir |
| Çıkarma (`-a`) | `.sau` arşiv dosyasından dosyaları çıkarır |
| Dosya İzinleri | Arşivlenen dosyaların izinleri korunur |
| Hata Yönetimi | Tüm hata durumları uygun mesajlarla ele alınır |
| Makefile | Proje `make` komutuyla derlenir |

### 1.2 Kısıtlamalar

- Giriş dosyaları yalnızca **ASCII metin** dosyaları olabilir
- Maksimum dosya sayısı: **32**
- Maksimum toplam boyut: **200 MB**

---

## 2. Kullanım

### 2.1 Derleme

```bash
make
```

### 2.2 Arşivleme (Dosyaları Birleştirme)

```bash
# Varsayılan çıktı (a.sau)
./tarsau -b dosya1.txt dosya2.txt dosya3.txt

# Belirli çıktı dosyası
./tarsau -b t1 t2 t3 t4.txt t5.dat -o arsiv.sau
```

**Örnek Çıktı:**
```
Dosyalar birlestirildi.
```

### 2.3 Arşivden Çıkarma

```bash
# Geçerli dizine çıkar
./tarsau -a arsiv.sau

# Belirli bir dizine çıkar
./tarsau -a arsiv.sau hedef_dizin
```

**Örnek Çıktı:**
```
hedef_dizin dizininde t1, t2, t3, t4.txt, t5.dat dosyalari acildi.
```

---

## 3. .sau Arşiv Dosyası Formatı

Arşiv dosyası iki ana bölümden oluşur:

### 3.1 Format Yapısı

```
┌──────────────┬─────────────────────────────────────────┬──────────────────────┐
│  10 Bayt     │     Organizasyon Bölümü                 │  Dosya İçerikleri    │
│  (Boyut)     │     |dosya_adi,izinler,boyut|...        │  (Art arda)          │
└──────────────┴─────────────────────────────────────────┴──────────────────────┘
```

### 3.2 Organizasyon (İçerik) Bölümü

- **İlk 10 bayt:** Organizasyon bölümünün toplam boyutunu ASCII formatında içerir
- **Kayıtlar:** `|` karakteri ile ayrılır
- **Alanlar:** Virgülle ayrılmış: `|dosya_adi,izinler,boyut|`

**Örnek:**
```
0000000062|test1.txt,rw-r--r--,35||test2.txt,rw-r--r--,42||test3.txt,rw-r--r--,28|
```

### 3.3 Arşivlenmiş Dosyalar Bölümü

- Dosya içerikleri ayırıcı kullanılmadan art arda yerleştirilir
- Her dosyanın boyutu organizasyon bölümünden okunarak dosya sınırları belirlenir

---

## 4. Kaynak Kod Açıklaması

### 4.1 Dosya Yapısı

```
proje/
├── tarsau.c      # Ana kaynak kod
├── Makefile       # Derleme dosyası
└── RAPOR.md       # Bu rapor
```

### 4.2 Temel Fonksiyonlar

#### `main()` - Giriş Noktası
Komut satırı argümanlarını ayrıştırır ve ilgili modu (`-b` veya `-a`) çağırır.

```c
int main(int argc, char *argv[])
{
    if (argc < 2) {
        // Kullanim bilgisi goster
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0) {
        // Arsivleme modu
        return arsivle(dosya_sayisi, dosya_adlari, arsiv_adi);
    }
    else if (strcmp(argv[1], "-a") == 0) {
        // Cikarma modu
        return arsivden_cikar(arsiv_adi, dizin_adi);
    }
}
```

#### `metin_dosyasi_mi()` - ASCII Doğrulama
Dosyanın geçerli bir ASCII metin dosyası olup olmadığını kontrol eder. Her baytın 0-127 aralığında olmasını sağlar.

```c
static int metin_dosyasi_mi(const char *dosya_adi)
{
    FILE *fp = fopen(dosya_adi, "rb");
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch > 127) {
            fclose(fp);
            return 0;  // ASCII degil
        }
    }
    fclose(fp);
    return 1;  // Gecerli ASCII
}
```

#### `arsivle()` - Arşivleme İşlemi
1. Tüm giriş dosyalarını doğrular (varlık, format, boyut)
2. Organizasyon bölümünü oluşturur
3. Arşiv dosyasını yazar (10 bayt boyut + org bölümü + dosya içerikleri)

```c
static int arsivle(int dosya_sayisi, char *dosya_adlari[], const char *arsiv_adi)
{
    // 1. Dogrulama
    for (i = 0; i < dosya_sayisi; i++) {
        if (!metin_dosyasi_mi(dosya_adlari[i])) {
            fprintf(stderr, "%s giris dosyasinin formati uyumsuzdur!\n", dosya_adlari[i]);
            return 1;
        }
    }

    // 2. Organizasyon bolumunu olustur
    for (i = 0; i < dosya_sayisi; i++) {
        snprintf(org_bolumu + org_pos, ..., "|%s,%s,%ld|",
                 dosya_adlari[i], izinler[i], boyutlar[i]);
    }

    // 3. Arsiv dosyasini yaz
    fprintf(arsiv_fp, "%010d", org_boyutu);  // 10 bayt boyut
    fwrite(org_bolumu, 1, org_boyutu, arsiv_fp);  // Org bolumu
    // Dosya iceriklerini art arda yaz
}
```

#### `arsivden_cikar()` - Çıkarma İşlemi
1. Arşiv dosyasını açar ve doğrular
2. Organizasyon bölümünü okur ve ayrıştırır
3. Hedef dizini oluşturur
4. Dosyaları çıkarır ve izinleri ayarlar

```c
static int arsivden_cikar(const char *arsiv_adi, const char *dizin_adi)
{
    // 1. Ilk 10 bayti oku
    fread(boyut_str, 1, 10, arsiv_fp);
    int org_boyutu = atoi(boyut_str);

    // 2. Org bolumunu oku ve ayristir
    // |dosya_adi,izinler,boyut| formatinda kayitlar

    // 3. Dizin olustur (gerekiyorsa)
    dizin_olustur_recursive(dizin_adi);

    // 4. Dosyalari cikar ve izinleri ayarla
    for (i = 0; i < dosya_sayisi; i++) {
        // Dosya icerigini yaz
        // chmod ile izinleri ayarla
    }
}
```

#### `izin_str_olustur()` / `izin_str_coz()` - İzin Yönetimi
Dosya izinlerini `rwxrwxrwx` formatında saklar ve geri çözer.

```c
static void izin_str_olustur(mode_t mode, char *buf)
{
    buf[0] = (mode & S_IRUSR) ? 'r' : '-';
    buf[1] = (mode & S_IWUSR) ? 'w' : '-';
    buf[2] = (mode & S_IXUSR) ? 'x' : '-';
    // ... (grup ve diger izinleri)
}

static mode_t izin_str_coz(const char *izin_str)
{
    mode_t mode = 0;
    if (izin_str[0] == 'r') mode |= S_IRUSR;
    if (izin_str[1] == 'w') mode |= S_IWUSR;
    // ...
    return mode;
}
```

---

## 5. Hata Yönetimi

Program aşağıdaki hata durumlarını ele alır:

| Hata Durumu | Mesaj |
|-------------|-------|
| Dosya bulunamadı | `'dosya' dosyasi bulunamadi!` |
| ASCII olmayan dosya | `dosya giris dosyasinin formati uyumsuzdur!` |
| 200 MB sınırı aşıldı | `Giris dosyalarinin toplam boyutu 200 MB'i geciyor!` |
| 32 dosya sınırı aşıldı | `En fazla 32 dosya arsivlenebilir!` |
| Bozuk arşiv dosyası | `Arsiv dosyasi uygunsuz veya bozuk!` |
| Dizin oluşturma hatası | `'dizin' dizini olusturulamadi!` |

---

## 6. Test Sonuçları

### 6.1 Arşivleme Testi

```bash
$ echo "Merhaba Dunya!" > t1.txt
$ echo "Test dosyasi 2" > t2.txt
$ echo "Ucuncu dosya" > t3.txt
$ ./tarsau -b t1.txt t2.txt t3.txt -o test.sau
Dosyalar birlestirildi.
```

### 6.2 Arşivden Çıkarma Testi

```bash
$ ./tarsau -a test.sau cikarilan
cikarilan dizininde t1.txt, t2.txt, t3.txt dosyalari acildi.
$ diff t1.txt cikarilan/t1.txt  # Fark yok
$ diff t2.txt cikarilan/t2.txt  # Fark yok
$ diff t3.txt cikarilan/t3.txt  # Fark yok
```

### 6.3 Hata Durumu Testleri

```bash
$ ./tarsau -b resim.png -o test.sau
resim.png giris dosyasinin formati uyumsuzdur!

$ ./tarsau -a bozuk.txt
Arsiv dosyasi uygunsuz veya bozuk!
```

---

## 7. Derleme ve Çalıştırma

```bash
# Derleme
make

# Temizleme
make clean

# Otomatik test
make test

# Test dosyalarini temizle
make testclean
```

---

## 8. GitHub Deposu

Proje geliştirme süreci aşağıdaki GitHub adresi üzerinden yürütülmüştür:

> **GitHub:** [https://github.com/Awab21-joke/tarsau](https://github.com/Awab21-joke/tarsau)

---

## 9. Sonuç

`tarsau` programı, belirtilen tüm gereksinimleri karşılayan bir arşivleme aracı olarak başarıyla geliştirilmiştir:

- ✅ ASCII metin dosyalarını `.sau` formatında arşivler
- ✅ Arşiv dosyasından orijinal dosyaları çıkarır
- ✅ Dosya izinlerini korur
- ✅ 200 MB ve 32 dosya sınırlamalarını uygular
- ✅ Hata durumlarını uygun mesajlarla ele alır
- ✅ Makefile ile derlenir
- ✅ Linux/Unix ortamında komut satırından çalışır
