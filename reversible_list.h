/********************************************************************
Copyright 2017 Hung Le

********************************************************************/
#pragma once
#include "stdafx.h"
/**
* 
* A simply reversible list that supports reverse operation and splice operation both in O(1) time
* The internal implementation is a doubly-linked list without distinguishing between next pointer and previous pointer
*/
template <class T>
struct _srl_node {

	T _data{};
	std::array<_srl_node<T>*, 2> _neighbors;
	int _color;

	_srl_node();
	_srl_node(T arg_data);
};

template <class T>
_srl_node<T>::_srl_node() {
	_color = -1;
	_neighbors[0] = nullptr;
	_neighbors[1] = nullptr;
}
template <class T>
_srl_node<T>::_srl_node(T arg_data) : _data(arg_data) {
	_color = -1;
	_neighbors[0] = nullptr;
	_neighbors[1] = nullptr;
}


//	We always gurantee that head->neighbros[1] = null and tail->neighbors[1] = null
template<typename T>
class srlist {
	int _size;
public:
	_srl_node<T>* _head;
	_srl_node<T>* _tail;
	srlist();
	T next(_srl_node<T>* _prev_curr, _srl_node<T>* _curr);
	void push_back(T elem);
	void remove_back();
	T back();
	void push_front(T elem);
	void remove_front();
	T front();
	void splice(srlist<T> _arg_list);
	void reverse();
	bool is_empty();
	int size();
	void print();
	void debug();


};

template<typename T>
srlist<T>::srlist() {	// default constructor
	_head = new _srl_node<T>();
	_tail = new _srl_node<T>();
	_head->_neighbors[0] = _tail;
	_tail->_neighbors[0] = _head;
	_size = 0;
}
template<typename T>
T srlist<T>::next(_srl_node<T>* _prev_curr, _srl_node<T>* _curr) {
	int i = (_curr->_neighbors[0] == _prev_curr) ? 0 : 1;
	return _curr->_neighbors[1 - i]->_data;
}

template<typename T>
void srlist<T>::push_back(T elem) {
	//	std::cout << "Start pushing " << elem << std::endl;
	_srl_node<T>* _new_node = new _srl_node<T>(elem);
	_srl_node<T>* _end_node = _tail->_neighbors[0];
	int i = (_end_node->_neighbors[0] == _tail) ? 0 : 1;
	// _end_node._neighbors[i] = _tail
	_end_node->_neighbors[i] = _new_node;
	_new_node->_neighbors[1] = _end_node;
	_new_node->_neighbors[0] = _tail; //
	_tail->_neighbors[0] = _new_node;
	_size++;
//	std::cout << "Successfully pushing " << elem << std::endl;
}

// precondition the list must be non-empty, i.e, _size != 0
template<typename T>
void srlist<T>::remove_back() {
	_srl_node<T> *_end_node = _tail->_neighbors[0]; // the node to be removed
	_end_node->_color = 0;
	int i = (_end_node->_neighbors[0] == _tail) ? 0 : 1;
	_srl_node<T>* _prev_end_node = _end_node->_neighbors[1 - i];
	i = (_prev_end_node->_neighbors[0]->_color == 0) ? 0 : 1;
	// _prev_end_node->neighbors[i] = _end_node; 
	_prev_end_node->_neighbors[i] = _tail;
	_tail->_neighbors[0] = _prev_end_node;
	_size--;
	//std::cout << "Removing " << _end_node->_data << std::endl;
	delete _end_node;
}

template<typename T>
T srlist<T>::back() {
	return _tail->_neighbors[0]->_data;
}

template<typename T>
void srlist<T>::push_front(T elem){
	_srl_node<T> *_new_node = new _srl_node<T>(elem);
	_srl_node<T>* _start_node = _head->_neighbors[0];
	_head->_neighbors[0] = _new_node;
	_new_node->_neighbors[0] = _head;
	_new_node->_neighbors[1] = _start_node;
	int i = (_start_node->_neighbors[0] == _head) ? 0 : 1;
	// _start_node->neighbors[i] = head
	_start_node->_neighbors[i] = _new_node;
	_size++;
}

// precondition: the list must be non-empty
template<typename T>
void srlist<T>::remove_front() {
	_srl_node<T>* _start_node = _head->_neighbors[0];
	_start_node->_color = 0;
	int i = (_start_node->_neighbors[0] == _head) ? 0 : 1;
	// _start_node->neighbors[i] = _head
	_srl_node<T> * _next_of_start_node = _start_node->_neighbors[1 - i];
	_head->_neighbors[0] = _next_of_start_node;
	i = (_next_of_start_node->_neighbors[0]->_color == 0) ? 0 : 1;
	//_next_of_start_node->_neighbors[i] = _start_node
	_next_of_start_node->_neighbors[i] = _head;
	_size--;
	//std::cout << "Removing " << _start_node->_data << " from the front" << std::endl;
	delete _start_node;
}

template<typename T>
T srlist<T>::front() {
	return _head->_neighbors[0]->_data;
}

//http://www.cplusplus.com/reference/list/list/splice/
template<typename T>
void srlist<T>::splice(srlist<T> _arg_list) {
	_size += _arg_list.size();
	_srl_node<T>* _end_node = _tail->_neighbors[0];
	_srl_node<T>* _arg_start_node = _arg_list._head->_neighbors[0];

	// connect end_node to arg_start_node and vice versa
	int i = (_end_node->_neighbors[0] == _tail) ? 0 : 1;
	_end_node->_neighbors[i] = _arg_start_node;
	i = (_arg_start_node->_neighbors[0] == _arg_list._head) ? 0 : 1;
	_arg_start_node->_neighbors[i] = _end_node;
	// replace tail by the tail of arg_list
	_srl_node<T>* _arg_end_node = _arg_list._tail->_neighbors[0];
	i = (_arg_end_node->_neighbors[0] == _arg_list._tail) ? 0 : 1;
	_tail->_neighbors[0] = _arg_end_node;
	_arg_end_node->_neighbors[i] = _tail;
}


template<typename T>
void srlist<T>::reverse() {
	_srl_node<T>* tmp = _head;
	_head = _tail;
	_tail = tmp;
}
template<typename T>
bool srlist<T>::is_empty() {
	return _size == 0 ? true : false;
}

template<typename T>
int srlist<T>::size() {
	return _size;
}

template <typename T>
void srlist<T>::print() {
	_srl_node<T> *_it = _head->_neighbors[0];
	_head->_color = 0;
	_it->_color = 0;
	int i = 0;
	while (_it->_neighbors[1] != nullptr) {
		std::cout << _it->_data << "\t";
		i = 0;
		if (_it->_neighbors[0]->_color == 0) i = 1;
		//_it->_neighbors[i] is the unvisited child
		_it = _it->_neighbors[i];
		_it->_color = 0;
	}
	std::cout << std::endl;
	// reset the color
	_head->_color = -1;
	_it = _head->_neighbors[0];
	_it->_color = -1;
	while (_it->_neighbors[1] != nullptr) {
		i = 0;
		if (_it->_neighbors[0]->_color == -1) i = 1;
		//_it->->_neighbors[i] must have the color updated
		_it = _it->_neighbors[i];
		_it->_color = -1;
	}
}


template <typename T>
void srlist<T>::debug() {
	srlist<int> rev_list;
	rev_list.push_back(3);
	rev_list.push_back(2);
	rev_list.push_back(1);
	std::cout << "rev_list size: " << rev_list.size() << std::endl;
	rev_list.print();
	std::cout << "remove back: " << std::endl;
	rev_list.remove_back();
	std::cout << "rev_list size: " << rev_list.size() << std::endl;
	rev_list.print();
	std::cout << "push front: 0"<< std::endl;
	rev_list.push_front(0);
	std::cout << "push front: -1" << std::endl;
	rev_list.push_front(-1);
	std::cout << "rev_list size: " << rev_list.size() << std::endl;
	rev_list.print();
	rev_list.remove_front();
	std::cout << "remove front: " << std::endl;
	std::cout << "rev_list size: " << rev_list.size() << std::endl;
	rev_list.print();
	rev_list.remove_front();
	std::cout << "remove front: " << std::endl;
	std::cout << "rev_list size: " << rev_list.size() << std::endl;
	rev_list.print();
	std::cout << "reverse: " << std::endl;
	rev_list.reverse();
	rev_list.print();

	srlist<int> rev_list1;
	rev_list1.push_back(10);
	rev_list1.push_back(20);
	rev_list1.push_back(30);
	rev_list1.print();
	rev_list.splice(rev_list1);
	std::cout << "splice: " << std::endl;
	std::cout << "rev_list size: " << rev_list.size() << std::endl;
	rev_list.print();
	rev_list.reverse();
	std::cout << "revere the spliced list" << std::endl;
	std::cout << "rev_list size: " << rev_list.size() << std::endl;
	rev_list.print();

	srlist<int> rev_list2;
	rev_list2.push_back(50);
	rev_list2.push_back(60);
	rev_list2.push_back(70);
	rev_list2.print();
	std::cout << "rev_list size: " << rev_list.size() << std::endl;
	std::cout << "rev_list2 size: " << rev_list2.size() << std::endl;
	rev_list.splice(rev_list2);
	std::cout << "splice: " << std::endl;
	std::cout << "rev_list size: " << rev_list.size() << std::endl;
	rev_list.print();

}
