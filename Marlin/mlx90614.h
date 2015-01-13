#ifndef _MLX90164_H
#define _MLX90164_H

#ifdef HEATER_0_USES_MLX90614

#include "Configuration.h"
#include "Wire.h"

// Read temperature from MLX90614 IR sensor

static float mlx90614_i2c_readT( void )
{
   static unsigned long lastRead=0;
   static float temp=20.0;
   uint16_t ret=0;
   uint8_t pec;

   if ( ( millis() - lastRead ) > 500 ) {
      Wire.beginTransmission( (uint8_t)0x5A ); // Fixed device address
      Wire.write(0x07);            // sends register address to read from
      Wire.endTransmission(false); // end transmission
  
      Wire.requestFrom((uint8_t)0x5A, (uint8_t)3 );// send data 3-bytes read
      ret = Wire.read(); // receive DATA
      ret |= Wire.read() << 8; // receive DATA

      pec = Wire.read();
      // Valid range is from -70 to 380
      if ( ( ret > 10000 ) && ( ret <= 32768 ) ) {
         temp=ret*0.02-273.15;
      }
   }

   return temp;
}

void mlx90614_i2c_init()
{
   Wire.begin();
}

#endif //HEATER_0_USES_MLX90614

#endif
