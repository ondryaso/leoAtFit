// htab.h
// Řešení IJC-DU2
// příklad B
// Datum: 12. 4. 2020
// Autor: Ondřej Ondryáš (xondry02), FIT
// -------------------------------------
// Hlavičkový soubor je převzat ze zadání a okomentován podle implementace.
// Funkce jsou implementovány zvlášť v jednotlivých modulech, i když osobně ve většině případů nechápu, jaký
// to má význam, protože většina funkcí pracuje s interními strukturami, které "nejsou mimo knihovnu viditelné",
// vyměňovat tak ve výsledném programu jednu z těchto funkcí bez výměny většiny ostatních nedává valný smysl.

#ifndef __HTAB_H__
#define __HTAB_H__

#include <string.h>
#include <stdbool.h>

struct htab;
typedef struct htab htab_t;

typedef const char *htab_key_t;
typedef int htab_value_t;

struct htab_item;

typedef struct htab_iterator {
    struct htab_item *ptr;
    const htab_t *t;
    size_t idx;
} htab_iterator_t;

// Returns a hash of the specified string, which is used for inserting and looking up an entry in a hashtable
size_t htab_hash_fun(htab_key_t str);

// Creates a new hashtable with the specified number of buckets and returns a pointer to it.
htab_t *htab_init(size_t n);
// Destructs and removes all the entries in a hashtable.
void htab_clear(htab_t *t);
// Removes all the entries in a hashtable and frees the hashtable structure from memory.
void htab_free(htab_t *t);

// Returns the number of entries in a hashtable.
size_t htab_size(const htab_t *t);
// Returns the number of buckets of a hashtable.
size_t htab_bucket_count(const htab_t *t);

// Finds an entry with the specified key in a hashtable and returns an iterator pointing to it.
// If such an entry doesn't exist, returns an "end iterator".
htab_iterator_t htab_find(htab_t *t, htab_key_t key);

// Finds an entry with the specified key in a hashtable and returns an iterator pointing to it.
// If such an entry doesn't exist, creates it, add it to the hashtable and returns an iterator pointing to it.
// If adding the entry fails (e.g. because malloc fails), returns an "end iterator".
htab_iterator_t htab_lookup_add(htab_t *t, htab_key_t key);

// Removes the entry pointed to by the specified iterator from the hashtable it is a part of.
void htab_erase(htab_t *t, htab_iterator_t it);

// Returns an iterator pointing to the first entry in a hashtable.
// If the hashtable is empty, the returned iterator is equal to an "end iterator".
// If t is NULL, returns an invalid iterator.
htab_iterator_t htab_begin(const htab_t *t);

// Returns an "end iterator" associated with the specified hashtable.
// The ptr pointer of the returned htab_iterator structure must not be dereferenced.
htab_iterator_t htab_end(const htab_t *t);

// Returns an iterator pointing to the hashtable entry following the entry pointed to by the specified iterator.
// If there are no more entries in the hashtable, returns an "end iterator".
htab_iterator_t htab_iterator_next(htab_iterator_t it);

// Returns true if the iterator points to data.
// Returns false if the iterator is an "end iterator" or an invalid iterator.
inline bool htab_iterator_valid(htab_iterator_t it) { return it.ptr != NULL; }

// Returns true if the two specified iterators point to the same entry of the same hashtable.
inline bool htab_iterator_equal(htab_iterator_t it1, htab_iterator_t it2) {
    return it1.ptr == it2.ptr && it1.t == it2.t;
}

// Returns the key of the entry pointed to by the specified iterator.
// Returns a NULL pointer if the specified iterator is invalid (or an "end iterator").
htab_key_t htab_iterator_get_key(htab_iterator_t it);

// Returns the value of the entry pointed to by the specified iterator.
// Returns zero if the specified iterator is invalid (or an "end iterator").
htab_value_t htab_iterator_get_value(htab_iterator_t it);

// Sets the value of the entry pointed to by the specified iterator to the specified value and returns the new value.
// If the iterator is invalid (or an "end iterator"), returns a value different from _val_.
htab_value_t htab_iterator_set_value(htab_iterator_t it, htab_value_t val);
#endif // __HTAB_H__