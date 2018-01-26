#include <cstddef>
#include <utility>
#include <iostream>

namespace fwd_list {
    template <typename T>
    class Node {
    public:
        Node* next;
        T value;

        explicit Node(Node* next) :
                next(next),
                value()
        {}

        Node(Node* next, const T& elem) :
                next(next),
                value(elem)
        { }
    };

    template <typename T>
    class ListIterator {
    public:
        explicit ListIterator(Node<T>* it) : cur(it)
        { }

        ListIterator& operator++() {
            cur = cur->next;
            return *this;
        }

        ListIterator operator++(int) {
            auto copy = ListIterator(*this);
            ++(*this);
            return copy;
        }

        const T& operator* () const {
            return cur->value;
        }

        T& operator* () {
            return cur->value;
        }

        Node<T>* ptr () {
            return cur;
        }

        bool operator==(const ListIterator& oth) const {
            return cur == oth.cur;
        }

        bool operator!=(const ListIterator& oth) const {
            return cur != oth.cur;
        }

    private:
        Node<T>* cur;
    };
}  // namespace fwd_list

template <typename T>
class List {
public:
    List():
            sz(0)
    {
        tail = new fwd_list::Node<T>(nullptr);
        head = tail;
    }

    List(fwd_list::Node<T>* head, fwd_list::Node<T>* tail, size_t n) :
            head(head),
            tail(tail),
            sz(n)
    { }

    ~List() {
        DeleteAllNodes(head);
    }

    void push_front(const T& x) {
        head = new fwd_list::Node<T>(head, x);
        ++sz;
    }

    fwd_list::ListIterator<T> begin() {
        return fwd_list::ListIterator<T>(head);
    }

    fwd_list::ListIterator<T> end() {
        return fwd_list::ListIterator<T>(tail);
    }

    size_t size() const {
        return sz;
    }

    bool empty() const {
        return (sz == 0);
    }

    void sort() {
        size_t n = sz;
        if (n < 2) return;

        List<T> left;
        for (size_t m = 0; 2*m < n; ++m) {
            left.push_ptr(pop_ptr());
        }

        sort();
        left.sort();

        List<T> res;
        while (!empty() || !left.empty()) {
            if (left.empty() ||
                    (!empty() && head->value < left.head->value)) {
                res.push_ptr(pop_ptr());
            } else {
                res.push_ptr(left.pop_ptr());
            }
        }

        while (!res.empty()){
            push_ptr(res.pop_ptr());
        }
    }
private:
    void DeleteAllNodes(fwd_list::Node<T>* cur) {
        if (cur != tail) {
            DeleteAllNodes(cur->next);
        }
        delete cur;
    }

    fwd_list::Node<T>* pop_ptr() {
        auto x = head;
        head = x->next;
        x->next = nullptr;
        --sz;
        return x;
    }

    void push_ptr(fwd_list::Node<T>* ptr) {
        ptr->next = head;
        head = ptr;
        ++sz;
    }

    fwd_list::Node<T>* head;
    fwd_list::Node<T>* tail;
    size_t sz;
};


int main() {
    List<int> lst;
    size_t n;
    std::cin >> n;
    for (size_t i = 0; i != n; ++i) {
        int x;
        std::cin >> x;
        lst.push_front(x);
    }

    lst.sort();

    for (auto elem : lst) {
        std::cout << elem << ' ';
    }

    std::cout << '\n';
}
