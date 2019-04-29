#ifndef priority_list_h
#define priority_list_h
#include "define.h"
template<class T>
class priorityList
{
public:
	priorityList() :m_nSize(0)
	{
		head = nullptr;
	}
	~priorityList()
	{
		clear();
	}
	void clear()
	{
		Node* node = head;
		do
		{
			/*if (node->data)
			{
				delete node->data;
			}*/
			node = head;
			head = head->next;
			delete node;
		} while (head);
		m_nSize = 0;
	}
	bool ResetPriority(T* data, int32_t priority)
	{
		remove(data);
		return add(data, priority);
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
		if (head == nullptr)
		{
			head = element;
		}
		else
		{
			Node* node = head;
			Node* prev = head;
			while (node->priority > priority)
			{
				prev = node;
				node = node->next;
				if (!node)
				{
					break;;
				}
			}
			if (!prev->next)
			{
				prev->next = element;
			}
			else
			{
				node = prev->next;
				prev->next = element;
				element->next = node;
			}
		}
		m_nSize++;
		return true;
	}
	bool remove(const T* data)
	{
		if (m_nSize <= 0)
		{
			return false;
		}
		Node* node = head;
		Node* prev = head;
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
				m_nSize--;
				return true;
			}
			prev = node;
			node = node->next;
		} while (node->next);
		return false;
	}
	T* GetAtIndex(int32_t index)
	{
		if (index >= 0 && index < m_nSize)
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
		return m_nSize;
	}
private:
	struct Node
	{
		Node() = delete;
		explicit Node(T* data) :data(data) {}
		T* data;
		Node* next;
		int32_t priority;
	};
	Node* head;
	int32_t m_nSize;
};
#endif
