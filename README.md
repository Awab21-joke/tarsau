# TARSAU - Basit Arşivleme Programı

## Sistem Programlama 2025-2026 Bahar Dönemi Projesi

---

## 1. Proje Hakkında

Bu projede, `tar`, `rar` veya `zip` gibi çalışan ancak sıkıştırma yapmayan **"tarsau"** adlı bir arşivleme programı geliştirilmiştir. Program, Linux/Unix işletim sisteminde C dilinde yazılmış olup komut satırından çalıştırılmaktadır.

### 1.1 Özellikler
- **Arşivleme (`-b`)**: Metin dosyalarını `.sau` formatında birleştirir
- **Çıkarma (`-a`)**: `.sau` arşiv dosyasından dosyaları çıkarır
- **Dosya İzinleri**: Arşivlenen dosyaların izinleri korunur
- **Hata Yönetimi**: Tüm hata durumları uygun mesajlarla ele alınır

### 1.2 Kısıtlamalar
- Giriş dosyaları yalnızca **ASCII metin** dosyaları olabilir
- Maksimum dosya sayısı: **32**
- Maksimum toplam boyut: **200 MB**

---

## 2. Geliştirme Süreci

Proje, git versiyon kontrol sistemi kullanılarak adım adım geliştirilmiştir. Geliştirme süreci aşağıdaki temel aşamalardan oluşmaktadır:

1. **Temel Yapı ve Makefile:** Projenin başlangıç dosyaları (`tarsau.c`, `Makefile`, `.gitignore`) oluşturuldu. Argüman okuma ve temel hata kontrolleri eklendi.
2. **Yardımcı Fonksiyonlar:** Dosyaların ASCII olup olmadığını kontrol eden, dosya boyutunu bulan ve dosya izinlerini (`rwxrwxrwx` formatında) okuyup dönüştüren fonksiyonlar implement edildi.
3. **Arşivleme Modu -b:** `.sau` arşiv dosyasının organizasyon bölümünü ve içerik kısmını kurallara uygun şekilde oluşturan fonksiyonlar yazıldı.
4. **Arşivden Çıkarma Modu -a:** Arşiv dosyasını okuyup ayrıştıran, gerekli dizinleri oluşturan ve dosyaları orijinal izinleriyle geri çıkaran kodlar eklendi.
5. **Test ve Hata Düzeltmeleri:** Sınır durumlar test edildi ve hatalar giderildi.
6. **Raporlama:** Geliştirme süreci ve kod açıklamalarını içeren bu dosya hazırlandı.

---

## 3. Kullanım

### 3.1 Derleme
```bash
make
```

### 3.2 Arşivleme (Dosyaları Birleştirme)
```bash
# Varsayılan çıktı (a.sau)
./tarsau -b dosya1.txt dosya2.txt dosya3.txt

# Belirli çıktı dosyası
./tarsau -b t1.txt t2.txt t3.txt -o arsiv.sau
```
**Örnek Çıktı:** `Dosyalar birlestirildi.`

### 3.3 Arşivden Çıkarma
```bash
# Geçerli dizine çıkar
./tarsau -a arsiv.sau

# Belirli bir dizine çıkar
./tarsau -a arsiv.sau hedef_dizin
```
**Örnek Çıktı:** `hedef_dizin dizininde t1.txt, t2.txt dosyalari acildi.`

---

## 4. .sau Arşiv Dosyası Formatı

Arşiv dosyası iki ana bölümden oluşur:

### 4.1 Format Yapısı
```text
┌──────────────┬─────────────────────────────────────────┬──────────────────────┐
│  10 Bayt     │     Organizasyon Bölümü                 │  Dosya İçerikleri    │
│  (Boyut)     │     |dosya_adi,izinler,boyut|...        │  (Art arda)          │
└──────────────┴─────────────────────────────────────────┴──────────────────────┘
```

### 4.2 Organizasyon (İçerik) Bölümü
- **İlk 10 bayt:** Organizasyon bölümünün toplam boyutunu ASCII formatında içerir
- **Kayıtlar:** `|` karakteri ile ayrılır
- **Alanlar:** Virgülle ayrılmış: `|dosya_adi,izinler,boyut|`

**Örnek:**
`0000000062|test1.txt,rw-r--r--,35||test2.txt,rw-r--r--,42|`

### 4.3 Arşivlenmiş Dosyalar Bölümü
- Dosya içerikleri ayırıcı kullanılmadan art arda yerleştirilir
- Her dosyanın boyutu organizasyon bölümünden okunarak dosya sınırları belirlenir

---

## 5. Kaynak Kod Açıklaması

### 5.1 Dosya Yapısı
```text
proje/
├── tarsau.c       # Ana kaynak kod
├── Makefile       # Derleme dosyası
└── README.md      # Bu rapor/belge
```

### 5.2 Temel Fonksiyonlar

#### `main()` - Giriş Noktası
Komut satırı argümanlarını ayrıştırır ve ilgili modu (`-b` veya `-a`) çağırır.

```c
int main(int argc, char *argv[])
{
    if (argc < 2) return 1;
    if (strcmp(argv[1], "-b") == 0) return arsivle(dosya_sayisi, dosya_adlari, arsiv_adi);
    if (strcmp(argv[1], "-a") == 0) return arsivden_cikar(arsiv_adi, dizin_adi);
}
```

#### `metin_dosyasi_mi()` - ASCII Doğrulama
Dosyanın geçerli bir ASCII metin dosyası olup olmadığını kontrol eder. (0-127 aralığı)

```c
static int metin_dosyasi_mi(const char *dosya_adi)
{
    FILE *fp = fopen(dosya_adi, "rb");
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch > 127) { fclose(fp); return 0; }
    }
    fclose(fp);
    return 1;
}
```

#### `arsivle()` - Arşivleme İşlemi
Dosyaları doğrular, organizasyon bölümünü oluşturur ve dosyaları arşivler.

```c
static int arsivle(int dosya_sayisi, char *dosya_adlari[], const char *arsiv_adi)
{
    // Dosyalari dogrula ve org bolumunu olustur
    fprintf(arsiv_fp, "%010d", org_boyutu);       // 10 bayt boyut yaz
    fwrite(org_bolumu, 1, org_boyutu, arsiv_fp);  // Organizasyon verisini yaz
    // Dosya iceriklerini ekle
}
```

#### `arsivden_cikar()` - Çıkarma İşlemi
Arşivi okur, hedef dizini oluşturur ve dosyaları izinleriyle birlikte çıkarır.

```c
static int arsivden_cikar(const char *arsiv_adi, const char *dizin_adi)
{
    fread(boyut_str, 1, 10, arsiv_fp); // Boyutu oku
    // Organizasyonu ayristir ve dizin olustur
    // Dosyalari disari cikar ve chmod() ile izinleri ata
}
```

#### `izin_str_olustur()` / `izin_str_coz()` - İzin Yönetimi
İzinleri `rwxrwxrwx` string formatına ve geri Linux `mode_t` formatına çevirir.

---

## 6. Hata Yönetimi

Program aşağıdaki hata durumlarını ele alır:
- Dosya bulunamadı -> `'dosya' dosyasi bulunamadi!`
- ASCII olmayan dosya -> `dosya formati uyumsuzdur!`
- 200 MB sınırı aşıldı -> `Toplam boyut 200 MB'i geciyor!`
- 32 dosya sınırı aşıldı -> `En fazla 32 dosya arsivlenebilir!`
- Bozuk arşiv dosyası -> `Arsiv dosyasi uygunsuz veya bozuk!`

---

## 7. Test Sonuçları

Aşağıdaki ekran çıktısında programın başarıyla derlendiği (`make`), hata durumlarının başarıyla yakalandığı ve arşivleme/çıkarma işlemlerinin sorunsuz çalıştığı görülmektedir:

*(PASTE YOUR SCREENSHOT HERE)*

---

## 8. Derleme ve Çalıştırma

```bash
make          # Kodu derler
make clean    # Derlenen programi siler
```
