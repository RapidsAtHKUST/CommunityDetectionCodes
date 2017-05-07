//
//  atomic_set.h
//  MSCD
//
//  Created by Erwan Le Martelot on 19/04/2012.
//  Copyright (c) 2012 Erwan Le Martelot. All rights reserved.
//

#ifndef MSCD_ATOMIC_SET_H_
#define MSCD_ATOMIC_SET_H_

#include <set>
#include <mutex>
#include <vector>
#include <thread>

namespace toolkit {

/// Atomic set implementing a readers-writers problem solution with priority to writers (2nd r/w problem) 
template <class T>
class atomic_set {

public:
	
	/// Constructor
	atomic_set() : nbr(0), nbw(0) {}
	
	/// Find (read operation)
	inline bool find(const T & item) {
		//printf("%d: find IN\n",std::this_thread::get_id());
		priority_mut.lock();
		r_mut.lock();
		nbr_mut.lock();
		++nbr;
		if (nbr == 1) w_mut.lock();
		nbr_mut.unlock();
		r_mut.unlock();
		priority_mut.unlock();
		
		bool result = (this->set.find(item) != this->set.end());
		
		nbr_mut.lock();
		--nbr;
		if (nbr == 0) w_mut.unlock();
		nbr_mut.unlock();
		//printf("%d: find OUT\n",std::this_thread::get_id());
		return result;
	}
	
	/// Return items (read operation)
	inline void items(std::vector<T> & items) {
		//printf("%d: items IN\n",std::this_thread::get_id());
		items.clear();
		priority_mut.lock();
		r_mut.lock();
		nbr_mut.lock();
		++nbr;
		if (nbr == 1) w_mut.lock();
		nbr_mut.unlock();
		r_mut.unlock();
		priority_mut.unlock();

		items.insert(items.begin(), this->set.begin(), this->set.end());
		
		nbr_mut.lock();
		--nbr;
		if (nbr == 0) w_mut.unlock();
		nbr_mut.unlock();
		//printf("%d: items OUT\n",std::this_thread::get_id());
	}
	
	/// Return number of items (read operation)
	inline long nb_items() {
		long nb;
		//printf("%d: items IN\n",std::this_thread::get_id());
		priority_mut.lock();
		r_mut.lock();
		nbr_mut.lock();
		++nbr;
		if (nbr == 1) w_mut.lock();
		nbr_mut.unlock();
		r_mut.unlock();
		priority_mut.unlock();
		
		nb = this->set.size();
		
		nbr_mut.lock();
		--nbr;
		if (nbr == 0) w_mut.unlock();
		nbr_mut.unlock();
		//printf("%d: items OUT\n",std::this_thread::get_id());
		return nb;
	}
	
	/// Insert (write operations)
	inline void insert(const T & item) {
		//printf("%d: insert IN\n",std::this_thread::get_id());
		nbw_mut.lock();
		++nbw;
		if (nbw == 1) r_mut.lock();
		nbw_mut.unlock();
		
		w_mut.lock();
		this->set.insert(item);
		w_mut.unlock();
		
		nbw_mut.lock();
		--nbw;
		if (nbw == 0) r_mut.unlock();
		nbw_mut.unlock();
		//printf("%d: insert OUT\n",std::this_thread::get_id());
	}
	
	/// Erase (write operations)
	inline void erase(const T & item) {
		//printf("%d: erase IN\n",std::this_thread::get_id());
		nbw_mut.lock();
		++nbw;
		if (nbw == 1) r_mut.lock();
		nbw_mut.unlock();
		
		w_mut.lock();
		this->set.erase(item);
		w_mut.unlock();
		
		nbw_mut.lock();
		--nbw;
		if (nbw == 0) r_mut.unlock();
		nbw_mut.unlock();
		//printf("%d: erase OUT\n",std::this_thread::get_id());
	}
	
	/// Clear (write operations)
	inline void clear() {
		//printf("%d: clear IN\n",std::this_thread::get_id());
		nbw_mut.lock();
		++nbw;
		if (nbw == 1) r_mut.lock();
		nbw_mut.unlock();
		
		w_mut.lock();
		this->set.clear();
		w_mut.unlock();
		
		nbw_mut.lock();
		--nbw;
		if (nbw == 0) r_mut.unlock();
		nbw_mut.unlock();
		//printf("%d: clear OUT\n",std::this_thread::get_id());
	}

	/// Insert non-atomic
	inline void insert_na(const T & item) {
		this->set.insert(item);
	}
	
	/// Erase non-atomic
	inline void erase_na(const T & item) {
		this->set.erase(item);
	}
	
private:
	
	// Set that should be made atomic
	std::set<T> set;
	
	// Mutexes
	std::mutex nbr_mut, nbw_mut, r_mut, w_mut, priority_mut;
	
	/// Number of readers and writers
	unsigned int nbr, nbw;
	
};

} // namespace toolkit
	
#endif
