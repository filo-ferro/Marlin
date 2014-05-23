#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include "Configuration.h"

void Config_ResetDefault( int s );

#ifndef DISABLE_M503
void Config_PrintSettings( int s );
#else
FORCE_INLINE void Config_PrintSettings( int s ) {}
#endif

#ifdef EEPROM_SETTINGS
void Config_StoreSettings( int s );
void Config_RetrieveSettings( int s );
#else
FORCE_INLINE void Config_StoreSettings( int s ) {}
FORCE_INLINE void Config_RetrieveSettings( int s ) { Config_ResetDefault(); Config_PrintSettings(s); }
#endif

#endif//CONFIG_STORE_H
