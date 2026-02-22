#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

static int readKey() {
	unsigned char c;
	ssize_t n = ::read(STDIN_FILENO, &c, 1);
	if (n==1)
		return c;
	return -1;
}

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
};

class Player : public Entity {
	public:
		using Entity::Entity;
};

class Enemy : Entity {
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

int main(){
	Player p1;
	Player p2(1,2, "Carlos", 'C');
	Player p3(2,3, "Albert", 'A');
	Enemy e1;
	Enemy e2(1,2, "Blanchard", 'B');

	//lets make a 64 by 64 screen.
	//we will never delete 'pixels' on the screen, just replace them with new 'pixels'
	//so 2d array or vector is fine
	std::vector< std::vector< char> > screen(64, std::vector<char>(64, '#'));

	std::vector< std::vector< Entity> > game_state(64, std::vector<char>(64));
	//lets add our obejcts into the game_state

	//game loop
	bool escape_pressed = false;
	while( !escape_pressed ) {
		//clear the screen
		clear_screen(screen);
		//get the player input
		int key = readKey();
		if(key != -1) {
			std::cout << "you pressed: "<< key << "\n";
		}
		//
		//compute what is on the screen
		for(Entity things : game_state) {
			screen[things.pos_x][things.pos_y] = things.body;
		}

		//render the game
		print_screen(screen);
	}


	return 0;
}
