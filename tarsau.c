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

    if (strcmp(argv[1], "-b") == 0) {
        printf("Arsivleme modu (henuz implement edilmedi)\n");
    }
    else if (strcmp(argv[1], "-a") == 0) {
        printf("Cikarma modu (henuz implement edilmedi)\n");
    }
    else {
        fprintf(stderr, "Hata: Bilinmeyen parametre '%s'\n", argv[1]);
        return 1;
    }

    return 0;
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
