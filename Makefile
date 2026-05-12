# tarsau - Basit Arsivleme Programi
# Sistem Programlama 2025-2026 Bahar Donemi Projesi
# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
TARGET = tarsau
SRC = tarsau.c

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

# Test hedefi: ornek dosyalar olusturur ve programi test eder
test: $(TARGET)
	@echo "=== Test basliyor ==="
	@echo "Merhaba Dunya! Bu birinci dosya." > test1.txt
	@echo "Bu ikinci test dosyasidir.\nIcerik burada." > test2.txt
	@echo "Ucuncu dosya icerigi." > test3.txt
	@echo ""
	@echo "--- Arsivleme testi ---"
	./$(TARGET) -b test1.txt test2.txt test3.txt -o test_arsiv.sau
	@echo ""
	@echo "--- Arsiv dosyasi icerigi ---"
	@cat test_arsiv.sau
	@echo ""
	@echo ""
	@echo "--- Arsivden cikarma testi ---"
	./$(TARGET) -a test_arsiv.sau cikarilan
	@echo ""
	@echo "--- Cikarilan dosyalar ---"
	@ls -la cikarilan/
	@echo ""
	@echo "--- Icerik karsilastirma ---"
	@diff test1.txt cikarilan/test1.txt && echo "test1.txt: OK" || echo "test1.txt: FARKLI!"
	@diff test2.txt cikarilan/test2.txt && echo "test2.txt: OK" || echo "test2.txt: FARKLI!"
	@diff test3.txt cikarilan/test3.txt && echo "test3.txt: OK" || echo "test3.txt: FARKLI!"
	@echo ""
	@echo "=== Test tamamlandi ==="

# Test dosyalarini temizle
testclean: clean
	rm -f test1.txt test2.txt test3.txt test_arsiv.sau
	rm -rf cikarilan
