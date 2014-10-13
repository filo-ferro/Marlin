/* -*- c++ -*- */

#include "Marlin.h"
#include "memreader.h"
#include "stepper.h"
#include "temperature.h"
#include "language.h"
#include "ultralcd.h"

#include "language.h"

#if LANGUAGE_CHOICE == 7
#if MACHINE_3D == 1
#include "sc_3d_level_plate_a.h"
#include "sc_3d_level_plate_m.h"
#else
#include "sc_level_plate_a.h"
#include "sc_level_plate_m.h"
#endif
#elif LANGUAGE_CHOICE == 1
#if MACHINE_3D == 1
#include "sc_3d_level_plate_a_en.h"
#include "sc_3d_level_plate_m_en.h"
#else
#include "sc_level_plate_a_en.h"
#include "sc_level_plate_m_en.h"
#endif
#else
#include "sc_level_plate_a.h"
#include "sc_level_plate_m.h"
#endif

#if MACHINE_3D == 1
#include "sc_3d_change_left.h"
#include "sc_3d_change_right.h"
#else
#include "sc_change_left.h"
#include "sc_change_right.h"
#endif

#include "sc_nozzles.h"
#include "sc_movedown.h"

#if EXTRUDERS > 1
#include "sc_dual.h"
#endif

MemReader::MemReader()
{
   program = 0;
   pos = 0;
   isprinting = false;

   Script = { NULL,
                              LEVEL_PLATE_A,
                              LEVEL_PLATE_M,
                              CHANGE_RIGHT,
                              MOVEDOWN,
#if EXTRUDERS > 1
                              CHANGE_LEFT,
                              DUAL
#else
                              NULL
#endif
};

  // Utilities program length
  ScriptLength = { -1,
                                     LEVEL_PLATE_A_LENGTH,
                                     LEVEL_PLATE_M_LENGTH,
                                     CHANGE_RIGHT_LENGTH,
                                     MOVEDOWN_LENGTH,
#if EXTRUDERS > 1
                                     CHANGE_LEFT_LENGTH,
                                     DUAL_LENGTH
#else
                                     -1
#endif
};

}

void MemReader::startMemprint(const uint8_t nr)
{
  if ( ScriptLength[nr] > 0 ) {
    program = nr;
    pos = 0;
    isprinting = true;
    lcd_ForceStatusScreen(true);
    lcd_ForceStatusScreen(false);
  }
}

void MemReader::pauseMemPrint( void )
{
   isprinting=false;
}

void MemReader::resumeMemPrint( void )
{
   isprinting=true;
}


/* ??? Check se serve */
void MemReader::getStatus()
{
  if(isprinting){
    SERIAL_PROTOCOLPGM(MSG_SD_PRINTING_BYTE);
    SERIAL_PROTOCOL(pos);
    SERIAL_PROTOCOLPGM("/");
    SERIAL_PROTOCOLLN(ScriptLength[program]);
  }
  else{
    SERIAL_PROTOCOLLNPGM(MSG_SD_NOT_PRINTING);
  }
}

void MemReader::printingHasFinished()
{
    st_synchronize();
    quickStop();
    isprinting = false;
    autotempShutdown();
}
