/* $Id: //depot/blt/lib/libblt/hash.c#3 $
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

/*
 * these algorithms are based on knuth's algorithm 6.4d, as described in
 * vol. 3, p. 529 of taocp.
 *
 * XXX - keys must be unique!
 */

/*

the primes were computed by using the following program:

int main (void)
{
    unsigned int i, j, good, last;

    for (i = 17, last = 0; i < 65536 * 16384 - 1; i++)
        {   
            for (j = 2, good = 1; (j <= floor (sqrt ((double) i))) && good; j++)
                if (!(i % j))
                    good = 0;
            if (good)
                {
                    if (last == i - 2)
                        {
                            printf ("%d %d\n", last, i);
                            i *= 2;
                        }
                    last = i;
                }
        }

    return 0;
}

*/

#include <stddef.h>
#include <stdlib.h>
#include <blt/hash.h>

static unsigned int table_size[] = { 7, 19, 43, 103, 229, 463, 1021, 2083, 4219,
	8539, 17191, 34471, 69031, 138079, 276373, 552751, 1105549, 2211259,
	4422619, 8845453, 17691043, 35382091, 70764259, 141528559, 283057213,
	566115259 };

static int hash (int key, int size);
static int rehash (int key, int size);
static void hashtable_rebuild (hashtable_t *t, int dir);

static int hash (int key, int size)
{
	return key % table_size[size];
}

static int rehash (int key, int size)
{
	return 1 + (key % (table_size[size] - 2));
}

hashtable_t *hashtable_new (float max_load)
{
	int i;
	hashtable_t *t;

	t = (hashtable_t *) malloc (sizeof (hashtable_t));
	t->size_index = 0;
	t->used = t->dirty = 0;
	t->max_load = max_load;
	t->table = (hashnode_t *) malloc (sizeof (hashnode_t) * *table_size);
	for (i = 0; i < table_size[t->size_index]; i++)
		t->table[i].valid = t->table[i].dirty = 0;
	return t;
}

void hashtable_del (hashtable_t *t)
{
	free (t->table);
}

void hashtable_insert (hashtable_t *t, int key, void *data, int dsize)
{
	int i, c;

	if (t->used > (t->max_load * table_size[t->size_index]))
		hashtable_rebuild (t, 1);

	i = hash (key, t->size_index);
	if (t->table[i].valid)
		do
			{
				c = rehash (key, t->size_index);
				i -= c;
				if (i < 0)
					i += table_size[t->size_index];
			}
		while (t->table[i].valid && !t->table[i].dirty);
	if (!t->table[i].dirty)
		t->used++;
	t->table[i].valid = 1;
	t->table[i].dirty = 0;
	t->table[i].key = key;
	t->table[i].dsize = dsize;
	t->table[i].data = data;
}

static void hashtable_rebuild (hashtable_t *t, int dir)
{
	int i, old_index;
	hashnode_t *old;

	old = t->table;
	old_index = t->size_index;
	switch (dir)
		{
			case -1:
				t->size_index = t->size_index ? t->size_index - 1 : 0;
				break;
			case 0:
				break;
			case 1:
				t->size_index++;
				break;
			default:
				return;
		}

	t->used = t->dirty = 0;
	t->table = (hashnode_t *) malloc (sizeof (hashnode_t) *
		table_size[t->size_index]);
	for (i = 0; i < table_size[t->size_index]; i++)
		t->table[i].valid = t->table[i].dirty = 0;
	for (i = 0; i < table_size[old_index]; i++)
		if (old[i].valid && !old[i].dirty)
			hashtable_insert (t, old[i].key, old[i].data, old[i].dsize);
	free (old);
}

void *hashtable_lookup (hashtable_t *t, int key, int *dsize)
{
	int i, c;

	i = hash (key, t->size_index);
	while (t->table[i].valid)
		{
			if ((t->table[i].key == key) && !t->table[i].dirty)
				{
					if (dsize != NULL)
						*dsize = t->table[i].dsize;
					return t->table[i].data;
				}
			c = rehash (key, t->size_index);
			i -= c;
			if (i < 0)
				i += table_size[t->size_index];
		}
	if (dsize != NULL)
		*dsize = 0;
	return NULL;
}

void *hashtable_remove (hashtable_t *t, int key, int *dsize)
{
	int i, c;
	void *temp;

	i = hash (key, t->size_index);
	while (t->table[i].valid)
		{
			if ((t->table[i].key == key) && !t->table[i].dirty)
				{
					if (dsize != NULL)
						*dsize = t->table[i].dsize;
					temp = t->table[i].data;
					t->table[i].dirty = 1;
					t->dirty++;
					if ((t->dirty > ((1 - t->max_load) *
							table_size[t->size_index])) && t->size_index)
						hashtable_rebuild (t, -1);
					return temp;
				}
			c = rehash (key, t->size_index);
			i -= c;
			if (i < 0)
				i += table_size[t->size_index];
		}
	if (dsize != NULL)
		*dsize = 0;
	return NULL;
}

/*
void hashtable_print (hashtable_t *t)
{
	int i;

	printf ("used:  %d\ndirty: %d\n\n", t->used, t->dirty);
	for (i = 0; i < table_size[t->size_index]; i++)
		printf ("%3d: %d %d %3d %3d %3d\n", i, t->table[i].valid,
			t->table[i].dirty, t->table[i].key,
			t->table[i].data, t->table[i].dsize);
}
*/

