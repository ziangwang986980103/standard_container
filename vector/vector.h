//This is the vector for cpp 20
//Since cpp 20, all member functions of vector are constexpr

//to include the std::allocator
#pragma once
#include <memory>
#include <bit>
#include <algorithm>
#include <limits>
#include <iterator>
#include <compare>

namespace std{
    template <class T, class Allocator = std::allocator<T>>
    class vector{
        static_assert(std::is_same_v<typename std::allocator_traits<Allocator>::value_type, T>,
                  "Allocator must have the same value_type as vector after rebinding");
    public:
        //type alias
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        //rebind the allocator for T. For example, if the allocator is of type double and T is int,
        //we can rebind it back to T.
        using rebound_alloc_type = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
        using pointer = typename std::allocator_traits<rebound_alloc_type>::pointer;
        using const_pointer = typename std::allocator_traits<rebound_alloc_type>::const_pointer;
        
        template <typename Iterator>
        class normal_iterator{
        public:
            using trait_type = std::iterator_traits<Iterator>;
            using iterator_category = std::random_access_iterator_tag;
            using value_type = typename trait_type::value_type;
            using difference_type = typename trait_type::difference_type;
            using reference = typename trait_type::reference;
            using pointer = typename trait_type::pointer; 

            constexpr normal_iterator() noexcept: m_pointer() {}

            constexpr explicit normal_iterator(pointer p) noexcept: m_pointer(p){}

            //enable from iterator to const iterator
            template<typename Iter> 
                requires std::is_convertible_v<Iter, Iterator>
            constexpr normal_iterator(const normal_iterator<Iter>& i)noexcept : m_pointer(i.base()) {}

            //access
            constexpr reference operator*() const noexcept{
                return *m_pointer;
            }

            constexpr pointer operator->() const noexcept{
                return m_pointer;
            }

            [[nodiscard]] constexpr reference operator[](difference_type n) const noexcept{
                return *(m_pointer+n);
            }

            //++it
            constexpr normal_iterator& operator++() noexcept{
                ++m_pointer;
                return *this;
            }

            //it++
            constexpr normal_iterator operator++(int) noexcept{
                return normal_iterator(m_pointer++);
            }

            //--it
            constexpr normal_iterator& operator--() noexcept{
                --m_pointer;
                return *this;
            }

            //it--
            constexpr normal_iterator operator--(int) noexcept{
                return normal_iterator(m_pointer--);
            }

            constexpr normal_iterator& operator+=(difference_type t) noexcept{
                m_pointer += t;
                return *this;
            }

            constexpr normal_iterator& operator-=(difference_type t) noexcept{
                m_pointer -= t;
                return *this;
            }

            constexpr normal_iterator operator+(difference_type t) const noexcept{
                return normal_iterator(m_pointer+t);
            }

            constexpr normal_iterator operator-(difference_type t) const noexcept{
                return normal_iterator(m_pointer-t);
            }

            constexpr Iterator base() const noexcept {
                return m_pointer;
            }

        private:
            friend constexpr bool operator==(const normal_iterator& lhs, const normal_iterator& rhs) noexcept{
                return lhs.m_pointer == rhs.m_pointer;
            }

            friend constexpr bool operator!=(const normal_iterator& lhs, const normal_iterator& rhs) noexcept{
                return lhs.m_pointer != rhs.m_pointer;
            }

            friend constexpr bool operator>(const normal_iterator& lhs, const normal_iterator& rhs) noexcept{
                return lhs.m_pointer > rhs.m_pointer;
            }

            friend constexpr bool operator<(const normal_iterator& lhs, const normal_iterator& rhs) noexcept{
                return lhs.m_pointer < rhs.m_pointer;
            }

            friend constexpr bool operator>=(const normal_iterator& lhs, const normal_iterator& rhs) noexcept{
                return lhs.m_pointer >= rhs.m_pointer;
            }
            
            friend constexpr bool operator<=(const normal_iterator& lhs, const normal_iterator& rhs) noexcept{
                return lhs.m_pointer <= rhs.m_pointer;
            }

            friend constexpr difference_type operator-(const normal_iterator& lhs, const normal_iterator& rhs) noexcept{
                return lhs.m_pointer - rhs.m_pointer;
            }

            friend constexpr normal_iterator operator+(difference_type n, const normal_iterator& i) noexcept{
                return normal_iterator(i.m_pointer+n);
            }

            //supports the comparison between iterator and const_iterator
            template<typename OtherIter>
            friend constexpr bool operator==(const OtherIter& lhs, const normal_iterator& rhs) noexcept{
                return lhs.base() == rhs.base();
            }

            template<typename OtherIter>
            friend constexpr auto operator<=>(const normal_iterator<OtherIter>& lhs, const normal_iterator& rhs) noexcept
                requires std::three_way_comparable_with<OtherIter, Iterator, std::partial_ordering>
            {
                return lhs.base() <=> rhs.base();
            }

            //support the operator- between iterator and const_iterator
            template<typename OtherIterator>
            friend constexpr difference_type operator-(const normal_iterator& lhs, const normal_iterator<OtherIterator>& rhs) noexcept {
                return lhs.m_pointer - rhs.base();
            }

            Iterator m_pointer{};
        };

        using iterator = normal_iterator<pointer>;
        using const_iterator = normal_iterator<const_pointer>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        //Constructor
        constexpr vector() noexcept(noexcept(Allocator())) requires std::is_default_constructible_v<Allocator>: vector(Allocator()){}

        //Note: usually if the constructor only takes one argument/only the first argument not having
        //the default value, we will set it as explicit to avoid implict conversion and construction
        constexpr explicit vector(const Allocator& alloc) noexcept : rebound_alloc(alloc), m_start(nullptr), m_finish(nullptr), m_end_of_storage(nullptr){};


        constexpr explicit vector(size_type count, const Allocator& alloc = Allocator()): rebound_alloc(alloc), m_start(nullptr), m_finish(nullptr), m_end_of_storage(nullptr){
            if(count == 0) return;

            if (count > max_size()) {
                throw std::length_error("vector::vector: count exceeds max_size()");
            }

            grow(count);
            try{
                for(size_type i = 0; i < count; ++i){
                    // use std::to_address to convert any pointer(including fancy pointer) to raw pointer
                    std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish));
                    m_finish++;
                }
            }
            catch(...){
                destroy_and_deallocate(m_start, m_finish, count);
                m_start = m_finish = m_end_of_storage = nullptr;
                throw;
            }
        }

        constexpr vector(size_type count, const T& value, const Allocator& alloc = Allocator()): rebound_alloc(alloc), m_start(nullptr), m_finish(nullptr), m_end_of_storage(nullptr){
            if(count == 0) return;

            if (count > max_size()) {
                throw std::length_error("vector::vector: count exceeds max_size()");
            }

            grow(count);
            try{
                for(size_type i = 0; i < count; ++i){
                    std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish), value);
                    m_finish++;
                }
            }
            catch(...){
                destroy_and_deallocate(m_start, m_finish, count);
                m_start = m_finish = m_end_of_storage = nullptr;
                throw;
            }
        }

        template<class InputIt>
            requires std::input_iterator<InputIt>
        constexpr vector(InputIt first, InputIt last, const Allocator& alloc = Allocator()): rebound_alloc(alloc), m_start(nullptr), m_finish(nullptr), m_end_of_storage(nullptr){
            range_initialize(first,last);
        }

        constexpr vector(std::initializer_list<T> init, const Allocator& alloc = Allocator()): rebound_alloc(alloc), m_start(nullptr), m_finish(nullptr), m_end_of_storage(nullptr){
            range_initialize(init.begin(),init.end());
        }

        //Copy Constructor
        constexpr vector(const vector& other): vector(other.m_start, other.m_finish, std::allocator_traits<rebound_alloc_type>::select_on_container_copy_construction(other.rebound_alloc)){}

        constexpr vector(const vector& other, const Allocator& alloc): vector(other.m_start, other.m_finish, alloc){}

        //Move Constructor
        //Note: noexcept here so that we can use it in the resizing function. if it's not noexcept,
        //the cpp will ignore the move constructor and use copy consturctor in the resizing
        //for strong exception safety.
        constexpr vector(vector&& other) noexcept: rebound_alloc(std::move(other.get_allocator())), m_start(other.m_start), m_finish(other.m_finish), m_end_of_storage(other.m_end_of_storage){
            other.m_start = other.m_finish = other.m_end_of_storage = nullptr;
        }

        // Move consturctor. 
        // Can accept an allocator AllocArg of different type from Allocator but Allocator must
        // be constructible from AllocArg.
        // Conditional noexcept if the Allocator instances are all equal(eg. std::allocator<int>)
        // because this means it can steals the pointer directly.
        template<typename AllocArg> requires std::constructible_from<Allocator, const AllocArg&>
        constexpr vector(vector&& other, const AllocArg& alloc) noexcept(std::allocator_traits<Allocator>::is_always_equal::value): rebound_alloc(alloc), m_start(nullptr), m_finish(nullptr), m_end_of_storage(nullptr){
            if(rebound_alloc == other.rebound_alloc){
                m_start = other.m_start;
                m_finish = other.m_finish;
                m_end_of_storage = other.m_end_of_storage;

                other.m_start = other.m_finish = other.m_end_of_storage = nullptr;
            }
            //if the allocator is different, we need to construct new memory and move the elemtns from other to this vector.
            // we can't steal pointer if the allocator is different because only the other.rebound_alloc can deallocate the memory
            // it allocates
            else{
                size_type new_cap = other.capacity();
                if(new_cap == 0) return;

                grow(new_cap);
                try{
                    for(auto it = other.m_start; it != other.m_finish; ++it){
                        std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish), std::move_if_noexcept(*it));
                        m_finish++;
                    }
                }
                catch(...){
                    destroy_and_deallocate(m_start, m_finish, new_cap);
                    m_start = m_finish = m_end_of_storage = nullptr;
                    throw;
                }
            }
        }

        //Destructor
        constexpr ~vector(){
            destroy_and_deallocate(m_start, m_finish, capacity());
        }

        //Copy assignment operator
        constexpr vector& operator=(const vector& other){
            if(this != &other){
                if constexpr (std::allocator_traits<rebound_alloc_type>::propagate_on_container_copy_assignment::value) {
                    // propagate allocator
                    if (rebound_alloc != other.rebound_alloc) {
                        // free current storage with current allocator first
                        destroy_and_deallocate(m_start,m_finish,capacity());
                    }
                    rebound_alloc = other.rebound_alloc;
                }
                const size_type n = other.size();
                if(n <= capacity()){
                    size_type i = 0;
                    size_type cur = size();

                    for (; i < cur && i < n; ++i)
                        m_start[i] = other.m_start[i];

                    // construct new elements if other is larger
                    for (; i < n; ++i) {
                        std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc,std::to_address(m_start + i), other.m_start[i]);
                    }

                    // destroy surplus elements if this is larger
                    for (size_type j = n; j < cur; ++j) {
                        std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc,std::to_address(m_start + j));
                    }

                    m_finish = m_start + n;
                }
                else{
                    vector temp(other);
                    swap(temp);
                }
            }
            return *this;
        }

        //Move assignment operator
        //Note: noexcept is conditional on the allocator's type
        constexpr vector& operator=(vector&& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value
        || std::allocator_traits<Allocator>::is_always_equal::value){
            if(this != &other){
                if constexpr(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value){
                    destroy_and_deallocate(m_start,m_finish,capacity());
                    //if the allocator can be moved, we will move the other's allocator to this allocator.
                    //for the other case, this allocator will work so we do nothing.
                    if constexpr(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value){
                        rebound_alloc = std::move(other.rebound_alloc);
                    }
                    m_start = other.m_start;
                    m_finish = other.m_finish;
                    m_end_of_storage = other.m_end_of_storage;
                    other.m_start = other.m_finish = other.m_end_of_storage = nullptr;
                }
                else{
                    if(rebound_alloc == other.rebound_alloc){
                        destroy_and_deallocate(m_start,m_finish,capacity());
                        m_start = other.m_start;
                        m_finish = other.m_finish;
                        m_end_of_storage = other.m_end_of_storage;
                        other.m_start = other.m_finish = other.m_end_of_storage = nullptr;
                    }
                    else{
                        //different allocators can't propogate, have to move elements
                        size_type this_size = size();
                        size_type other_size = other.size();
                        //reserve first for better exception safety
                        if(other_size > this_size){
                            reserve(other_size);
                        }
                        size_type common = std::min(this_size, other_size);
                        for(size_type i = 0; i <  common; ++i){
                            m_start[i] = std::move(other.m_start[i]);
                        }
                        if(this_size > other_size){
                            for(pointer it = m_start+other_size; it != m_finish; ++it){
                                std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(it));
                            }
                        }
                        else if(this_size < other_size){
                            for(size_type i = this_size; i < other_size; ++i){
                                std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_start+i), std::move_if_noexcept(other.m_start[i]));
                            }
                        }
                        m_finish = m_start+other_size;
                    }
                }
            }
            return *this;
        }

        constexpr vector& operator=(std::initializer_list<value_type> ilist){
            assign(ilist);
            return *this;
        }

        //assign
        constexpr void assign(size_type count, const T& value){
            if(count > capacity()) reserve(count);
            size_type this_size = size();
            size_type common = std::min(count,this_size);
            std::fill_n(m_start, common, value);
            if(count > this_size){
                for(size_type i = this_size; i < count; ++i){
                    std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish), value);
                    m_finish++;
                }
            }
            else if(count < this_size){
                for(pointer it = m_start+count; it != m_finish; ++it){
                    std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(it));
                }
                m_finish = m_start+count;
            }
        }

        template <class InputIt> requires std::input_iterator<InputIt>
        constexpr void assign(InputIt first, InputIt last){
            range_assign(first,last);
        }

        constexpr void assign(std::initializer_list<T> ilist){
            assign(ilist.begin(), ilist.end());
        }

        
        //get allocator
        [[nodiscard]] constexpr allocator_type get_allocator() const noexcept{
            return allocator_type(rebound_alloc);
        }

        //at
        [[nodiscard]] constexpr reference at(size_type pos){
            if (pos >= size()) {
                if (std::is_constant_evaluated()) {
                    throw 0;
                }
                throw std::out_of_range("vector::at");
            }
            return m_start[pos];
        }

        constexpr const_reference at(size_type pos) const{
            if (pos >= size()) {
                if (std::is_constant_evaluated()) {
                    throw 0;
                }
                throw std::out_of_range("vector::at");
            }
            return m_start[pos];
        }

        //operator[]
        [[nodiscard]] constexpr reference operator[](size_type pos){
            if (pos >= size()) {
                if (std::is_constant_evaluated()) {
                    throw 0;
                }
                throw std::out_of_range("vector::[]");
            }
            return m_start[pos];
        }

        [[nodiscard]] constexpr const_reference operator[](size_type pos) const{
            if (pos >= size()) {
                if (std::is_constant_evaluated()) {
                    throw 0;
                }
                throw std::out_of_range("vector::[]");
            }
            return m_start[pos];
        }

        //front
        [[nodiscard]] constexpr reference front(){
            return m_start[0];
        }

        [[nodiscard]] constexpr const_reference front() const{
            return m_start[0];
        }

        //back
        [[nodiscard]] constexpr reference back(){
            return m_start[size()-1];
        }

        [[nodiscard]] constexpr const_reference back() const{
            return m_start[size()-1];
        }

        //data
        [[nodiscard]] constexpr T* data() noexcept{
            // m_start may be a fancy pointer, use std::to_address to return 
            // the raw pointer
            return std::to_address(m_start);
        }

        [[nodiscard]] constexpr const T* data() const noexcept{
            return std::to_address(m_start);
        }

        //begin
        [[nodiscard]] constexpr iterator begin() noexcept{
            return iterator(m_start);
        }

        [[nodiscard]] constexpr const_iterator begin() const noexcept{
            return const_iterator(m_start);
        }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept{
            return const_iterator(m_start);
        }

        //end
        [[nodiscard]] constexpr iterator end() noexcept{
            return iterator(m_finish);
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept{
            return const_iterator(m_finish);
        }

        [[nodiscard]] constexpr const_iterator cend() const noexcept{
            return end();
        }
        
        //rbegin
        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept{
            return reverse_iterator(iterator(m_finish));
        }

        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept{
            return const_reverse_iterator(const_iterator(m_finish));
        }

        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept{
            return rbegin();
        }

        //rend
        [[nodiscard]] constexpr reverse_iterator rend() noexcept{
            return reverse_iterator(iterator(m_start));
        }

        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept{
            return const_reverse_iterator(const_iterator(m_start));
        }

        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept{
            return rend();
        }

        //empty
        [[nodiscard]] constexpr bool empty() const noexcept{
            return m_finish == m_start;
        }

        //size
        [[nodiscard]] constexpr size_type size() const noexcept{
            return m_finish-m_start;
        }

        //max_size
        [[nodiscard]] constexpr size_type max_size() const noexcept{
            size_type alloc_max = std::allocator_traits<rebound_alloc_type>::max_size(rebound_alloc);
            size_type diff_max = std::numeric_limits<difference_type>::max();
            return std::min(alloc_max, diff_max);
        }

        //reserve, strong exception gaurantee
        constexpr void reserve(size_type new_cap){
            if(new_cap <= capacity()) return;
             if(new_cap > max_size()){
                throw std::length_error("vector::reserve: new_cap exceeds max_size()");
            }
            grow(new_cap);
        }

        //capacity
        [[nodiscard]] constexpr size_type capacity() const noexcept{
            return m_end_of_storage - m_start;
        }

        //shrink to fit
        constexpr void shrink_to_fit(){
            size_type this_size = size();
            if(capacity() > this_size){
                vector temp(rebound_alloc);
                temp.reserve(this_size);
                for(auto it = m_start; it != m_finish; ++it){
                    std::allocator_traits<rebound_alloc_type>::construct(temp.rebound_alloc, std::to_address(temp.m_finish), std::move_if_noexcept(*it));
                    temp.m_finish++;
                }
                swap(temp);
            }
        }

        //clear
        constexpr void clear() noexcept{
            for(pointer it = m_start; it != m_finish; ++it){
                std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(it));
            }
            m_finish = m_start;
        }

        //insert
        constexpr iterator insert(const_iterator pos, const T& value){
            size_type idx = pos - cbegin();
            size_type cap = capacity();
            size_type this_size = size();
            if (this_size == cap){
                realloc_insert(pos,value);
            }
            else if(idx != this_size){
                // T temp = value;
                temp_value temp(this, value);
                std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish), std::move(m_start[this_size-1]));
                std::move_backward(m_start + idx, m_start + this_size - 1, m_start + this_size);
                m_start[idx] = std::move(temp.get());
                m_finish++;
            }
            else{
                std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish), value);
                m_finish++;
            }
            
            return iterator(m_start + idx);
        }

        constexpr iterator insert(const_iterator pos, T&& value){
            difference_type idx = pos - cbegin();
            size_type cap = capacity();
            size_type this_size = size();
            if (this_size == cap){
                // T temp = std::move(value);
                temp_value temp(this, value);
                size_type new_cap = calculate_growth(1);
                grow(new_cap);
                return insert(begin() + idx, std::move(temp.get()));
            } 

            if(idx != this_size){
                std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish), std::move(m_start[this_size-1]));
                std::move_backward(m_start + idx, m_start + this_size - 1, m_start + this_size);
                m_start[idx] = std::move(value);
            }
            else{
                std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish), std::move(value));
            }
            
            m_finish++;
        
            return iterator(m_start + idx);
        }

        constexpr iterator insert(const_iterator pos, size_type count, const T& value){
            if(count == 0) return iterator(std::next(begin(), pos - cbegin()));
            if (count > max_size()) {
                throw std::length_error("vector: range size exceeds max_size()");
            }
            size_type cap = capacity();
            size_type this_size = size();
            size_type start_idx = pos-cbegin();
            if(cap-this_size < count){
                //allocate new memory
                size_type new_cap = calculate_growth(count);
                pointer new_start = std::allocator_traits<rebound_alloc_type>::allocate(rebound_alloc, new_cap);
                pointer new_finish = new_start;
                pointer new_end_of_storage = new_start + new_cap;

                size_type i = start_idx;
                try{
                    for(; i < start_idx+count; ++i){
                        std::allocator_traits<rebound_alloc_type>::construct(
                            rebound_alloc, std::to_address(new_start+i), value
                        );
                    }
                }
                catch(...){
                    for(size_type j = start_idx; j < i; ++j){
                        std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(new_start+j));
                    }
                    std::allocator_traits<rebound_alloc_type>::deallocate(rebound_alloc, new_start, new_cap);
                    throw;
                }

                try{
                    //move the elements before inserted range
                    for(auto it = begin(); it != pos; ++it){
                        std::allocator_traits<rebound_alloc_type>::construct(
                            rebound_alloc, std::to_address(new_finish), std::move_if_noexcept(*it)
                        );
                        ++new_finish;
                    }
                }
                catch(...){
                    for(auto it = new_start; it != new_finish; ++it){
                        std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(it));
                    }
                    for(size_type j = start_idx; j < start_idx+count; ++j){
                        std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(new_start+j));
                    }
                    std::allocator_traits<rebound_alloc_type>::deallocate(rebound_alloc, new_start, new_cap);
                    throw;
                }

                new_finish += count;

                try{
                    //move the elements after the inserted range
                    for(size_type i = start_idx; i < this_size; ++i){
                        std::allocator_traits<rebound_alloc_type>::construct(
                            rebound_alloc, std::to_address(new_finish), std::move_if_noexcept(m_start[i])
                        );
                        ++new_finish;
                    }
                }
                catch(...){
                    destroy_and_deallocate(new_start,new_finish,new_cap);
                    throw;
                }

                //destroy and deallocate the original memory
                destroy_and_deallocate(m_start,m_finish,cap);
                m_start = new_start;
                m_finish = new_finish;
                m_end_of_storage = new_end_of_storage;
            }
            else if(start_idx == this_size){
                try{
                    for(size_type i = 0; i < count; ++i){
                        std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish), value);
                        ++m_finish;
                    }
                }
                catch(...){
                    destroy_and_deallocate(m_start,m_finish, cap);
                    throw;
                }
                
            }
            else{
                // T temp = value;
                temp_value temp(this, value);

                // move back the elements after the inserted pos
                for(size_type i = this_size-1+count; i >= start_idx+count; --i){
                    if(i >= this_size){
                        std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_start+i), std::move(m_start[i-count]));
                    }
                    else{
                        m_start[i] = std::move(m_start[i-count]);
                    }
                }

                //constructed the new elements
                for(size_type i = start_idx; i< start_idx+count; ++i){
                    if(i < this_size){
                        m_start[i] = value;
                    }
                    else{
                        std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_start+i), temp.get());
                    }
                }
                
                m_finish = m_start + this_size + count;
            }

            return iterator(m_start+start_idx);
        }

        template<class InputIt> requires std::input_iterator<InputIt>
        constexpr iterator insert(const_iterator pos, InputIt first, InputIt last){
            size_type count = std::distance(first,last);
            if(count == 0) return iterator(m_start + (pos-begin()));

            size_type idx = pos - cbegin();
            size_type this_size = size();
            if(capacity()-this_size < count){
                size_type new_cap = calculate_growth(count);
                grow(new_cap);
            }

            for(size_type i = this_size-1+count; i >= idx+count; --i){
                if(i >= this_size){
                    std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_start+i), std::move_if_noexcept(m_start[i-count]));
                }
                else{
                    m_start[i] = std::move(m_start[i-count]);
                }
            }

            size_type i = idx;
            for(auto it = first; it != last; ++it,++i){
                if(i < this_size){
                    m_start[i] = *it;
                }
                else{
                    std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_start+i), *it);
                }
            }
            m_finish = m_start+this_size+count;

            return iterator(m_start+idx);
        }

        constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist){
            return insert(pos,ilist.begin(),ilist.end());
        }

        // emplace, conditional strong exception gaurantee
        // quote from cpp standard:
        /*
            If an exception is thrown other than by the copy constructor, move constructor, 
            assignment operator, or move assignment operator of T, or if an exception is 
            thrown while emplace is used to insert a single element at the end and T is either 
            CopyInsertable or nothrow move constructible, there are no effects (strong exception 
            guarantee). Otherwise, the effects are unspecified.
        */
        template<class... Args>
        constexpr iterator emplace(const_iterator pos, Args&&... args){
            size_type idx = std::distance(cbegin(), pos);
            size_type cap = capacity();
            size_type this_size = size();
            if (this_size == cap){
                realloc_insert(pos, std::forward<Args>(args)...);
            } 
            else if(idx == this_size){
                std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_finish), std::forward<Args>(args)...);
                m_finish++;
            }
            else{
                // Emplacing in middle - ALWAYS make temporary to avoid self-reference
                // T temp(std::forward<Args>(args)...);
                temp_value temp(this, std::forward<Args>(args)...);
                
                // Now safe to shift
                std::allocator_traits<rebound_alloc_type>::construct(
                    rebound_alloc, std::to_address(m_finish),
                    std::move_if_noexcept(m_start[this_size - 1]));
                
                std::move_backward(m_start + idx, m_start + this_size - 1, m_start + this_size);
                
                m_start[idx] = std::move(temp.get());
                ++m_finish;
            }
            
            // m_finish++;
        
            return iterator(m_start + idx);
        }

        //erase
        constexpr iterator erase(const_iterator pos){
            size_type idx = pos-begin();
            size_type sz = size();
            for(size_type i = idx+1; i < sz; ++i){
                m_start[i-1] = std::move(m_start[i]);
            }
            std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(m_start + sz - 1));
            m_finish--;
            return iterator(m_start+idx);
        }

        constexpr iterator erase(const_iterator first, const_iterator last){
            if(first == last) return iterator(m_start + (first - cbegin()));
        
            size_type first_idx = first - begin();
            size_type last_idx = last - begin();
            size_type sz = size();
            size_type range = last_idx - first_idx;
            
            // Shift elements left
            for(size_type i = last_idx; i < sz; ++i){
                m_start[first_idx + (i - last_idx)] = std::move(m_start[i]);
            }
            
            // Destroy the elements at the end that are now duplicates
            for(size_type i = sz - range; i < sz; ++i){
                std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(m_start + i));
            }
            
            m_finish -= range;
            return iterator(m_start + first_idx);
        }
        
        //push_back, strong exception gaurantee
        constexpr void push_back(const value_type& value){
            if(m_finish != m_end_of_storage){
                std::allocator_traits<rebound_alloc_type>::construct(
                    rebound_alloc, std::to_address(m_finish), value
                );
                m_finish++;
            }
            else{
                realloc_append(value);
            }
        }


        constexpr void push_back(value_type&& value){
            emplace_back(std::move(value));
        }

        //emplace_back
        template<class... Args>
        constexpr reference emplace_back(Args&&... args){
            if(m_finish != m_end_of_storage){
                std::allocator_traits<rebound_alloc_type>::construct(
                    rebound_alloc, std::to_address(m_finish), std::forward<Args>(args)...
                );
                m_finish++;
            }
            else{
                realloc_append(std::forward<Args>(args)...);
            }
            return back();
        }

        //pop_back
        constexpr void pop_back(){
            if(empty()) throw std::out_of_range("vector::pop_back: empty vector");
            m_finish--;
            std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(m_finish));
        }

        //resize, strong exception gaurantee
        constexpr void resize(size_type count){
            //don't use the overload resize(count, value) to avoid copy of value
            if(count > max_size()){
                throw std::length_error("vector::resize: new_cap exceeds max_size()");
            }
            if(count == size()) return;
            
            // for strong exception gaurantee, we need to follow these steps
            // 1. allocate new memory
            // 2. construct the new elements in the new memory
            // 3. move the old elements to the new memory
            // 4. destroy the old memory
            // if(count > capacity()){
            //     grow(count);
            // }
            size_type sz = size();
            if(count > sz){
                if(count > capacity()){
                    realloc_resize(count);
                    return;
                }
                else{
                    size_type i = sz;
                    try{
                        for(; i < count; ++i){
                            std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_start+i));
                        }
                    }
                    catch(...){
                        // Rollback: destroy successfully constructed elements
                        for(size_type j = sz; j < i; ++j){
                            std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(m_start+j));
                        }
                        throw;  // Rethrow - vector size unchanged (strong guarantee)
                    }
                }
            }
            else{
                for(pointer it = m_start+count; it != m_finish; ++it){
                    std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(it));
                }
            }
            m_finish = m_start+count;
        }

        constexpr void resize(size_type count, const value_type& value){
            if(count > max_size()){
                throw std::length_error("vector::resize: new_cap exceeds max_size()");
            }
            if(count == size()) return;
            size_type sz = size();
            if(count > sz){
                if(count > capacity()){
                    realloc_resize(count,value);
                    return;
                }
                else{
                    size_type i = sz;
                    try{
                        for(; i < count; ++i){
                            std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(m_start+i),value);
                        }
                    }
                    catch(...){
                        // Rollback: destroy successfully constructed elements
                        for(size_type j = sz; j < i; ++j){
                            std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(m_start+j));
                        }
                        throw;  // Rethrow - vector size unchanged (strong guarantee)
                    }
                }
            }
            else{
                for(pointer it = m_start+count; it != m_finish; ++it){
                    std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(it));
                }
            }
            m_finish = m_start+count;
        }

        //swap
        constexpr void swap(vector& other) noexcept{
            std::swap(m_start, other.m_start);
            std::swap(m_finish, other.m_finish);
            std::swap(m_end_of_storage, other.m_end_of_storage);
            if constexpr(std::allocator_traits<Allocator>::propagate_on_container_swap::value){
                std::swap(rebound_alloc, other.rebound_alloc);
            }
        }

        // friend constexpr bool operator==(const vector& lhs, const vector& rhs)
        //     requires std::equality_comparable<T> 
        // {
        //     return (lhs.size() == rhs.size()) && std::equal(lhs.begin(),lhs.end(),rhs.begin());
        // }
        friend constexpr bool operator==(const vector& lhs, const vector& rhs)
            requires std::equality_comparable<T> ||
                    std::three_way_comparable<T> ||
                    requires(const T& a, const T& b) { { a < b } -> std::convertible_to<bool>; }
        {
            if (lhs.size() != rhs.size()) return false;
            
            if constexpr (std::equality_comparable<T>) {
                return std::equal(lhs.begin(), lhs.end(), rhs.begin());
            } else {
                // Synthesize equality from ordering
                for (auto it1 = lhs.begin(), it2 = rhs.begin(); it1 != lhs.end(); ++it1, ++it2) {
                    if constexpr (std::three_way_comparable<T>) {
                        if ((*it1 <=> *it2) != 0) return false;
                    } else {
                        // Use < to check equality: equal if neither is less than the other
                        if ((*it1 < *it2) || (*it2 < *it1)) return false;
                    }
                }
                return true;
            }
        }

        friend constexpr auto operator<=>(const vector& lhs, const vector& rhs)
            requires std::three_way_comparable<T> ||
                     requires(const T& a, const T& b) { { a < b } -> std::convertible_to<bool>; }
        {
            auto synth_three_way = []<class U>(const U& a, const U& b) {
                if constexpr (std::three_way_comparable<U>) {
                    return a <=> b;
                } else {
                    if (a < b) return std::weak_ordering::less;
                    if (b < a) return std::weak_ordering::greater;
                    return std::weak_ordering::equivalent;
                }
            };
            return std::lexicographical_compare_three_way(
                lhs.begin(), lhs.end(),
                rhs.begin(), rhs.end(),
                synth_three_way
            );
        }   


    private:
        template<typename It>
        constexpr void range_initialize(It first, It last) {
            if constexpr(std::forward_iterator<It>){
                //iterators support multipass, can use std::distance

                // 1. Calculate distance (O(1) for random access, O(N) for input iterators)
                size_type count = static_cast<size_type>(std::distance(first, last));
                if (count > max_size()) {
                    throw std::length_error("vector: range size exceeds max_size()");
                }
                // 2. Allocate memory, only initialize to count to save memory
                grow(count);

                // 3. Construct elements with a try-catch for exception safety
                try {
                    for (auto it = first; it != last; ++it) {
                        std::allocator_traits<rebound_alloc_type>::construct(
                            rebound_alloc, std::to_address(m_finish), *it
                        );
                        m_finish++;
                    }
                } catch (...) {
                    // Roll back if an element constructor throws
                    destroy_and_deallocate(m_start, m_finish, count);
                    // m_start = m_finish = m_end_of_storage = nullptr;
                    throw;
                }
            }
            else{
                // iterators doesn't support multipass
                try{
                    while(first != last){
                        // can't use the push_back directly because the constructor of T may be private
                        // push_back will implictly call T's contructor and cause compilation error
                        // realloc_append takes the argument so it's okay
                        if (m_finish == m_end_of_storage) {
                            realloc_append(*first);
                        } else {
                            std::allocator_traits<rebound_alloc_type>::construct(
                                rebound_alloc, std::to_address(m_finish), *first
                            );
                            ++m_finish;
                        }
                        ++first;
                    }
                }
                catch(...){
                    //the push_back only destroy the new allocated memory,
                    //we need to also to destroy and deallocate the already allocated
                    //memory when the constructor fails
                    destroy_and_deallocate(m_start,m_finish,capacity()); 
                    throw;
                }
            }
            
        }

        template <typename It>
        constexpr void range_assign(It first, It last){
            if constexpr(std::forward_iterator<It>){
                size_type count = static_cast<size_type>(std::distance(first, last));
                if (count > max_size()) {
                    throw std::length_error("vector: range size exceeds max_size()");
                }
                size_type cap = capacity();
                if(count > cap){
                    destroy_and_deallocate(m_start,m_end_of_storage,cap);
                    grow(count);
                    try {
                        for (auto it = first; it != last; ++it) {
                            std::allocator_traits<rebound_alloc_type>::construct(
                                rebound_alloc, std::to_address(m_finish), *it
                            );
                            m_finish++;
                        }
                    } catch (...) {
                        // Roll back if an element constructor throws
                        destroy_and_deallocate(m_start, m_finish, count);
                        throw;
                    }
                }
                else{
                    // Enough capacity - assign in place
                    pointer current = m_start;
                    auto it = first;
                    
                    // Copy assign to existing elements
                    while (current != m_finish && it != last) {
                        *current = *it;
                        ++current;
                        ++it;
                    }
                    
                    if (it == last) {
                        // New range is shorter - destroy excess elements
                        for(auto i = current; i != m_finish; ++i){
                            std::allocator_traits<rebound_alloc_type>::destroy(
                                rebound_alloc, std::to_address(i)
                            );
                        }
                        m_finish = current;
                    }
                    else {
                        // New range is longer - construct additional elements
                        m_finish = current;
                        try {
                            for (; it != last; ++it) {
                                std::allocator_traits<rebound_alloc_type>::construct(
                                    rebound_alloc, std::to_address(m_finish), *it
                                );
                                ++m_finish;
                            }
                        } catch (...) {
                            // m_finish points to last successfully constructed element
                            destroy_and_deallocate(m_start,m_finish,cap);
                            throw;
                        }
                    }
                }
            }
            else{
                size_type cap = capacity();
                destroy_and_deallocate(m_start,m_finish,cap);
                try{
                    while(first != last){
                        // can't use the push_back directly because the constructor of T may be private
                        // push_back will implictly call T's contructor and cause compilation error
                        // realloc_append takes the argument so it's okay
                        if (m_finish == m_end_of_storage) {
                            realloc_append(*first);
                        } else {
                            std::allocator_traits<rebound_alloc_type>::construct(
                                rebound_alloc, std::to_address(m_finish), *first
                            );
                            ++m_finish;
                        }
                        ++first;
                    }
                }
                catch(...){
                    //the push_back only destroy the new allocated memory,
                    //we need to also to destroy and deallocate the already allocated
                    //memory when the constructor fails
                    destroy_and_deallocate(m_start,m_finish,capacity()); 
                    throw;
                }
            }
        }

        constexpr void destroy_and_deallocate(pointer start, pointer finish, size_type cap){
            if(!start) return;
            for(auto it = start; it != finish; ++it){
                std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(it));
            }
            std::allocator_traits<rebound_alloc_type>::deallocate(rebound_alloc, start, cap);
            m_start = m_finish = m_end_of_storage = nullptr;
        }

        constexpr void grow(size_type new_cap){
            if(new_cap > max_size()){
                grow(max_size());
            }

            if(new_cap <= capacity()){
                return;
            }

            pointer new_start = std::allocator_traits<rebound_alloc_type>::allocate(rebound_alloc, new_cap);
            pointer new_finish = new_start;
            try{
                for(pointer cur = m_start; cur != m_finish; ++cur){
                    std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(new_finish), std::move_if_noexcept(*cur));
                    ++new_finish;
                }
            }
            catch(...){
                // rollback if exception
                destroy_and_deallocate(new_start, new_finish, new_cap);
                throw;
            }

            //destroy and free the original memory
            destroy_and_deallocate(m_start, m_finish, m_end_of_storage-m_start);

            m_start = new_start;
            m_finish = new_finish;
            m_end_of_storage = new_start + new_cap;
        }

        //used for push_back and emplace_back, strong exception gaurantee
        template<class... Args>
        constexpr void realloc_append(Args&&... args){
            // for strong exception gaurantee, we need to follow these steps
            // 1. allocate new memory
            // 2. construct the new element in the new memory
            // 3. move the old elements to the new memory
            // 4. clean the old memory and reset the pointers
            
            // 1. allocate new memory
            size_type old_size = size();
            size_type new_cap = calculate_growth(1);
            pointer new_start = std::allocator_traits<rebound_alloc_type>::allocate(rebound_alloc, new_cap);
            pointer new_finish = new_start;

            try{
                // 2. construct the new element in the new memory
                std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(new_start+old_size), std::forward<Args>(args)...);
            }
            catch(...){
                std::allocator_traits<rebound_alloc_type>::deallocate(rebound_alloc, new_start, new_cap);
                throw;
            }
            
            try{
                // 3. move the old elements to the new memory
                for(pointer cur = m_start; cur != m_finish; ++cur){
                    std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(new_finish), std::move_if_noexcept(*cur));
                    ++new_finish;
                }
                ++new_finish;
            }
            catch(...){

                std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(new_start+old_size));
                destroy_and_deallocate(new_start,new_finish,new_cap);
                throw;
            }

            // 4. clean the old memory and reset the pointers
            destroy_and_deallocate(m_start, m_finish, capacity());
            m_start = new_start;
            m_finish = new_finish;
            m_end_of_storage = new_start + new_cap;
        }

        //used for resize, strong exception gaurantee
        // input
        // count: the new size
        // args: the arguements used for constructed new elements
        template<class... Args>
        constexpr void realloc_resize(size_type count, Args&&... args){
            size_type old_size = size();
            size_type new_cap = calculate_growth(count-old_size);
            pointer new_start = std::allocator_traits<rebound_alloc_type>::allocate(rebound_alloc, new_cap);
            pointer new_finish = new_start;

            size_type cur = old_size;
            try{
                // 2. construct the new element in the new memory
                for(; cur < count; ++cur){
                    std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(new_start+cur), std::forward<Args>(args)...);
                }
            }
            catch(...){
                for(size_type i = old_size; i < cur; ++i){
                    std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(new_start+i));
                }
                std::allocator_traits<rebound_alloc_type>::deallocate(rebound_alloc, new_start, new_cap);
                throw;
            }

            try{
                // 3. move the old elements to the new memory
                for(pointer it = m_start; it != m_finish; ++it){
                    std::allocator_traits<rebound_alloc_type>::construct(rebound_alloc, std::to_address(new_finish), std::move_if_noexcept(*it));
                    ++new_finish;
                }
                new_finish = new_start+count;
            }
            catch(...){
                for(size_type i = old_size; i < count; ++i){
                    std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(new_start+i));
                }
                destroy_and_deallocate(new_start,new_finish,new_cap);
                throw;
            }

            // 4. clean the old memory and reset the pointers
            destroy_and_deallocate(m_start, m_finish, capacity());
            m_start = new_start;
            m_finish = new_finish;
            m_end_of_storage = new_start + new_cap;
        }
        
        // used for emplace and insert
        template<class... Args>
        constexpr void realloc_insert(const_iterator pos, Args&&... args){
            size_type idx = std::distance(cbegin(), pos);
            size_type this_size = size();
            size_type new_cap = calculate_growth(1);
            pointer old_start = m_start;
            pointer old_finish = m_finish;
            size_type old_cap = capacity();
            
            pointer new_start = std::allocator_traits<rebound_alloc_type>::allocate(rebound_alloc, new_cap);
            pointer new_finish = new_start;
            
            try {
                // Copy elements before insertion point
                for (size_type i = 0; i < idx; ++i, ++new_finish) {
                    std::allocator_traits<rebound_alloc_type>::construct(
                        rebound_alloc, std::to_address(new_finish),
                        std::move_if_noexcept(old_start[i]));
                }
                
                // Construct new element directly (args... is still valid)
                std::allocator_traits<rebound_alloc_type>::construct(
                    rebound_alloc, std::to_address(new_finish),
                    std::forward<Args>(args)...);
                ++new_finish;
                
                // Copy elements after insertion point
                for (size_type i = idx; i < this_size; ++i, ++new_finish) {
                    std::allocator_traits<rebound_alloc_type>::construct(
                        rebound_alloc, std::to_address(new_finish),
                        std::move_if_noexcept(old_start[i]));
                }
                
                destroy_and_deallocate(old_start, old_finish, old_cap);
                m_start = new_start;
                m_finish = new_finish;
                m_end_of_storage = new_start + new_cap;
                
            } catch (...) {
                for (pointer p = new_start; p != new_finish; ++p){
                    std::allocator_traits<rebound_alloc_type>::destroy(rebound_alloc, std::to_address(p));
                }
                std::allocator_traits<rebound_alloc_type>::deallocate(rebound_alloc, new_start, new_cap);
                throw;
            }
        }

        //reference of gcc
        //input: the size of inserted/pushed elements
        //output: the new capacity
        constexpr size_type calculate_growth(size_type count_new_eles) {
            if (max_size() - size() < count_new_eles) throw std::length_error("vector too long");
            const size_type new_cap = size() + (std::max)(size(), count_new_eles);
            return (new_cap < size() || new_cap > max_size()) ? max_size() : new_cap;
        }

        //a value type object constructed with std::allocator_traits<rebound_alloc>::construct and 
        //std::allocator_traits<rebound_alloc>::destroy
        //copied and edited from libstdc++
        struct temp_value {
            vector* v;
            pointer p;

            template<class... Args>
            constexpr explicit temp_value(vector* v_, Args&&... args) : v(v_) {
                p = std::allocator_traits<rebound_alloc_type>::allocate(v->rebound_alloc, 1);
                std::allocator_traits<rebound_alloc_type>::construct(
                v->rebound_alloc, std::to_address(p),
                std::forward<Args>(args)...);
            }

            constexpr ~temp_value() {
                std::allocator_traits<rebound_alloc_type>::destroy(
                v->rebound_alloc, std::to_address(p));
                std::allocator_traits<rebound_alloc_type>::deallocate(
                v->rebound_alloc, p, 1);
            }

            constexpr T& get() noexcept { return *p; }
        };

        rebound_alloc_type rebound_alloc;
        pointer m_start;
        pointer m_finish;
        pointer m_end_of_storage;
    };

    /*
        Free functions, std::erase and std::erase_if
    */
    template< class T, class Alloc, class U >
    constexpr typename std::vector<T, Alloc>::size_type
    erase(vector<T, Alloc>& c, const U& value ){
        auto it = std::remove(c.begin(), c.end(), value);
        auto r = std::distance(it, c.end());
        c.erase(it, c.end());
        return r;
    }

    template< class T, class Alloc, class Pred >
    constexpr typename std::vector<T, Alloc>::size_type
    erase_if(vector<T, Alloc>& c, Pred pred ){
        auto it = std::remove_if(c.begin(), c.end(), pred);
        auto r = std::distance(it, c.end());
        c.erase(it, c.end());
        return r;
    }

    /*
    Type deduction
    */
    template<typename T, typename Allocator, typename AllocArg>
    vector(vector<T, Allocator>&, const AllocArg&) -> vector<T, Allocator>;

    template<typename T, typename Allocator, typename AllocArg>
    vector(vector<T, Allocator>&&, const AllocArg&) -> vector<T, Allocator>;

    template<typename InputIt, typename Alloc>
    vector(InputIt, InputIt, const Alloc&) -> vector<typename std::iterator_traits<InputIt>::value_type, Alloc>;

    template<typename InputIt>
    vector(InputIt, InputIt) -> vector<typename std::iterator_traits<InputIt>::value_type>;
}
