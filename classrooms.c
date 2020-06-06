#include <stdio.h>
#include <stdlib.h>
#include "places.h"


struct classroom {
  int id;
  double square;
};

struct IndexRow {
  int id;
  int address;
};

const char INDEX_FILE[] = "classrooms.idx";
const char GARBAGE_FILE[] = "classrooms.gb";
const char DATA_FILE[] = "classrooms.fl";

void read_classroom_from_index(FILE *fptr, int *id, int *address) {
  fread(id, sizeof(int), 1, fptr);
  fread(address, sizeof(int), 1, fptr);
}

int get_address() {
  FILE* garbage_fptr = fopen(GARBAGE_FILE, "r+");
  if (garbage_fptr == NULL) {
    garbage_fptr = fopen(GARBAGE_FILE, "w");
    int zero = 0;
    fwrite(&zero, sizeof(int), 1, garbage_fptr);
  }
  fseek(garbage_fptr, 0, SEEK_END);
  if (ftell(garbage_fptr) == 0) {
    int zero = 0;
    fwrite(&zero, sizeof(int), 1, garbage_fptr);
  }
  fseek(garbage_fptr, 0, SEEK_SET);
  int n;
  fread(&n, sizeof(int), 1, garbage_fptr);
  if (n > 0) {
    fseek(garbage_fptr, sizeof(int) + (n - 1) * sizeof(int), SEEK_SET);
    int address;
    fread(&address, sizeof(int), 1, garbage_fptr);
    fseek(garbage_fptr, 0, SEEK_SET);
    int new_n = n - 1;
    fwrite(&new_n, sizeof(int), 1, garbage_fptr);
    fclose(garbage_fptr);
    return address;
  } else {
    FILE *data_fptr = fopen(DATA_FILE, "r");
    fseek(data_fptr, 0, SEEK_END);
    int m = ftell(data_fptr);
    fclose(garbage_fptr);
    fclose(data_fptr);
    return m;
  }
}

int get_classroom_address(int index) {
  FILE *index_fptr = fopen(INDEX_FILE, "r");
  fseek(index_fptr, index * sizeof(struct IndexRow) + sizeof(int), SEEK_SET);
  int address;
  fread(&address, sizeof(int), 1, index_fptr);
  fclose(index_fptr);
  return address;
}

int write_classroom_to_data(double *square) {
  int address = get_address();
  FILE *fptr = fopen(DATA_FILE, "r+");
  fseek(fptr, address, SEEK_SET);
  fwrite(square, sizeof(double), 1, fptr);
  int first_place = -1;
  fwrite(&first_place, sizeof(int), 1, fptr);
  fclose(fptr);
  return address;
}

void write_classroom_to_index(FILE *fptr, int *id, int *address) {
  fwrite(id, sizeof(int), 1, fptr);
  fwrite(address, sizeof(int), 1, fptr);
}

// returns all rows that are in index file
struct IndexRow* read_all_index(int * n) {
  FILE* index_fptr = fopen(INDEX_FILE, "r");
  fseek(index_fptr, 0, SEEK_END);
  *n = ftell(index_fptr) / sizeof(struct IndexRow);
  struct IndexRow* rows = (struct IndexRow *) malloc (*n);


  if (*n == 0) {
    return rows;
  }

  fseek(index_fptr, 0, SEEK_SET);

  int i = 0;
  while (!feof(index_fptr)) {
    read_classroom_from_index(index_fptr, &rows[i].id, &rows[i].address);
    i++;
  }

  fclose(index_fptr);

  return rows;
}

/**
 * returns the addres of index row of classroom
 * if it doesn't exist in index_rows returns -1
 **/
int classroom_index(int id) {
  FILE* index_fptr = fopen(INDEX_FILE, "r");

  fseek(index_fptr, 0, SEEK_END);
  int n = ftell(index_fptr);

  int l = 0, r = n / 8 - 1;
  while (l <= r) {
    int m = (l + r) / 2;
    fseek(index_fptr, m * 8, SEEK_SET);
    int key, address;
    read_classroom_from_index(index_fptr, &key, &address);

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


int insert_classroom(struct classroom classroom) {

  FILE *index_file_ptr;

  int n;
  struct IndexRow *index_rows = read_all_index(&n);


  int index = classroom_index(classroom.id);

  if (index == -1) {
    if (n == 0) {
      index_file_ptr = fopen(INDEX_FILE, "a");
      int address = 0;
      write_classroom_to_index(index_file_ptr, &classroom.id, &address);
      fclose(index_file_ptr);

      write_classroom_to_data(&classroom.square);
      return 0;
    } else {
      int address = write_classroom_to_data(&classroom.square);

      int inserted = 0;
      index_file_ptr = fopen(INDEX_FILE, "w");
      for (int i = 0; i < n + 1; i++) {
        if (i < n && index_rows[i].id < classroom.id) {
          write_classroom_to_index(index_file_ptr, &index_rows[i].id, &index_rows[i].address);
        } else if (inserted == 0) {
          inserted = 1;
          write_classroom_to_index(index_file_ptr, &classroom.id, &address);
        } else {
          write_classroom_to_index(index_file_ptr, &index_rows[i - 1].id, &index_rows[i - 1].address);
        }
      }
      fclose(index_file_ptr);
      return 0;
    }
  } else {
    return -1;
  }
}

struct classroom *get_classroom(int id) {
  int index = classroom_index(id);
  if (index != -1) {
    int address = get_classroom_address(index);

    FILE *data_fptr = fopen(DATA_FILE, "r");
    struct classroom* classroom = (struct classroom *) malloc (sizeof(struct classroom));
    classroom->id = id;
    fseek(data_fptr, address, SEEK_SET);
    fread(&classroom->square, sizeof(double), 1, data_fptr);
    int first_place;
    fread(&first_place, sizeof(int), 1, data_fptr);
    fclose(data_fptr);
    return classroom;
  } else {
    return NULL;
  }
}

int get_first_place(int id) {
  int index = classroom_index(id);
  if (index != -1) {
    int address = get_classroom_address(index);

    FILE *data_fptr = fopen(DATA_FILE, "r");
    int first_place;
    fseek(data_fptr, address + sizeof(double), SEEK_SET);
    fread(&first_place, sizeof(int), 1, data_fptr);
    fclose(data_fptr);
    return first_place;
  } else {
    return -2;
  }
}

int classroom_is_deleted(int id) {
  int index = classroom_index(id);
  return index == -1;
}

/**
 * deletes classroom from index file
 * returns 0 if deletion has completed
 * returns -1 if nothing to delete
 **/
int delete_classroom(int id) {
  int index = classroom_index(id);
  if (index != -1) {
    delete_all(get_first_place(id));
    int n;
    struct IndexRow *rows = read_all_index(&n);
    int address = rows[index].address;
    // inserting new block to garbage file
    FILE *garbage_fptr = fopen(GARBAGE_FILE, "r+");
    if (garbage_fptr == NULL) {
      garbage_fptr = fopen(GARBAGE_FILE, "w");
      int zero = 0;
      fwrite(&zero, sizeof(int), 1, garbage_fptr);
    }
    fseek(garbage_fptr, 0, SEEK_END);
    if (ftell(garbage_fptr) == 0) {
      int zero = 0;
      fwrite(&zero, sizeof(int), 1, garbage_fptr);
    }
    fseek(garbage_fptr, 0, SEEK_SET);

    int m;
    fread(&m, sizeof(int), 1, garbage_fptr);
    fseek(garbage_fptr, 0, SEEK_SET);
    int new_m = m + 1;
    fwrite(&new_m, sizeof(int), 1, garbage_fptr);
    fseek(garbage_fptr, m * sizeof(int) + sizeof(int), SEEK_SET);
    fwrite(&address, sizeof(int), 1, garbage_fptr);
    fclose(garbage_fptr);

    FILE *index_fptr = fopen(INDEX_FILE, "w+");
    for (int i = 0; i < n; i++) {
      if (rows[i].id != id) {
        write_classroom_to_index(index_fptr, &rows[i].id, &rows[i].address);
      }
    }
    fclose(index_fptr);
    return 0;
  } else {
    return -1;
  }
}

int update_classroom(int id, double square) {
  int index = classroom_index(id);

  if (index == -1) {
    return -1;
  }

  FILE *data_fptr = fopen(DATA_FILE, "r+");
  int address = get_classroom_address(index);
  fseek(data_fptr, address, SEEK_SET);

  if (square >= 0) {
    fwrite(&square, sizeof(double), 1, data_fptr);
  }

  fclose(data_fptr);
  return 0;
}

int set_first_place(int classroom_id, int place_address) {
  int index = classroom_index(classroom_id);
  int address = get_classroom_address(index);
  FILE *classroom_data_fptr = fopen(DATA_FILE, "r+");
  fseek(classroom_data_fptr, address + sizeof(double), SEEK_SET);
  fwrite(&place_address, sizeof(int), 1, classroom_data_fptr);
  fclose(classroom_data_fptr);
  return 0;
}

int *all_classroom_id(int *n) {
  int m;
  struct IndexRow *rows = read_all_index(&m);
  int *res = malloc(sizeof(int) * m);
  *n = m;
  for (int i = 0; i < m; i++) {
    res[i] = rows[i].id;
  }
  return res;
}
