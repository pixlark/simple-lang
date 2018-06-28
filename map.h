#pragma once
#include "common.h"

/*
 * Simple u64->u64 hash map
 */

// TODO(pixlark): Resizing
typedef struct Map {
	u64  * keys;
	u64  * values;
	bool * taken;
	size_t size;
} Map;

Map * make_map(size_t size);
u64 map_hash(Map * map, u64 key);
void map_insert(Map * map, u64 key, u64 value);
bool map_index(Map * map, u64 key, u64 * value);
void map_test();
