#ifndef HEADER_FILE
#define HEADER_FILE
struct classroom {
  int id;
  double square;
};
int insert_classroom(struct classroom classroom);
struct classroom *get_classroom(int id);
int classroom_is_deleted(int id);
int delete_classroom(int id);
int update_classroom(int id, double square);
int classroom_index(int id);
int get_first_place(int id);
int get_classroom_address(int id);
int set_first_place(int room_idx, int address);
int *all_classroom_id(int *n);
#endif
