#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <exception>
#include <deque>
#include <iomanip>
#include <cassert>

template <typename C, typename W>
class Edge {
private:
    size_t id;

    size_t v_;
    size_t to_;

    C capacity_;
    C flow_;
    W weight_;

    size_t rev_edge_;
public:
    Edge(size_t id, size_t v, size_t to, const W& weight, const C& capacity) :
            id(id),
            v_(v),
            to_(to),
            capacity_(capacity),
            flow_(W(0)),
            weight_(weight)
    { }

    const size_t& get_id() const {
        return id;
    }

    size_t& rev_edge() {
        return rev_edge_;
    }

    C res_capacity() const {
        return capacity_ - flow_;
    }

    void clear_flow() {
        flow_ = 0;
    }

    void change_flow(const C& delta) {
        flow_ += delta;
    }

    const C& flow() const {
        return flow_;
    }

    const W& weight() const {
        return weight_;
    }

    size_t v() const {
        return v_;
    }

    size_t to() const {
        return to_;
    }
};

template <typename C, typename W>
class TransportNet {
private:
    std::vector <std::vector <size_t> > vertex_;
    std::vector <Edge<C, W> > edges_;
    std::vector <W> potential_;
    W infinity;

    void ford_bellman(size_t s, std::vector <W>& dist) {
        dist.assign(vertex_.size(), infinity);
        dist[s] = W(0);

        for (size_t i = 0; i != vertex_.size(); ++i) {
            for (auto e : edges_) {
                if (e.res_capacity() == C(0))
                    continue;
                size_t v = e.v();
                size_t to = e.to();
                W weight = e.weight();

                if (dist[to] > dist[v] + weight) {
                    dist[to] = dist[v] + weight;
                }
            }
        }
    }

    void dijkstra(size_t s, std::vector <W>& dist, std::vector <size_t>& parent) {
        dist.assign(vertex_.size(), infinity);
        parent.resize(vertex_.size());
        std::vector <bool> used(vertex_.size());

        dist[s] = 0;

        for (size_t i = 0; i != vertex_.size(); ++i) {
            W cur_dist = infinity;
            size_t v = 0;

            for (size_t u = 0; u != dist.size(); ++u) {
                if (!used[u] && cur_dist > dist[u]) {
                    cur_dist = dist[u];
                    v = u;
                }
            }

            if (used[v]) return;
            used[v] = true;

            for (auto id : vertex_[v]) {
                if (edges_[id].res_capacity() == 0)
                    continue;
                size_t to = edges_[id].to();
                W weight = edges_[id].weight() - potential_[to] + potential_[v];

                if (dist[to] > dist[v] + weight) {
                    parent[to] = id;
                    dist[to] = dist[v] + weight;
                }
            }
        }
    }

    void clear_flow() {
        for (auto e : edges_) {
            e.clear_flow();
        }
    }

    C res_flow(const std::vector <size_t> & path) {
        C result = 0;
        for (auto id : path) {
            result = result > 0 ? std::min(result, edges_[id].res_capacity()) : edges_[id].res_capacity();
        }
        return result;
    }

    C flow(const std::vector <size_t> & path) {
        C result = 0;
        for (auto id : path) {
            result = result > 0 ? std::min(result, edges_[id].flow()) : edges_[id].flow();
        }
        return result;
    }

    void push_flow(const std::vector <size_t>& path, const C& delta) {
        for (auto id : path) {
            edges_[id].change_flow(delta);
            size_t rev = edges_[id].rev_edge();
            edges_[rev].change_flow(-delta);
        }
    }

    void init_potential(size_t s) {
        ford_bellman(s, potential_);
    }

    void update_potentital(size_t s) {
        std::vector <W> dist;
        std::vector <size_t> parent;
        dijkstra(s, dist, parent);
        for (size_t i = 0; i < potential_.size(); ++i) {
            potential_[i] = std::min(potential_[i] + dist[i], infinity);
        }
    }

    bool get_shortest_path(size_t s, size_t t, std::vector <size_t>& path) {
        std::vector <W> dist;
        std::vector <size_t> parent;
        dijkstra(s, dist, parent);

        if (dist[t] == infinity) return false;

        path.clear();
        size_t v = t;

        while (v != s) {
            path.push_back(parent[v]);
            v = edges_[parent[v]].v();
        }

        std::reverse(path.begin(), path.end());

        return true;
    }

public:
    TransportNet(size_t n, const W& inf = 1e9) :
            infinity(inf)
    {
        vertex_.resize(n);
        potential_.resize(n);
    }

    void add_edge(size_t id, size_t v, size_t u,  const W& weight, const C& capacity) {
        size_t k = edges_.size();

        edges_.emplace_back(id, v, u, weight, capacity);
        vertex_[v].push_back(k);
        edges_[k].rev_edge() = k+1;

        ++k;

        edges_.emplace_back(id, u, v, -weight, C(0));
        vertex_[u].push_back(k);
        edges_[k].rev_edge() = k-1;
    }

    std::vector <Edge <C, W> >& all_edges() {
        return edges_;
    }

    size_t n () const {
        return vertex_.size();
    }

    size_t m() const {
        return edges_.size();
    }

    bool min_cost_flow(size_t s, size_t t, C need_flow) {
        clear_flow();
        C current_flow = 0;

        init_potential(s);

        std::vector <size_t> path;
        while(need_flow > current_flow && get_shortest_path(s, t, path)) {
            C delta = std::min(res_flow(path), need_flow - current_flow);
            push_flow(path, delta);

            current_flow += delta;
            update_potentital(s);
        }

        return current_flow == need_flow;
    }

    W flow_cost() const {
        W result = 0;
        for (auto e : edges_) {
            result += e.flow() * e.weight();
        }
        return result / 2;
    }

    int simple_decomposition(size_t s, size_t t, std::vector <size_t> & path) {
        std::deque <size_t> q;
        std::vector<bool> used(vertex_.size());
        size_t v = s;
        while (!used[v] && v != t) {
            size_t id = m() + 1;
            for (auto e : vertex_[v]) {
                if (edges_[e].flow() > 0) {
                    id = e;
                    break;
                }
            }

            if (id > m()) {
                return -1;
            }

            q.push_back(id);
            used[v] = true;
            v = edges_[id].to();
        }

        if (used[v]) {
            while (edges_[q.front()].v() != v) {
                q.pop_front();
            }
            path.assign(q.begin(), q.end());
            return 0;
        }

        path.assign(q.begin(), q.end());
        return 1;
    }

    void paths_decomposition(size_t s, size_t t,
                            std::vector <std::vector <size_t> > & paths) {

        std::vector <size_t> path;
        int res = simple_decomposition(s, t, path);

        while (res != -1) {
            C delta = -flow(path);
            push_flow(path, delta);
            if (res == 1) {
                paths.push_back(path);
            }
            res = simple_decomposition(s, t, path);
        }
    }
};

int main() {
    size_t n, m, k;
    std::cin >> n >> m >> k;

    TransportNet<long long, long long> g(n + 1, 1e18);
    for (size_t i = 0; i < m; ++i) {
        size_t v, u;
        long long w;
        std::cin >> v >> u >> w;

        g.add_edge(i+1, v, u, w, 1);
        g.add_edge(i+1, u, v, w, 1);
    }

    if (g.min_cost_flow(1, n, k))
    {
        std::cout << std::setprecision(20) << static_cast<double>(g.flow_cost())/k << '\n';
        std::vector <std::vector <size_t> > paths;

        g.paths_decomposition(1, n, paths);
        for (auto& path : paths) {
            std::cout << path.size() << " ";
            for (auto& road : path) {
                std::cout << g.all_edges()[road].get_id() << " ";
            }
            std::cout << '\n';
        }
    } else {
        std::cout << "-1\n";
    }
}
