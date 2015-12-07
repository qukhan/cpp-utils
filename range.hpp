#ifndef _UTIL_RANGE_HPP_
#define _UTIL_RANGE_HPP_

/* A range dummy container used to create integer iterators
 *
 * usage:
 *    for(int i : range(10))
 *        std::cout << i << std::endl;
 *    for(char c : iter_range(std::string("Hello")))
 *        std::cout << c << std::endl;
 */

#include <cstddef>
#include <iterator>

namespace util {
    /* A dummy container to create integer iterators */
    template<typename IntegerType = std::size_t>
    class Range {
        IntegerType _start;
        IntegerType _end;
        IntegerType _pace;
    public:

        /* Integer iterator */
        class Iterator {
        protected:
            IntegerType _val = 0;
            IntegerType _pace = 1;
        public:
            Iterator() {}
            Iterator(IntegerType val, IntegerType pace = 1) : _val(val), _pace(pace) {}
            IntegerType& operator*() {return _val;}
            IntegerType operator*() const {return this->_val;}
            Iterator& operator+=(const std::size_t n)    {_val += n*_pace; return *this;}
            Iterator& operator++()    {return (*this) += 1;}
            Iterator  operator++(int) {Iterator tmp = *this; ++(*this); return tmp;}
            friend Iterator  operator+(Iterator lhs, const std::size_t& n) { return lhs += n;}
            Iterator& operator-=(const std::size_t n)    {_val -= n*_pace; return *this;}
            Iterator& operator--()    {return *this -= 1;}
            Iterator  operator--(int) {Iterator tmp = *this; --(*this); return tmp;}
            friend Iterator  operator-(Iterator lhs, const std::size_t& n) { return lhs -= n;}
            bool operator==(const Iterator& o) const {return _val == o._val;}
            bool operator!=(const Iterator& o) const {return ! (*this == o);}
            bool operator<(const Iterator& o) const {return _val < o._val;}
            bool operator>(const Iterator& o) const {return o < *this;}
            bool operator>=(const Iterator& o) const {return ! (*this < o);}
            bool operator<=(const Iterator& o) const {return ! (o < *this);}
        };

        class Const_Iterator : public Iterator {
        public:
            using Iterator::Iterator;
            const IntegerType& operator*() const {return this->val;}
        };

        /// Integer type used to generate the range
        using value_type = IntegerType;
        /// Iterator type
        using iterator = Iterator;
        /// Const iterator type
        using const_iterator = Const_Iterator;
        // Reference to pointed type
        using reference = IntegerType&;
        // Const reference to pointed type
        using const_reference = const IntegerType&;

        Range(const IntegerType& start, const IntegerType& end, const IntegerType& pace = 1)
            : _start(start), _end(end), _pace(pace){}

        template<class T, class iterator = typename T::iterator>
        Range(const T& iterable) :  Range(0, std::distance(iterable.begin(), iterable.end()), 1) {}

        iterator begin() {
            return Iterator{_start, _pace};
        }

        const_iterator begin() const {
            return Iterator{_start, _pace};
        }

        iterator end() {
            return Iterator{_end, _pace};
        }

        const_iterator end() const {
            return Iterator{_end, _pace};
        }

        iterator rbegin() {
            return --end();
        }

        iterator rend() {
            return --begin();
        }
    };

    template <typename IntegerType = std::size_t>
    Range<IntegerType> range(IntegerType start, IntegerType end, IntegerType pace = 1) {
        return Range<IntegerType>(start, end, pace);
    }

    template <typename IntegerType = std::size_t>
    Range<IntegerType> range(IntegerType end) {
        return Range<IntegerType>(0, end, 1);
    }

    template <class iterable, typename IntegerType = std::size_t>
    Range<IntegerType> iter_range(const iterable& it) {
        return Range<IntegerType>(it);
    }
}
#endif
