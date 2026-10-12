#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>
#include "uart.h"
#include "../common/common.h"

#define to_hex_digit(n) ('0' + (n) + ((n) < 10 ? 0 : 'a' - '0' - 10))

/*
 * Initialize NS16550A UART
 */
// uart is the most basic io. It allows for reading characters and printing it to a terminal
// it is just a memory region as given by base_addr
// A UART chip takes parallel data (a whole byte from your CPU's bus) and "unrolls" it into a serial stream of 1s and 0s over a single wire (the TX line). On the flip side, it catches a stream of bits on the RX line and "rolls" them back up into a byte for your CPU to read.
void uart_init(size_t base_addr) {
	// ptr refers to the start of uart
	// the memory regions from an offset of 0-5 mean different things
  volatile uint8_t *ptr = (uint8_t *)base_addr;

  // Set word length to 8 (LCR[1:0])
  const uint8_t LCR = 0b11;
	// memory offset 3 is line control register which controls info sent over the 'wire'
	// 0b11 simply means send groups of 8 bits(roughly; not super accurate); there's 00, 01, 10 as well for 5,6,7 respectively
	// the other bits of this memory region control other information and probably the most interesting is bit 7 which essentially
	// changes the meaning of the settings and allows for controlling baud rate(the sender and receiver rate must match)
	// other settings are not so important but note them in this order: stop bit, parity bits, dlab(bit 7)
  ptr[3] = LCR;

  // Enable FIFO (FCR[0])
	// this controls whether bits are overwritten or new incoming bits are simply discarded(roughly)
  ptr[2] = 0b1;

  // Enable receiver buffer interrupts (IER[0])
	// this basically allows for interrupting the cpu when data is received by uart. which then jumps to mtvec
  ptr[1] = 0b1;

  // For a real UART, we need to compute and set the baud rate
  // But since this is an emulated UART, we don't need to do anything
  //
  // Assuming clock rate of 22.729 MHz, set signaling rate to 2400 baud
  // divisor = ceil(CLOCK_HZ / (16 * BAUD_RATE))
  //         = ceil(22729000 / (16 * 2400))
  //         = 592
  //
  // uint16_t divisor = 592;
  // uint8_t divisor_least = divisor & 0xFF;
  // uint8_t divisor_most = divisor >> 8;
  // ptr[3] = LCR | 0x80;
  // ptr[0] = divisor_least;
  // ptr[1] = divisor_most;
  // ptr[3] = LCR;
}

static void uart_put(size_t base_addr, uint8_t c) {
  *(uint8_t *)base_addr = c;
}

int kputchar(int character) {
  uart_put(UART_ADDR, (uint8_t)character);
  return character;
}

// assumes str is null terminated
static void kprint(const char *str) {
  while (*str) {
    kputchar((int)*str);
    ++str;
  }
}

int kputs(const char *str) {
  kprint(str);
  kputchar((int)'\n');
  return 0;
}

// Limited version of vprintf() which only supports the following specifiers:
//
// - d/i: Signed decimal integer
// - u: Unsigned decimal integer
// - o: Unsigned octal
// - x: Unsigned hexadecimal integer
// - X: Unsigned hexadecimal integer (uppercase)
// - c: Character
// - s: String of characters
// - p: Pointer address
// - %: Literal '%'
//
// None of the sub-specifiers are supported for the sake of simplicity.
// The `n` specifier is not supported since that is a major source of
// security vulnerabilities. None of the floating-point specifiers are
// supported since floating point operations don't make sense in kernel
// space
//
// Anyway, this subset should suffice for printf debugging
void kvprintf(const char *format, va_list arg) {
  while (*format) {
    if (*format == '%') {
      ++format;
      if (!*format)
	return;
      switch (*format) {
      case 'd':
      case 'i':
	{
	  int n = va_arg(arg, int);
	  if (n == INT_MIN) {
	    kprint("-2147483648");
	    break;
	  }
	  if (n < 0) {
	    kputchar('-');
	    n = ~n + 1;
	  }
	  char lsh = '0' + n % 10;
	  n /= 10;
	  char buf[9];
	  char *p_buf = buf;
	  while (n) {
            *p_buf++ = '0' + n % 10;
	    n /= 10;
	  }
	  while (p_buf != buf)
	    kputchar(*--p_buf);
	  kputchar(lsh);
	}
	break;
      case 'u':
        {
	  unsigned n = va_arg(arg, unsigned);
	  char lsh = '0' + n % 10;
	  n /= 10;
	  char buf[9];
	  char *p_buf = buf;
	  while (n) {
            *p_buf++ = '0' + n % 10;
	    n /= 10;
	  }
	  while (p_buf != buf)
	    kputchar(*--p_buf);
	  kputchar(lsh);
	}
	break;
      case 'o':
        {
	  unsigned n = va_arg(arg, unsigned);
	  char lsh = '0' + n % 8;
	  n /= 8;
	  char buf[10];
	  char *p_buf = buf;
	  while (n) {
            *p_buf++ = '0' + n % 8;
	    n /= 8;
	  }
	  while (p_buf != buf)
	    kputchar(*--p_buf);
	  kputchar(lsh);
	}
	break;
      case 'x':
        {
	  unsigned n = va_arg(arg, unsigned);
	  char lsh = to_hex_digit(n % 16);
	  n /= 16;
	  char buf[7];
	  char *p_buf = buf;
	  while (n) {
            *p_buf++ = to_hex_digit(n % 16);
	    n /= 16;
	  }
	  while (p_buf != buf)
	    kputchar(*--p_buf);
	  kputchar(lsh);
	}
	break;
      case 'X':
        {
	  unsigned n = va_arg(arg, unsigned);
	  char lsh = to_hex_digit(n % 16);
	  n /= 16;
	  char buf[7];
	  char *p_buf = buf;
	  while (n) {
            *p_buf++ = to_hex_digit(n % 16);
	    n /= 16;
	  }
	  while (p_buf != buf)
	    kputchar(toupper(*--p_buf));
	  kputchar(toupper(lsh));
	}
	break;
      case 'c':
	kputchar(va_arg(arg, int));
	break;
      case 's':
	kprint(va_arg(arg, char *));
	break;
      case 'p':
	{
          kprint("0x");
	  size_t ptr = va_arg(arg, size_t);
	  char lsh = to_hex_digit(ptr % 16);
	  ptr /= 16;
	  char buf[15];
	  char *p_buf = buf;
	  while (ptr) {
            *p_buf++ = to_hex_digit(ptr % 16);
	    ptr /= 16;
	  }
	  while (p_buf != buf)
	    kputchar(*--p_buf);
	  kputchar(lsh);
	}
	break;
      case '%':
	kputchar('%');
	break;
      default:
	kputchar('%');
	kputchar(*format);
      }
    } else
      kputchar(*format);
    ++format;
  }
}

void kprintf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  kvprintf(format, arg);
  va_end(arg);
}