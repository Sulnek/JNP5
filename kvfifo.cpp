#include "kvfifo.h"

template <typename K, typename V> class Obj {
public:
    const K key;
    V val;
    Obj * next;
    Obj * nextWithKey;
    Obj * prev;
};

template <typename K, typename V> class kvfifo {
private:
    Obj<K, V> * firstMainQueue;
    Obj<K, V> * lastMainQueue;
    std::map<K, Obj<K, V> *> firstWithKey;
    std::map<K, Obj<K, V> *> lastWithKey;
    std::map<K, size_t> sizeWithKey;
    size_t sizeOfMain;

    //just helpful function that patches the list while removing
    //from it
    void removeFromMain(Obj<K ,V> * toDelete) {
        if (toDelete->next != nullptr) {
            (toDelete->next)->prev = toDelete->prev;
        }
        if (toDelete->prev != nullptr) {
            (toDelete->prev)->next = toDelete->next;
        }
    }

public:
    kvfifo():
            firstMainQueue(nullptr),
            lastMainQueue(nullptr),
            firstWithKey(),
            sizeWithKey(),
            lastWithKey(),
            sizeOfMain(0){}

    //kvfifo(kvfifo const &);

    //kvfifo(kvfifo &&);

    //kvfifo & operator= (kvfifo other);

    bool empty() const {
        return sizeOfMain == 0;
    }

    size_t size() const {
        return sizeOfMain;
    }

    void push(K const &k, V const &v) {
        Obj<K, V> helper = new Obj<K, V>();
        helper.val = v;
        helper.key = k;
        helper.next = nullptr;
        helper.nextWithKey = nullptr;
        if (empty()) {
            firstMainQueue = &helper;
            lastMainQueue = &helper;
            helper.prev = nullptr;
        } else {
            lastMainQueue->next = &helper;
            helper.prev = lastMainQueue;
        }
        sizeOfMain++;

        auto it = lastWithKey.find(k);
        if (it == lastWithKey.end()) { //first of such key
            firstWithKey.insert(k, &helper);
            sizeWithKey.insert(k, 1);
            lastWithKey.insert(k, helper);
        } else {
            (it->second)->nextWithKey = &helper;
            auto sizeIt = sizeWithKey.find(k);
            sizeIt->second++;
        }
    }

    void pop() {
        if (firstMainQueue == nullptr) {
            throw std::invalid_argument("Queue was empty, yet pop was called");
        }
        pop(firstMainQueue->key);
    }

    void pop(K k) {
        auto it = firstWithKey.find(k);
        if (it == firstWithKey.end()) {
            throw std::invalid_argument("Queue did not have such a key!");
        }
        Obj<K, V> * toDelete = it->second;
        removeFromMain(toDelete);
        it->second = toDelete->nextWithKey;
        //TODO: free(toDelete) - but idk if valgrind will be angry
        sizeOfMain--;
        auto itSize = sizeWithKey.find(k);
        //there was k in firstWithKey, so k is in here too
        itSize->second--;
        if (itSize->second == 0) {
            sizeWithKey.erase(itSize);
            firstWithKey.erase(it);
            lastWithKey.erase(k);
        }
    }

    void move_to_back(K const &k) {
        auto itFirst = firstWithKey.find(k);
        if (itFirst == firstWithKey.end()) {
            throw std::invalid_argument("Queue did not have elements of such key!");
        }
        Obj<K, V> * helper = itFirst->second;
        while (helper != nullptr) {
            removeFromMain(helper);
            lastMainQueue->next = helper;
            helper->prev = lastMainQueue;
            helper->next = nullptr;
            lastMainQueue = helper;

            helper = helper->nextWithKey;
        }
    }

    std::pair<K const &, V &> front(){
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        return new std::pair<K const &, V &>(
                &(firstMainQueue->key),
                &(firstMainQueue->val)
                );
    }

    //TODO: Verify next two WARNINGs and maybe fix them
    //WARNING: seems sth is wrong but compiles??
    std::pair<K const &, V const &> front() const {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        return new std::pair<K const &, V const &>(
                &(firstMainQueue->key),
                &(firstMainQueue->val)
        );
    }

    std::pair<K const &, V &> back() {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        return new std::pair<K const &, V &>(
                &(lastMainQueue->key),
                &(lastMainQueue->val)
                );
    }
    //WARNING: same as above
    std::pair<K const &, V const &> back() const {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        return new std::pair<K const &, V const &>(
                &(lastMainQueue->key),
                &(lastMainQueue->val)
        );
    }

    //TODO: first i last from tresc.txt simillarilly
    //TODO: Iterator po zbiorze kluczy (kt??ra?? mapa .keyValues() zwraca wszystkie klucze chyba)
    //W niekt??rych miejscach (np w push()) alokujemy now?? pami????, co wtedy z exceptionsami?
    //TODO: sprawdzic gdzie mozna dac const lub noexcept
    //TODO: naprawic alokowanie pamieci - chyba robie to w zbyt javovy spos??b
};