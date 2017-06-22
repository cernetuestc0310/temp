/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * $Id: SmartPtr.h,v 1.7 2007-01-02 01:39:01 thomson Exp $
 *
 * $Log: SmartPtr.h,v $
 * Revision 1.7  2007-01-02 01:39:01  thomson
 * Code cleanup.
 *
 * Revision 1.6  2006-12-25 20:47:01  thomson
 * Some memory leaks fixes, valgrind info added.
 *
 * Revision 1.5  2006-07-03 18:01:51  thomson
 * SPtr define added.
 *
 * Revision 1.4  2004/06/04 19:47:06  thomson
 * Various fixes.
 *
 * Revision 1.3  2004/06/04 16:55:27  thomson
 * *** empty log message ***
 *
 * Revision 1.2  2004/03/29 22:06:49  thomson
 * 0.1.1 version
 *
 *
 * Released under GNU GPL v2 licence
 *
 */

#ifndef SMARTPTR_H
#define SMARTPTR_H

#include <iostream>

#define SPtr SmartPtr

class Ptr {
public:
    //constructor used in case of NULL SmartPtr
    Ptr() {
	ptr=NULL;
	refcount=1;
    }
    //Constructor used in case of non NULL SmartPtr
    Ptr(void* sth) {
	ptr=sth;
	refcount=1;
    }
    
    ~Ptr() {
	//if(ptr) delete ptr;
    }
    int refcount; //refrence counter
    void * ptr;	  //pointer to the real object
};

template <class T>
class SmartPtr
{

	//Don't use this class alone, it's used only in casting 
	//one smartpointer to another smartpointer 
	//e.g.
	//SmartPtr<a> a(new a()); SmartPtr<b> b(new(b)); a=b;

public:
    SmartPtr();
    SmartPtr(T* something);
	SmartPtr(Ptr *voidptr) { 
        if(voidptr) 
        {
            this->ptr=voidptr; 
            this->ptr->refcount++;
        }
        else
            this->ptr=new Ptr();
    }
    SmartPtr(const SmartPtr & ref);
	SmartPtr(int onlyNull);
	SmartPtr& operator=(const SmartPtr& old);

	operator Ptr*() {
	    if (this->ptr->ptr) 
		return this->ptr;
	    else
		return (Ptr*)NULL;
	}

    int refCount();
    ~SmartPtr();
    T& operator*() const;
    T* operator->() const;

 private:
    Ptr * ptr;
};

template <class T> SmartPtr<T>::SmartPtr() {
    ptr = new Ptr();
}

template <class T> int SmartPtr<T>::refCount() {
    if (this->ptr)
	return this->ptr->refcount;
    return 0;
}

template <class T>
SmartPtr<T>::SmartPtr(T* something) {
    ptr = new Ptr(something);
}

template <class T>
SmartPtr<T>::SmartPtr(const SmartPtr& old) {
	old.ptr->refcount++;
	this->ptr = old.ptr;
    this->ptr->refcount=old.ptr->refcount;
}

template <class T>
SmartPtr<T>::~SmartPtr() {
    if (!(--(ptr->refcount))) {
	delete (T*)(ptr->ptr);
	delete ptr;
    }
}

template <class T>
T& SmartPtr<T>::operator*() const {
    return *((T*)(ptr->ptr)); //it can return NULL
}

template <class T>
T* SmartPtr<T>::operator->() const {
    if (!ptr) {
	return 0;
    }
    return (T*)(ptr->ptr); //it can return NULL
}

//It's is called in eg. instrusction: return NULL;
//and SmartPtr is returned in function
template <class T>
SmartPtr<T>::SmartPtr(int )
{
	ptr=new Ptr(); //this->ptr->ptr is NULL
}

template <class T>
SmartPtr<T>& SmartPtr<T>::operator=(const SmartPtr& old) {
	if (this==&old)
		return *this;
	if (this->ptr) 
		if(!(--this->ptr->refcount))
		{
		    delete (T*)(this->ptr->ptr);
		    delete this->ptr;
		    this->ptr=NULL;
		}
		this->ptr=old.ptr;
		old.ptr->refcount++;
		//    cout << "operator=" << endl;
		return *this;
}
#endif
