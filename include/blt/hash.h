/* $Id: //depot/blt/include/blt/hash.h#3 $
**
** Copyright 1999 Sidney Cammeresi.
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS' AND ANY EXPRESS OR IMPLIED
** WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
** NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _BLT_HASH_H_
#define _BLT_HASH_H_

typedef struct
{
	int valid, dirty, key, dsize;
	void *data;
} hashnode_t;

typedef struct
{
	hashnode_t *table;
	int size_index, used, dirty;
	float max_load;
} hashtable_t;

#ifdef __cplusplus
extern "C" {
#endif

	hashtable_t *hashtable_new (float max_load);
	void hashtable_del (hashtable_t *t);
	void hashtable_insert (hashtable_t *t, int key, void *data, int dsize);
	void *hashtable_lookup (hashtable_t *t, int key, int *dsize);
	void *hashtable_remove (hashtable_t *t, int key, int *dsize);
	void hashtable_print (hashtable_t *t);

#ifdef __cplusplus
}
#endif


#endif

