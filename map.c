#include "map.h"

Map * make_map(size_t size)
{
	Map * map = malloc(sizeof(Map));
	map->keys   = malloc(sizeof(u64)  * size);
	map->values = malloc(sizeof(u64)  * size);
	map->taken  = calloc(size, sizeof(bool));
	map->size = size;
	return map;
}

u64 map_hash(Map * map, u64 key)
{
	return key % map->size;
}

void map_insert(Map * map, u64 key, u64 value)
{
	int position = map_hash(map, key);
	int counter = 0;
	while (map->taken[position]) {
		if (map->keys[position] == key) break;
		assert(counter++ < map->size); // Don't exceed maximum table size
		position = (position + 1) % map->size;
	}
	map->keys[position]   = key;
	map->values[position] = value;
	map->taken[position]  = true;
}

bool map_index(Map * map, u64 key, u64 * value)
{
	int position = map_hash(map, key);
	int counter = 0;
	while (1) {
		if (!map->taken[position]) {
			return false;
		} else if (map->keys[position] == key) {
			break;
		}
		if (counter++ >= map->size) return false;
		position = (position + 1) % map->size;
	}
	if (value) *value = map->values[position];
	return true;
}

int map_iter(Map * map, int iter)
{
	while (1) {
		if (iter + 1 >= map->size) return -1;
		iter++;
		if (map->taken[iter]) return iter;
	}
}

void map_test()
{
	Map * map = make_map(512);
	map_insert(map, 15, 0xDEAD);
	map_insert(map, 527, 0xBEEF);
	u64 u1;
	map_index(map, 15, &u1);
	assert(u1 == 0xDEAD);
	u64 u2;
	map_index(map, 527, &u2);
	assert(u2 == 0xBEEF);
}
