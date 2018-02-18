#include <functional>
#include <initializer_list>
#include <type_traits>
#include <list>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
public:
    using CNode = std::pair<const KeyType, ValueType>;
    using Node =  std::pair<KeyType, ValueType>;

    // CONSTRUCTORS

    explicit HashMap() {
        _table.resize(_START_SIZE);
    }


    template<typename Iter>
    HashMap(Iter begin, Iter end) {
        _table.resize(_START_SIZE);
        for (auto it = begin; it != end; ++it) {
            insert(*it);
        }
    }

    HashMap(std::initializer_list<Node> lst) {
        _table.resize(2*lst.size());
        for (auto it : lst) {
            add(it, true);
        }
    }

    // CONSTRUCTORS WITH HASHERS

    explicit HashMap(Hash hs) :
            _hasher(hs)
    {
        _table.resize(_START_SIZE);
    }

    template<typename Iter>
    HashMap(Iter begin, Iter end, Hash hs) :
            _hasher(hs)
    {
        _table.resize(_START_SIZE);
        for (auto it = begin; it != end; ++it) {
            insert(*it);
        }
    }

    HashMap(std::initializer_list<Node> lst, Hash hs) :
            _hasher(hs)
    {
        _table.resize(2*lst.size());
        for (auto it : lst) {
            add(it, true);
        }
    }

    // COPY

    HashMap(const HashMap& oth)
    {
        _table.resize(oth._table.size());
        for (auto it : oth) {
            insert(it);
        }
    }

    HashMap& operator=(const HashMap& oth) {
        HashMap copy(oth);

        rebuild(copy._table.size());
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

    auto hash_function() const {
        return _hasher;
    }

    // INSERT-ERASE



    void insert(const Node& x) {
        add(x, false);
    }

    void erase(const KeyType& key) {
        size_t it = find_key(key);
        if (!_table[it].used)
            return;

        _lst.erase(_table[it].it);
        _table[it].used = false;
        _table[it].deleted = true;

        --_size;
    }

    // ITERATORS

    using iterator = typename std::list<CNode>::iterator;
    using const_iterator = typename std::list<CNode>::const_iterator;

    iterator begin() {
        return _lst.begin();
    }
    const_iterator begin() const {
        return _lst.begin();
    }

    iterator end() {
        return _lst.end();
    }
    const_iterator end() const {
        return _lst.end();
    }

    // FIND

    const_iterator find(const KeyType& key) const {
        size_t it = find_key(key);
        if (_table[it].used)
            return _table[it].it;
        return end();
    }

    iterator find(const KeyType& key) {
        size_t it = find_key(key);
        if (_table[it].used)
            return _table[it].it;
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
        rebuild(_START_SIZE);
    }

private:
    struct TableValue {
        bool used;
        bool deleted;
        typename std::list<CNode>::iterator it;
    };

    void rebuild(size_t new_size) {
        _fullness = 0;
        _size = 0;
        _lst.clear();
        _table.clear();

        _table.resize(new_size);
    }

    void normalise() {
        if (_fullness > _table.size() * 3/4) {
            size_t n = _table.size();
            std::list<CNode> copy(_lst);

            rebuild(2*n);

            for (auto it = copy.begin(); it != copy.end(); ++it) {
                add(*it, true);
            }
        }

        if (_fullness < _table.size() * 1/4) {
            size_t n = _table.size();
            std::list<CNode> copy(_lst);

            rebuild(n/2);

            for (auto it = copy.begin(); it != copy.end(); ++it) {
                add(*it, true);
            }
        }
    }

    void add(const Node& x, bool normal) {
        if (find(x.first) != end())
            return;

        size_t it = find_free(x.first);
        bool was_deleted = _table[it].deleted;

        _lst.push_back(x);
        _table[it] = {true, false, --_lst.end()};

        ++_size;
        if (!was_deleted) {
            ++_fullness;
            if (!normal) normalise();
        }
    }

    size_t find_free(const KeyType& x) const {
        size_t it = _hasher(x)%(_table.size());

        while (_table[it].used) {
            it = (it + 1)%(_table.size());
        }

        return it;
    }

    size_t find_key(const KeyType& x) const {
        size_t it = _hasher(x)%(_table.size());

        while (_table[it].deleted || (_table[it].used && !(_table[it].it->first == x))) {
            it = (it + 1)%(_table.size());
        }

        return it;
    }

    const size_t _START_SIZE = 4;

    std::vector<TableValue> _table;
    std::list<CNode> _lst;
    Hash _hasher;
    size_t _size = 0;
    size_t _fullness = 0;
};
