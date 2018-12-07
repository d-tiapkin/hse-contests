#include <functional>
#include <initializer_list>
#include <type_traits>
#include <list>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    using CNode = std::pair<const KeyType, ValueType>;
    using Node =  std::pair<KeyType, ValueType>;

    // CONSTRUCTORS

    HashMap(Hash hash = Hash{}) :
            _hasher(hash)
    {
        _table.resize(INITIAL_SIZE);
    }

    template<typename Iter>
    HashMap(Iter begin, Iter end, Hash hash = Hash{}) :
            _hasher(hash)
    {
        _table.resize(INITIAL_SIZE);
        for (auto it = begin; it != end; ++it) {
            add(*it);
        }
    }

    HashMap(std::initializer_list<Node> list, Hash hash = Hash{}) :
            _hasher(hash)
    {
        _table.resize(list.size() * GROW_FACTOR);
        for (const auto& it : list) {
            add(it, true);
        }
    }

    // COPY

    HashMap(const HashMap& oth) {
        _table.resize(oth._table.size());
        for (auto it : oth) {
            insert(it);
        }
    }

    HashMap& operator=(const HashMap& oth) {
        HashMap copy(oth);

        clear();
        _table.resize(oth._table.size());

        for (auto it : copy) {
            insert(it);
        }

        return *this;
    }

    // SIZE

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    // HASH-FUNCTION

    auto hash_function() const {
        return _hasher;
    }

    // INSERT-ERASE

    void insert(const Node& x) {
        add(x);
    }

    void erase(const KeyType& key) {
        size_t it = find_key(key);
        if (_table[it].state != USED)
            return;

        _list.erase(_table[it].iter);
        _table[it].state = DELETED;

        --_size;
        normalize();
    }

    // ITERATORS

    using iterator = typename std::list<CNode>::iterator;
    using const_iterator = typename std::list<CNode>::const_iterator;

    iterator begin() {
        return _list.begin();
    }
    const_iterator begin() const {
        return _list.begin();
    }

    iterator end() {
        return _list.end();
    }
    const_iterator end() const {
        return _list.end();
    }

    // FIND

    const_iterator find(const KeyType& key) const {
        size_t it = find_key(key);
        if (_table[it].state == USED)
            return _table[it].iter;
        return end();
    }

    iterator find(const KeyType& key) {
        size_t it = find_key(key);
        if (_table[it].state == USED)
            return _table[it].iter;
        return end();
    }

    ValueType& operator [] (const KeyType& key) {
        auto iter = find(key);
        if (iter != end())
            return iter->second;

        insert({key, ValueType()});
        iter = find(key);
        return iter->second;
    }

    const ValueType& at(const KeyType& key) const {
        auto iter = find(key);
        if (iter != end())
            return iter->second;
        throw std::out_of_range("Invalid key");
    }

    // CLEAR

    void clear() {
        _fullness = 0;
        _size = 0;
        _list.clear();
        _table.clear();
        _table.resize(INITIAL_SIZE);
    }

    // REHASH
    void rehash(size_t new_size) {
        std::list<CNode> copy(std::move(_list));

        clear();
        _table.resize(new_size);

        for (const auto& elem : copy) {
            add(elem, true);
        }
    }

private:
    enum State { FREE, USED, DELETED };

    struct TableEntry {
        State state;
        iterator iter;

        TableEntry() :
                state(FREE)
        { }

        TableEntry(iterator it) :
                state(USED),
                iter(it)
        { }
    };


    void normalize() {
        if (_fullness > _table.size() * LOAD_FACTOR_UP) {
            size_t n = _table.size();
            rehash(n * GROW_FACTOR);
        }

        if (_size < _table.size() * LOAD_FACTOR_DOWN) {
            size_t n = _table.size();
            rehash(n / GROW_FACTOR);
        }
    }

    void add(const Node& x, bool skipNormalize = false) {
        if (find(x.first) != end())
            return;

        size_t it = find_free(x.first);
        _list.push_back(x);
        _table[it] = TableEntry(--_list.end());

        ++_size;
        if (_table[it].state != DELETED) {
            ++_fullness;
            if (!skipNormalize) normalize();
        }
    }

    size_t find_free(const KeyType& x) const {
        size_t it = _hasher(x)%(_table.size());

        while (_table[it].state == USED) {
            if (++it == _table.size()) it = 0;
        }
        return it;
    }

    size_t find_key(const KeyType& x) const {
        size_t it = _hasher(x) % (_table.size());

        while (_table[it].state == DELETED ||
                (_table[it].state == USED && !(_table[it].iter->first == x))) {
            if (++it == _table.size()) it = 0;
        }
        return it;
    }

    const size_t INITIAL_SIZE = 4;
    const int GROW_FACTOR = 2;
    const float LOAD_FACTOR_UP = 0.75;
    const float LOAD_FACTOR_DOWN = 0.25;

    std::vector<TableEntry> _table;
    std::list<CNode> _list;
    Hash _hasher;
    size_t _size = 0;
    size_t _fullness = 0;
};
