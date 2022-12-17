#ifndef JNP5_KVFIFO_H
#define JNP5_KVFIFO_H

#include <map>
#include <stdexcept>

template <typename K, typename V> class kvfifo;
/*
void clear();
size_t count(K const &) const;
bool empty() const;
size_t size() const;
std::pair<K const &, V &> first(K const &key);
std::pair<K const &, V const &> first(K const &key) const;
std::pair<K const &, V &> last(K const &key);
std::pair<K const &, V const &> last(K const &key) const;
std::pair<K const &, V &> front();
std::pair<K const &, V const &> front() const;
std::pair<K const &, V &> back();
std::pair<K const &, V const &> back() const;
void move_to_back(K const &k);
void pop(K const &);
void pop();
void push(K const &k, V const &v);
kvfifo();
kvfifo(kvfifo const &);
kvfifo(kvfifo &&);
kvfifo& operator=(kvfifo other); */

#endif //JNP5_KVFIFO_H
