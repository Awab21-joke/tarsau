/*
 * tarsau - Basit Arsivleme Programi
 * Sistem Programlama 2025-2026 Bahar Donemi Projesi
 *
 * Bu program, tar/rar/zip gibi calisan ancak sikistirma yapmayan
 * bir arsivleme aracidir. Metin dosyalarini .sau formatinda arsivler.
 *
 * Kullanim:
 *   tarsau -b dosya1 dosya2 ... -o arsiv.sau   (Arsivleme)
 *   tarsau -a arsiv.sau [dizin]                 (Arsivden cikarma)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

/* --- Sabitler --- */
#define MAX_DOSYA_SAYISI   32
#define MAX_TOPLAM_BOYUT   (200UL * 1024UL * 1024UL)  /* 200 MB */
#define ORG_BOYUT_ALAN     10   /* Organizasyon bolumu boyut alani (10 bayt) */
#define VARSAYILAN_ARSIV   "a.sau"

/* --- Yardimci Fonksiyon Prototipleri --- */
static int metin_dosyasi_mi(const char *dosya_adi);
static long dosya_boyutu_al(const char *dosya_adi);
static int  dosya_izinleri_al(const char *dosya_adi, char *izin_str);
static void izin_str_olustur(mode_t mode, char *buf);
static mode_t izin_str_coz(const char *izin_str);
static int  dizin_olustur_recursive(const char *yol);

/* --- Ana Islem Fonksiyonlari --- */
static int arsivle(int dosya_sayisi, char *dosya_adlari[], const char *arsiv_adi);
static int arsivden_cikar(const char *arsiv_adi, const char *dizin_adi);

/* ================================================================
 * main - Komut satiri argümanlarini ayristirir ve ilgili islemi cagirir
 * ================================================================ */
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Kullanim:\n");
        fprintf(stderr, "  %s -b dosya1 dosya2 ... [-o arsiv.sau]\n", argv[0]);
        fprintf(stderr, "  %s -a arsiv.sau [dizin]\n", argv[0]);
        return 1;
    }

    /* -b modu: Arsivleme */
    if (strcmp(argv[1], "-b") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Hata: Arsivlenecek dosya belirtilmedi!\n");
            return 1;
        }

        char *dosya_adlari[MAX_DOSYA_SAYISI];
        int dosya_sayisi = 0;
        const char *arsiv_adi = VARSAYILAN_ARSIV;

        /* Argumanlari ayristir: dosya isimleri ve -o parametresi */
        int i = 2;
        while (i < argc) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    arsiv_adi = argv[i + 1];
                    i += 2;
                } else {
                    fprintf(stderr, "Hata: -o parametresinden sonra arsiv adi belirtilmedi!\n");
                    return 1;
                }
            } else {
                if (dosya_sayisi >= MAX_DOSYA_SAYISI) {
                    fprintf(stderr, "Hata: En fazla %d dosya arsivlenebilir!\n", MAX_DOSYA_SAYISI);
                    return 1;
                }
                dosya_adlari[dosya_sayisi++] = argv[i];
                i++;
            }
        }

        if (dosya_sayisi == 0) {
            fprintf(stderr, "Hata: Arsivlenecek dosya belirtilmedi!\n");
            return 1;
        }

        return arsivle(dosya_sayisi, dosya_adlari, arsiv_adi);
    }
    /* -a modu: Arsivden cikarma */
    else if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3 || argc > 4) {
            fprintf(stderr, "Hata: -a parametresinden sonra en fazla 2 parametre alinabilir!\n");
            fprintf(stderr, "Kullanim: %s -a arsiv.sau [dizin]\n", argv[0]);
            return 1;
        }

        const char *arsiv_adi = argv[2];
        const char *dizin_adi = NULL;

        if (argc == 4) {
            dizin_adi = argv[3];
        }

        return arsivden_cikar(arsiv_adi, dizin_adi);
    }
    else {
        fprintf(stderr, "Hata: Bilinmeyen parametre '%s'\n", argv[1]);
        fprintf(stderr, "Kullanim:\n");
        fprintf(stderr, "  %s -b dosya1 dosya2 ... [-o arsiv.sau]\n", argv[0]);
        fprintf(stderr, "  %s -a arsiv.sau [dizin]\n", argv[0]);
        return 1;
    }
}

/* ================================================================
 * metin_dosyasi_mi - Dosyanin ASCII metin dosyasi olup olmadigini kontrol eder
 * Her baytin 0-127 araliginda olmasi gerekir (ASCII).
 * ================================================================ */
static int metin_dosyasi_mi(const char *dosya_adi)
{
    FILE *fp = fopen(dosya_adi, "rb");
    if (!fp) {
        return 0;
    }

    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        /* ASCII: 0-127 arasi. Yalnizca yazdirilabiilir karakterler,
           tab (9), newline (10), carriage return (13) kabul edilir */
        if (ch > 127) {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}

/* ================================================================
 * dosya_boyutu_al - Dosyanin boyutunu bayt olarak dondurur
 * ================================================================ */
static long dosya_boyutu_al(const char *dosya_adi)
{
    struct stat st;
    if (stat(dosya_adi, &st) != 0) {
        return -1;
    }
    return (long)st.st_size;
}

/* ================================================================
 * dosya_izinleri_al - Dosyanin izinlerini rwxrwxrwx formatinda alir
 * ================================================================ */
static int dosya_izinleri_al(const char *dosya_adi, char *izin_str)
{
    struct stat st;
    if (stat(dosya_adi, &st) != 0) {
        return -1;
    }
    izin_str_olustur(st.st_mode, izin_str);
    return 0;
}

/* ================================================================
 * izin_str_olustur - mode_t degerini rwxrwxrwx formatina cevirir
 * ================================================================ */
static void izin_str_olustur(mode_t mode, char *buf)
{
    buf[0] = (mode & S_IRUSR) ? 'r' : '-';
    buf[1] = (mode & S_IWUSR) ? 'w' : '-';
    buf[2] = (mode & S_IXUSR) ? 'x' : '-';
    buf[3] = (mode & S_IRGRP) ? 'r' : '-';
    buf[4] = (mode & S_IWGRP) ? 'w' : '-';
    buf[5] = (mode & S_IXGRP) ? 'x' : '-';
    buf[6] = (mode & S_IROTH) ? 'r' : '-';
    buf[7] = (mode & S_IWOTH) ? 'w' : '-';
    buf[8] = (mode & S_IXOTH) ? 'x' : '-';
    buf[9] = '\0';
}

/* ================================================================
 * izin_str_coz - rwxrwxrwx formatindaki stringi mode_t degerine cevirir
 * ================================================================ */
static mode_t izin_str_coz(const char *izin_str)
{
    mode_t mode = 0;

    if (strlen(izin_str) != 9) {
        return 0644; /* varsayilan izin */
    }

    if (izin_str[0] == 'r') mode |= S_IRUSR;
    if (izin_str[1] == 'w') mode |= S_IWUSR;
    if (izin_str[2] == 'x') mode |= S_IXUSR;
    if (izin_str[3] == 'r') mode |= S_IRGRP;
    if (izin_str[4] == 'w') mode |= S_IWGRP;
    if (izin_str[5] == 'x') mode |= S_IXGRP;
    if (izin_str[6] == 'r') mode |= S_IROTH;
    if (izin_str[7] == 'w') mode |= S_IWOTH;
    if (izin_str[8] == 'x') mode |= S_IXOTH;

    return mode;
}

/* ================================================================
 * dizin_olustur_recursive - Dizinleri iç içe olarak oluşturur (mkdir -p)
 * ================================================================ */
static int dizin_olustur_recursive(const char *yol)
{
    char tmp[1024];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", yol);
    len = strlen(tmp);

    /* Sondaki / isaretini kaldir */
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                perror("mkdir");
                return -1;
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return -1;
    }

    return 0;
}

/* ================================================================
 * arsivle - Dosyalari .sau formatinda arsivler (-b modu)
 *
 * .sau Formati:
 *   [10 bayt: org bolum boyutu][|dosya_adi,izinler,boyut|...][dosya icerikleri]
 * ================================================================ */
static int arsivle(int dosya_sayisi, char *dosya_adlari[], const char *arsiv_adi)
{
    int i;
    unsigned long toplam_boyut = 0;
    long boyutlar[MAX_DOSYA_SAYISI];
    char izinler[MAX_DOSYA_SAYISI][10];

    /* --- Adim 1: Tum giris dosyalarini dogrula --- */
    for (i = 0; i < dosya_sayisi; i++) {
        /* Dosya var mi? */
        if (access(dosya_adlari[i], F_OK) != 0) {
            fprintf(stderr, "Hata: '%s' dosyasi bulunamadi!\n", dosya_adlari[i]);
            return 1;
        }

        /* Metin dosyasi mi? */
        if (!metin_dosyasi_mi(dosya_adlari[i])) {
            fprintf(stderr, "%s giris dosyasinin formati uyumsuzdur!\n", dosya_adlari[i]);
            return 1;
        }

        /* Boyut al */
        boyutlar[i] = dosya_boyutu_al(dosya_adlari[i]);
        if (boyutlar[i] < 0) {
            fprintf(stderr, "Hata: '%s' dosyasinin boyutu alinamadi!\n", dosya_adlari[i]);
            return 1;
        }

        toplam_boyut += (unsigned long)boyutlar[i];

        /* Izinleri al */
        if (dosya_izinleri_al(dosya_adlari[i], izinler[i]) != 0) {
            fprintf(stderr, "Hata: '%s' dosyasinin izinleri alinamadi!\n", dosya_adlari[i]);
            return 1;
        }
    }

    /* Toplam boyut kontrolu (200 MB) */
    if (toplam_boyut > MAX_TOPLAM_BOYUT) {
        fprintf(stderr, "Hata: Giris dosyalarinin toplam boyutu 200 MB'i geciyor!\n");
        return 1;
    }

    /* --- Adim 2: Organizasyon (icerik) bolumunu olustur --- */
    /*
     * Format: |dosya_adi,izinler,boyut|dosya_adi2,izinler2,boyut2|...
     */
    char org_bolumu[65536]; /* Organizasyon bolumu icin yeterli tampon */
    int org_pos = 0;

    for (i = 0; i < dosya_sayisi; i++) {
        int yazilan = snprintf(org_bolumu + org_pos, sizeof(org_bolumu) - org_pos,
                               "|%s,%s,%ld|", dosya_adlari[i], izinler[i], boyutlar[i]);
        if (yazilan < 0 || (size_t)(org_pos + yazilan) >= sizeof(org_bolumu)) {
            fprintf(stderr, "Hata: Organizasyon bolumu icin tampon yetersiz!\n");
            return 1;
        }
        org_pos += yazilan;
    }

    int org_boyutu = org_pos;

    /* --- Adim 3: Arsiv dosyasini yaz --- */
    FILE *arsiv_fp = fopen(arsiv_adi, "w");
    if (!arsiv_fp) {
        fprintf(stderr, "Hata: '%s' arsiv dosyasi olusturulamadi!\n", arsiv_adi);
        return 1;
    }

    /* Ilk 10 bayt: organizasyon bolumunun boyutu (sag tarafa yaslanmis, sola sifir doldurulmus) */
    char boyut_str[ORG_BOYUT_ALAN + 1];
    snprintf(boyut_str, sizeof(boyut_str), "%010d", org_boyutu);
    fwrite(boyut_str, 1, ORG_BOYUT_ALAN, arsiv_fp);

    /* Organizasyon bolumunu yaz */
    fwrite(org_bolumu, 1, org_boyutu, arsiv_fp);

    /* Dosya iceriklerini art arda yaz (ayirici yok) */
    for (i = 0; i < dosya_sayisi; i++) {
        FILE *giris_fp = fopen(dosya_adlari[i], "r");
        if (!giris_fp) {
            fprintf(stderr, "Hata: '%s' dosyasi okunamadi!\n", dosya_adlari[i]);
            fclose(arsiv_fp);
            return 1;
        }

        char tampon[4096];
        size_t okunan;
        while ((okunan = fread(tampon, 1, sizeof(tampon), giris_fp)) > 0) {
            fwrite(tampon, 1, okunan, arsiv_fp);
        }

        fclose(giris_fp);
    }

    fclose(arsiv_fp);
    printf("Dosyalar birlestirildi.\n");
    return 0;
}

/* ================================================================
 * arsivden_cikar - .sau arsiv dosyasindan dosyalari cikarir (-a modu)
 * ================================================================ */
static int arsivden_cikar(const char *arsiv_adi, const char *dizin_adi)
{
    /* Arsiv dosyasinin .sau uzantili olduğunu kontrol et */
    const char *uzanti = strrchr(arsiv_adi, '.');
    if (!uzanti || strcmp(uzanti, ".sau") != 0) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        return 1;
    }

    /* Arsiv dosyasini ac */
    FILE *arsiv_fp = fopen(arsiv_adi, "r");
    if (!arsiv_fp) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        return 1;
    }

    /* --- Adim 1: Ilk 10 bayti oku (organizasyon bolumu boyutu) --- */
    char boyut_str[ORG_BOYUT_ALAN + 1];
    if (fread(boyut_str, 1, ORG_BOYUT_ALAN, arsiv_fp) != ORG_BOYUT_ALAN) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(arsiv_fp);
        return 1;
    }
    boyut_str[ORG_BOYUT_ALAN] = '\0';

    int org_boyutu = atoi(boyut_str);
    if (org_boyutu <= 0) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(arsiv_fp);
        return 1;
    }

    /* --- Adim 2: Organizasyon bolumunu oku --- */
    char *org_bolumu = (char *)malloc(org_boyutu + 1);
    if (!org_bolumu) {
        fprintf(stderr, "Hata: Bellek ayrilamadi!\n");
        fclose(arsiv_fp);
        return 1;
    }

    if ((int)fread(org_bolumu, 1, org_boyutu, arsiv_fp) != org_boyutu) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        free(org_bolumu);
        fclose(arsiv_fp);
        return 1;
    }
    org_bolumu[org_boyutu] = '\0';

    /* --- Adim 3: Organizasyon bolumunu ayristir --- */
    /* Format: |dosya_adi,izinler,boyut|dosya_adi2,izinler2,boyut2| */
    char dosya_adlari[MAX_DOSYA_SAYISI][256];
    char izinler[MAX_DOSYA_SAYISI][10];
    long boyutlar[MAX_DOSYA_SAYISI];
    int dosya_sayisi = 0;

    char *kayit_ptr = org_bolumu;
    while (*kayit_ptr && dosya_sayisi < MAX_DOSYA_SAYISI) {
        /* Basi '|' ise atla */
        if (*kayit_ptr == '|') {
            kayit_ptr++;
        }

        /* Bos string kontrolu */
        if (*kayit_ptr == '\0' || *kayit_ptr == '|') {
            continue;
        }

        /* Bir sonraki '|' isaretini bul */
        char *kayit_sonu = strchr(kayit_ptr, '|');
        if (!kayit_sonu) {
            break;
        }

        /* Kaydi gecici tampon icine kopyala */
        size_t kayit_uzunluk = kayit_sonu - kayit_ptr;
        char kayit[512];
        if (kayit_uzunluk >= sizeof(kayit)) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(org_bolumu);
            fclose(arsiv_fp);
            return 1;
        }
        strncpy(kayit, kayit_ptr, kayit_uzunluk);
        kayit[kayit_uzunluk] = '\0';

        /* Kaydi ayristir: dosya_adi,izinler,boyut */
        char *virgul1 = strchr(kayit, ',');
        if (!virgul1) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(org_bolumu);
            fclose(arsiv_fp);
            return 1;
        }
        *virgul1 = '\0';

        char *virgul2 = strchr(virgul1 + 1, ',');
        if (!virgul2) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(org_bolumu);
            fclose(arsiv_fp);
            return 1;
        }
        *virgul2 = '\0';

        strncpy(dosya_adlari[dosya_sayisi], kayit, sizeof(dosya_adlari[dosya_sayisi]) - 1);
        dosya_adlari[dosya_sayisi][sizeof(dosya_adlari[dosya_sayisi]) - 1] = '\0';

        strncpy(izinler[dosya_sayisi], virgul1 + 1, sizeof(izinler[dosya_sayisi]) - 1);
        izinler[dosya_sayisi][sizeof(izinler[dosya_sayisi]) - 1] = '\0';

        boyutlar[dosya_sayisi] = atol(virgul2 + 1);

        dosya_sayisi++;
        kayit_ptr = kayit_sonu + 1;
    }

    free(org_bolumu);

    if (dosya_sayisi == 0) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(arsiv_fp);
        return 1;
    }

    /* --- Adim 4: Hedef dizini olustur (gerekiyorsa) --- */
    if (dizin_adi != NULL && strlen(dizin_adi) > 0) {
        if (dizin_olustur_recursive(dizin_adi) != 0) {
            fprintf(stderr, "Hata: '%s' dizini olusturulamadi!\n", dizin_adi);
            fclose(arsiv_fp);
            return 1;
        }
    }

    /* --- Adim 5: Dosyalari cikar --- */
    int i;
    for (i = 0; i < dosya_sayisi; i++) {
        /* Hedef dosya yolunu olustur */
        char hedef_yol[1024];
        if (dizin_adi != NULL && strlen(dizin_adi) > 0) {
            snprintf(hedef_yol, sizeof(hedef_yol), "%s/%s", dizin_adi, dosya_adlari[i]);
        } else {
            snprintf(hedef_yol, sizeof(hedef_yol), "%s", dosya_adlari[i]);
        }

        /* Dosyayi olustur ve icerigi yaz */
        FILE *cikti_fp = fopen(hedef_yol, "w");
        if (!cikti_fp) {
            fprintf(stderr, "Hata: '%s' dosyasi olusturulamadi!\n", hedef_yol);
            fclose(arsiv_fp);
            return 1;
        }

        long kalan = boyutlar[i];
        char tampon[4096];
        while (kalan > 0) {
            size_t okunacak = (kalan > (long)sizeof(tampon)) ? sizeof(tampon) : (size_t)kalan;
            size_t okunan = fread(tampon, 1, okunacak, arsiv_fp);
            if (okunan == 0) {
                fprintf(stderr, "Hata: Arsiv dosyasi beklenenden kisa!\n");
                fclose(cikti_fp);
                fclose(arsiv_fp);
                return 1;
            }
            fwrite(tampon, 1, okunan, cikti_fp);
            kalan -= (long)okunan;
        }

        fclose(cikti_fp);

        /* Dosya izinlerini ayarla */
        mode_t mode = izin_str_coz(izinler[i]);
        chmod(hedef_yol, mode);
    }

    fclose(arsiv_fp);

    /* Basari mesaji */
    if (dizin_adi != NULL && strlen(dizin_adi) > 0) {
        printf("%s dizininde ", dizin_adi);
    }

    for (i = 0; i < dosya_sayisi; i++) {
        if (i > 0) printf(", ");
        printf("%s", dosya_adlari[i]);
    }
    printf(" dosyalari acildi.\n");

    return 0;
}
