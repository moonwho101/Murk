//Murk Function Declares


//#include <afx.h>
//#include <winsock2.h>
//#include <windows.h>
#include <string.h>
#include <winuser.h>
#include "resource.h"
#include "cdirdraw.h"
#include "cdirsnd.h"
#include "cwave.h"
#include "cmydd.h"

LRESULT APIENTRY WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam);
void read_levels(void);
// Function prototypes.
BOOL CALLBACK DlgProcQCreate(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void sendtext(char p[100]);
void waitabit(int i);
void assign_pit(int level);
void showsplash();
void showselect();
static bool SolveMaze3(int x, int y, int level, int dir);
void out_map(int sx, int sy, int dir, int shortestroute, int saveroute, int size);
BOOL InitGraphics();
int computer_m(int x, int y, int level);
bool OutsideMaze(int x, int y);
bool AtFinish(int x, int y);
static bool SolveMaze(int x, int y, int level, int dir);
void MarkSquare(int x, int y, int level);
bool IsMarked(int x, int y, int level);
void UnmarkSquare(int x, int y, int level);
bool WallExists(int x, int y, int level, int dir);
int count_nodes(int lvlnum);
int random_num(int num);
void OnLButtonDblClk(LPARAM lParam);
int attack_player(void);
int findcloseststairs(char way);
void OnLButtonDown(LPARAM lParam);
void calc_player_move(void);
void wakeup_sleepy_heads(void);
void generate_dungeon(void);
void OnKeyDown(WPARAM wParam);
void OnDestroy();
void LoadFloorDibs(char* p);
void LoadDudeDibs(UINT floor);
void ShowPlayer(int animate);
void animate_player(int startpos);
void delay_s(unsigned char seconds);
void OnTimer(void);
void init_dungeon(void);
void display_dungeon(int player);

void debug_me(char* u, char* s, int a, int b);
void clear_log();
void check_dungeon(void);
void make_your_move(void);
void play_sound(void);
void CALLBACK EXPORT TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime);
void apply_damage(void);
void show_panel();
void load_panel();
void assemble_board(BOOL dung_on, BOOL player_on, BOOL panel_on, BOOL turnoffuser);
void assign_player_cop(void);
void LoadSelect();
void LoadSplash();
void mouse_adjust(int mx, int my, int px, int py, int type);
void calc_animation();
void assign_treasure(void);
void assign_treasure2(void);
void display_items(void);
void LoadScroll();
void showscroll();
void whosmove(void);
int monster_shoot(void);
void movedown();
void computermove();
void checkmovenum();
void player_found_something(char* item);
void destination();
void check_dead();
void assign_elixir(int length);
void kill_current(int loser, int dropit, int die);
void kill_notcurrent(int loser, int dropit, int die);


void handle_mouse(int x, int y);
int findstairs(char way);
int findbeholder(int x, int y, int lvl);
void OnRButtonDown(LPARAM lParam);
void OnLButtonUp(LPARAM lParam);
void handle_left();
void handle_right();
void handle_up();
void handle_down();
void handle_stairs();
int findelixir(int item);
void new_level(int nextlev);
void show_missle(void);
void start_missle(int x, int y, int dir, int lvl, int sx, int sy, int owner);
void monster_cop();
void message(void);



void send_chat(int client);
void send_dungeon_square(int x, int y, int lvl, int forsure);
int find_free_slot();
void showdialogs();
void send_playerquick(int i, int client, int slot);
void send_player2(int i, int client, int slot);
void send_missle(int client, int misslex, int missley, int misslelvl, int frame, int image, int direction, int owner, int missledx, int missledy, int slot);
void send_playernum(int slot);
void request_player_info(int i);
void send_numplayers(int slot);
void send_treasure(int slot, int forsure);
void send_treasure2(int slot, int forsure);
void send_dungeon_info(int slot);
void send_player(int i, int client, int slot);
void DPEventMsg2();
void send_quit();
void ReceiveThread(LPVOID lpThreadParameter);
void send_deleteme();
void DPShutDown();
void clear_player(int id);
void send_deleteme();
void reset_networkserver();
void send_new_level();



void send_stoptimer(int slot);
void send_settimer(int slot);
void init_chat();




void make_thread2();
void createthread();
void ThreadFunction(LPVOID lpParam);

void OnCharDown(WPARAM wParam);
void clear_dungeon();
void swap_players();
void apply_body_damage(int loser, int winner);
void went_up_level();


LRESULT PerformMenuCommand(HWND hWnd, WPARAM wParam);
int save_server_settings();
int read_server_settings();
