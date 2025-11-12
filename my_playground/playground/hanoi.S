    .text
    .globl hanoi_asm

hanoi_asm:
    addi    x2, x2, -56
    sw      x8,  0(x2)
    sw      x9,  4(x2)
    sw      x18, 8(x2)
    sw      x19, 12(x2)
    sw      x20, 16(x2)
    sw      s2, 20(x2)
    sw      s3, 24(x2)
    sw      s4, 28(x2)
    sw      s5, 32(x2)
    sw      s6, 36(x2)
    sw      s7, 40(x2)

    # 3 disk positions stored at offsets 44,48,52
    sw      x0, 44(x2)
    sw      x0, 48(x2)
    sw      x0, 52(x2)

    li      x8, 1             # iteration counter

game_loop:
    # stop after 7 moves for 3 disks (2^3 - 1 = 7)
    addi    x5, x0, 8
    beq     x8, x5, finish_game

    # Gray code n: g(n) = n XOR (n >> 1)
    srli    x5, x8, 1
    xor     x6, x8, x5

    # Gray code n-1: g(n-1) = (n-1) XOR ((n-1)>>1)
    addi    x7, x8, -1
    srli    x28, x7, 1
    xor     x7, x7, x28

    # changed bit = g(n) XOR g(n-1)
    xor     x5, x6, x7

    # determine disk number
    addi    x9, x0, 0
    andi    x6, x5, 1
    bne     x6, x0, disk_found
    addi    x9, x0, 1
    andi    x6, x5, 2
    bne     x6, x0, disk_found
    addi    x9, x0, 2

disk_found:
    # base position entry = 44 + disk*4
    slli    x5, x9, 2
    addi    x5, x5, 44
    add     x5, x2, x5
    lw      x18, 0(x5)        # x18 = old source peg

    # compute destination peg
    bne     x9, x0, handle_large

    # disk 0 moves +2 mod 3
    addi    x19, x18, 2
    li      x6, 3
    blt     x19, x6, display_move
    sub     x19, x19, x6
    jal     x0, display_move

handle_large:
    lw      x6, 44(x2)        # pos of largest disk
    li      x19, 3
    sub     x19, x19, x18
    sub     x19, x19, x6

display_move:
    # x9  = disk index
    # x18 = source peg index
    # x19 = destination peg index

    # convert peg index → ASCII ('A'+index)
    addi    t2, x18, 65       # source char
    addi    t3, x19, 65       # dest char

    # ----- print "Move Disk " -----
    li      a0, 1
    la      a1, str1
    li      a2, 10
    li      a7, 64
    ecall

    # ----- disk number (ASCII) -----
    addi    t0, x9, 1         # disk number 1..3
    addi    t0, t0, 48        # '0' + number
    la      t1, char_buffer
    sb      t0, 0(t1)

    li      a0, 1
    la      a1, char_buffer
    li      a2, 1
    li      a7, 64
    ecall

    # ----- " from " -----
    li      a0, 1
    la      a1, str2
    li      a2, 6
    li      a7, 64
    ecall

    # ----- source peg -----
    la      t1, char_buffer
    sb      t2, 0(t1)

    li      a0, 1
    la      a1, char_buffer
    li      a2, 1
    li      a7, 64
    ecall

    # ----- " to " -----
    li      a0, 1
    la      a1, str3
    li      a2, 4
    li      a7, 64
    ecall

    # ----- destination peg -----
    la      t1, char_buffer
    sb      t3, 0(t1)

    li      a0, 1
    la      a1, char_buffer
    li      a2, 1
    li      a7, 64
    ecall

    # ----- newline -----
    li      t0, 10
    la      t1, char_buffer
    sb      t0, 0(t1)

    li      a0, 1
    la      a1, char_buffer
    li      a2, 1
    li      a7, 64
    ecall

    # update disk position table
    slli    x5, x9, 2
    addi    x5, x5, 44
    add     x5, x2, x5
    sw      x19, 0(x5)

    addi    x8, x8, 1
    jal     x0, game_loop

# -------------------------------------------------------
# RETURN
# -------------------------------------------------------
finish_game:
    lw      x8,  0(x2)
    lw      x9,  4(x2)
    lw      x18, 8(x2)
    lw      x19, 12(x2)
    lw      x20, 16(x2)
    lw      s2, 20(x2)
    lw      s3, 24(x2)
    lw      s4, 28(x2)
    lw      s5, 32(x2)
    lw      s6, 36(x2)
    lw      s7, 40(x2)

    addi    x2, x2, 56
    ret


# -------------------------------------------------------
# DATA
# -------------------------------------------------------
    .data
str1:        .asciz  "Move Disk "
str2:        .asciz  " from "
str3:        .asciz  " to "
char_buffer: .space  1
