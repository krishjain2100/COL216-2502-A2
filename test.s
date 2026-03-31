# This is a comment

.A: 10 20 30

addi x1, x0, 0    # x1 = 0
addi x2, x0, 3    # x2 = 3

loop:
    addi x1, x1, 1
    blt x1, x2, loop

end:
    j end