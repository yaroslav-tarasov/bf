#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "hash_table.h"

/* suppose a user wants to insert stuff
 * into the hash table all she needs to do
 * is make sure hash_entry is part of the item
 *
 * Note: a single hash_table can contain multiple
 * types as the hash_table is oblivious to user defined
 * types. However, at run time you need to know/guess the
 * type of data each hash_entry is holding. You can, for
 * example, reserve the first byte of your data for holding
 * this type info.
 */
struct myitem {
	struct hash_entry entry;
	unsigned char foo[32];
	unsigned int bar;
};

int main(int argc, char **argv)
{
	struct myitem *tmp;
	struct hash_table mytable;
	struct hash_entry *hentry;
	unsigned char str[256];
	unsigned int len, i, flowid;

	/* initialize the hash table with 10 buckets */
	hash_table_init(&mytable, 10, NULL);

	fprintf(stdout, "input\n");
	for (i = 0; i < 5; ++i) {
		fprintf(stdout, "key: ");
		fscanf(stdin, "%s", str);
		len = strlen(str);
		fprintf(stdout, "flowid: ");
		fscanf(stdin, "%d", &flowid);

		tmp = (struct myitem *)malloc(sizeof(struct myitem));
		memset(tmp, 0, sizeof(struct myitem));
		memcpy(tmp->foo, str, len);
		tmp->bar = flowid;

		/* insert this entry into hash table mytable with
		 * str (of length len) as the key
		 */
		hash_table_insert(&mytable, &tmp->entry, str, len);
	}

	for (i = 0; i < 3; ++i) {
		fprintf(stdout, "enter key to search:");
		fscanf(stdin, "%s", str);

		if ((hentry =
		     hash_table_lookup_key(&mytable, str,
					   strlen(str))) == NULL) {
			fprintf(stdout, "could not find entry for %s\n", str);
		} else {
			/* just like the list_item() */
			tmp = hash_entry(hentry, struct myitem, entry);
			fprintf(stdout, "key=%s\tfoo=%s\tbar=%d\n",
				str, tmp->foo, tmp->bar);
		}
	}

	/* loop thru all the entries in the hash-table */
	fprintf(stdout, "output\n");
	hash_table_for_each(hentry, &mytable) {
		tmp = hash_entry(hentry, struct myitem, entry);
		fprintf(stdout, "key: %s[%d] (%d)\n", tmp->foo, tmp->bar,
			hash_table_hash_code(&mytable, tmp->foo,
					     strlen(tmp->foo)));
	}

	/* finitialze the hash table */
	hash_table_finit(&mytable);
	return 0;
}
