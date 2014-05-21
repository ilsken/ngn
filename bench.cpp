#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cstring>
#include <cassert>

typedef std::vector<int> vector_type;

void test_memcpy(vector_type & destv, vector_type const & srcv)
{
    vector_type::pointer       const dest = destv.data();
    vector_type::const_pointer const src  = srcv.data();

    std::memcpy(dest, src, srcv.size() * sizeof(vector_type::value_type));
}

void test_memmove(vector_type & destv, vector_type const & srcv)
{
    vector_type::pointer       const dest = destv.data();
    vector_type::const_pointer const src  = srcv.data();

    std::memmove(dest, src, srcv.size() * sizeof(vector_type::value_type));
}

void test_std_copy(vector_type & dest, vector_type const & src)
{
    std::copy(src.begin(), src.end(), dest.begin());
}

void test_assignment(vector_type & dest, vector_type const & src)
{
    dest = src;
}

auto
benchmark(std::function<void(vector_type &, vector_type const &)> copy_func)
    ->decltype(std::chrono::milliseconds().count())
{
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<vector_type::value_type> distribution;

    static vector_type::size_type const num_elems = 2000;

    vector_type dest(num_elems);
    vector_type src(num_elems);

    // Fill the source and destination vectors with random data.
    for (vector_type::size_type i = 0; i < num_elems; ++i) {
        src.push_back(distribution(generator));
        dest.push_back(distribution(generator));
    }

    static int const iterations = 50000;

    std::chrono::time_point<std::chrono::system_clock> start, end;

    start = std::chrono::system_clock::now();

    for (int i = 0; i != iterations; ++i)
        copy_func(dest, src);

    end = std::chrono::system_clock::now();

    assert(src == dest);

    return
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start).count();
}

int main()
{
    std::cout
        << "memcpy:     " << benchmark(test_memcpy)     << " ms" << std::endl
        << "memmove:    " << benchmark(test_memmove)    << " ms" << std::endl
        << "std::copy:  " << benchmark(test_std_copy)   << " ms" << std::endl
        << "assignment: " << benchmark(test_assignment) << " ms" << std::endl
        << std::endl;
}
