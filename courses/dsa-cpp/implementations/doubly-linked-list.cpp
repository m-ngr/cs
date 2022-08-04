#include<initializer_list>
#include<stdexcept>

template <typename T> class DLList {
	class Node; // private: Implementation Details
public:
	class Iterator;
	using value_type = T;
	using size_type = unsigned;

#pragma region The Class Interface
	DLList() {}

	~DLList() { remove_list(); }

	explicit DLList(size_type number, const value_type& default_value = {}) {
		for (auto i = 0; i < number; ++i) { push_back(default_value); }
	}
	
	DLList(const std::initializer_list<value_type>& init) { push_back(init); }

	DLList(const DLList& list) { push_back(list); }

	DLList(DLList&& list) { move_list(list); }

	DLList& operator=(const DLList& list) {
		remove_list();
		push_back(list);
		return *this;
	}

	DLList& operator=(DLList&& list) {
		move_list(list);
		return *this;
	}

	bool operator==(const DLList& list) {
		if (size() != list.size()) return false;
		for (auto i = begin(), j = list.begin(); i != end() && j != list.end(); ++i, ++j) {
			if (*i != *j) return false;
		}
		return true;
	}

	bool operator!=(const DLList& list) { return !(*this == list); }

#pragma endregion

#pragma region The Standard Interface
	DLList& push_back(const value_type& item) {
		create_node(item, nullptr, tail);
		return *this;
	}

	DLList& push_back(const DLList& list) {
		for (auto& item : list) {
			push_back(item);
		}
		return *this;
	}

	DLList& push_back(const std::initializer_list<value_type>& init_list) {
		for (auto& item : init_list) {
			push_back(item);
		}
		return *this;
	}

	DLList& push_front(const value_type& item) {
		create_node(item, head, nullptr);
		return *this;
	}

	DLList& push_front(const DLList& list) {
		for (auto& item : list) {
			push_front(item);
		}
		return *this;
	}

	DLList& push_front(const std::initializer_list<value_type>& init_list) {
		for (auto& item : init_list) {
			push_front(item);
		}
		return *this;
	}

	value_type pop_back() throw (std::runtime_error) {
		if (empty()) { throw std::runtime_error("Can't pop items from empty list"); }
		return remove_node(tail);
	}

	value_type pop_front() throw (std::runtime_error) {
		if (empty()) { throw std::runtime_error("Can't pop items from empty list"); }
		return remove_node(head);
	}

	DLList& reverse() {
		Node* curr = head;
		while (curr) { 
			std::swap(curr->next, curr->prev);
			curr = curr->prev;
		}
		std::swap(head, tail);
		return *this;
	}

	const value_type& front() const throw(std::runtime_error){
		if (empty()) { throw std::runtime_error("Can't read values from empty list"); }
		return head->value;
	}

	const value_type& back() const throw(std::runtime_error) {
		if (empty()) { throw std::runtime_error("Can't read values from empty list"); }
		return tail->value;
	}

	bool empty()const { return (head == nullptr || tail == nullptr); }

	Iterator begin() { return Iterator(head); }

	Iterator end() { return Iterator(nullptr); }

#pragma endregion

#pragma region The Index Interface

	DLList& insert(size_type index, const value_type& item) {
		if (index == 0) return push_front(item);
		if (index == size()) return push_back(item);
		Node* prev = node_at(index - 1);
		create_node(item, prev->next, prev);
		return *this;
	}

	value_type remove(size_type index) { return remove_node(node_at(index)); }

	size_type size() const { return length; }

	const value_type& at(size_type index) const { return node_at(index)->value; }

	value_type& at(size_type index) { return node_at(index)->value; }

	const value_type& operator[](size_type index) const { return begin()[index].value; }

	value_type& operator[](size_type index) { return begin()[index].value; }

#pragma endregion

#pragma region The Functional Interface

	template<typename F>
	DLList& filter_self(F& predicate) {
		Node* curr = head;
		while (curr) {
			Node* next = curr->next;
			if (!predicate(curr->value)) { remove_node(curr);}
			curr = next;
		}
		return *this;
	}
	
	template<typename F>
	DLList filter(F& predicate) {
		DLList result;
		for (auto item : *this) {
			if (predicate(item)) result.push_back(item);
		}
		return result;
	}

	template <typename T2>
	DLList<T2> map(T2(*transform) (const value_type&)) {
		DLList<T2> result;
		for (auto item : *this) {
			result.push_back(transform(item));
		}
		return result;
	}

	template <typename T2>
	T2 reduce(const T2& init, T2(*accumulate) (const T2&, const value_type&)) {
		T2 result = init;
		for (auto item : *this) {
			result = accumulate(result, item);
		}
		return result;
	}

	template <typename F>
	bool exists(F& predicate) {
		for (auto item : *this) {
			if (predicate(item)) return true;
		}
		return false;
	}

	DLList& remove_all(const value_type& item) {
		auto pred = [=](auto i) {return i != item; };
		return filter_self(pred);
	}
#pragma endregion

	DLList& remove_first(const value_type& value) throw (std::exception){
		if (empty()) throw std::exception("Can't remove a value from empty list");

		for (auto curr = head; curr; curr = curr->next) {
			if (curr->value == value) {
				remove_node(curr);
				break;
			}
		}

		return *this;
	}

	DLList& remove_last(const value_type& value) throw (std::exception) {
		if (empty()) throw std::exception("Can't remove a value from empty list");
		
		for (auto curr = tail; curr; curr = curr->prev) {
			if (curr->value == value) {
				remove_node(curr);
				break;
			}
		}

		return *this;
	}

private:
	Node* head{};
	Node* tail{};
	size_type length{ 0 };

#pragma region Helpers
	void move_list(DLList& list) {
		length = list.length;
		head = list.head;
		tail = list.tail;

		list.length = 0;
		list.head = nullptr;
		list.tail = nullptr;
	}
	
	void remove_list() {
		while (head) {
			Node* tmp = head;
			head = head->next;
			delete tmp;
		}
		length = 0;
		tail = head = nullptr;
	}

	Node* create_node(const value_type& value = {}, Node* next = {}, Node* prev = {}) {
		Node* newNode = new Node{ value, next, prev };
		if (next) next->prev = newNode;
		if (prev) prev->next = newNode;
		if (prev == tail) tail = newNode;
		if (next == head) head = newNode;
		++length;
		return newNode;
	}

	value_type remove_node(Node* target) {
		Node* next = target->next;
		Node* prev = target->prev;
		value_type value = target->value;

		if (target == tail) tail = prev;
		if (target == head) head = next;

		if (next) next->prev = prev;
		if (prev) prev->next = next;

		delete target;
		--length;
		return value;
	}

	Node* node_at(size_type i) const throw(std::out_of_range) {
		if (i >= size()) throw std::out_of_range("Index out of range");
		Node* element = head;
		while (i != 0 && element != nullptr) {
			element = element->next;
			--i;
		}
		return element;
	}

#pragma endregion
};

template <typename T> class DLList<T>::Node {
public:
	Node(const value_type& value = {}, Node* next = {}, Node* prev = {})
		: value{ value }, next{ next }, prev{ prev }{};
	value_type value{};
	Node* next{};
	Node* prev{};
};

template <typename T> class DLList<T>::Iterator {
public:
	Iterator(Node* ptr = {}) : ptr{ ptr } {};

	Iterator& operator++() {
		ptr = ptr->next;
		return *this;
	}

	Iterator& operator--() {
		ptr = ptr->prev;
		return *this;
	}

	Iterator& go_next() throw (std::runtime_error) {
		if (ptr == nullptr) {throw std::runtime_error("Can't get the next of a null node");}
		return operator++();
	}

	Iterator& go_back() throw (std::runtime_error) {
		if (ptr == nullptr) { throw std::runtime_error("Can't get the next of a null node"); }
		return operator--();
	}

	value_type& operator*() { return ptr->value; }

	value_type& value() throw (std::runtime_error) {
		if (ptr == nullptr) { throw std::runtime_error("Can't get a value of a null node"); }
		return operator*();
	}

	Iterator& value(const value_type& value) throw (std::runtime_error) {
		if (ptr == nullptr) { throw std::runtime_error("Can't set a value for a null node"); }
		ptr->value = value;
		return *this;
	}

	Iterator next() throw (std::runtime_error) {
		if (ptr == nullptr) { throw std::runtime_error("Can't get the next of a null node"); }
		return Iterator(ptr->next);
	}

	Iterator& next(const Iterator& it) throw (std::runtime_error) {
		if (ptr == nullptr) { throw std::runtime_error("Can't get the next of a null node"); }
		ptr->next = it.ptr;
		return *this;
	}

	Iterator previous() throw (std::runtime_error) {
		if (ptr == nullptr) { throw std::runtime_error("Can't get the next of a null node"); }
		return Iterator(ptr->prev);
	}

	Iterator& previous(const Iterator& it) throw (std::runtime_error) {
		if (ptr == nullptr) { throw std::runtime_error("Can't get the next of a null node"); }
		ptr->prev = it.ptr;
		return *this;
	}

	Iterator operator+(size_type index) {
		auto curr = ptr;
		while (index--) curr = curr->next;
		return Iterator(curr);
	}

	Iterator operator-(size_type index) {
		auto curr = ptr;
		while (index--) curr = curr->prev;
		return Iterator(curr);
	}

	value_type& operator[](int index) {
		auto curr = ptr;
		if (index > 0) while (index--) curr = curr->next;
		else if (index < 0) while (index++) curr = curr->prev;
		return curr->value;
	}

	bool operator==(const Iterator& it) const { return ptr == it.ptr; }
	bool operator==(Node* node) const { return ptr == node; }

	bool operator!=(const Iterator& it)  const{ return ptr != it.ptr; }
	bool operator!=(Node* node) const { return ptr != node; }

	explicit operator bool() const { return ptr != nullptr; }

private:
	Node* ptr{};
};
