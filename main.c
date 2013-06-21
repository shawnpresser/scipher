/*-
 * Copyright 2009 Colin Percival
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "scrypt_platform.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "readpass.h"
#include "scryptenc.h"
#include "warn.h"
#include "b64.h"

static void
usage(void)
{

	fprintf(stderr,
	    "usage: scipher {enc | dec} [...] infile [outfile]\n");
	exit(1);
}

static char*
grow_buffer( char* src, size_t srclen, size_t newsize )
{
  char* grow = calloc( 1, newsize+1 );
  if ( srclen > newsize )
    srclen = newsize;
  memcpy( grow, src, srclen );
  free( src );
  return grow;
}

static char*
read_entire_file( FILE* fp, size_t* out_size )
{
  size_t size = 0;
  size_t max = 1024;
  char* buffer = calloc( 1, max );
  *out_size = 0;

  if ( fp != stdin || !isatty( fileno( stdin ) ) )
  {
    while ( fread( &buffer[size], 1, 1, fp ) )
    {
      ++size;
      if ( size >= max )
      {
        buffer = grow_buffer( buffer, max, 2*max );
        max *= 2;
      }
    }
  }

  *out_size = size;
  return grow_buffer( buffer, max, size );
}

int
main(int argc, char *argv[])
{
	FILE * infile = NULL;
	FILE * outfile = stdout;
  FILE * pwfile = stdin;
	int dec = 0;
	size_t maxmem = 0;
	double maxmemfrac = 0.5;
	double maxtime = 300.0;
	char ch;
	char * passwd;
	int rc;
  uint8_t* input = NULL;
  size_t inputlen = 0;
  size_t passwdlen = 0;

#ifdef NEED_WARN_PROGNAME
	warn_progname = "scipher";
#endif

	/* We should have "enc" or "dec" first. */
	if (argc < 2)
		usage();
	if (strcmp(argv[1], "enc") == 0) {
		maxmem = 0;
		maxmemfrac = 0.125;
		maxtime = 1.0;
	} else if (strcmp(argv[1], "dec") == 0) {
		dec = 1;
	} else
		usage();
	argc--;
	argv++;

	/* Parse arguments. */
	while ((ch = getopt(argc, argv, "hm:M:t:")) != -1) {
		switch (ch) {
		case 'M':
			maxmem = strtoumax(optarg, NULL, 0);
			break;
		case 'm':
			maxmemfrac = strtod(optarg, NULL);
			break;
		case 't':
			maxtime = strtod(optarg, NULL);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 2)
		usage();
  if (dec && argc < 1)
    usage();

	/* Open the input file. */
  if (argc > 0) {
    if ((infile = fopen(argv[0], "r")) == NULL) {
      warn("Cannot open input file: %s", argv[0]);
      exit(1);
    }
    input = read_entire_file( infile, &inputlen );
  } else {
    input = calloc(1, 1);
    inputlen = 0;
  }
  passwd = read_entire_file( pwfile, &passwdlen );

	/* If we have an output file, open it. */
	if (argc > 1) {
		if ((outfile = fopen(argv[1], "w")) == NULL) {
			warn("Cannot open output file: %s", argv[1]);
			exit(1);
		}
	}

	/* Prompt for a password. */
  /*
	if (tarsnap_readpass(&passwd, "Please enter passphrase",
	    dec ? NULL : "Please confirm passphrase", 1))
		exit(1);
    */

	/* Encrypt or decrypt. */
	if (dec) {
    /* Base64 decode. */
    size_t inlen = 0;
    uint8_t* in = calloc( 1, inputlen );
    inlen = b64_decode( (const char*)input, inputlen, in );
    {
      size_t outputlen = inlen;
      uint8_t* output = calloc( 1, outputlen );
      rc = scryptdec_buf( in, inlen,
          output, &outputlen,
          passwd, strlen(passwd),
          maxmem, maxmemfrac, maxtime );
      fwrite( output, outputlen, 1, outfile );
      free( output );
    }
    free( in );
  } else {
    size_t outputlen = inputlen + 128;
    uint8_t* output = calloc( 1, outputlen );
    rc = scryptenc_buf( (const uint8_t*)input, inputlen,
        output,
        passwd, strlen(passwd),
        maxmem, maxmemfrac, maxtime );
    /* Base64 encode. */
    {
      size_t outlen = 0;
      uint8_t* out = calloc( 1, 8*outputlen );
      outlen = b64_encode( (const char*)output, outputlen, out );

      fwrite( out, outlen, 1, outfile );
      /*fwrite( "\n", 1, 1, outfile );*/

      free( output );
      free( out );
    }
  }

	/* Zero and free the password. */
	memset(passwd, 0, strlen(passwd));
	free(passwd);

	/* If we failed, print the right error message and exit. */
	if (rc != 0) {
		switch (rc) {
		case 1:
			warn("Error determining amount of available memory");
			break;
		case 2:
			warn("Error reading clocks");
			break;
		case 3:
			warn("Error computing derived key");
			break;
		case 4:
			warn("Error reading salt");
			break;
		case 5:
			warn("OpenSSL error");
			break;
		case 6:
			warn("Error allocating memory");
			break;
		case 7:
			warnx("Input is not valid scrypt-encrypted block");
			break;
		case 8:
			warnx("Unrecognized scrypt format version");
			break;
		case 9:
			warnx("Decrypting file would require too much memory");
			break;
		case 10:
			warnx("Decrypting file would take too much CPU time");
			break;
		case 11:
			warnx("Passphrase is incorrect");
			break;
		case 12:
			warn("Error writing file: %s",
			    (argc > 1) ? argv[1] : "standard output");
			break;
		case 13:
			warn("Error reading file: %s", argv[0]);
			break;
		}
		exit(1);
	}

	return (0);
}
