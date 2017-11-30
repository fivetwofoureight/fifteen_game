#include "lib/std_lib_facilities_5.h"
#include "lib/Window.h"
#include "lib/Graph.h"
#include "FL/Fl_JPEG_Image.H"
#include "lib/Simple_window.h"


enum Game_state { Default = 0, Splash = 1, Instruct = 2, Level = 3, Game_10 = 10, Game_20 = 20, Game_40 = 40, Game_80 = 80, End = 5, Quit = 6 };

struct player_score {
	//for task 3 and 6, assigns two different values to 1 element in a vector made of player_scores
	string name;
	int score;
};


struct Tile_button : public Button {
	Tile_button(int x_coord, int y_coord, int number, Callback cb)
		:Button{ Point(100 + (100 * x_coord),200 + (100 * y_coord)),100, 100, to_string(number), cb },
		x_coord{ x_coord },
		y_coord{ y_coord },
		number{ number }
	{}

	int x() {
		return x_coord;
	}

	int y() {
		return y_coord;
	}
	int val() {
		return number;
	}
	void set_x(int x) {

		move(100 * (x - x_coord), 0);
		x_coord = x;
		Fl::redraw();
	}

	void set_y(int y) {
		move(0, 100 * (y - y_coord));
		y_coord = y;
		Fl::redraw();
	}

	void pseudo_set_xy(int x, int y) {
		move(100 * (x - x_coord), 0);
		x_coord = x;
		move(0, 100 * (y - y_coord));
		y_coord = y;
	}

	int manhattan() {
		//do: calculate position from value
		int ideal_x = (number - 1) % 4;
		int ideal_y = (number - 1) / 4;
		if (number == 0) {//blank in bottom right
			ideal_x = 3;
			ideal_y = 3;
		}
		int dy = abs(y_coord - ideal_y);
		int dx = abs(x_coord - ideal_x);
		return (dx + dy);
	}


private:
	int x_coord;
	int y_coord;
	int number;
};

struct Project_window : Graph_lib::Window {
	Project_window(Point xy, int w, int h, const string& title)
		:Window{ xy,w,h,title },
		username{ Text(Point(360,650),"username") },
		quit_button{ Point{ 70,0 }, 70, 20, "Quit",[](Address, Address pw) {reference_to<Project_window>(pw).quit(); } },
		button_pushed{ false },
		state{ Game_state(Default) }
	{
		attach(quit_button);
	}

	void set_username(string new_name) {
		username.set_label(new_name);
		Fl::redraw();
	}

	Game_state wait_for_button() {
		make_current();
		show();
		cout << label() << endl;
		while (!button_pushed) {
			Fl::wait();
		}
		button_pushed = false;
		hide();
		return state;
	}

	void set_state(Game_state new_state) {
		state = new_state;
		button_pushed = true;
	}

	void quit() {
		button_pushed = true;
		make_current();
		hide();
		state = Game_state(Quit);
	}
	Text username;
	Button quit_button;
	bool button_pushed;
	Game_state state;
};

struct End_screen : public Project_window {


	End_screen(Point xy, int w, int h, const string& title, int final_player_score, string user)
		:Project_window{ xy,w,h,title },
		new_game_button{ Point{ 150,70 }, 70, 70, "New Game",[](Address, Address pw) { reference_to<End_screen>(pw).set_state(Game_state(Level)); } },
		final_player_score{ final_player_score }
	{
		cout << final_player_score << endl;
		score.set_label("Final Score: " + to_string(final_player_score));
		attach(username);
		attach(score);
		set_username(user);
		attach(new_game_button);
	}


private:
	Button new_game_button;
	string text_score;
	Text score = Text{ Point{ 100,100 }, text_score };
	int final_player_score;
};

bool operator<(player_score p1, player_score p2) {
	if (p1.score < p2.score) {
		return true;
	}
	else {
		return false;
	}
}

struct Game_screen : public Project_window {

	Game_screen(Point xy, int w, int h, const string& title, int difficulty, string user)
		:Project_window{ xy,w,h,title },
		difficulty{ difficulty },
		hint_button{ Point(360,10), 160, 100, "Hint", [](Address, Address pw) {reference_to<Game_screen>(pw).hint(); } },
		advice{ Point{ 360, 30 }, "A helpful hint if you click the button" }
	{
		attach(username);
		set_username(user);
		score = 0;
		num_right = 0;
		moves_remain = difficulty;
		attach(leader_title);
		attach(first);
		attach(second);
		attach(third);
		attach(fourth);
		attach(fifth);
		attach(moves);
		attach(right);
		attach(hint_button);
		attach(advice);
		game_init();
	}

	void set_difficulty(int diff) {
		difficulty = diff;
		order_tiles();
	}

	int get_score() {
		return score;
	}

	void pseudo_swap(int val) {
		//moves, but does not redraw or decrement counters
		int empty = 0;//location of empty tile
		int temp_x = 0;
		int temp_y = 0;
		bool valid_swap = false;
		for (int i = 0; i < 16; ++i) {
			if (tiles[i].val() == 0) {
				empty = i;
			}
		}
		if (tiles[empty].x() == tiles[val].x()) {
			if (abs(tiles[empty].y() - tiles[val].y()) == 1) {
				valid_swap = true;
			}
		}
		if (tiles[empty].y() == tiles[val].y()) {
			if (abs(tiles[empty].x() - tiles[val].x()) == 1) {
				valid_swap = true;
			}
		}
		if (valid_swap) {
			temp_x = tiles[empty].x();
			temp_y = tiles[empty].y();
			tiles[empty].pseudo_set_xy(tiles[val].x(), tiles[val].y());
			tiles[val].pseudo_set_xy(temp_x, temp_y);
			//cout << "\tinvisible move of tiles" << endl;
		}
	}

	int locate_tile(int x, int y) {
		//returns the location in the array of the tile at (x,y)
		for (int i = 0; i < tiles.size(); ++i) {
			if (tiles[i].x() == x && tiles[i].y() == y) {
				//cout << "tile found at location (" << x << ", " << y << ")" << endl;
				return i;
			}
		}
		cout << "error: tile at (" << x << "," << y << ") not found" << endl;
	}

	int locate_tile(int tile_number) {
		for (int i = 0; i < 16; ++i) {
			if (tiles[i].val() == tile_number) {
				//cout << "tile found at location " << i << endl;
				return i;
			}
		}
		cout << "error, tile " << tile_number << " not found" << endl;
	}

	void hint() {
		int empty = locate_tile(0);
		int empty_y = tiles[empty].y();
		int empty_x = tiles[empty].x();
		int min_error = 160;//more than possible on one board
		int curr_error = 0;//error of current layout
		vector<int> errors = { 0,0,0,0 };
		//case of upwards move (row 1 to 3)
		if (empty_y > 0) {
			curr_error = 0;
			pseudo_swap(locate_tile(empty_x, empty_y - 1));
			for (int i = 0; i < tiles.size(); ++i) {
				curr_error += tiles[i].manhattan();
			}
			min_error = min(min_error, curr_error);
			pseudo_swap(locate_tile(empty_x, empty_y));//reset board
			errors[0] = curr_error;
		}
		//case of downwards move
		if (empty_y < 3) {
			curr_error = 0;
			pseudo_swap(locate_tile(empty_x, empty_y + 1));
			for (int i = 0; i < tiles.size(); ++i) {
				curr_error += tiles[i].manhattan();
			}
			min_error = min(min_error, curr_error);
			pseudo_swap(locate_tile(empty_x, empty_y));//reset board
			errors[1] = curr_error;
		}
		//case of left move
		if (empty_x > 0) {
			curr_error = 0;
			pseudo_swap(locate_tile(empty_x - 1, empty_y));
			for (int i = 0; i < tiles.size(); ++i) {
				curr_error += tiles[i].manhattan();
			}
			min_error = min(min_error, curr_error);
			pseudo_swap(locate_tile(empty_x, empty_y));//reset board
			errors[2] = curr_error;
		}
		//case of right move
		if (empty_x < 3) {
			curr_error = 0;
			pseudo_swap(locate_tile(empty_x + 1, empty_y));
			for (int i = 0; i < tiles.size(); ++i) {
				curr_error += tiles[i].manhattan();
			}
			min_error = min(min_error, curr_error);
			pseudo_swap(locate_tile(empty_x, empty_y));//reset board
			errors[3] = curr_error;
		}
		//choose best moves
		vector<string> directions = { "Up","Down","Left","Right" };
		string good_advice = "";
		for (int i = 0; i < errors.size(); ++i) {
			if (errors[i] == min_error) {
				good_advice += directions[i] + " ";
			}
		}
		advice.set_label("Try one of these moves: " + good_advice);
		Fl::redraw();
	}


	void number_right() {
		num_right = 0;
		for (int i = 0; i < 16; ++i) {
			if (tiles[i].manhattan() != 0) {
				++num_right;
			}
		}
		right.set_label(to_string(num_right));
	}

	vector<player_score> pulling_scores()
	{
		vector<player_score> original_scores;
		ifstream Scores;
		switch (difficulty) {
		case 10:
			Scores.open("usr/Scores_list_10.txt");
			break;
		case 20:
			Scores.open("usr/Scores_list_20.txt");
			break;
		case 40:
			Scores.open("usr/Scores_list_40.txt");
			break;
		case 80:
			Scores.open("usr/Scores_list_80.txt");
			break;
		default:
			cout << "Error with selecting difficulty, cannot display scores" << endl;
		}
		if (Scores.fail()) {
			cerr << "Error Opening File" << endl;
			keep_window_open();
			exit(1);
		}

		player_score set_scores;
		while (Scores >> set_scores.name >> set_scores.score) {
			original_scores.push_back(set_scores);
		}
		Scores.close();//can put into a separate file later	

		return original_scores;

	}
	vector<string> leaderboard() {
		vector<string>top_5;

		for (int i = 0; i < 5; ++i) {
			string combined_name_score = pulling_scores()[i].name + "         " + to_string(pulling_scores()[i].score);
			top_5.push_back(combined_name_score);
		}

		return top_5;
	}

	void tile(int tile_num) {
		int place = 0;
		for (int i = 0; i < tiles.size(); ++i) {
			if (tiles[i].val() == tile_num) {
				place = i;
			}
		}

		cout << "The tile you clicked is " << tile_num;
		cout << " (" << tiles[place].y() << "," << tiles[place].x() << ")" << endl;
		cout << "Manhattan: " << tiles[place].manhattan() << endl;
		swap(place);
	}


	void final_scores_list(int final_score) {
		string fake_player_name = "YAY";
		cout << "name: " << fake_player_name << endl;
		vector<player_score>sort_scores = pulling_scores();

		player_score set_player_info;
		set_player_info.name = fake_player_name;
		set_player_info.score = final_score;


		sort_scores.push_back(set_player_info);
		sort(sort_scores.begin(), sort_scores.end());

		ofstream new_score_list;
		switch (difficulty) {
		case 10:
			new_score_list.open("usr/Scores_list_10.txt");
			break;
		case 20:
			new_score_list.open("usr/Scores_list_20.txt");
			break;
		case 40:
			new_score_list.open("usr/Scores_list_40.txt");
			break;
		case 80:
			new_score_list.open("usr/Scores_list_80.txt");
			break;
		default:
			cout << "Error with selecting difficulty, cannot display scores" << endl;
		}
		if (new_score_list.fail()) {
			cerr << "Error Opening File" << endl;
			keep_window_open();
			exit(1);
		}
		cout << "what is entered into the txt file: " << endl;
		for (int i = sort_scores.size() - 1; i >= 0; --i) {
			new_score_list << sort_scores[i].name << " " << sort_scores[i].score << endl << endl;
			cout << sort_scores[i].name << "   " << sort_scores[i].score << endl;
		}

		//writes the new list of player names and the scores into the file
		new_score_list.close();
		//closes the file 
	}

	void check_game_over() {
		if (moves_remain == 0) {
			final_scores_list(score);
			state = Game_state(End);
		}
	}

	void swap(int val) {
		int empty = 0;//location of empty tile
		int temp_x = 0;
		int temp_y = 0;
		bool valid_swap = false;
		for (int i = 0; i < 16; ++i) {
			if (tiles[i].val() == 0) {
				empty = i;
			}
		}
		if (tiles[empty].x() == tiles[val].x()) {
			if (abs(tiles[empty].y() - tiles[val].y()) == 1) {
				valid_swap = true;
			}
		}
		if (tiles[empty].y() == tiles[val].y()) {
			if (abs(tiles[empty].x() - tiles[val].x()) == 1) {
				valid_swap = true;
			}
		}
		if (valid_swap) {
			temp_x = tiles[empty].x();
			temp_y = tiles[empty].y();
			tiles[empty].set_x(tiles[val].x());
			tiles[empty].set_y(tiles[val].y());
			tiles[val].set_x(temp_x);
			tiles[val].set_y(temp_y);
			--moves_remain;
			moves.set_label(to_string(moves_remain));
			score = difficulty * (16 - num_right);
			check_game_over();
		}
		number_right();
	}


	void load_values() {

		switch (difficulty) {//populate vector<int> numbers
		case 10: numbers = ten_nums;
			break;
		case 20: numbers = twenty_nums;
			break;
		case 40: numbers = forty_nums;
			break;
		case 80: numbers = eighty_nums;
			break;
		default: cout << "error in choosing difficulty";
			break;
		}
	}

	void order_tiles() {
		tiles = Vector_ref<Tile_button>{};
		for (int i = 0; i < tile_bag.size(); i++) {
			detach(tile_bag[i]);
			tiles.push_back(tile_bag[numbers.at(i)]);//puts tiles in the correct order
			tiles[i].set_x(i % 4);
			tiles[i].set_y(i / 4);
			attach(tiles[i]);
		}
	}

	void game_init() {
		load_values();
		tile_bag.push_back(new Tile_button{ 0,0,0,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(0); } });
		tile_bag.push_back(new Tile_button{ 0,0,1,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(1); } });
		tile_bag.push_back(new Tile_button{ 0,0,2,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(2); } });
		tile_bag.push_back(new Tile_button{ 0,0,3,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(3); } });
		tile_bag.push_back(new Tile_button{ 0,0,4,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(4); } });
		tile_bag.push_back(new Tile_button{ 0,0,5,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(5); } });
		tile_bag.push_back(new Tile_button{ 0,0,6,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(6); } });
		tile_bag.push_back(new Tile_button{ 0,0,7,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(7); } });
		tile_bag.push_back(new Tile_button{ 0,0,8,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(8); } });
		tile_bag.push_back(new Tile_button{ 0,0,9,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(9); } });
		tile_bag.push_back(new Tile_button{ 0,0,10,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(10); } });
		tile_bag.push_back(new Tile_button{ 0,0,11,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(11); } });
		tile_bag.push_back(new Tile_button{ 0,0,12,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(12); } });
		tile_bag.push_back(new Tile_button{ 0,0,13,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(13); } });
		tile_bag.push_back(new Tile_button{ 0,0,14,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(14); } });
		tile_bag.push_back(new Tile_button{ 0,0,15,[](Address,Address pw) {reference_to<Game_screen>(pw).tile(15); } });
		for (int i = 0; i < tile_bag.size(); ++i) {
			attach(tile_bag[i]);
		}
		order_tiles();

	}

private:
	int score;
	int moves_remain;
	int difficulty;
	int num_right;
	Vector_ref<Tile_button> tile_bag;
	Vector_ref<Tile_button> tiles;

	vector<int> numbers;
	vector<int> ten_nums = { 1, 5, 9, 13, 2, 6, 10, 14, 3, 12, 0, 8, 4, 7, 15, 11 };
	vector<int> twenty_nums = { 1, 5, 9, 13, 6, 0, 10, 15, 3, 2, 14, 12, 4, 11, 7, 8 };
	vector<int> forty_nums = { 6, 10, 9, 14, 5, 13, 15, 12, 11, 2, 7, 8, 4, 1, 3, 0 };
	vector<int> eighty_nums = { 0, 15, 3, 4, 12, 14, 7, 8, 11, 10, 6, 5, 13, 9, 2, 1 };

	Button hint_button;
	Text advice;
	Text moves = Text{ Point{ 360,128 }, to_string(difficulty) };
	Text right = Text{ Point{ 360, 148 }, "##" };
	Text leader_title = Text{ Point{ 550,200 }, "Leaderboard" };
	Text first = Text{ Point{ 550,250 }, leaderboard()[0] };
	Text second = Text{ Point{ 550,300 }, leaderboard()[1] };
	Text third = Text{ Point{ 550,350 }, leaderboard()[2] };
	Text fourth = Text{ Point{ 550,400 }, leaderboard()[3] };
	Text fifth = Text{ Point{ 550,450 }, leaderboard()[4] };
};

struct Level_select : public Project_window {

	Level_select(Point xy, int w, int h, const string& title, string user)
		:Project_window{ xy,w,h,title },
		ten_button{ Point(200,50), 320, 100, "10", [](Address, Address pw) { reference_to<Level_select>(pw).set_state(Game_state(Game_10));} },
		twenty_button{ Point(200,150), 320, 100, "20", [](Address, Address pw) {reference_to<Level_select>(pw).set_state(Game_state(Game_20)); } },
		forty_button{ Point(200,250), 320, 100, "40", [](Address, Address pw) {reference_to<Level_select>(pw).set_state(Game_state(Game_40)); } },
		eighty_button{ Point(200,350), 320, 100, "80", [](Address, Address pw) {reference_to<Level_select>(pw).set_state(Game_state(Game_80)); } }
	{
		attach(username);
		set_username(user);
		attach(ten_button);
		attach(twenty_button);
		attach(forty_button);
		attach(eighty_button);
		attach(username);
	}

private:
	Button ten_button;
	Button twenty_button;
	Button forty_button;
	Button eighty_button;

	Text difficulty_10 = Text{ Point{ 100,100 }, "Difficulty 10" };
	Text difficulty_20 = Text{ Point{ 100,100 }, "Difficulty 20" };
	Text difficulty_40 = Text{ Point{ 100,100 }, "Difficulty 40" };
	Text difficulty_80 = Text{ Point{ 100,100 }, "Difficulty 80" };
};

struct Instruct_screen : public Project_window {


	Instruct_screen(Point xy, int w, int h, const string& title)
		:Project_window{ xy,w,h,title },
		go_to_levels{ Point{ 360 - 64,360 - 32 }, 128, 64, "Start",  [](Address, Address pw) { reference_to<Instruct_screen>(pw).set_state(Game_state(Level)); } }
	{
		attach(instruct1);
		attach(instruct2);
		attach(go_to_levels);
	}

private:
	Button go_to_levels;
	Text instruct1 = Text{ Point{ 100,100 }, "Click a tile next to the empty tile to move the tile into the empty tile's spot. Continue" };
	Text instruct2 = Text{ Point{ 100,115 }, "to do so until the tiles are in the correct numerical order." };
};

struct Splash_screen : public Project_window {
	Splash_screen(Point xy, int w, int h, const string& title)
		:Project_window{ xy,w,h,title },
		show_instructions{ Point{ 360 - 64,360 + 32 }, 128, 64, "Instuctions",  [](Address, Address pw) { reference_to<Splash_screen>(pw).set_state(Game_state(Instruct));} },
		play_button{ Point{ 360 - 64,360 - 32 }, 128, 64, "Start",  [](Address, Address pw) {  reference_to<Splash_screen>(pw).set_state(Game_state(Level)); } },
		name_entry{ In_box(Point(x_max() - 310, 0), 70, 30, "Enter initials") }
	{
		attach(play_button);
		attach(show_instructions);
		attach(game_name);
		attach(team_info);
		attach(team_roster);
		attach(name_entry);
	}

	string get_username() {
		return name_entry.get_string();
	}

private:
	Text game_name = Text{ Point{ 100,100 }, "Fifteen Game" };
	Text team_info = Text{ Point{ 100,150 }, "Team 41: TeamName" };
	Text team_roster = Text{ Point{ 100,200 }, "Charles Wong Savannah Yu Cindy Zhang Eric Zhang" };


	Button show_instructions;
	Button play_button;
	In_box name_entry;
};

struct Game_manager {

	Game_manager()
	:splash { Splash_screen(Point(0, 0), 720, 720, "Splash Screen")},
	instruct { Instruct_screen(Point(0, 0), 720, 720, "Instruct Screen")},
	level { Level_select(Point(0, 0), 720, 720, "Level Select", "username")},
	game { Game_screen(Point(0, 0), 720, 720, "Game Screen", 10, "username")},
	end { End_screen(Point(0, 0), 720, 720, "End Screen", 0, "username")}
	{
		instruct.hide();		
		level.hide();
		game.hide();
		end.hide();	
		current = splash.wait_for_button();
		username = splash.get_username();
		level.set_username(username);
		game.set_username(username);
		end.set_username(username);
		splash.hide();
	}
	void run() {
		while (current != Game_state(Quit)) {
			switch (current) {
			case (Game_state(Instruct)):
				instruct.show();
				current = instruct.wait_for_button();
				break;
			case (Game_state(Level)):
				level.show();
				current = level.wait_for_button();
				break;
			case (Game_state(Game_10)):
				game.show();
				game.set_difficulty(10);
				current = game.wait_for_button();
				break;
			case (Game_state(Game_20)):
				game.show();
				game.set_difficulty(20);
				current = game.wait_for_button();
				break;
			case (Game_state(Game_40)):
				game.show();
				game.set_difficulty(40);
				current = game.wait_for_button();
				break;
			case (Game_state(Game_80)):
				game.show();
				game.set_difficulty(80);
				current = game.wait_for_button();
				break;
			case (Game_state(End)):
				end.show();
				current = end.wait_for_button();
				break;
			case (Game_state(Quit)):
				return;
				break;
			default:
				cout << "Unexpected game state." << endl;
				cout << "Quitting..." << endl;
				return;
				break;
			}
		}
		return;
	}
private:
	int difficulty;
	Game_state current;
	string username;
	Splash_screen splash;
	Instruct_screen instruct;
	Level_select level;
	Game_screen game;
	End_screen end;
};

int main() {
	try {
		Game_manager game = Game_manager();
		game.run();
		//keep_window_open();
		return 0;

	}
	catch (exception& e) {
		cerr << "error:" << e.what() << endl;
		keep_window_open();
		return 1;
	}
	catch (...) {
		cerr << "Oops, unknown exception!" << endl;
		keep_window_open();
		return 2;
	}
}