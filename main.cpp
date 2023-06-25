#include <iostream>
#include <vector>
#define DUMP(x) std::cout << #x << ":\t" << x << std::endl;
#include <string>
#include <sstream>
#include <random>
#include <iostream>
#include <queue>
#include <memory>
#include <algorithm>
#include <assert.h>
#include <chrono>
#include <thread>
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::vector;

constexpr const int N = 30;
constexpr const int M = 465;
constexpr const int K = 3;

constexpr const int64_t INF = 1000000000LL; // あり得ないぐらい大きなスコアの例を用意しておく

int min_id[N] = {};
int id_to_x[M] = {};
int id_to_y[M] = {};
int xy_toid[N][N] = {};
constexpr int dx[] = {-1, -1};
constexpr int dy[] = {-1, 0};
constexpr int DL = 2;

struct Coord
{
    int y_;
    int x_;
    Coord(const int y = 0, const int x = 0) : y_(y), x_(x) {}
};

class State : public std::enable_shared_from_this<State>
{
private:
public:
    std::shared_ptr<State> parent_ = nullptr;
    int64_t gap_sum_ = 0;                        // 探索上で評価したスコア
    std::pair<int, int> last_action_ = {-1, -1}; // 探索木のルートノードで最初に選択した行動
    int filled_n = 0;
    int64_t evaluated_score_ = 0;
    int nums[M] = {};
    int target_x_ = 0;
    int turn_ = 0;
    bool done_ = false;
    State()
    {
    }

    bool is_in(int x, int y)
    {
        return x >= 0 && y >= 0 && x >= y;
    }

    std::vector<int> neigbors(int id)
    {
        vector<int> ret = vector<int>();
        int x = id_to_x[id];
        int y = id_to_y[id];
        for (int di = 0; di < DL; di++)
        {
            int tx = x + dx[di];
            int ty = y + dy[di];
            if (is_in(tx, ty))
            {
                ret.emplace_back(xy_toid[tx][ty]);
            }
        }
        return ret;
    }

    int get_gap(int id)
    {
        int num = nums[id];
        int id_x = id_to_x[id];
        int num_x = id_to_x[num];
        return std::abs(num_x - id_x);
    }

    void read()
    {

        std::string buffer; // 1行ずつ読み込むバッファ
        int id = 0;
        this->gap_sum_ = 0;
        for (int x = 0; x < N; x++)
        {
            for (int y = 0; y < x + 1; y++)
            {
                int num = 0;
                cin >> num;
                nums[id] = num;
                gap_sum_ -= get_gap(id);

                id++;
            }
        }
    }
    // // ゲームの終了判定
    bool is_done()
    {
        return done_;
    }

    // // ゲームの終了判定（合法だが悪い終わり方。罠にはまるなど）
    bool is_dead()
    {
        return false;
    }

    // // 探索用の盤面評価をする
    void evaluate_score()
    {
        this->evaluated_score_ = target_x_ * (N * M) + gap_sum_;
    }

    // // 指定したactionでゲームを1ターン進める
    void advance(const std::pair<int, int> &action)
    {
        this->turn_++;
        auto id0 = action.first;
        auto id1 = action.second;

        auto gap0 = get_gap(id0);
        auto gap1 = get_gap(id1);
        if (gap0 == 0)
        {
            filled_n--;
        }
        this->gap_sum_ += gap0 + gap1;

        std::swap(nums[id0], nums[id1]);
        gap0 = get_gap(id0);
        gap1 = get_gap(id1);

        if (gap0 == 0)
        {
            filled_n++;
        }
        this->gap_sum_ -= gap0 + gap1;
        if (filled_n == target_x_ + 1)
        {
            filled_n = 0;
            target_x_++;
            if (target_x_ >= N)
            {
                done_ = true;
                return;
            }
            for (int y = 0; y < target_x_ + 1; y++)
            {
                auto target_id = xy_toid[target_x_][y];
                if (get_gap(target_id) == 0)
                {
                    filled_n++;
                }
            }
        }
        if (turn_ >= K)
        {
            done_ = true;
            return;
        }
    }

    // // 現在の状況でプレイヤーが可能な行動を全て取得する
    std::vector<std::pair<int, int>> legal_actions()
    {
        auto actions = vector<std::pair<int, int>>();
        for (int id = 0; id < M; id++)
        {
            auto num = nums[id];
            int id_x = id_to_x[id];
            int num_x = id_to_x[num];
            if (num_x == target_x_ && id_x != num_x)
            {
                for (auto &neigbor : this->neigbors(id_x))
                {
                    actions.emplace_back(id_x, neigbor);
                }
            }
        }
        assert(actions.size() > 0);
        return actions;
    }

    // virtual std::string __str__() = 0;

    std::shared_ptr<State> cloneAdvanced(const std::pair<int, int> &action)
    {
        auto clone = std::make_shared<State>();
        *clone = *this;
        auto actions = clone->legal_actions();
        clone->advance(action);
        clone->parent_ = shared_from_this();
        clone->last_action_ = action;
        return clone;
    }
};

// ビーム幅を指定してビームサーチで行動を決定する
std::vector<std::pair<int, int>> beamSearchAction(std::shared_ptr<State> state, const int beam_width)
{
    using StatePtr = std::shared_ptr<State>;
    std::priority_queue<StatePtr, std::vector<StatePtr>, std::greater<StatePtr>> now_beam;
    std::shared_ptr<State> best_state = nullptr;
    // cerr << "deb " << __LINE__ << endl;

    now_beam.emplace(state);
    for (int t = 0;; t++)
    {
        // cerr << "t " << t << "," << __LINE__ << endl;
        std::priority_queue<StatePtr, std::vector<StatePtr>, std::greater<StatePtr>> next_beam;

        for (int i = 0; i < beam_width; i++)
        {
            if (now_beam.empty())
                break;
            std::shared_ptr<State> now_state = now_beam.top();

            now_beam.pop();
            auto legal_actions = now_state->legal_actions();
            for (const auto &action : legal_actions)
            {
                // cerr << "deb " << __LINE__ << endl;

                auto next_state = now_state->cloneAdvanced(action);
                if (next_state->is_dead())
                {
                    continue;
                }
                next_state->evaluate_score();

                if (next_beam.size() >= beam_width && next_beam.top()->evaluated_score_ >= next_state->evaluated_score_)
                {
                    continue;
                }

                assert(next_state->parent_ != nullptr);

                if (next_state->is_done())
                {
                    if (best_state == nullptr || next_state > best_state)
                    {
                        best_state = next_state;
                    }
                    continue;
                }

                next_beam.emplace(next_state);
                if (next_beam.size() > beam_width)
                {
                    next_beam.pop();
                }
            }
        }

        if (best_state != nullptr)
        {
            break;
        }
        now_beam = next_beam;
    }

    std::vector<std::pair<int, int>> actions{};
    while (best_state->parent_ != nullptr)
    {
        actions.emplace_back(best_state->last_action_);
        best_state = best_state->parent_;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
}

int main()
{
    for (int x = 1; x < N; x++)
    {
        min_id[x] = (x) * (x + 1) / 2;
        // cerr << min_id[x] << ":";
        for (int y = 0; y < x + 1; y++)
        {
            int id = min_id[x] + y;
            id_to_x[id] = x;
            id_to_y[id] = y;
            xy_toid[x][y] = id;
            // cerr << id << " " << id_to_x[id] << " , ";
        }
        // cerr << endl;
    }
    // cerr << "xy_to_id:" << xy_toid[0][0] << endl;
    // cerr << "deb " << __LINE__ << endl;
    auto state_ptr = std::make_shared<State>();
    state_ptr->read();
    // cerr << "deb " << __LINE__ << endl;
    auto actions = beamSearchAction(state_ptr, 20);
    // cerr << "deb " << __LINE__ << endl;
    cout << actions.size() << endl;
    for (auto &action : actions)
    {
        cout << id_to_x[action.first] << " " << id_to_y[action.first] << " " << id_to_x[action.second] << " " << id_to_y[action.second] << endl;
    }

    return 0;
}