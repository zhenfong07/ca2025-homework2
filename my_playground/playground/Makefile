# Xác định kiến trúc và bộ biên dịch (toolchain)
ARCH = -march=rv32izicsr
CROSS_COMPILE = riscv64-unknown-elf-

# Tệp script liên kết (linker script)
LINKER_SCRIPT = linker.ld

# Đường dẫn đến trình giả lập rv32emu
# Sửa đường dẫn này nếu rv32emu của bạn ở nơi 
EMU = ../../../rv32emu/build/rv32emu
# Cờ (flags) cho trình biên dịch
# Thêm '-mabi=ilp32' để build 32-bit trên toolchain 64-bit
AFLAGS = -g $(ARCH) -mabi=ilp32
CFLAGS = -g -march=rv32i_zicsr -mabi=ilp32 -ffreestanding
LDFLAGS = -T $(LINKER_SCRIPT)

# Tên tệp thực thi cuối cùng
EXEC = test.elf

# Các công cụ
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump

# Các tệp đối tượng (object files) cần build
# Đã xóa 'hanoi.o'. Chỉ build các tệp cho Problem C.
OBJS =  start.o main.o perfcounter.o ticks.o

# Mức tối ưu hóa (O2, Os, O3, Ofast)
OPT_LEVEL = -Ofast

.PHONY: all run dump clean

all: $(EXEC)

# Quy tắc liên kết (linking rule)
# Dùng $(CC) để liên kết, thêm -nostdlib và -lgcc
$(EXEC): $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OPT_LEVEL) -o $@ $(OBJS) -nostdlib -lgcc

# Quy tắc build tệp Assembly (.S)
%.o: %.S
	$(AS) $(AFLAGS) $< -o $@

# Quy tắc build tệp C (.c)
%.o: %.c
	$(CC) $(CFLAGS) $(OPT_LEVEL) $< -o $@ -c

# Quy tắc để chạy
# ĐÃ THÊM cờ '-system'
run: $(EXEC)
	@echo "--- Dang chay test.elf (Che do SYSTEM) ---"
	$(EMU)  $<

# Quy tắc để tháo gỡ (disassemble)
dump: $(EXEC)
	$(OBJDUMP) -Ds $< | less

# Quy tắc để dọn dẹp
clean:
	rm -f $(EXEC) $(OBJS)
