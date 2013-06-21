#pragma once

/* Hi there!  Be sure to include <stddef.h> or <stdlib.h> before
   including b64.h, in order to import the size_t type. */


/* 'out' should be large enough to hold 2x the size of 'in'. */
size_t
b64_encode( const char* in, size_t insize, char* out );

/* 'out' should be at least as large as 'in'. */
size_t
b64_decode( const char* in, size_t insize, char* out );

