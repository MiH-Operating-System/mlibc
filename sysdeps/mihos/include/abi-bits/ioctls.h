#ifndef _ABIBITS_IOCTLS_H
#define _ABIBITS_IOCTLS_H

/*
 * MiHOS uses Linux's x86-64 ioctl numbers (asm-generic/ioctls.h). Linux's own
 * port takes them from the kernel's headers under the linux option, which
 * MiHOS does not enable, so they are listed here. The ones the kernel
 * implements are also in its <mihos/ioctl.h> (MIHOS_TCGETS ...), with the same
 * values; any other one gets ENOTTY from the kernel, as on Linux for a request
 * the driver does not know.
 */

#define TCGETS 0x5401
#define TCSETS 0x5402
#define TCSETSW 0x5403
#define TCSETSF 0x5404
#define TCGETA 0x5405
#define TCSETA 0x5406
#define TCSETAW 0x5407
#define TCSETAF 0x5408
#define TCSBRK 0x5409
#define TCXONC 0x540A
#define TCFLSH 0x540B
#define TIOCEXCL 0x540C
#define TIOCNXCL 0x540D
#define TIOCSCTTY 0x540E
#define TIOCGPGRP 0x540F
#define TIOCSPGRP 0x5410
#define TIOCOUTQ 0x5411
#define TIOCSTI 0x5412
#define TIOCGWINSZ 0x5413
#define TIOCSWINSZ 0x5414
#define TIOCMGET 0x5415
#define TIOCMBIS 0x5416
#define TIOCMBIC 0x5417
#define TIOCMSET 0x5418
#define TIOCGSOFTCAR 0x5419
#define TIOCSSOFTCAR 0x541A
#define FIONREAD 0x541B
#define TIOCINQ FIONREAD
#define TIOCLINUX 0x541C
#define TIOCCONS 0x541D
#define TIOCGSERIAL 0x541E
#define TIOCSSERIAL 0x541F
#define TIOCPKT 0x5420
#define FIONBIO 0x5421
#define TIOCNOTTY 0x5422
#define TIOCSETD 0x5423
#define TIOCGETD 0x5424
#define TCSBRKP 0x5425
#define TIOCSBRK 0x5427
#define TIOCCBRK 0x5428
#define TIOCGSID 0x5429
#define TIOCGPTN 0x80045430   /* _IOR('T', 0x30, unsigned int) */
#define TIOCSPTLCK 0x40045431 /* _IOW('T', 0x31, int) */
#define TIOCGPTPEER 0x5441    /* _IO('T', 0x41) */
#define TIOCVHANGUP 0x5437
#define FIONCLEX 0x5450
#define FIOCLEX 0x5451
#define FIOASYNC 0x5452

#define TIOCPKT_DATA 0
#define TIOCPKT_FLUSHREAD 1
#define TIOCPKT_FLUSHWRITE 2
#define TIOCPKT_STOP 4
#define TIOCPKT_START 8
#define TIOCPKT_NOSTOP 16
#define TIOCPKT_DOSTOP 32
#define TIOCPKT_IOCTL 64

#endif /* _ABIBITS_IOCTLS_H */
