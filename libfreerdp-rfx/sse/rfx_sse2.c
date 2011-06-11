/*
   FreeRDP: A Remote Desktop Protocol client.
   RemoteFX Codec Library - SSE2 Optimizations

   Copyright 2011 Stephen Erisman

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rfx_sse.h"

#include "rfx_sse2.h"

void rfx_decode_YCbCr_to_RGB_SSE2(uint16 * y_r_buffer, uint16 * cb_g_buffer, uint16 * cr_b_buffer)
{
	_mm_prefetch((char*)y_r_buffer, _MM_HINT_NTA);
	_mm_prefetch((char*)cb_g_buffer, _MM_HINT_NTA);
	_mm_prefetch((char*)cr_b_buffer, _MM_HINT_NTA);	
	
	__m128i y_add = _mm_set1_epi16(128);
	__m128i zero = _mm_setzero_si128();
	__m128i max = _mm_set1_epi16(255);
	
	__m128i y, cb, cr;
	__m128i r, g, b;
	
	__m128i * y_r_buf = (__m128i*) y_r_buffer;
	__m128i * cb_g_buf = (__m128i*) cb_g_buffer;
	__m128i * cr_b_buf = (__m128i*) cr_b_buffer;

	int i;
	for (i = 0; i < (4096 / 8); i++)
	{		
		y = _mm_add_epi16(*y_r_buf, y_add);	// y = y_r_buf[i] + 128;
		cb = *cb_g_buf;						// cb = cb_g_buf[i];
		cr = *cr_b_buf;						// cr = cr_b_buf[i];
			
		// r = between(y + cr + (cr >> 2) + (cr >> 3) + (cr >> 5), 0, 255);
		r = _mm_add_epi16(y, cr);
		r = _mm_add_epi16(r, _mm_srai_epi16(cr, 2));
		r = _mm_add_epi16(r, _mm_srai_epi16(cr, 3));
		r = _mm_add_epi16(r, _mm_srai_epi16(cr, 5));
		r = _mm_between_epi16(r, zero, max);
		_mm_store_si128(y_r_buf, r);

		// g = between(y - (cb >> 2) - (cb >> 4) - (cb >> 5) - (cr >> 1) - (cr >> 3) - (cr >> 4) - (cr >> 5), 0, 255);
		g = _mm_sub_epi16(y, _mm_srai_epi16(cb, 2));
		g = _mm_sub_epi16(g, _mm_srai_epi16(cb, 4));
		g = _mm_sub_epi16(g, _mm_srai_epi16(cb, 5));
		g = _mm_sub_epi16(g, _mm_srai_epi16(cr, 1));
		g = _mm_sub_epi16(g, _mm_srai_epi16(cr, 3));
		g = _mm_sub_epi16(g, _mm_srai_epi16(cr, 4));
		g = _mm_sub_epi16(g, _mm_srai_epi16(cr, 5));
		g = _mm_between_epi16(g, zero, max);
		_mm_store_si128(cb_g_buf, g);
		
		// b = between(y + cb + (cb >> 1) + (cb >> 2) + (cb >> 6), 0, 255);
		b = _mm_add_epi16(y, cb);
		b = _mm_add_epi16(b, _mm_srai_epi16(cb, 1));
		b = _mm_add_epi16(b, _mm_srai_epi16(cb, 2));
		b = _mm_add_epi16(b, _mm_srai_epi16(cb, 6));
		b = _mm_between_epi16(b, zero, max);
		_mm_store_si128(cr_b_buf, b);
		
		y_r_buf++;
		cb_g_buf++;
		cr_b_buf++;
	}
}
