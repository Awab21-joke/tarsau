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

/* --- Sabitler --- */
#define MAX_DOSYA_SAYISI   32
#define MAX_TOPLAM_BOYUT   (200UL * 1024UL * 1024UL)  /* 200 MB */
#define ORG_BOYUT_ALAN     10   /* Organizasyon bolumu boyut alani (10 bayt) */
#define VARSAYILAN_ARSIV   "a.sau"

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
