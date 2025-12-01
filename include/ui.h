#ifndef UI_H
#define UI_H

void init_ui(void);
void cleanup_ui(void);
void show_main_menu(void);
void display_message(const char *msg);
char* get_user_input(const char *prompt);
void show_help(void);

#endif
