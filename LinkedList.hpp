#pragma once
#ifndef CASTLE_LINKED_LIST
#define CASTLE_LINKED_LIST

#include <vector>
#include <functional>
#include <type_traits>

// this could be usefull for class pointers. not suitable for structures such as vectors

template<class T>
class LinkedList
{
public:
	typedef void(*IterateFunc)(T*);

	struct Node {
		T* data;
		Node* next;
		Node(T* _data, Node* _next) : data(_data), next(_next) {  }
	};

	Node* rootNode, *endNode;
	int nodeCount;

	LinkedList() : rootNode(nullptr), endNode(nullptr), nodeCount(0) { }

	LinkedList(T* firstClass) : rootNode(new Node(firstClass, nullptr)), nodeCount(1) { endNode = rootNode; }

	LinkedList(Node* _rootNode) : rootNode(_rootNode), endNode(_rootNode), nodeCount(1) { }
	
	// do not initialize with stack allocated array
	LinkedList(T** array, int arraySize) : nodeCount(arraySize) {
		for (int i = 0; i < arraySize; ++i) 
		{
			AddFront(array[i]);
		}
	}

	LinkedList(const std::vector<T*>& vector) : nodeCount(vector.size())
	{
		for (int i = 0; i < vector.size(); ++i) 
		{
			AddFront(vector[i]);
		}
	}

	~LinkedList() 
	{
		IterateRecDestroy(rootNode);
	}

	// not recomended for iterating. for iterating Iterate function is more efficient
	T* operator[](int index) const
	{
		int startIndex = 0;
		return FindDataByIndexRec(rootNode, index, startIndex);
	}
	
	#define _Template template<typename Derived, typename std::enable_if < \
		std::is_base_of<T, Derived>{} || std::is_same<T, Derived>{}, bool > ::type = true >
	
	_Template void AddFront(Derived* data)
	{
		Node* newNode = new Node(dynamic_cast<T*>(data), nullptr);
		if (!endNode)  {
			endNode = newNode;
			rootNode = newNode;
		}
		endNode->next = newNode;
		endNode = newNode;
		nodeCount++;
	}

	// sets data as first node(root node)
	_Template void AddBack(Derived* data)
	{
		Node* oldRoot = rootNode;
		rootNode = new Node(dynamic_cast<T*>(data), oldRoot);
		nodeCount++;
	}

	_Template void Remove(Derived* ptr)
	{
		Node* currentNode = rootNode;
	
		if (rootNode->data == ptr)  {
			rootNode = rootNode->next;
			delete currentNode;
			currentNode = rootNode;
			return;
		}

		do  {
			if (currentNode->next->data == ptr)
			{
				Node* removedNode = currentNode->next;
				currentNode->next = removedNode->next;
				delete removedNode;
				break;
			}
			currentNode = currentNode->next;
		} while (currentNode->next != nullptr);
	}

	_Template bool TryGetData(Derived** component)
	{
		*component = FindNodeByType<Derived>();
		return component[0];
	}

	#undef _Template

	// returns removed data
	T* RemoveFront() 
	{
		if (nodeCount == 0) return nullptr;
		T* oldEndNodeData = endNode->data;
		--nodeCount;
		endNode = FindSecondEndNodeRec(rootNode); // new end node
		endNode->next = nullptr;
		return oldEndNodeData;
	}

	// returns removed data
	// removes first node(root node)
	T* RemoveBack()
	{
		if (!rootNode->next) return nullptr;
		T* oldRootNodeData = rootNode->data;
		--nodeCount;
		rootNode = rootNode->next;
		return oldRootNodeData;
	}

	template<class Desired> Desired* FindNodeByType() const
	{
		return FindNodeByTypeRec<Desired>(rootNode);
	}

	T* FindNodeFromPtr(T* ptr) const
	{
		Node* currentNode = rootNode;
		if (currentNode->data == ptr) {
			return currentNode->data;
		}

		while (currentNode->next != nullptr) {
			currentNode = currentNode->next;
			if (currentNode->data == ptr) {
				return currentNode->data;
			}
		}
		return nullptr;
	}

	// this function is more performant than reaching every node by index
	void Iterate(IterateFunc func) const
	{
		Node* currentNode = rootNode;
		func(currentNode->data);

		while (currentNode->next != nullptr) {
			currentNode = currentNode->next;
			func(currentNode->data);
		}
	}

	template<class UserClass>
	void Iterate(UserClass* userClass, IterateFunc func) const
	{
		Node* currentNode = rootNode;
		do {
			std::invoke(func, *userClass, currentNode->data);
			currentNode = currentNode->next;
		}
		while (currentNode->next != nullptr);
	}

private:

	bool HasNodeFindByPtrRec(Node* node, T* data) 
	{
		if (node->next == nullptr) return false;
		if (node->data == data)    return true;
		return HasNodeFindByPtrRec(node->next);
	}

	// finds this[nodeCount-2] not last one but one before lastNode
	Node* FindSecondEndNodeRec(Node* node)
	{
		if (node->next == nullptr) return nullptr;
		if (node->next->next == nullptr) return node;
		return FindSecondEndNodeRec(node->next);
	}

	Node* FindNodeByIndexRec(Node* node, int targetIndex, int& index) const
	{
		if (index == targetIndex || node->next == nullptr) return node;
		return FindNodeByIndexRec(node->next, targetIndex, ++index);
	}

	T* FindDataByIndexRec(Node* node, int targetIndex, int& index) const
	{
		if (index == targetIndex || node->next == nullptr) return node->data;
		return FindDataByIndexRec(node->next, targetIndex, ++index);
	}

	template<class Desired> Desired* FindNodeByTypeRec(Node* node) const
	{
		if (node->data != nullptr && dynamic_cast<Desired*>(node->data))
			return dynamic_cast<Desired*>(node->data);
			
		if(node->next != nullptr) return FindNodeByTypeRec<Desired>(node->next);
		
		return nullptr;
	}

	void IterateRecDestroy(Node* node) 
	{
		if(node->next != nullptr)
		IterateRecDestroy(node->next);
		node->data->~T();
		free(node->data);
		free(node);
	}
};


#endif