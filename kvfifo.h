#ifndef JNP5_KVFIFO_H
#define JNP5_KVFIFO_H

#include <map>
#include <stdexcept>
#include<iostream>

template <typename K, typename V> class Obj {
public:
    const K key;
    V val;
    Obj * next;
    Obj * nextWithKey;
    Obj * prev;

    explicit Obj(K k):
        key(k),
        val(),
        next(nullptr),
        nextWithKey(nullptr),
        prev(nullptr){}

};

template <typename K, typename V> class Wrapper {
public:
    Obj<K, V> *firstMainQueue;
    Obj<K, V> *lastMainQueue;
    std::map<K, Obj<K, V> *> firstWithKey;
    std::map<K, Obj<K, V> *> lastWithKey;
    std::map<K, size_t> sizeWithKey;
    size_t sizeOfMain;
    size_t amountOfCopies;
    bool refExist;

    Wrapper():
            firstMainQueue(nullptr),
            lastMainQueue(nullptr),
            firstWithKey(),
            lastWithKey(),
            sizeWithKey(),
            sizeOfMain(0),
            amountOfCopies(1),
            refExist(false){}

    void pushWrap(K const &k, V const &v) {
        Obj<K, V> * helper = new Obj<K, V>(k);
        helper->val = v;
        helper->next = nullptr;
        helper->nextWithKey = nullptr;

        if (sizeOfMain == 0) {
            firstMainQueue = helper;
            lastMainQueue = helper;
            helper->prev = nullptr;
        } else {
            lastMainQueue->next = helper;
            helper->prev = lastMainQueue;
            lastMainQueue = helper;

        }
        sizeOfMain++;


        auto it = lastWithKey.find(k);
        if (it == lastWithKey.end()) { //first of such key
            firstWithKey.insert({k, helper});
            sizeWithKey.insert({k, 1});
            lastWithKey.insert({k, helper});
        } else {
            (it->second)->nextWithKey = helper;
            auto sizeIt = sizeWithKey.find(k);
            sizeIt->second++;
            (it->second) = helper;
        }
    }
};

template <typename K, typename V>
struct Iterator 
    {
        Iterator(typename std::map<K, Obj<K, V> *>::iterator ptr) : m_ptr(ptr) {}
        
        auto operator*() const { return m_ptr->first; }
        Iterator& operator++() { m_ptr++; return *this; }  
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        Iterator& operator--() { m_ptr--; return *this; }
        Iterator operator--(int) { Iterator tmp = *this; --(*this); return tmp; }
        friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };  

        private:
            typename std::map<K, Obj<K, V> *>::iterator m_ptr;
        
    };


template <typename K, typename V> class kvfifo{

public:
    Wrapper<K, V> *Wrap;
    
    //just helpful function that patches the list while removing
    //from it
    void removeFromMain(Obj<K ,V> * toDelete) {
        if (toDelete->next != nullptr) {
            (toDelete->next)->prev = toDelete->prev;
        }
        else{
            Wrap->lastMainQueue = toDelete->prev;
        }
        if (toDelete->prev != nullptr) {
            (toDelete->prev)->next = toDelete->next;
        }
        else{
            Wrap->firstMainQueue = toDelete->next;

        }
    }

    Wrapper<K, V> * createCopy(){
        Wrapper<K, V> *wrapCopy = new Wrapper<K, V>();//nasz
            wrapCopy->amountOfCopies = 1;
            if(Wrap->sizeOfMain > 0){
                auto x = Wrap->firstMainQueue;
                size_t i = 0;
                while(i < Wrap->sizeOfMain){
                    const K fi = x->key;
                    const V sn = x->val;
                    std::cout << fi <<": " << sn<<"\n";
                    wrapCopy->pushWrap(fi, sn);
                    x = x->next;
                    i++;
                }
            }
            Wrap->amountOfCopies--;
            return wrapCopy;
    }

    void checkCopy(){
        if(Wrap->amountOfCopies > 1){
            Wrapper<K, V> *wrapCopy  = createCopy();
            Wrap = wrapCopy;
        }
    }

    void checkRef(){
        if(Wrap->refExist){
            Wrapper<K, V> *wrapCopy = createCopy();
            Wrap = wrapCopy;
        }
    }

    void checkRefAndCopy(){
        if(Wrap->amountOfCopies > 1 || Wrap->refExist){
            Wrapper<K, V> *wrapCopy = createCopy();
            Wrap = wrapCopy;
        }
    }



public:
    kvfifo():
        Wrap(new Wrapper<K, V>()){}

    kvfifo(kvfifo const &other){
        Wrap = other.Wrap;
        Wrap->amountOfCopies++;
        checkRef();
    }

    //to robi, ze other jest pusty teraz?
    kvfifo(kvfifo &&other){
        //chyba trzeba clear ten poprzedni
        if(&other != this){
            Wrap = other.Wrap;
            // checkRef();
            other.Wrap = nullptr;
        }
    }

    kvfifo & operator= (kvfifo other){
        //chyba trzeba clear ten poprzedni
        this->Wrap = other.Wrap;
        Wrap->amountOfCopies++;
        checkRef();
        return *this;
    }

    void clear() {
        while(!empty()) {
            pop();
        }
    }
    size_t count(K const & k) const noexcept {
        auto ret = Wrap->sizeWithKey.find(k);
        if (ret == Wrap->sizeWithKey.end()) {
            return 0;
        }
        return ret->second;
    }

    bool empty() const noexcept {
        return Wrap->sizeOfMain == 0;
    }

    size_t size() const noexcept {
        return Wrap->sizeOfMain;
    }

    void push(K const &k, V const &v) {
        checkCopy();
        Wrap->refExist = false;
        Wrap->pushWrap(k, v);
    }

    void pop() {
        if (Wrap->firstMainQueue == nullptr) {
            throw std::invalid_argument("Queue was empty, yet pop was called");
        }
        pop(Wrap->firstMainQueue->key);
    }

    void pop(K k) {
        auto it = Wrap->firstWithKey.find(k);
        if (it == Wrap->firstWithKey.end()) {
            throw std::invalid_argument("Queue did not have such a key!");
        }
        //
        checkCopy();
        Wrap->refExist = false;
        Obj<K, V> * toDelete = it->second;
        removeFromMain(toDelete);
        it->second = toDelete->nextWithKey;
        delete toDelete;
        Wrap->sizeOfMain--;
        auto itSize = Wrap->sizeWithKey.find(k);
        //there was k in Wrap->firstWithKey, so k is in here too
        itSize->second--;
        if (itSize->second == 0) {
            Wrap->sizeWithKey.erase(itSize);
            Wrap->firstWithKey.erase(it);
            Wrap->lastWithKey.erase(k);
        }
    }

    void move_to_back(K const &k) {
        auto itFirst = Wrap->firstWithKey.find(k);
        if (itFirst == Wrap->firstWithKey.end()) {
            throw std::invalid_argument("Queue did not have elements of such key!");
        }
        checkCopy();
        Wrap->refExist = false;
        Obj<K, V> * helper = itFirst->second;
        while (helper != nullptr) {
            removeFromMain(helper);
            Wrap->lastMainQueue->next = helper;
            helper->prev = Wrap->lastMainQueue;
            helper->next = nullptr;
            Wrap->lastMainQueue = helper;

            helper = helper->nextWithKey;
        }
    }

    std::pair<K const &, V &> front() {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        // mozna edytowac, czyli copy?
        checkCopy();
        Wrap->refExist = true;
        return std::pair<K const &, V &>(
                Wrap->firstMainQueue->key,
                Wrap->firstMainQueue->val
                );
    }

    std::pair<K const &, V const &> front() const {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        return std::pair<K const &, V const &>(
                Wrap->firstMainQueue->key,
                Wrap->firstMainQueue->val
        );
    }

    std::pair<K const &, V &> back() {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        //mozna edytroac wiec copy
        checkCopy();
        Wrap->refExist = true;
        return std::pair<K const &, V &>(
                Wrap->lastMainQueue->key,
                Wrap->lastMainQueue->val
                );
    }
    // //WARNING: same as above
    std::pair<K const &, V const &> back() const {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        return std::pair<K const &, V const &>(
                Wrap->lastMainQueue->key,
                Wrap->lastMainQueue->val
        );
    }

    //TODO: first i last from tresc.txt simillarilly
      std::pair<K const &, V &> first(K const &key){
        if(empty()){
            throw std::invalid_argument("Queue was empty!");
        } else if(Wrap->firstWithKey.find(key) == Wrap->firstWithKey.end()){
            throw std::invalid_argument("Queue does not have a key!");
        }
        // mozna edytowac wiec copy
        checkCopy();
        Wrap->refExist = true;
        return std::pair<K const &, V &>(
                (Wrap->firstWithKey.find(key)->second->key),
                (Wrap->firstWithKey.find(key)->second->val)
        );
      }
        std::pair<K const &, V const &> first(K const &key) const{
        if(empty()){
            throw std::invalid_argument("Queue was empty!");
        } else if(Wrap->firstWithKey.find(key) == Wrap->firstWithKey.end()){
            throw std::invalid_argument("Queue does not have a key!");
        }
        return std::pair<K const &, V &>(
                Wrap->firstWithKey.find(key)->second->key,
                Wrap->firstWithKey.find(key)->second->val
        );
      }


      std::pair<K const &, V &> last(K const &key){
        if(empty()){
            throw std::invalid_argument("Queue was empty!");
        } else if(Wrap->lastWithKey.find(key) == Wrap->lastWithKey.end()){
            throw std::invalid_argument("Queue does not have a key!");
        }
        //mozna edytowac wiec copy
        checkCopy();
        Wrap->refExist = true;
        return std::pair<K const &, V &>(
                (Wrap->lastWithKey.find(key)->second->key),
                (Wrap->lastWithKey.find(key)->second->val)
        );
      }

      std::pair<K const &, V const &> last(K const &key) const {
        if(empty()){
            throw std::invalid_argument("Queue was empty!");
        } else if(Wrap->lastWithKey.find(key) == Wrap->lastWithKey.end()){
            throw std::invalid_argument("Queue does not have a key!");
        }
        //mozna edytowac wiec copy
        return std::pair<K const &, V &>(
                Wrap->lastWithKey.find(key)->second->key,
                Wrap->lastWithKey.find(key)->second->val
        );
      }
      
    Iterator<K, V> k_begin() { return Iterator<K, V>(Wrap->firstWithKey.begin()); }
    Iterator<K, V> k_end()   { return Iterator<K, V>(Wrap->firstWithKey.end()); }

    //W niektórych miejscach (np w push()) alokujemy nową pamięć, co wtedy z exceptionsami?
    //TODO: sprawdzic gdzie mozna dac const lub noexcept
    //TODO: naprawic alokowanie pamieci - chyba robie to w zbyt javovy sposób

};

#endif //JNP5_KVFIFO_H
