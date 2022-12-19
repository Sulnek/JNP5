#include "kvfifo.h"
#include <cassert>
#include <memory>
#include <vector>

auto f(kvfifo<int, int> q) {
    return q;
}

int main() {
    int keys[] = {3, 1, 1};

    kvfifo<int, int> kvf1 = f({});

    for (int i = 0; i < 3; ++i){
        kvf1.push(keys[i], i);
    }
    assert(kvf1.count(1) == 2);
    assert(kvf1.size() == 3);


    kvf1.move_to_back(1);
    assert(kvf1.size() == 3);
    // kvf1.clear();
    // assert(kvf1.empty());
    kvf1.pop(3);
    // kvf1.pop(1);
    // kvf1.pop();
    // kvf1.pop();


    auto &ref = kvf1.front().second;

    // auto const &ref = kvf1.front().second;


    // kvfifo<int, int> kvf2(kvf1); // Wykonuje się pełna kopia, dlaczego?
    // kvfifo<int, int> kvf3;
    // kvf3 = kvf2;

    assert(kvf1.front().second == 1);
    ref = 10;
    // auto ref = kvf1.front();


    assert(kvf1.front().second == 10);
    kvf1.pop();
    assert(kvf1.front().second == 2);
    assert(kvf1.back().second == 2);

    kvf1.pop();
    assert(kvf1.empty());

    kvf1.push(69, 420);
    kvf1.push(69, 18);

    assert(kvf1.back().second == 18);
    assert(kvf1.first(69).second == 420);

    auto &ref1 = kvf1.first(69).second;
    // ref1 = 123;
    // assert(kvf1.first(69).second == 123);

    assert(kvf1.back().second == 18);
    assert(kvf1.last(69).second == 18);

    auto &ref2 = kvf1.last(69).second;
    ref2 = 0;
    assert(kvf1.last(69).second == 0);
    assert(kvf1.back().second == 0);

    kvf1.push(1, 12345);
    kvf1.move_to_back(69);
    assert(kvf1.front().second == 12345);
    assert(kvf1.back().second == 0);
    assert(kvf1.last(69).second == 0);

    kvf1.clear();
    assert(kvf1.empty());

    // assert(kvf2.front().second != 10);

    // kvf2.pop(); // Obiekt kvf2 dokonuje kopii i przestaje współdzielić dane z kvf3.
    // assert(kvf2.size() == 2);
    // assert(kvf2.count(3) == 0);
    // assert(kvf2.count(2) == 1);

    // assert(kvf3.size() == 3);
    // assert(kvf3.count(3) == 1);

    // kvf2.push(1, 3);
    // kvf2.move_to_back(1);
    // assert(kvf2.size() == 3);
    // assert(kvf2.front().second == 2 &&
    //        kvf2.first(1).second == 1 &&
    //        kvf2.last(1).second == 3 &&
    //        kvf2.back().second == 3);

    // kvfifo<int, int> const kvf4 = kvf2;
    // assert(kvf4.front().second == 2 &&
    //        kvf4.first(1).second == 1 &&
    //        kvf4.last(1).second == 3 &&
    //        kvf4.back().second == 3);

    // int i = 1;
    // for (auto k_it = kvf1.k_begin(), k_end = kvf1.k_end(); k_it != k_end; ++k_it, ++i)
    //     assert(i <= 3 && *k_it == i);

    // auto kvf5 = std::make_unique<kvfifo<int, int>>();
    // kvf5->push(4, 0);
    // assert(kvf5->front().first == 4 && kvf5->front().second == 0);
    // auto kvf6(*kvf5);
    // kvf5.reset();
    // assert(kvf6.front().first == 4 && kvf6.front().second == 0);

    // std::swap(kvf1, kvf2);
    // std::vector<kvfifo<int, int>> vec;
    // for (int i = 0; i < 100000; i++)
    //     kvf1.push(i, i);
    // for (int i = 0; i < 1000000; i++)
    //     vec.push_back(kvf1);  // Wszystkie obiekty w vec współdzielą dane.
}

