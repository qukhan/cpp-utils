#ifndef _UTIL_ZIP_HPP_
#define _UTIL_ZIP_HPP_

/* A zip implementation  for use in ranged for loops introduced in
 * C++11.
 *
 * usage:
 *     std::array<int, 10> int_array;         // Assume the containers have been
 *     std::vector<std::string> string_array; // initialized and filled
 *     for(auto it : zip(int_array, string_array)) {
 *         int x;
 *         std::string s;
 *         std::tie(x, s) = *it;
 *
 *         int& x_bis = std::get<O>(*it);
 *         // ... anything with x and s
 *     }
 */

#include <tuple>
#include <type_traits>

namespace util {


    /* This structure creates a parameter pack type for a late use.
     * The type is accessible through the #type attribute.
     */

    // Dummy structure to define the template squeletton
    template<template<class...> class pack_template, class A, class actual_pack>
    struct tuple_concat_left {
    };
    // The actual specialization that is used
    template<template<class...> class pack_template, class A, class... pack_content>
    struct tuple_concat_left<pack_template, A, pack_template<pack_content...> > {
        using type = pack_template<A, pack_content...>;
    };
    /**********************************************************/


    /* This structure creates a std::tuple of iterator types from a variadic
     * template parameter pack.
     */
    // Recursive definition : while you have a non-empty list, prepend the new
    // iterator type to the iterator list.
    template<template<class> class type_extractor, class A, class... B>
    struct type_extractor_tuple {
        using type = typename tuple_concat_left<
            std::tuple,
            typename type_extractor<A>::type,
            typename type_extractor_tuple<type_extractor, B...>::type
            >::type;
    };
    // Recursion end
    template<template<class> class type_extractor, class A>
    struct type_extractor_tuple<type_extractor, A> {
        using type = std::tuple<typename type_extractor<A>::type>;
    };
    /**********************************************************/

    /// Get the iterator from a class
    template<class A>
    struct iterator_extractor {
        using type = typename std::conditional<
            std::is_const<
                typename std::remove_reference<A>::type
                >::value,
            typename std::remove_reference<A>::type::const_iterator,
            typename std::remove_reference<A>::type::iterator
            >::type;
    };

    template<class... A>
    using iterator_type_tuple = type_extractor_tuple<iterator_extractor, A...>;
    /**********************************************************/

    /// Get the reference from a class
    template<class A>
    struct reference_extractor {
        using type = typename std::conditional<
            std::is_const<typename std::remove_reference<A>::type>::value,
            typename std::remove_reference<A>::type::const_reference,
            typename std::remove_reference<A>::type::reference
            >::type;
    };

    template<class... A>
    using reference_type_tuple = type_extractor_tuple<reference_extractor, A...>;
    /**********************************************************/


    /** Tuple zipped lists iterator
     *
     * This iterator is meant to be used in C++(11) for-ranged loops. It combines
     * several iterators in one.
     *
     * \tparam Args... A list of iterable classes. These classes must define the following types:
     *   - iterator
     *   - reference
     * The classe iterators must also define the following members:
     *   - default constuctor
     *   - operator=(const it& other)
     *   - operator++() //prefix
     *   - operator*()  //dereference
     *   - operator!=(const it& other)
     */
    template<class... Args>
    class zip_iterator: public iterator_type_tuple<Args...>::type {

    public:
        using reference_tuple = typename reference_type_tuple<Args...>::type;
        constexpr static std::size_t len = sizeof...(Args);

        /// Prefix increment operator, moves the iterator to the next objects
        zip_iterator& operator++() {
            return (*this += 1);
        }

        zip_iterator& operator--() {
            operator--<0,Args...>();
            return *this;
        }

        zip_iterator& operator+=(const std::size_t& n) {
            operator+=<0, Args...>(n);
            return *this;
        }

        zip_iterator& operator-=(const std::size_t& n) {
            for(std::size_t i = 0; i < n; ++i) {
                --(*this);
            }
            return *this;
        }

        friend zip_iterator operator-(zip_iterator it, const std::size_t& n) {
            return (it -= n);
        }

        friend zip_iterator operator+(zip_iterator it, const std::size_t& n) {
            return (it += n);
        }

        /// Dereference operator, gets the pointed objects
        reference_tuple operator*() {
            return this->operator*<0, Args...>();
        }

        bool operator==(const zip_iterator<Args...> & other) const {
            return this->operator==<0, Args...>(other);
        }

        /// Difference test operator
        bool operator!=(const zip_iterator<Args...> & other) const {
            return ! this->operator==<0,Args...>(other);
        }

    private:

        template<int Idx, class A, class... B>
        void operator+=(const std::size_t& n) {
            operator+=<Idx+1,B...>(n);
            std::get<Idx>(*this) = std::get<Idx>(*this) + n;
        }

        template<int>
        void operator+=(const std::size_t&) {}

        template<int Idx, class A, class... B>
        void operator--() {
            operator--<Idx+1,B...>();
            --std::get<Idx>(*this);
        }

        template<int>
        void operator--() {}

        template<int, class A, class... B>
        typename reference_type_tuple<A, B...>::type operator*() {
            std::tuple<typename reference_extractor<A>::type> temp(*(std::get<len - sizeof...(B)-1>(*this)));
            return std::tuple_cat(temp, this->operator*<1, B...>());
        }

        template<int>
        std::tuple<> operator*() const {
            return std::tuple<>();
        }

        template<int Idx, class A, class... B>
        bool operator!=(const zip_iterator<Args...> & other) const {
            return std::get<Idx>(*this) != std::get<Idx>(other)
                // zip_iterators are different if all their iterators are different
                && this->operator!=<Idx+1,B...>(other);
        }

        template<int>
        bool operator!=(const zip_iterator<Args...> & ) const {
            return true;
        }

        template<int Idx, class A, class... B>
        bool operator==(const zip_iterator<Args...> & other) const {
            return std::get<Idx>(*this) == std::get<Idx>(other)
                // zip_iterators are equal if one pair of their iterators is equal
                || this->operator==<Idx+1,B...>(other);
        }

        template<int>
        bool operator==(const zip_iterator<Args...> & ) const {
            return false;
        }

    };


    /** Dummy container for ranged loop iteration
     *
     * This class wraps several containers and offers the required methods to be
     * used with the ranged for loop syntax introduced by C++11.
     *
     * \tparam Args... The types of the wrapped containers.
     */
    template <class... Args>
    struct zip_impl {
        /// The zip iterator type
        using iterator = zip_iterator<Args...>;
        /// The number of containers wrapped inside
        constexpr static std::size_t len = sizeof...(Args);
        /// An array of references to the actual containers
        std::tuple<Args&&...> containers;

        /*zip_impl() {}*/

        zip_impl(Args&&... args) : containers(std::forward<Args>(args)...) {
        }

        iterator begin() {
            iterator begin_it;
            begin<1,Args...>(begin_it);
            return begin_it;
        }

        template<int,class A, class... B>
        void begin(iterator& begin_it) {
            begin<1,B...>(begin_it);
            std::get<len - sizeof...(B)-1>(begin_it) = std::get<len - sizeof...(B)-1>(containers).begin();
        }

        template<int>
        void begin(iterator&) {}

        iterator end()  {
            iterator end_it;
            end<1,Args...>(end_it);
            return end_it;
        }

        template<int,class A, class... B>
        void end(iterator& end_it) {
            end<1,B...>(end_it);
            std::get<len - sizeof...(B)-1>(end_it) = std::get<len - sizeof...(B)-1>(containers).end();
        }

        template<int>
        void end(iterator&) {}
    };

    /* A zip wrapper function for ranged iteration.
     *
     * This function returns a dummy container that represents all of its arguments.
     *
     * usage:
     *     std::array<int, 10> int_array;         // Assume the containers have been
     *     std::vector<std::string> string_array; // initialized and filled
     *     for(auto it : zip(int_array, string_array)) {
     *         int x;
     *         std::string s;
     *         std::tie(x, s) = *it;
     *
     *         int& x_bis = std::get<O>(*it);
     *         // ... anything with x and s
     *     }
     */
    template<typename... T>
    auto zip(T&&... containers) -> zip_impl<T...> {
        return zip_impl<T...>(std::forward<T>(containers)...);
    }

}




#endif
