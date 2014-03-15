/*
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
*/

#include "Marlin.h"
#include "MarlinBT.h"

#ifndef AT90USB
// this next line disables the entire HardwareSerial.cpp, 
// this is so I can support Attiny series and any other chip without a uart
#if defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H) || defined(UBRR2H) || defined(UBRR3H)

#if UART_PRESENT(SERIAL_BT)
  bt_ring_buffer bt_rx_buffer  =  { { 0 }, 0, 0 };
#endif

FORCE_INLINE void store_char(unsigned char c)
{
  int i = (unsigned int)(bt_rx_buffer.head + 1) % BT_RX_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != bt_rx_buffer.tail) {
    bt_rx_buffer.buffer[bt_rx_buffer.head] = c;
    bt_rx_buffer.head = i;
  }
}


//#elif defined(SIG_USART_RECV)
#if defined(BT_USARTx_RX_vect)
  // fixed by Mark Sproul this is on the 644/644p
  //SIGNAL(SIG_USART_RECV)
  SIGNAL(BT_USARTx_RX_vect)
  {
    unsigned char c  =  BT_UDRx;
    store_char(c);
  }
#endif

// Constructors ////////////////////////////////////////////////////////////////

MarlinBT::MarlinBT()
{

}

// Public Methods //////////////////////////////////////////////////////////////

void MarlinBT::begin(long baud)
{
  uint16_t baud_setting;
  bool useU2X = true;

#if F_CPU == 16000000UL && SERIAL_BT == 0
  // hardcoded exception for compatibility with the bootloader shipped
  // with the Duemilanove and previous boards and the firmware on the 8U2
  // on the Uno and Mega 2560.
  if (baud == 57600) {
    useU2X = false;
  }
#endif
  
  if (useU2X) {
    BT_UCSRxA = 1 << BT_U2Xx;
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  } else {
    BT_UCSRxA = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  BT_UBRRxH = baud_setting >> 8;
  BT_UBRRxL = baud_setting;

  sbi(BT_UCSRxB, BT_RXENx);
  sbi(BT_UCSRxB, BT_TXENx);
  sbi(BT_UCSRxB, BT_RXCIEx);
}

void MarlinBT::end()
{
  cbi(BT_UCSRxB, BT_RXENx);
  cbi(BT_UCSRxB, BT_TXENx);
  cbi(BT_UCSRxB, BT_RXCIEx);  
}



int MarlinBT::peek(void)
{
  if (bt_rx_buffer.head == bt_rx_buffer.tail) {
    return -1;
  } else {
    return bt_rx_buffer.buffer[bt_rx_buffer.tail];
  }
}

int MarlinBT::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (bt_rx_buffer.head == bt_rx_buffer.tail) {
    return -1;
  } else {
    unsigned char c = bt_rx_buffer.buffer[bt_rx_buffer.tail];
    bt_rx_buffer.tail = (unsigned int)(bt_rx_buffer.tail + 1) % BT_RX_BUFFER_SIZE;
    return c;
  }
}

void MarlinBT::flush()
{
  // don't reverse this or there may be problems if the RX interrupt
  // occurs after reading the value of bt_rx_buffer_head but before writing
  // the value to bt_rx_buffer_tail; the previous value of bt_rx_buffer_head
  // may be written to bt_rx_buffer_tail, making it appear as if the buffer
  // don't reverse this or there may be problems if the RX interrupt
  // occurs after reading the value of bt_rx_buffer_head but before writing
  // the value to bt_rx_buffer_tail; the previous value of bt_rx_buffer_head
  // may be written to bt_rx_buffer_tail, making it appear as if the buffer
  // were full, not empty.
  bt_rx_buffer.head = bt_rx_buffer.tail;
}




/// imports from print.h




void MarlinBT::print(char c, int base)
{
  print((long) c, base);
}

void MarlinBT::print(unsigned char b, int base)
{
  print((unsigned long) b, base);
}

void MarlinBT::print(int n, int base)
{
  print((long) n, base);
}

void MarlinBT::print(unsigned int n, int base)
{
  print((unsigned long) n, base);
}

void MarlinBT::print(long n, int base)
{
  if (base == 0) {
    write(n);
  } else if (base == 10) {
    if (n < 0) {
      print('-');
      n = -n;
    }
    printNumber(n, 10);
  } else {
    printNumber(n, base);
  }
}

void MarlinBT::print(unsigned long n, int base)
{
  if (base == 0) write(n);
  else printNumber(n, base);
}

void MarlinBT::print(double n, int digits)
{
  printFloat(n, digits);
}

void MarlinBT::println(void)
{
  print('\r');
  print('\n');  
}

void MarlinBT::println(const String &s)
{
  print(s);
  println();
}

void MarlinBT::println(const char c[])
{
  print(c);
  println();
}

void MarlinBT::println(char c, int base)
{
  print(c, base);
  println();
}

void MarlinBT::println(unsigned char b, int base)
{
  print(b, base);
  println();
}

void MarlinBT::println(int n, int base)
{
  print(n, base);
  println();
}

void MarlinBT::println(unsigned int n, int base)
{
  print(n, base);
  println();
}

void MarlinBT::println(long n, int base)
{
  print(n, base);
  println();
}

void MarlinBT::println(unsigned long n, int base)
{
  print(n, base);
  println();
}

void MarlinBT::println(double n, int digits)
{
  print(n, digits);
  println();
}

// Private Methods /////////////////////////////////////////////////////////////

void MarlinBT::printNumber(unsigned long n, uint8_t base)
{
  unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars. 
  unsigned long i = 0;

  if (n == 0) {
    print('0');
    return;
  } 

  while (n > 0) {
    buf[i++] = n % base;
    n /= base;
  }

  for (; i > 0; i--)
    print((char) (buf[i - 1] < 10 ?
      '0' + buf[i - 1] :
      'A' + buf[i - 1] - 10));
}

void MarlinBT::printFloat(double number, uint8_t digits) 
{ 
  // Handle negative numbers
  if (number < 0.0)
  {
     print('-');
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;
  
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    print("."); 

  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = int(remainder);
    print(toPrint);
    remainder -= toPrint; 
  } 
}
// Preinstantiate Objects //////////////////////////////////////////////////////


MarlinBT BTSerial;

#endif // whole file
#endif // !AT90USB

// For AT90USB targets use the UART for BT interfacing
#if defined(AT90USB) && defined (BTENABLED)
   HardwareSerial bt;
#endif

