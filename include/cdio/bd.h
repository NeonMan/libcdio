
/*
    Copyright (C) 2026 J.Luis Alvarez <jlalvarez@heavydeck.net>
    Modeled after libcdio's cdio/dvd.h

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
   \file bd.h 
   \brief Definitions for BluRay access.

   The documents we make use of are described Multi-Media Commands
   (MMC). This document generally has a numeric level number
   appended. For example MMC-6 refers to 'Multi-Media Commands - 6'
   which is the current version in 2026.

*/

#ifndef CDIO_BD_H_
#define CDIO_BD_H_

/* Refer to T10/1675-D Revision 4  -- MMC-5 6.23.3.3 -- BD Disc Structures */
/* Refer to T10/1836-D Revision 2g -- MMC-6 6.22.3.3 -- BD Disc Structures */

#define CDIO_MMC_BD_SET_PAC_ID(cdb, pac_id) \
  do{ \
  cdb[2] = (pac_id >> 16) & 0xff; \
  cdb[3] = (pac_id >>  8) & 0xff; \
  cdb[4] = (pac_id      ) & 0xff; \
  } while(false)

#define CDIO_MMC_BD_SET_PAC_NUMBER(cdb, format_number) \
  do{ \
  cdb[5] = (format_number) & 0xff; \
  } while(false)


#endif
