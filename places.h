struct place {
    int id;
    int people;
    int classroom_id;
};
int insert_place(int classroom_id, struct place place);
struct place *get_place(int id);
int delete_place(int id);
int update_place(int id, struct place place);
void delete_all(int id);
int *all_places_id(int *n);
