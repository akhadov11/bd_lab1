#include <stdio.h>
#include <string.h>

const char *INDEX_placeS_FILE = "places.idx";
const char *DATA_placeS_FILE = "places.fl";

const char *GARBAGE_placeS_FILE = "places.gb";
const char INDEX_FILE[] = "classrooms.idx";
const char GARBAGE_FILE[] = "classrooms.gb";
const char DATA_FILE[] = "classrooms.fl";

const char *GET_classroom = "get_classroom";
const char *INSERT_classroom = "insert_classroom";
const char *classroom_IS_DELETED = "classroom_is_deleted";
const char *DELETE_classroom = "delete_classroom";
const char *UPDATE_classroom = "update_classroom";
const char *INSERT_place = "insert_place";
const char *GET_place = "get_place";
const char *DELETE_place = "delete_place";
const char *UPDATE_place = "update_place";
const char *ALL_classroom_ID = "all_classroom_count";
const int NAME_LENGTH = 20;


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

struct classroom {
  int id;
  double square;
};

struct IndexRow {
  int id;
  int address;
};



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

/**
 * inserts a classroom in database
 * returns 0 if insertion has completed
 * return -1 if an error has occured
 **/
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


void process_all_classroom_id() {
  int n;
  int *ids = all_classroom_id(&n);
  printf("-\n");
  printf("%d\n", n);
  printf("-\n");
}

void process_all_place_id() {
  int n;
  int *ids = all_places_id(&n);
  printf("-\n");
  for (int i = 0; i < n; i++) {
    printf("%d\n", ids[i]);
  }
  printf("-\n");
}

void process_update_place() {
  printf("Enter id: ");
  int id;
  scanf("%d", &id);
  printf("Enter people: ");
  int people;
  scanf("%d", &people);
  printf("Enter classroom id: ");
  int classroom_id;
  scanf("%d", &classroom_id);
  struct place place = {.id = id, .people = people, .classroom_id = classroom_id};
  if (update_place(id, place) != -1) {
    printf("------------------------\n");
    printf("Updated place successfully\n");
    printf("------------------------\n");
  } else {
    printf("----------------------------------------------------------------\n");
    printf("place with such id already exists or there is no classroom with such id\n");
    printf("----------------------------------------------------------------\n");

  }
}

void process_delete_place() {
  printf("Enter id: ");
  int id;
  scanf("%d", &id);
  if (delete_place(id) == 0) {
    printf("----------------------------\n");
    printf("place was deleted successfully\n");
    printf("----------------------------\n");
  } else {
    printf("------------------------------\n");
    printf("place with such id doesn't exist\n");
    printf("------------------------------\n");
  }
}

void process_insert_place() {
  printf("Enter id: ");
  int id;
  scanf("%d", &id);
  printf("Enter people ");
  int people;
  scanf("%d", &people);
  printf("Enter classroom id: ");
  int classroom_id;
  scanf("%d", &classroom_id);
  struct place place= {.id = id, .people = people, .classroom_id = classroom_id};
  if (insert_place(classroom_id, place) != -1) {
    printf("----------------------------\n");
    printf("Successfully added a new place\n");
    printf("----------------------------\n");
  } else {
    printf("----------------------------------------------------------------\n");
    printf("place with such id already exists or there is no classroom with such id\n");
    printf("----------------------------------------------------------------\n");

  }

}

void process_get_place() {
  printf("Enter id: ");
  int id;
  scanf("%d", &id);
  struct place *place = get_place(id);
  if (place == NULL) {
    printf("----------------------------------\n");
    printf("The place with such id doesn't exist\n");
    printf("----------------------------------\n");
    return;
  }
  printf("-------------------\n");
  printf("id: %d\n", place->id);
  printf("people: %d\n", place->people);
  printf("classroom_id: %d\n", place->classroom_id);

  printf("-------------------\n");
}

void process_update_classroom() {
  printf("Enter id: ");
  int id;
  scanf("%d", &id);
  printf("Enter new square: ");
  double square;
  scanf("%lf", &square);
  if (update_classroom(id, square) == -1) {
    printf("-----------------------------------\n");
    printf("The classroom with such id doesn't exist\n");
    printf("-----------------------------------\n");
  } else {
    printf("-----------------------------\n");
    printf("Successfully updated the classroom\n");
    printf("-----------------------------\n");

  }
}

void process_delete_classroom() {
  printf("Enter id: ");
  int id;
  scanf("%d", &id);
  delete_classroom(id);
  printf("Successfully deleted classroom\n");
}

void process_classroom_is_deleted() {
  printf("Enter id: ");
  int id;
  scanf("%d", &id);
  if (classroom_is_deleted(id)) {
    printf("YES\n");
  } else {
    printf("NO\n");
  }
}

void process_get_classroom() {
  printf("Enter id: ");
  int id;
  scanf("%d", &id);
  struct classroom *classroom = get_classroom(id);

  if (classroom != NULL) {
    printf("---------------------\n");
    printf("classroom:\n");
    printf("id: %d\n", classroom->id);
    printf("square: %lf\n", classroom->square);
    printf("---------------------\n");
  } else {
    printf("---------------------------------\n");
    printf("The classroom with id %d doesn't exist\n", id);
    printf("---------------------------------\n");
  }
}

void process_insert_classroom() {
  int id;
  printf("id: ");
  scanf("%d", &id);
  printf("square : ");
  double square;
  scanf("%lf", &square);
  struct classroom classroom= {.id = id, .square = square};
  if (insert_classroom(classroom) == 0) {
    printf("--------------------------------\n");
    printf("Successfully added a new classroom\n");
    printf("--------------------------------\n");
  } else {
    printf("------------------------------------\n");
    printf("The classroom with such id already exists\n");
    printf("------------------------------------\n");
  }
}

int main() {
  while (1) {
    char command[30];
    printf(">>> ");
    scanf("%29s", command);
    if (strcmp(command, GET_classroom) == 0) {
      process_get_classroom();
    } else if (strcmp(command, INSERT_classroom) == 0) {
      process_insert_classroom();
    } else if (strcmp(command, classroom_IS_DELETED) == 0) {
      process_classroom_is_deleted();
    } else if (strcmp(command, DELETE_classroom) == 0) {
      process_delete_classroom();
    } else if (strcmp(command, UPDATE_classroom) == 0) {
      process_update_classroom();
    } else if (strcmp(command, GET_place) == 0) {
      process_get_place();
    } else if (strcmp(command, INSERT_place) == 0) {
      process_insert_place();
    } else if (strcmp(command, DELETE_place) == 0) {
      process_delete_place();
    } else if (strcmp(command, UPDATE_place) == 0) {
      process_update_place();
    } else if (strcmp(command, ALL_classroom_ID) == 0) {
      process_all_classroom_id();
    }
  }
}
