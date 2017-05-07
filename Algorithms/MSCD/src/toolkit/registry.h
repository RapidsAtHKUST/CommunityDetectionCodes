/*
 *  registry.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 12/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_REGISTRY_H_
#define MSCD_REGISTRY_H_

// Includes
#include <map>
#include <utility>

namespace toolkit {

/** Registry pattern
 */
template <class T>
class Registry {
	
public:
	
	/// Return the singleton instance of registry
	static Registry<T> & GetInstance() {
		static Registry<T> instance;
		return instance;
	}
	
	/// Register a new item
	inline const bool Register(T * inst) {
		if (this->items.find(inst->GetName()) == this->items.end()) {
			this->items.insert(std::make_pair(inst->GetName(), inst));
			return true;
		}
		return false;
	}
	
	/// True if the registry contains the item
	inline bool Contain(const std::string & n) const {
		return (this->items.find(n) != this->items.end());
	}
	
	/// Return the item associated with the name if registered
	inline T* Lookup(const std::string & n) {
		if (this->items.find(n) != this->items.end())
			return this->items[n];
		return NULL;
	}
	
	/// Return the registered items
	inline const std::map<std::string,T*> & GetItems() const { return this->items; }
	
	/// Return the key registered at the given position
	inline const std::string GetKey(const int i) const {
		typedef typename std::map<std::string,T*>::const_iterator Iterator;
		Iterator it = this->items.begin();
		int n = 0;
		while ((it != this->items.end()) && (n != i)) {
			++it;
			++n;
		}
		if (it != this->items.end()) return it->first;
		return "";
	}
	
	/// Return the number of registered items
	inline int Count() const { return this->items.size(); }
	
private:
	
	/// Registry hash map
	std::map<std::string,T*> items;
	
	/// Private constructor
	Registry() {}
	
	/// No-copy constructor
	Registry(const Registry & inst);
	
	/// Private destructor
	~Registry() {
		typedef typename std::map<std::string,T*>::iterator Iterator;
		for (Iterator it = this->items.begin(); it != this->items.end(); ++it)
			delete it->second;
	}

};

} // namespace toolkit

#endif