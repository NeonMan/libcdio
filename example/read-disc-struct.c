/*
  Copyright (C) 2019 Thomas Schmitt

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
/* READ DVD STRUCTURE */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <cdio/cdio.h>
#include <cdio/mmc.h>

#define BUFF_LEN (32768 + 1) /* 32K + 1*/

static void
hexdump (FILE *stream,  uint8_t * buffer, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++, buffer++)
    {
	if (i % 16 == 0)
	    fprintf (stream, "0x%04x: ", i);
	fprintf (stream, "%02x", *buffer);
	if (i % 2 == 1)
	    fprintf (stream, " ");
	if (i % 16 == 15) {
	    uint8_t *p;
	    fprintf (stream, "  ");
	    for (p=buffer-15; p <= buffer; p++) {
		fprintf(stream, "%c", isprint(*p) ?  *p : '.');
	    }
	    fprintf (stream, "\n");
	}
    }
    fprintf (stream, "\n");
    fflush (stream);
}

static int
get_disc_structure_data_len(const uint8_t *buff){
    int rv = (buff[0] * 256) + buff[1] + 2;
    if (rv > BUFF_LEN){
        printf("Size %dB (%04Xh) -- Warning! Clipped to %dB\n", rv, rv, BUFF_LEN);
        rv = BUFF_LEN;
    }
    else{
        printf("Size %dB (%04Xh)\n", rv, rv);
    }

    return rv;
}

int
main(int argc, const char *argv[])
{
  CdIo_t *p_cdio;

  const char *psz_drive = NULL;

  /* Put in "/dev/sr0" here. */
  if (argc > 1) psz_drive = argv[1];

  p_cdio = cdio_open (psz_drive, DRIVER_DEVICE);

  if (NULL == p_cdio) {
    printf("Couldn't find DVD\n");
    return 77;
  } else {
    int i_status;                /* Result of MMC command */
    uint8_t buf[BUFF_LEN] = { 0, }; /* Place to hold returned data */
    mmc_cdb_t cdb = {{0, }};     /* Command Descriptor Buffer */
    int i;

    CDIO_MMC_SET_COMMAND(cdb.field, CDIO_MMC_GPCMD_READ_DISC_STRUCTURE);

    CDIO_MMC_SET_READ_LENGTH16(cdb.field, sizeof(buf));

    // Issue READ DISC STRUCTURE for media type 0 (DVD) for formats specified as mandatory on MMC spec 6.23.1
    static const uint8_t dvd_format_codes[] = {0, 1, 2, 3, 4, 5, 13, 0x0f, 0x20, 0x30, 0xff};
    for (i=dvd_format_codes[0]; i<sizeof(dvd_format_codes); i++) {
        memset(buf, 0x55, BUFF_LEN); /* Make unwritten bytes easy to spot */
        printf("== DVD %02Xh =====================================\n", dvd_format_codes[i]);
        cdb.field[7] = dvd_format_codes[i]; /* The format field */
        i_status = mmc_run_cmd(p_cdio, 0, &cdb, SCSI_MMC_DATA_READ,
                    sizeof(buf), &buf);
        if (i_status == 0) {
            hexdump(stdout, buf, get_disc_structure_data_len(buf));
        } else {
            printf("Didn't get DVD Structure.\n");
        }
    }

    
    // Issue READ DISC STRUCTURE for media type 1 (BD) for formats specified on MMC 6.23.3.3
    static const uint8_t bd_format_codes[] = {0x00, 0x08, 0x09, 0x0A, 0x30, 0xFF};
    cdb.field[1] = 0x01; /* Media type (BD) */
    for (i=bd_format_codes[0]; i<sizeof(bd_format_codes); i++) {
        memset(buf, 0x55, BUFF_LEN); /* Make unwritten bytes easy to spot */
        printf("== BD %02Xh ======================================\n", bd_format_codes[i]);
        cdb.field[7] = bd_format_codes[i]; /* The format field */
        i_status = mmc_run_cmd(p_cdio, 0, &cdb, SCSI_MMC_DATA_READ,
                    sizeof(buf), &buf);
        if (i_status == 0) {
            hexdump(stdout, buf, get_disc_structure_data_len(buf));
        } else {
            printf("Didn't get BD Structure.\n");
        }
    }

    //Poll the 0x30 format code with PAC ID and format number set to FF
    //It should enumerate all the PAC headers this drive supports
    CDIO_MMC_BD_SET_PAC_ID(cdb.field, 0xFFFFFF);
    CDIO_MMC_BD_SET_PAC_NUMBER(cdb.field, 0xFF);
    cdb.field[7] = 0x30;

    memset(buf, 0x55, BUFF_LEN); /* Make unwritten bytes easy to spot */
    printf("== BD %02Xh ======================================\n", cdb.field[7]);
    i_status = mmc_run_cmd(p_cdio, 0, &cdb, SCSI_MMC_DATA_READ,
                sizeof(buf), &buf);
    if (i_status == 0) {
        hexdump(stdout, buf, get_disc_structure_data_len(buf));
    } else {
        printf("Didn't get BD Structure.\n");
    }
  }


  cdio_destroy(p_cdio);

  return 0;
}
