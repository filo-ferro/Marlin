/*
  HardwareSerial.h - Hardware serial library for Wiring
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

  Modified 28 September 2010 by Mark Sproul
*/

#ifndef MarlinBT_h
#define MarlinBT_h
#include "Marlin.h"

#if !defined(SERIAL_BT) 
#define SERIAL_BT 1
#endif

// The presence of the UBRRH register is used to detect a UART.
#define UART_PRESENT(port) ((port == 0 && (defined(UBRRH) || defined(UBRR0H))) || \
						(port == 1 && defined(UBRR1H)) || (port == 2 && defined(UBRR2H)) || \
						(port == 3 && defined(UBRR3H)))				
						
// These are macros to build serial port register names for the selected SERIAL_BT (C preprocessor
// requires two levels of indirection to expand macro values properly)
#define BT_SERIAL_REGNAME(registerbase,number,suffix) BT_SERIAL_REGNAME_INTERNAL(registerbase,number,suffix)
#if SERIAL_BT == 0 && (!defined(UBRR0H) || !defined(UDR0)) // use un-numbered registers if necessary
#define BT_SERIAL_REGNAME_INTERNAL(registerbase,number,suffix) registerbase##suffix
#else
#define BT_SERIAL_REGNAME_INTERNAL(registerbase,number,suffix) registerbase##number##suffix
#endif

// Registers used by MarlinBT class (these are expanded 
// depending on selected serial port
#define BT_UCSRxA BT_SERIAL_REGNAME(UCSR,SERIAL_BT,A) // defines BT_UCSRxA to be UCSRnA where n is the serial port number
#define BT_UCSRxB BT_SERIAL_REGNAME(UCSR,SERIAL_BT,B) 
#define BT_RXENx BT_SERIAL_REGNAME(RXEN,SERIAL_BT,)    
#define BT_TXENx BT_SERIAL_REGNAME(TXEN,SERIAL_BT,)    
#define BT_RXCIEx BT_SERIAL_REGNAME(RXCIE,SERIAL_BT,)    
#define BT_UDREx BT_SERIAL_REGNAME(UDRE,SERIAL_BT,)    
#define BT_UDRx BT_SERIAL_REGNAME(UDR,SERIAL_BT,)  
#define BT_UBRRxH BT_SERIAL_REGNAME(UBRR,SERIAL_BT,H)
#define BT_UBRRxL BT_SERIAL_REGNAME(UBRR,SERIAL_BT,L)
#define BT_RXCx BT_SERIAL_REGNAME(RXC,SERIAL_BT,)
#define BT_USARTx_RX_vect BT_SERIAL_REGNAME(USART,SERIAL_BT,_RX_vect)
#define BT_U2Xx BT_SERIAL_REGNAME(U2X,SERIAL_BT,)



#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYTE 0


// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which bt_rx_buffer_head is the index of the
// location to which to write the next incoming character and bt_rx_buffer_tail
// is the index of the location from which to read.
#define BT_RX_BUFFER_SIZE 128


struct bt_ring_buffer
{
  unsigned char buffer[BT_RX_BUFFER_SIZE];
  int head;
  int tail;
};

#if UART_PRESENT(SERIAL_BT)
  extern bt_ring_buffer bt_rx_buffer;
#endif

class MarlinBT //: public Stream
{

  public:
    MarlinBT();
    void begin(long);
    void end();
    int peek(void);
    int read(void);
    void flush(void);
    
    FORCE_INLINE int available(void)
    {
      return (unsigned int)(BT_RX_BUFFER_SIZE + bt_rx_buffer.head - bt_rx_buffer.tail) % BT_RX_BUFFER_SIZE;
    }
    
    FORCE_INLINE void write(uint8_t c)
    {
      while (!((BT_UCSRxA) & (1 << BT_UDREx)))
        ;

      BT_UDRx = c;
    }
    
    
    FORCE_INLINE void checkRx(void)
    {
      if((BT_UCSRxA & (1<<BT_RXCx)) != 0) {
        unsigned char c  =  BT_UDRx;
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
    }
    
    
    private:
    void printNumber(unsigned long, uint8_t);
    void printFloat(double, uint8_t);
    
    
  public:
    
    FORCE_INLINE void write(const char *str)
    {
      while (*str)
        write(*str++);
    }


    FORCE_INLINE void write(const uint8_t *buffer, size_t size)
    {
      while (size--)
        write(*buffer++);
    }

    FORCE_INLINE void print(const String &s)
    {
      for (int i = 0; i < (int)s.length(); i++) {
        write(s[i]);
      }
    }
    
    FORCE_INLINE void print(const char *str)
    {
      write(str);
    }
    void print(char, int = BYTE);
    void print(unsigned char, int = BYTE);
    void print(int, int = DEC);
    void print(unsigned int, int = DEC);
    void print(long, int = DEC);
    void print(unsigned long, int = DEC);
    void print(double, int = 2);

    void println(const String &s);
    void println(const char[]);
    void println(char, int = BYTE);
    void println(unsigned char, int = BYTE);
    void println(int, int = DEC);
    void println(unsigned int, int = DEC);
    void println(long, int = DEC);
    void println(unsigned long, int = DEC);
    void println(double, int = 2);
    void println(void);
};

extern MarlinBT BTSerial;

#endif
