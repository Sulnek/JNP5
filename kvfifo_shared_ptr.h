#ifndef JNP5_KVFIFO_H
#define JNP5_KVFIFO_H

#include <map>
#include <stdexcept>
#include <iostream>
#include <memory>

template <typename K, typename V> class Obj {
public:
    const K key;
    V val;
    std::shared_ptr< Obj<K, V> > next;
    std::shared_ptr< Obj<K, V> > nextWithKey;
    std::shared_ptr< Obj<K, V> > prev;


    explicit Obj(K k):
        key(k),
        val(),
        next(nullptr),
        nextWithKey(nullptr),
        prev(nullptr){}

};

template <typename K, typename V> class Wrapper {
public:
    std::shared_ptr< Obj<K, V> > firstMainQueue;
    std::shared_ptr< Obj<K, V> > lastMainQueue;
    std::map<K, std::shared_ptr< Obj<K, V> > > firstWithKey;
    std::map<K, std::shared_ptr< Obj<K, V> > > lastWithKey;
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
        std::shared_ptr< Obj<K, V> > sP = std::make_shared< Obj<K, V> >(k);
        sP.get();
        sP.get()->val = v;
        sP.get()->next = nullptr;
        sP.get()->nextWithKey = nullptr;

        if (sizeOfMain == 0) {
            firstMainQueue = sP;
            lastMainQueue = sP;
            sP.get()->prev = nullptr;
        } else {
            lastMainQueue->next = sP;
            sP.get()->prev = lastMainQueue;
            lastMainQueue = sP;
        }
        sizeOfMain++;


        auto it = lastWithKey.find(k);
        if (it == lastWithKey.end()) {
            firstWithKey.insert({k, sP});
            sizeWithKey.insert({k, 1});
            lastWithKey.insert({k, sP});
        } else {
            (it->second)->nextWithKey = sP;
            auto sizeIt = sizeWithKey.find(k);
            sizeIt->second++;
            (it->second) = sP;
        }
    }
};

template <typename K, typename V>
struct Iterator 
    {
        Iterator(typename std::map<K, std::shared_ptr< Obj<K, V> >>::iterator ptr) : m_ptr(ptr) {}
        
        auto operator*() const { return m_ptr->first; }
        auto operator->() { return &(m_ptr->first); }
        Iterator& operator++() { m_ptr++; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        Iterator& operator--() { m_ptr--; return *this; }
        Iterator operator--(int) { Iterator tmp = *this; --(*this); return tmp; }
        friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };  

        private:
            typename std::map<K, std::shared_ptr< Obj<K, V> > >::iterator m_ptr;
        
    };


template <typename K, typename V> class kvfifo{

private:
    std::shared_ptr< Wrapper<K, V> > Wrap;
    void removeFromMain(std::shared_ptr< Obj<K, V> >toDel) {
        auto toDelete = toDel.get();
        if (toDelete->next != nullptr) {
            (toDelete->next)->prev = toDelete->prev;
        }
        else{
            Wrap.get()->lastMainQueue = toDelete->prev;
        }
        if (toDelete->prev != nullptr) {
            (toDelete->prev)->next = toDelete->next;
        }
        else{
            Wrap.get()->firstMainQueue = toDelete->next;

        }
    }

    std::shared_ptr< Wrapper<K, V> > createCopy(){
        std::shared_ptr< Wrapper<K, V> > wrapCopy = std::make_shared< Wrapper<K, V> >();
            wrapCopy->amountOfCopies = 1;
            if(Wrap.get()->sizeOfMain > 0){
                auto x = Wrap.get()->firstMainQueue.get();
                size_t i = 0;
                while(i < Wrap.get()->sizeOfMain){
                    K fi = x->key;
                    V sn = x->val;
                    wrapCopy->pushWrap(fi, sn);
                    x = (x->next).get();
                    i++;
                }
            }
            Wrap.get()->amountOfCopies--;
            return wrapCopy;
    }

    void checkCopy(){
        if(Wrap.get()->amountOfCopies > 1){
            std::shared_ptr< Wrapper<K, V> >wrapCopy  = createCopy();
            Wrap = wrapCopy;
        }

    }

    void checkRef(){
        if(Wrap.get()->refExist){
            std::shared_ptr< Wrapper<K, V> > wrapCopy = createCopy();
            Wrap = wrapCopy;
        }
    }


public:
    kvfifo(){
        Wrap = std::make_shared< Wrapper<K, V> >();
    }

    kvfifo(kvfifo const &other){
        Wrap = other.Wrap;
        Wrap.get()->amountOfCopies++;
        checkRef();
    }

    kvfifo(kvfifo &&other){
        if(&other != this){
            Wrap = other.Wrap;
            //no tutaj to najpierw trzeba wyczyscic stary :P
            other.Wrap = nullptr;
        }
    }

    kvfifo & operator= (kvfifo other){
        if(Wrap != nullptr){
            Wrap.get()->amountOfCopies--;
            if(Wrap.get()->amountOfCopies == 0){
                clear();
            }
        }
        Wrap = other.Wrap;
        Wrap.get()->amountOfCopies++;
        checkRef();
        return *this;
    }

    void clear() {
        while(!empty()) {
            pop();
        }
    }
    
    size_t count(K const & k) const noexcept {
        auto ret = Wrap.get()->sizeWithKey.find(k);
        if (ret == Wrap.get()->sizeWithKey.end()) {
            return 0;
        }
        return ret->second;
    }

    bool empty() const noexcept {
        return Wrap.get()->sizeOfMain == 0;
    }

    size_t size() const noexcept {
        return Wrap.get()->sizeOfMain;
    }

    void push(K const &k, V const &v) {
        checkCopy();
        Wrap.get()->refExist = false;
        Wrap.get()->pushWrap(k, v);
    }

    void pop() {
        if (Wrap.get()->firstMainQueue.get() == nullptr) {
            throw std::invalid_argument("Queue was empty, yet pop was called");
        }
        pop(Wrap.get()->firstMainQueue.get()->key);
    }

    void pop(K k) {
        auto it = Wrap.get()->firstWithKey.find(k);
        if (it == Wrap.get()->firstWithKey.end()) {
            throw std::invalid_argument("Queue did not have such a key!");
        }
        checkCopy();
        it = Wrap.get()->firstWithKey.find(k);
        if (it == Wrap.get()->firstWithKey.end()) {
            throw std::invalid_argument("Queue did not have such a key!");
        }
        Wrap.get()->refExist = false;
        removeFromMain(it->second);
        it->second = (it->second).get()->nextWithKey;
        Wrap.get()->sizeOfMain--;
        auto itSize = Wrap.get()->sizeWithKey.find(k);
        itSize->second--;
        if (itSize->second == 0) {
            Wrap.get()->sizeWithKey.erase(itSize);
            Wrap.get()->firstWithKey.erase(it);
            Wrap.get()->lastWithKey.erase(k);
        }
    }

    void move_to_back(K const &k) {
        auto itFirst = Wrap.get()->firstWithKey.find(k);
        if (itFirst == Wrap.get()->firstWithKey.end()) {
            throw std::invalid_argument("Queue did not have elements of such key!");
        }
        checkCopy();
        itFirst = Wrap.get()->firstWithKey.find(k);
        if (itFirst == Wrap.get()->firstWithKey.end()) {
            throw std::invalid_argument("Queue did not have elements of such key!");
        }
        Wrap.get()->refExist = false;
        std::shared_ptr< Obj<K, V> > helper = (itFirst->second);
        int i = count(k);
        for(int j = 0 ; j < i ; j++){
            removeFromMain(helper);
            Wrap.get()->lastMainQueue.get()->next = helper;
            helper.get()->prev = (Wrap.get()->lastMainQueue);
            helper.get()->next = nullptr;
            Wrap.get()->lastMainQueue = helper;
            helper = (helper).get()->nextWithKey;
        }
    }

    std::pair<K const &, V &> front() {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        checkCopy();
        Wrap.get()->refExist = true;
        return std::pair<K const &, V &>(
                Wrap.get()->firstMainQueue.get()->key,
                Wrap.get()->firstMainQueue.get()->val
                );
    }

    std::pair<K const &, V const &> front() const {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        return std::pair<K const &, V const &>(
                Wrap.get()->firstMainQueue.get()->key,
                Wrap.get()->firstMainQueue.get()->val
        );
    }

    std::pair<K const &, V &> back() {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        checkCopy();
        Wrap.get()->refExist = true;
        return std::pair<K const &, V &>(
                Wrap.get()->lastMainQueue.get()->key,
                Wrap.get()->lastMainQueue.get()->val
                );
    }

    std::pair<K const &, V const &> back() const {
        if (empty()) {
            throw std::invalid_argument("Queue was empty!");
        }
        return std::pair<K const &, V const &>(
                Wrap.get()->lastMainQueue.get()->key,
                Wrap.get()->lastMainQueue.get()->val
        );
    }

    std::pair<K const &, V &> first(K const &key){
        if(empty()){
            throw std::invalid_argument("Queue was empty!");
        } else if(Wrap.get()->firstWithKey.find(key) == Wrap.get()->firstWithKey.end()){
            throw std::invalid_argument("Queue does not have a key!");
        }
        checkCopy();
        Wrap.get()->refExist = true;
        return std::pair<K const &, V &>(
                (Wrap.get()->firstWithKey.find(key)->second.get()->key),
                (Wrap.get()->firstWithKey.find(key)->second.get()->val)
        );
      }
    
    std::pair<K const &, V const &> first(K const &key) const{
        if(empty()){
            throw std::invalid_argument("Queue was empty!");
        } else if(Wrap.get()->firstWithKey.find(key) == Wrap.get()->firstWithKey.end()){
            throw std::invalid_argument("Queue does not have a key!");
        }
        return std::pair<K const &, V &>(
                Wrap.get()->firstWithKey.find(key)->second.get()->key,
                Wrap.get()->firstWithKey.find(key)->second.get()->val
        );
      }

    std::pair<K const &, V &> last(K const &key){
        if(empty()){
            throw std::invalid_argument("Queue was empty!");
        } else if(Wrap.get()->lastWithKey.find(key) == Wrap.get()->lastWithKey.end()){
            throw std::invalid_argument("Queue does not have a key!");
        }
        checkCopy();
        Wrap.get()->refExist = true;
        return std::pair<K const &, V &>(
                (Wrap.get()->lastWithKey.find(key)->second.get()->key),
                (Wrap.get()->lastWithKey.find(key)->second.get()->val)
        );
      }

    std::pair<K const &, V const &> last(K const &key) const {
        if(empty()){
            throw std::invalid_argument("Queue was empty!");
        } else if(Wrap.get()->lastWithKey.find(key) == Wrap.get()->lastWithKey.end()){
            throw std::invalid_argument("Queue does not have a key!");
        }
        return std::pair<K const &, V &>(
                Wrap.get()->lastWithKey.find(key)->second.get()->key,
                Wrap.get()->lastWithKey.find(key)->second.get()->val
        );
      }
      
    Iterator<K, V> k_begin() { return Iterator<K, V>(Wrap.get()->firstWithKey.begin()); }
    Iterator<K, V> k_end()   { return Iterator<K, V>(Wrap.get()->firstWithKey.end()); }

    //W niektórych miejscach (np w push()) alokujemy nową pamięć, co wtedy z exceptionsami?
    //TODO: sprawdzic gdzie mozna dac const lub noexcept
    //TODO: naprawic alokowanie pamieci - chyba robie to w zbyt javovy sposób

};

#endif //JNP5_KVFIFO_H
