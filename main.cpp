#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <memory>
#include <termios.h>

static void ansi_clear_and_home() {
    std::cout << "\x1b[2J\x1b[H";
}

static int readKey() {
	unsigned char c;
	ssize_t n = ::read(STDIN_FILENO, &c, 1);
	if (n==1)
		return c;
	return -1;
}
enum class key_code {
    none,
    esc,
    up,
    down,
    left,
    right,
    w, a, s, d
};

static key_code read_key_code() {
    int c = readKey();
    if (c == -1) return key_code::none;

    if (c == 27) {
        int c1 = readKey();
        if (c1 == -1) return key_code::esc;

        if (c1 == '[') {
            int c2 = readKey();
            if (c2 == 'A') return key_code::up;
            if (c2 == 'B') return key_code::down;
            if (c2 == 'C') return key_code::right;
            if (c2 == 'D') return key_code::left;
        }
        return key_code::none;
    }

    if (c == 'w'){
		return key_code::w;
	}
    if (c == 'a'){
		return key_code::a;
	}
    if (c == 's'){
		return key_code::s;
	}
    if (c == 'd'){
		return key_code::d;
	}

    return key_code::none;
}



class terminal_raw_mode {
    termios old_{};
    bool active_ = false;

public:
    terminal_raw_mode() {
        if (tcgetattr(STDIN_FILENO, &old_) == -1) return;

        termios raw = old_;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 1;

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) return;
        active_ = true;
    }

    ~terminal_raw_mode() {
        if (active_) tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_);
    }

    terminal_raw_mode(const terminal_raw_mode&) = delete;
    terminal_raw_mode& operator=(const terminal_raw_mode&) = delete;
};

class Entity {
	protected:
		int pos_x, pos_y;
		std::string name;
		char body;
	public:
		Entity(): pos_x(0), pos_y(0), name("junk"), body('!'){
			std::cout << "default constructor" << std::endl;
		}

		Entity(int pos_x, int pos_y, std::string name, char body ): pos_x(pos_x), pos_y(pos_y), name(name), body(body) {
			std::cout << "parametric constructor" << std::endl;
		}

		void list_attributes(){
			std::cout << "name: " << name << " body: " << body << std::endl;
		}
		int x() const {
			return pos_x;
		}
		int y() const {
			return pos_y;
		}
		char get_body() const {
			return body;
		}

		void move_by(int dx, int dy, int max_w, int max_h) {
			int nx = pos_x + dx;
			int ny = pos_y + dy;
			if (0 <= nx && nx < max_w){
				pos_x = nx;
			}
			if (0 <= ny && ny < max_h){
				pos_y = ny;
			}
		}
};

class Player : public Entity {
	public:
		using Entity::Entity;
};

class Enemy : public Entity {
	public:
		using Entity::Entity;

};

void print_screen(std::vector< std::vector< char> >& screen){
	for(int i =0; i< screen.size(); i++){
		for(int j=0;j<screen[i].size();j++){
			std::cout << screen[i][j];
		}
		std::cout <<'\n';
	}
}

void clear_screen(std::vector< std::vector< char> >& screen){
    for(int i =0; i< screen.size(); i++){
        for(int j=0;j<screen[i].size();j++){
            screen[i][j] = ' ';
        }
    }
}

static void render_screen(const std::vector<std::vector<char>>& screen) {
    ansi_clear_and_home();
    for (int y = 0; y < (int)screen.size(); y++) {
        for (int x = 0; x < (int)screen[y].size(); x++) {
            std::cout << screen[y][x];
        }
        std::cout << '\n';
    }
    std::cout << "wasd move | press escape to quit\n";
    std::cout.flush();
}

int main(){
	terminal_raw_mode raw_guard;
	Player p1;
	Player p2(1,2, "Carlos", 'C');
	Player p3(2,3, "Albert", 'A');
	Enemy e1;
	Enemy e2(1,2, "Blanchard", 'B');

	//lets make a 64 by 64 screen.
	//we will never delete 'pixels' on the screen, just replace them with new 'pixels'
	//so 2d array or vector is fine
	int h = 20;
	int w = 40;
	// std::vector< std::vector< char> > screen(64, std::vector<char>(64, '#'));
	std::vector<std::vector<char>> screen(h, std::vector<char>(w, ' '));

	std::vector< std::unique_ptr<Entity> > entities;
	entities.push_back(std::make_unique<Player>(p1));
	entities.push_back(std::make_unique<Player>(p2));
	entities.push_back(std::make_unique<Enemy> (e2));
	//lets add our obejcts into the game_state

	//game loop
	bool escape_pressed = false;
	while( !escape_pressed ) {
		//clear the screen
		clear_screen(screen);

		//get the player input
		key_code key = read_key_code();
		if (key == key_code::esc) {
			escape_pressed = true;
		}

		if (entities.size() >= 2) {
			// p1  WASD
			if (key == key_code::w) entities[0]->move_by(0, -1, w, h);
			if (key == key_code::s) entities[0]->move_by(0,  1, w, h);
			if (key == key_code::a) entities[0]->move_by(-1, 0, w, h);
			if (key == key_code::d) entities[0]->move_by(1,  0, w, h);

			// p2  arrow keys
			if (key == key_code::up)    entities[1]->move_by(0, -1, w, h);
			if (key == key_code::down)  entities[1]->move_by(0,  1, w, h);
			if (key == key_code::left)  entities[1]->move_by(-1, 0, w, h);
			if (key == key_code::right) entities[1]->move_by(1,  0, w, h);
		}
		//compute what is on the screen
		for (const auto& e : entities) {
            int x = e->x();
            int y = e->y();
            if (0 <= y && y < (int)screen.size() && 0 <= x && x < (int)screen[y].size()) {
                screen[y][x] = e->get_body();
            }
        }

		//render the game
		render_screen(screen);
	}
	ansi_clear_and_home();
    std::cout << "bye\n";

	return 0;
}
