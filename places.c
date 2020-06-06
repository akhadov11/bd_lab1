#include <stdio.h>
#include <stdlib.h>
#include "classrooms.h"

const int NAME_LENGTH = 20;
const char *INDEX_placeS_FILE = "places.idx";
const char *DATA_placeS_FILE = "places.fl";
const char *GARBAGE_placeS_FILE = "places.gb";

struct place {
  int id;
  int people;
  int classroom_id;
};

struct Index_row {
  int id;
  int address;
};

void read_place_from_index(FILE* fptr, int *key, int *address) {
  fread(key, sizeof(int), 1, fptr);
  fread(address, sizeof(int), 1, fptr);
}

int get_place_address(int index) {
  FILE *index_fptr = fopen(INDEX_placeS_FILE, "r");
  fseek(index_fptr, index * sizeof(struct Index_row) + sizeof(int), SEEK_SET);
  int address;
  fread(&address, sizeof(int), 1, index_fptr);
  fclose(index_fptr);
  return address;
}

int get_address_for_place() {
  FILE *garbage_fptr = fopen(GARBAGE_placeS_FILE, "r+");
  if (garbage_fptr == NULL) {
    garbage_fptr = fopen(GARBAGE_placeS_FILE, "w");
    int zero = 0;
    fwrite(&zero, sizeof(int), 1, garbage_fptr);
  }
  fseek(garbage_fptr, 0, SEEK_END);
  int n = ftell(garbage_fptr);
  fseek(garbage_fptr, 0, SEEK_SET);
  if (n == 0) {
    int zero = 0;
    fwrite(&zero, sizeof(int), 1, garbage_fptr);
  }
  fseek(garbage_fptr, 0, SEEK_SET);

  int address;
  int m;
  fread(&m, sizeof(int), 1, garbage_fptr);
  if (m == 0) {
    FILE *data_fptr = fopen(DATA_placeS_FILE, "r");
    fseek(data_fptr, 0, SEEK_END);
    address = ftell(data_fptr);
    fclose(data_fptr);
  } else {
    fseek(garbage_fptr, m * sizeof(int), SEEK_SET);
    fread(&address, sizeof(int), 1, garbage_fptr);
    fseek(garbage_fptr, 0, SEEK_SET);
    int new_m = m - 1;
    fwrite(&new_m, sizeof(int), 1, garbage_fptr);
  }

  fclose(garbage_fptr);
  return address;
}

int place_index(int id) {
  FILE* index_fptr = fopen(INDEX_placeS_FILE, "r");

  fseek(index_fptr, 0, SEEK_END);
  int n = ftell(index_fptr);

  int l = 0, r = n / 8 - 1;
  while (l <= r) {
    int m = (l + r) / 2;
    fseek(index_fptr, m * sizeof(struct Index_row), SEEK_SET);
    int key, address;
    read_place_from_index(index_fptr, &key, &address);

    if (key == id) {
      fclose(index_fptr);
      return m;
    } else if (key > id) {
      r = m - 1;
    } else {
      l = m + 1;
    }
  }
  fclose(index_fptr);
  return -1;
}

struct Index_row *get_all_index(int *n) {
  FILE *index_fptr = fopen(INDEX_placeS_FILE, "r");
  fseek(index_fptr, 0, SEEK_END);
  *n = ftell(index_fptr) / 8;
  fseek(index_fptr, 0, SEEK_SET);
  struct Index_row *rows = (struct Index_row*) malloc (sizeof(struct Index_row));
  if (n == 0) {
    fclose(index_fptr);
    return rows;
  }

  int i = 0;
  while (!feof(index_fptr)) {
    int key, address;
    fread(&key, sizeof(int), 1, index_fptr);
    fread(&address, sizeof(int), 1, index_fptr);
    rows[i].id = key;
    rows[i].address = address;
    i++;
  }

  fclose(index_fptr);
  return rows;
}

int insert_place_to_index(int id, int address) {
  int n = 0;
  struct Index_row *rows = get_all_index(&n);

  if (n == 0) {
    FILE *index_fptr = fopen(INDEX_placeS_FILE, "w");
    fwrite(&id, sizeof(int), 1, index_fptr);
    fwrite(&address, sizeof(int), 1, index_fptr);
    fclose(index_fptr);
  } else {
    FILE *index_fptr = fopen(INDEX_placeS_FILE, "w");
    int inserted = 0;
    for (int i = 0; i < n + 1; i++) {
      if (i != n && rows[i].id < id) {
        fwrite(&rows[i].id, sizeof(int), 1, index_fptr);
        fwrite(&rows[i].address, sizeof(int), 1, index_fptr);
      } else if (inserted == 0) {
        inserted = 1;
        fwrite(&id, sizeof(int), 1, index_fptr);
        fwrite(&address, sizeof(int), 1, index_fptr);
      } else {
        fwrite(&rows[i - 1].id, sizeof(int), 1, index_fptr);
        fwrite(&rows[i - 1].address, sizeof(int), 1, index_fptr);
      }
    }
    fclose(index_fptr);
  }
  return 0;
}

int insert_place(int classroom_id, struct place place) {
  int place_idx = place_index(place.id);

  if (place_idx != -1) {
    return -1;
  } else {
    int classroom_idx = classroom_index(classroom_id);
    if (classroom_idx == -1) {
      return -1;
    }

    int first_place = get_first_place(classroom_id);
    int address = get_address_for_place();

    FILE *data_fptr = fopen(DATA_placeS_FILE, "r+");
    if (first_place != -1) {
      fseek(data_fptr, first_place + sizeof(int) * 3, SEEK_SET);
      fwrite(&address, sizeof(int), 1, data_fptr);
    }

    insert_place_to_index(place.id, address);

    fseek(data_fptr, address, SEEK_SET);
    fwrite(&place.id, sizeof(int), 1, data_fptr);
    fwrite(&place.people, sizeof(int), 1, data_fptr);
    fwrite(&classroom_id, sizeof(int), 1, data_fptr);
    int minus_one = -1;
    fwrite(&minus_one, sizeof(int), 1, data_fptr);
    fwrite(&first_place, sizeof(int), 1, data_fptr);
    fclose(data_fptr);

    set_first_place(classroom_id, address);

    return 0;
  }
  return -1;
}

struct place *get_place(int id) {
  int index = place_index(id);
  if (index == -1)
    return NULL;
  int address = get_place_address(index);
  FILE *data_fptr = fopen(DATA_placeS_FILE, "r");
  fseek(data_fptr, address, SEEK_SET);
  struct place *place = malloc(sizeof(struct place));
  fread(&place->id, sizeof(int), 1, data_fptr);
  fread(&place->people, sizeof(int), 1, data_fptr);
  fread(&place->classroom_id, sizeof(int), 1, data_fptr);
  int prev;
  fread(&prev, sizeof(int), 1, data_fptr);
  int next;
  fread(&next, sizeof(int), 1, data_fptr);
  fclose(data_fptr);
  return place;
}

void set_next(int address, int next) {
  FILE *data_fptr = fopen(DATA_placeS_FILE, "r+");
  fseek(data_fptr, address + sizeof(int) * 4, SEEK_SET);
  fwrite(&next, sizeof(int), 1, data_fptr);
  fclose(data_fptr);
}

int get_prev(int address) {
  FILE *data_fptr = fopen(DATA_placeS_FILE, "r+");
  fseek(data_fptr, address + sizeof(int) * 3, SEEK_SET);
  int prev;
  fread(&prev, sizeof(int), 1, data_fptr);
  fclose(data_fptr);
  return prev;
}

void set_prev(int address, int prev) {
  FILE *data_fptr = fopen(DATA_placeS_FILE, "r+");
  fseek(data_fptr, address + sizeof(int) * 3, SEEK_SET);
  fwrite(&prev, sizeof(int), 1, data_fptr);
  fclose(data_fptr);
}

int get_next(int address) {
  FILE *data_fptr = fopen(DATA_placeS_FILE, "r+");
  fseek(data_fptr, address + sizeof(int) * 4, SEEK_SET);
  int next;
  fread(&next, sizeof(int), 1, data_fptr);
  fclose(data_fptr);
  return next;
}

int delete_place(int id) {
  int index = place_index(id);
  if (index == -1) {
    return -1;
  } else {
    FILE *data_fptr;
    int address = get_place_address(index);
    int prev = get_prev(address);

    if (prev == -1) {
      data_fptr = fopen(DATA_placeS_FILE, "r+");
      fseek(data_fptr, address + 2 * sizeof(int), SEEK_SET);
      int classroom_id;
      fread(&classroom_id, sizeof(int), 1, data_fptr);
      fclose(data_fptr);
      int next = get_next(address);
      set_prev(next, -1);
      set_first_place(classroom_id, next);
    } else {
      int next = get_next(address);
      set_next(prev, next);
      if (next != -1)
        set_prev(next, prev);
    }

    FILE *garbage_fptr = fopen(GARBAGE_placeS_FILE, "r+");
    int n;
    fread(&n, sizeof(int), 1, garbage_fptr);
    int new_n = n + 1;
    fseek(garbage_fptr, 0, SEEK_SET);
    fwrite(&new_n, sizeof(int), 1, garbage_fptr);
    fseek(garbage_fptr, 0 , SEEK_END);
    fwrite(&address, sizeof(int), 1, garbage_fptr);
    fclose(garbage_fptr);

    int m;
    struct Index_row *rows = get_all_index(&m);
    FILE *index_fptr = fopen(INDEX_placeS_FILE, "w");
    for (int i = 0; i < m; i++) {
      if (rows[i].id != id) {
        fwrite(&rows[i].id, sizeof(int), 1, index_fptr);
        fwrite(&rows[i].address, sizeof(int), 1, index_fptr);
      }
    }
    fclose(index_fptr);

    return 0;
  }
}

int update_place(int id, struct place place) {
  int index = place_index(id);
  if (index == -1) {
    return -1;
  }
  int classroom_idx = classroom_index(place.classroom_id);
  if (classroom_idx == -1) {
    return -1;
  }
  delete_place(id);
  insert_place(place.classroom_id, place);
  return 0;
}

void delete_all(int address) {
  while (address != -1) {
    int temp = get_next(address);
    int id;
    FILE *data_fptr = fopen(DATA_placeS_FILE, "r+");
    fseek(data_fptr, address, SEEK_SET);
    fread(&id, sizeof(int), 1, data_fptr);
    fclose(data_fptr);
    delete_place(id);
    address = temp;
  }
}

int *all_places_id(int *n) {
  struct Index_row *rows = get_all_index(n);
  int *res = malloc(sizeof(int) * (*n));
  for (int i = 0; i < *n; i++) {
    res[i] = rows[i].id;
  }
  return res;
}
