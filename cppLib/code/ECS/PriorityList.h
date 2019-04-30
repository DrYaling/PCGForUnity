#ifndef priority_list_h
#define priority_list_h
#include "define.h"
#include "Logger/Logger.h"
template<class T>
class priorityList
{
public:
	class Node
	{
		friend priorityList;
	public:
		Node() = delete;
		explicit Node(T* data) :data(data)
		{
		}
		explicit Node(const Node& n) :data(n.data), next(n.next), priority(n.priority) {}
		T* data;
	private:
		Node * next;
		int32_t priority;
		void insertAfter(Node* node)
		{
			if (!node)
			{
				return;
			}
			node->next = next;
			next = node;
		}
	};
public:

	class iterator
	{
	public:
		friend class priorityList;  // suggest u to use friend class
		explicit iterator(Node* p = 0) {
			current = p;
		}
		iterator(const iterator& other) {
			current = other.current;
		}
		T* ptr()
		{
			return current->data;
		}
		iterator& operator++() {
			current = current->next;;
			return *this;
		}
		iterator operator++(int) {
			iterator temp = *this;
			++(*this);
			return temp;
		}
		T* operator->() {
			return current->data;
		}
		T& operator*() {
			return *current->data;
		}
		const T* operator->()const {
			return current->data;
		}
		const T& operator*() const {
			return current->data;
		}
		bool operator==(const iterator & rhs) const
		{
			return current == rhs.current;
		}
		bool operator!=(const iterator & rhs) const
		{
			return !(*this == rhs);
		}
		// u can use iterator as Node* sometimes
		operator T * () { return current->data; }  // conversion;
	private:
		Node * current;  // current listelem or 0;
	};

private:
	iterator __end;
public:
	/*
	C++ STL style iterator variable.  Call begin() to get
	the first iterator, pre-increment (++i) the iterator to get to
	the next value.  Use dereference (*i) to access the element.
	*/
	typedef iterator& Iterator;
	/** C++ STL style const iterator in same style as Iterator. */
	typedef const iterator& ConstIterator;

	/** stl porting compatibility helper */
	typedef ConstIterator const_iterator;
	/** stl porting compatibility helper */
	typedef iterator value_type;
	priorityList() :_wpos(0), head(nullptr), __end(nullptr)
	{
	}
	~priorityList()
	{
		clear();
	}
	ConstIterator begin() const
	{
		return head ? iterator(head) : __end;
	}
	ConstIterator end() const
	{
		return __end;
	}
	iterator erase(iterator position) {
		Node* p = position.current;
		Node* node = head;
		iterator retVal(p->next);
		Node* prev = nullptr;
		while (node != p)
		{
			prev = node;
			node = node->next;
			if (!node)
			{
				break;;
			}
		}
		if (prev)
		{
			prev->next = p->next;
		}
		else if (node)//remove first one element
		{
			head = p->next;
		}
		else
		{
			//??? next is null and no element is equals to current
		}
		delete p;
		_wpos--;
		return retVal;
	}
	void clear(bool deleteData = false)
	{
		Node* node = head;
		while (head)
		{
			if (deleteData && head->data)
			{
				delete head->data;
			}
			node = head;
			head = head->next;
			delete node;
		}
		_wpos = 0;
	}
	bool ResetPriority(T* data, int32_t priority)
	{
		if (_wpos <= 0)
		{
			LogError("no element was in this link list!");
			return false;
		}
		Node* element = nullptr;
		Node* prev = nullptr;
		Node* node = head;
		while (node)
		{
			if (node->data == data)
			{
				if (node->priority == priority)
				{
					return true;
				}
				element = node;
				element->priority = priority;
				//remove elememt from list
				if (node == head/*prev == nullptr*/)
				{
					head = node->next;
				}
				else
				{
					prev->next = node->next;
				}
				_wpos--;
				break;
			}
			prev = node;
			node = node->next;
		}
		if (element)//found
		{
			return add(element);
		}
		else//not found
		{
			return false;
		}
	}
	bool add(T* data, int32_t priority)
	{
		Node* element = new Node(data);
		if (!element)
		{
			return false;
		}
		element->data = data;
		element->priority = priority;
		element->next = nullptr;
		return add(element);
	}
	bool add(Node* element)
	{
		if (!element)
		{
			return false;
		}
		if (head == nullptr)
		{
			head = element;
		}
		else
		{
			Node* node = head;
			Node* prev = nullptr;
			while (node->priority >= element->priority)
			{
				prev = node;
				node = node->next;
				if (!node)
				{
					break;;
				}
			}
			if (prev)
			{
				prev->insertAfter(element);
			}
			else//prev == nullptr,choosen node is head
			{
				head = element;
				element->insertAfter(node);
			}
		}
		_wpos++;
		return true;
	}

	T* GetAtIndex(int32_t index)
	{
		if (index >= 0 && index < _wpos)
		{
			Node* pnode = head;
			while (index > 0)
			{
				index--;
				pnode = pnode->next;
				if (!pnode)
					return nullptr;
			}
			return pnode->data;
		}
		else
		{
			return nullptr;
		}
	}
	int32_t size()
	{
		return _wpos;
	}
private:
	bool _remove(const T* data)
	{
		if (_wpos <= 0)
		{
			return false;
		}
		Node* node = head;
		Node* prev = nullptr;
		do
		{
			if (node->data == data)
			{
				if (!prev)
				{
					head = node->next;
				}
				else
				{
					prev->next = node->next;
				}
				delete node;
				_wpos--;
				return true;
			}
			prev = node;
			node = node->next;
		} while (node);
		return false;
	}
private:
	Node * head;
	int32_t _wpos;
};
#endif
