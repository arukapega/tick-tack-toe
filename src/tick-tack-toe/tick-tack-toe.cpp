#include <memory>
#include <iostream>
#include <chrono>

class Mass {
public:
	enum status {
		BLANK,
		PLAYER,
		ENEMY,
	};
private:
	status s_ = BLANK;
public:
	void setStatus(status s) { s_ = s; }
	status getStatus() const { return s_; }

	bool put(status s) {
		if (s_ != BLANK) return false;
		s_ = s;
		return true;
	}
};

class Board;

class AI {
public:
	AI() {}
	virtual ~AI() {}

	virtual bool think(Board& b) = 0;

public:
	enum type {
		TYPE_ORDERED = 0,
		TYPE_NEGAMAX = 1,
		TYPE_NEGAMAX_ALPHA = 2,
		TYPE_MONTE_CARLO = 3,
		TYPE_MONTECARLO_TREE = 4
	};

	static AI* createAi(type type);
	int calcCount;
};

// 順番に打ってみる
class AI_ordered : public AI {
public:
	AI_ordered() {}
	~AI_ordered() {}

	bool think(Board& b);
};

class AI_negaMax : public AI {
protected:
	int evaluate(Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_negaMax() {}
	~AI_negaMax() {}

	bool think(Board& b);
};
class AI_negaMax_alpha : public AI {
	int evaluate(Board& b, Mass::status current, int& best_x, int& best_y, int alpha, int beta);
public:
	AI_negaMax_alpha() {}
	~AI_negaMax_alpha() {}

	bool think(Board& b);
};

class AI_monte_carlo : public AI {
	int evaluate(bool fiest_time,Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_monte_carlo() {}
	~AI_monte_carlo() {}

	bool think(Board& b);
};
class AI_montecarlo_tree : public AI {
	static int select_mass(int n, int *a_count, int *a_wins);
	int evaluate(bool all_search,int count, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_montecarlo_tree() {}
	~AI_montecarlo_tree() {}

	bool think(Board& b);
};
AI* AI::createAi(type type)
{
	switch (type) {
		case TYPE_NEGAMAX:
			return new AI_negaMax();
			break;
		case TYPE_NEGAMAX_ALPHA:
			return new AI_negaMax_alpha();
			break;
		case TYPE_MONTE_CARLO:
			return new AI_monte_carlo();
			break; 
		case TYPE_MONTECARLO_TREE:
			return new AI_montecarlo_tree();
			break;
	default:
		return new AI_ordered();
		break;
	}

	return nullptr;
}

class Board
{
	friend class AI_ordered; 
	friend class AI_negaMax;
	friend class AI_negaMax_alpha;
	friend class AI_monte_carlo;
	friend class AI_montecarlo_tree;

public:
	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};
private:
	enum {
		BOARD_SIZE = 3,
	};
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

public:
	Board() {
		//		mass_[0][0].setStatus(Mass::ENEMY); mass_[0][1].setStatus(Mass::PLAYER); 
	}
	Board::WINNER calc_result() const
	{
		// 縦横斜めに同じキャラが入っているか検索
		// 横
		for (int y = 0; y < BOARD_SIZE; y++) {
			Mass::status winner = mass_[y][0].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int x = 1;
			for (; x < BOARD_SIZE; x++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (x == BOARD_SIZE) { return (Board::WINNER)winner; }
		}
		// 縦
		for (int x = 0; x < BOARD_SIZE; x++) {
			Mass::status winner = mass_[0][x].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int y = 1;
			for (; y < BOARD_SIZE; y++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (y == BOARD_SIZE) { return(Board::WINNER) winner; }
		}
		// 斜め
		{
			Mass::status winner = mass_[0][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		{
			Mass::status winner = mass_[BOARD_SIZE - 1][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[BOARD_SIZE - 1 - idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		// 上記勝敗がついておらず、空いているマスがなければ引分け
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				Mass::status fill = mass_[y][x].getStatus();
				if (fill == Mass::BLANK) return NOT_FINISED;
			}
		}
		return DRAW;
	}

	bool put(int x, int y) {
		if (x < 0 || BOARD_SIZE <= x ||
			y < 0 || BOARD_SIZE <= y) return false;// 盤面外
		return mass_[y][x].put(Mass::PLAYER);
	}

	void show() const {
		std::cout << "　　";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << " " << x + 1 << "　";
		}
		std::cout << "\n　";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << "＋－";
		}
		std::cout << "＋\n";
		for (int y = 0; y < BOARD_SIZE; y++) {
			std::cout << " " << char('a' + y);
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "｜";
				switch (mass_[y][x].getStatus()) {
				case Mass::PLAYER:
					std::cout << "〇";
					break;
				case Mass::ENEMY:
					std::cout << "×";
					break;
				case Mass::BLANK:
					std::cout << "　";
					break;
				default:
//					if (mass_[y][x].isListed(Mass::CLOSE)) std::cout << "＋"; else
//					if (mass_[y][x].isListed(Mass::OPEN) ) std::cout << "＃"; else
					std::cout << "　";
				}
			}
			std::cout << "｜\n";
			std::cout << "　";
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "＋－";
			}
			std::cout << "＋\n";
		}
	}
};

bool AI_ordered::think(Board& b)
{
	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			if (b.mass_[y][x].put(Mass::ENEMY)) {
				return true;
			}
		}
	}
	return false;
}
int AI_negaMax_alpha::evaluate(Board& b, Mass::status current, int& best_x, int& best_y, int alpha, int beta) {
	Mass::status next = current == Mass::status::ENEMY ? Mass::status::PLAYER : Mass::status::ENEMY;

	int r = b.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;

	int score_max = -10001;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			calcCount++;
			Mass& m = b.mass_[x][y];
			if (m.getStatus() != Mass::BLANK) continue;

			// このマスにうったと仮定する
			m.setStatus(current);
			int dummy_x, dummy_y;
			int score = -evaluate(b, next, dummy_x, dummy_y, -beta, -alpha);
			m.setStatus(Mass::BLANK);

			if (score > score_max) {
				score_max = score;
				best_x = x;
				best_y = y;
			}
			//現在探索中の枝に置ける、最高スコアを求める
			alpha = std::max(alpha, score);
			if (alpha >= beta)//beta(これ以上は選ばれないとわかっているスコア)以上なら、もう探索する必要はない
			{
				//その時点でのスコアを返す
				return score_max;
			}
		}
	}
	return score_max;
}

bool AI_negaMax::think(Board& b)
{
	int x = -1, y=0;
	calcCount = 0;
	evaluate(b, Mass::status::ENEMY, x, y);
	std::cout << "evaluate内のループの中が呼ばれた回数は: " << calcCount << std::endl;
	//287757回

	if (x < 0) return false;
	return b.mass_[x][y].put(Mass::ENEMY);
}

int AI_negaMax::evaluate(Board& b, Mass::status current, int& best_x, int& best_y) {
	Mass::status next = current == Mass::status::ENEMY ? Mass::status::PLAYER : Mass::status::ENEMY;

	int r = b.calc_result();
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;
	int score_max = -10001;


	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			calcCount++;
			Mass& m = b.mass_[x][y];
			if (m.getStatus() != Mass::BLANK) continue;

			//このマスにうったと仮定する
			m.setStatus(current);
			int dummy;
			int score = -evaluate(b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			//現ノードの中で報酬が最も多いか？
			if (score_max < score) {
				score_max = score;
				best_x = x;
				best_y = y;
			}
		}
	}
	return score_max;
}
int AI_monte_carlo::evaluate(bool first_time,Board& board, Mass::status current, int& best_x, int& best_y) {
	Mass::status next = (current == Mass::status::ENEMY) ? Mass::status::PLAYER : Mass::status::ENEMY;

	int r = board.calc_result();
	
	if (r == current) return +10000;
	if (r == next) return -10000;
	if (r == Board::DRAW) return 0;


	//x,yのマスのテーブル(空いているマス)
	char x_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	char y_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	//勝ち負け
	int wins[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int loses[Board::BOARD_SIZE * Board::BOARD_SIZE];
	//空きマスの合計
	int blank_mass_num = 0;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass &m = board.mass_[y][x];
			if (m.getStatus() == Mass::BLANK)//配置できるなら
			{
				//x,yテーブルに空きマスを代入、勝ち負けを初期化する
				x_table[blank_mass_num] = x;
				y_table[blank_mass_num] = y;
				wins[blank_mass_num] = loses[blank_mass_num] = 0;
				blank_mass_num++;
			}
		}
	}

	//thinkで最初に呼ばれたときに限り
	if (first_time) {
		//10000回、ランダムに配置可能なマスから調べる
		for (int i = 0; i < 10000; i++) {
			//ランダムなので最善を返す保証はない
			int idx = rand() % blank_mass_num;
			Mass& m = board.mass_[y_table[idx]][x_table[idx]];

			//ランダムなマスに配置した際の勝敗を求める
			m.setStatus(current);
			int dummy;
			int score = -evaluate(false , board, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			//勝敗判定
			if (0 <= score) {
				wins[idx]++;
			}
			else {
				loses[idx]++;
			}
		}
		int score_max = -9999;
		for (int idx = 0; idx < blank_mass_num; idx++) {
			int score = wins[idx] + loses[idx];
			if (0 != score) {
				//配置した数(勝ち負けの合計)に対して、勝率を求める
				score = 100 * wins[idx] / score;
			}
			if (score_max < score)//今回評価したスコアの方が大きいなら更新
			{
				score_max = score;
				best_x = x_table[idx];
				best_y = y_table[idx];

			}
			std::cout << x_table[idx] + 1 << (char)('a' + y_table[idx]) << " score::" << score << "\n";
		}

		return score_max;
	}
	
	//firstTimeではないので、勝敗をカウントはしない、勝率も求めない
	//ただこのノードにおける残りの空きますに配置した時の評価を求める
	int idx = rand() % blank_mass_num;
	Mass &m = board.mass_[y_table[idx]][x_table[idx]];
	m.setStatus(current);
	int dummy_x,dummy_y;
	int score = -evaluate(false, board, next, dummy_x, dummy_y);
	m.setStatus(Mass::BLANK);

	return score;
}
bool AI_monte_carlo::think(Board& b)
{
	int best_x = -1, best_y = 0;
	calcCount = 0;
	evaluate(true, b, Mass::ENEMY, best_x, best_y);

	std::cout << "evaluate内のループの中が呼ばれた回数は: " << calcCount << std::endl;
	//

	if (best_x < 0) return false;
	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}


bool AI_negaMax_alpha::think(Board& b)
{
	int x = -1, y = 0;
	calcCount = 0;
	evaluate(b, Mass::status::ENEMY, x, y, -10001, 10001);

	std::cout << "evaluate内のループの中が呼ばれた回数は: " << calcCount << std::endl;
	//9293回～10092回

	if (x < 0) return false;
	return b.mass_[x][y].put(Mass::ENEMY);
}

int AI_montecarlo_tree::evaluate(bool all_search,int sim_count, Board& board, Mass::status current, int& best_x, int& best_y) {
	Mass::status next = (current == Mass::status::ENEMY) ? Mass::status::PLAYER : Mass::status::ENEMY;

	int r = board.calc_result();

	if (r == current) return +100;
	if (r == next) return -100;
	if (r == Board::DRAW) return 0;


	//x,yのマスのテーブル(空いているマス)
	char x_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	char y_table[Board::BOARD_SIZE * Board::BOARD_SIZE];
	//勝ち負け
	int wins[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int count[Board::BOARD_SIZE * Board::BOARD_SIZE];
	int scores[Board::BOARD_SIZE * Board::BOARD_SIZE];
	//空きマスの合計
	int blank_mass_num = 0;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass& m = board.mass_[y][x];
			if (m.getStatus() == Mass::BLANK)//配置できるなら
			{
				//x,yテーブルに空きマスを代入、勝ち負けを初期化する
				x_table[blank_mass_num] = x;
				y_table[blank_mass_num] = y;
				wins[blank_mass_num] = count[blank_mass_num] = 0;
				scores[blank_mass_num] = -1;
				blank_mass_num++;
			}
		}
	}

	//thinkで最初に呼ばれたときに限り
	if (all_search) {
		//10000回、ランダムに配置可能なマスから調べる
		for (int i = 0; i < sim_count; i++) {
			//ランダムなので最善を返す保証はない
			int idx = select_mass( blank_mass_num, count, wins);
			if (idx < 0) break;
			Mass& m = board.mass_[y_table[idx]][x_table[idx]];

			//ランダムなマスに配置した際の勝敗を求める
			m.setStatus(current);
			int dummy;
			int score = -evaluate(false,0, board, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			//勝敗判定
			if (0 < score) {
				wins[idx]++;
				count[idx]++;
			}
			else {
				count[idx]++;
			}

			if (sim_count / 10 < count[idx]
				&& 10 < sim_count) {
				m.setStatus(current);
				scores[idx] = 100 - evaluate(true, (int)sqrt(sim_count), board, next, dummy, dummy);
				m.setStatus(Mass::BLANK);
				wins[idx] = -1;
			}
		}
		int score_max = -9999;
		for (int idx = 0; idx < blank_mass_num; idx++) {
			int score;
			if (-1 == wins[idx]) {
				score = scores[idx];
			}
			else if (0 == count[idx]) {
				score = 0;
			}
			else {
				double c = 1 * sqrt(2 * log(sim_count) / count[idx]);
				score = 100 * wins[idx] / count[idx] + (int)(c);
			}

			if (score_max < score)//今回評価したスコアの方が大きいなら更新
			{
				score_max = score;
				best_x = x_table[idx];
				best_y = y_table[idx];
				//std::cout << "以下のマスが最高スコアに更新する\n";

			}
			std::cout << x_table[idx] + 1 << (char)('a' + y_table[idx]) << " score::" << score << "\n";
		}

		return score_max;
	}

	//all_serachではないので、勝敗をカウントはしない、勝率も求めない
	//ただこのノードにおける残りの空きますに配置した時の評価を求める
	int idx = rand() % blank_mass_num;
	Mass& m = board.mass_[y_table[idx]][x_table[idx]];
	m.setStatus(current);
	int dummy;
	int score = -evaluate(false,0, board, next, dummy, dummy);
	m.setStatus(Mass::BLANK);

	return score;
}
int AI_montecarlo_tree::select_mass(int n, int *a_count, int *a_wins)
{
	int total = 0;
	for (int i = 0; i < n; i++) {
		total += 10000 * (a_wins[i] + 1) / (a_count[i] + 1);
	}
	if (total <= 0) return -1;

	int r = rand() % total;
	for (int i = 0; i < n; i++) {
		r -= 10000 * (a_wins[i] + 1) / (a_count[i] + 1);
		if (r < 0) {
			return i;
		}
	}
	return -1;
}
bool AI_montecarlo_tree::think(Board& b)
{
	int x = -1, y = 0;
	evaluate(true, 10000, b, Mass::ENEMY, x, y);

	std::cout << "evaluate内のループの中が呼ばれた回数は: " << calcCount << std::endl;
	//

	if (x < 0) return false;
	return b.mass_[y][x].put(Mass::ENEMY);
}
class Game
{
private:
	const AI::type ai_type = AI::TYPE_MONTECARLO_TREE;

	Board board_;
	Board::WINNER winner_ = Board::NOT_FINISED;
	AI* pAI_ = nullptr;

public:
	Game() {
		pAI_ = AI::createAi(ai_type);
	}
	~Game() {
		delete pAI_;
	}

	bool put(int x, int y) {
		bool success = board_.put(x, y);
		if (success) winner_ = board_.calc_result();

		return success;
	}

	bool think() {
		bool success = pAI_->think(board_);
		if (success) winner_ = board_.calc_result();
		return success;
	}

	Board::WINNER is_finised() {
		return winner_;
	}

	void show() {
		board_.show();
	}
};




void show_start_message()
{
	std::cout << "========================" << std::endl;
	std::cout << "       GAME START       " << std::endl;
	std::cout << std::endl;
	std::cout << "input position likes 1 a" << std::endl;
	std::cout << "========================" << std::endl;
}

void show_end_message(Board::WINNER winner)
{
	if (winner == Board::PLAYER) {
		std::cout << "You win!" << std::endl;
	}
	else if (winner == Board::ENEMY)
	{
		std::cout << "You lose..." << std::endl;
	}
	else {
		std::cout << "Draw" << std::endl;
	}
	std::cout << std::endl;
}

int main()
{
	for (;;) {// 無限ループ
		show_start_message();

		// initialize
		unsigned int turn = 0;
		std::shared_ptr<Game> game(new Game());

		while (1) {
			game->show();// 盤面表示

			// 勝利判定
			Board::WINNER winner = game->is_finised();
			if (winner) {
				show_end_message(winner);
				break;
			}

			if (0 == turn) {
				// user input
				char col[1], row[1];
				do {
					std::cout << "? ";
					std::cin >> row >> col;
				} while (!game->put(row[0] - '1', col[0] - 'a'));
			}
			else {
				// AI
				if (!game->think()) {
					show_end_message(Board::WINNER::PLAYER);// 投了
				}
				std::cout << std::endl;
			}
			// プレイヤーとAIの切り替え
			turn = 1 - turn;
		}
	}

	return 0;
}
