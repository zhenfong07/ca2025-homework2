.text
    .globl hanoi_asm

hanoi_asm:
    # only save used registers
    addi    x2, x2, -32          # Reduced from 56 to 32 bytes
    sw      x8,  0(x2)
    sw      x9,  4(x2)
    sw      x18, 8(x2)
    sw      x19, 12(x2)
    # removed x20, s2-s7 (never used)

    # 3 disk positions stored at offsets 16,20,24
    sw      x0, 16(x2)
    sw      x0, 20(x2)
    sw      x0, 24(x2)

    li      x8, 1

game_loop:
    addi    x5, x0, 8
    beq     x8, x5, finish_game

    srli    x5, x8, 1
    xor     x6, x8, x5

    addi    x7, x8, -1
    srli    x28, x7, 1
    xor     x7, x7, x28

    xor     x5, x6, x7

    addi    x9, x0, 0
    andi    x6, x5, 1
    bne     x6, x0, disk_found
    addi    x9, x0, 1
    andi    x6, x5, 2
    bne     x6, x0, disk_found
    addi    x9, x0, 2

disk_found:
    slli    x5, x9, 2
    addi    x5, x5, 16  #Changed from 44 to 16
    add     x5, x2, x5
    lw      x18, 0(x5)

    bne     x9, x0, handle_large

    addi    x19, x18, 2
    li      x6, 3
    blt     x19, x6, display_move
    sub     x19, x19, x6
    jal     x0, display_move

handle_large:
    lw      x6, 16(x2)  #Changed from 44 to 16
    li      x19, 3
    sub     x19, x19, x18
    sub     x19, x19, x6

display_move:
    addi    t2, x18, 65
    addi    t3, x19, 65

    li      a0, 1
    la      a1, str1
    li      a2, 10
    li      a7, 64
    ecall

    addi    t0, x9, 1
    addi    t0, t0, 48
    la      t1, char_buffer
    sb      t0, 0(t1)

    li      a0, 1
    la      a1, char_buffer
    li      a2, 1
    li      a7, 64
    ecall

    li      a0, 1
    la      a1, str2
    li      a2, 6
    li      a7, 64
    ecall

    la      t1, char_buffer
    sb      t2, 0(t1)

    li      a0, 1
    la      a1, char_buffer
    li      a2, 1
    li      a7, 64
    ecall

    li      a0, 1
    la      a1, str3
    li      a2, 4
    li      a7, 64
    ecall

    la      t1, char_buffer
    sb      t3, 0(t1)

    li      a0, 1
    la      a1, char_buffer
    li      a2, 1
    li      a7, 64
    ecall

    li      t0, 10
    la      t1, char_buffer
    sb      t0, 0(t1)

    li      a0, 1
    la      a1, char_buffer
    li      a2, 1
    li      a7, 64
    ecall

    slli    x5, x9, 2
    addi    x5, x5, 16 #Changed from 44 to 16
    add     x5, x2, x5
    sw      x19, 0(x5)

    addi    x8, x8, 1
    jal     x0, game_loop

finish_game:
    lw      x8,  0(x2)
    lw      x9,  4(x2)
    lw      x18, 8(x2)
    lw      x19, 12(x2)
    # removed loads of x20, s2-s7

    addi    x2, x2, 32 # Changed from 56 to 32
    ret

    .data
str1:        .asciz  "Move Disk "
str2:        .asciz  " from "
str3:        .asciz  " to "
char_buffer: .space  1
