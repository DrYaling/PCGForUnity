#ifndef priority_list_h
#define priority_list_h
#include "define.h"
#include "Logger/Logger.h"
template<class T>
class priorityList
{
public:
	priorityList() :_wpos(0)
	{
		head = nullptr;
	}
	~priorityList()
	{
		clear();
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
		if (_rpos > 0 && _wpos > 0)
		{
			LogError("try to add new element when reading");
			return false;
		}
		remove(data);
		return add(data, priority);
	}
	bool add(T* data, int32_t priority)
	{
		if (_rpos > 0 && _wpos > 0)
		{
			LogError("try to add new element when reading");
			return false;
		}
		Node* element = new Node(data);
		if (!element)
		{
			return false;
		}
		element->data = data;
		element->priority = priority;
		element->next = nullptr;
		if (head == nullptr)
		{
			head = element;
		}
		else
		{
			Node* node = head;
			Node* prev = nullptr;
			while (node->priority >= priority)
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
	bool remove(const T* data)
	{
		if (_wpos <= 0)
		{
			return false;
		}
		if (_rpos > 0 && _wpos > 0)
		{
			LogError("try to add new element when reading");
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
	T* ReadNext()
	{
		return GetAtIndex(_rpos++);
	}
	void ReadReset()
	{
		_rpos = 0;
	}
	int32_t size()
	{
		return _wpos;
	}
private:
	struct Node
	{
		Node() = delete;
		explicit Node(T* data) :data(data) {}
		T* data;
		Node* next;
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
	Node* head;
	int32_t _wpos;
	int32_t _rpos;
};
#endif
