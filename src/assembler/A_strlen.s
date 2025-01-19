.def	TekstiPikkus;	.scl	2;	.type	32;	.endef     # Määratleb TekstiPikkus funktsiooni
TekstiPikkus:
    pushq	%rbp                  # Salvestab rbp algse väärtuse, st base pointeri, stacki.
    movq	%rsp, %rbp            # Paneb rbp väärtuseks praegusele stack pointeri.
    movq	%rcx, %rdi            # paneb rdi väärtuseks esimese argumendi, st teksti mäluaadressi
    xor	%rax, %rax                # rax nulliks.
.Tsukkel:
    cmpb	$0, (%rdi,%rax)       # Võrdleb baiti aadressil (%rdi + %rax) väärtusega 0, st '\0'.
    je	.LeitiNull                # Kui bait on 0, läheb täitma koodi nimega LeitiNull
    inc	%rax                      # Suurendab rax väärtust, st rax hakkab viitama järgmisele tähele.
    jmp	.Tsukkel                  # Uuesti tsükkel algusest.
.LeitiNull:
    popq	%rbp                  # Taastab base pointeri.
    ret                           # Retiga tagastatakse rax väärtus. See sisaldab teksti pikkust.