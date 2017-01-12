/*
lcelib, A high performance library for complex networks research
Copyright (C) 2011  Aalto University

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include <cassert>
#include <cmath>

#define PREFETCH_DIST 8

/*
 * Templates needed for calendar queue implementations. The event lists are
 * implemeted as linked lists. Due to simplicity and speed considerations
 * (cache misses, single manipulation interface etc.) the lists are
 * singly linked and do not have the usual mock elements at the beginning.
 * Therefore the removal of elements is only possible through the queue 
 * itself. 
 *
 * A basic implementation for events to be put in the queue is given
 * in CalQEvent. Real events can be made by (possibly multiple) inheritance.
 * In addition, by specializing EventTypeTraits it is possible to use
 * any other classes supporting forward-linking and some method
 * for querying time. The time type should be defined in the traits, also.
 * 
 * Only intergral type are supported in this version. The queue is intended
 * for event-based simulations, and using floating-point time representation
 * for such can be generally considered as Bad Idea.
 *
 * This implementation limits the bin sizes and the number of bins
 * to be powers of two. Considering the advantage in the calculation of 
 * the necessary modulo and division operations, this should not be a
 * problem. In addition, the first part of the modulo operation might be 
 * carried out by a cast from unsigned long to unsigned int. 
 *
 * Events are owned by the queues, that is, their destructors do away with 
 * any of them contained. When outside the queue, events are on their own.
 */

template<typename TimeType>
class EventBase {
public:
  EventBase * next;
  TimeType time;

  EventBase(): next(0) {}

  EventBase(const TimeType evTime): next(0), time(evTime) {}

  EventBase(const TimeType evTime, EventBase * event):
    next(event), time(evTime) {}

  /* It is crucial that this is not virtual. */
  EventBase & operator=(const EventBase & src) {
    next=src.next;
    time=src.time;
    return *this;
  } 
  bool operator<(const TimeType cmp) const {
    return time<cmp;
  }

  operator TimeType() const {
    return time;
  }

  bool empty() const {return next==0;}

  void prefetch() const {
#ifdef GNU_PREFETCH
    __builtin_prefetch(next, 0, 0);
#endif 
  }

  EventBase pop() {
    assert(!empty());
    EventBase retval=*this;
    *this=*(this->next);
    return retval;
  }

  unsigned push(EventBase event) {
    EventBase * curr=this;
    unsigned cost=0;
    while (!(curr->empty()) && (*curr<event)) { 
      curr=curr->next;
      ++cost;
    }
    EventBase temp=*curr;
    *curr=event;
    *(curr->next)=temp;
    return cost;
  }

  void del() {
    delete next;
  }

};

/**
 * This is conceptually rather hard. The EventBase is a generalized pointer
 * to an event, containing also its time of occurrence. 
 */

template<typename TimeType> 
class EvQCore {
public:
  typedef EventBase<TimeType> EventType;
private: 
 
  EventType * bins; // The root array


  unsigned logTableSize;   
  unsigned binSize; 
  unsigned numBins;

  /* All others redundant */

  unsigned divideShift; // log2 binSize;
  unsigned moduloMask; // yearLength-1, binSize*numBins-1;

  /* State variables: */
  
  unsigned currBin;
  TimeType nextYearStart;
  unsigned numEvents;
    
  unsigned getSlot(const TimeType time) const { 
    return (time & moduloMask) >> divideShift;
  }

  EvQCore() {};

public:

  EvQCore(unsigned logBinSize, // To enforce pow2 -rule 
	  unsigned logNumBins, 
	  TimeType startTime=0)
    : logTableSize(logNumBins),    
      binSize(1 << logBinSize),
      numBins(1 << logNumBins),
      divideShift(logBinSize),
      moduloMask(binSize*numBins-1),
      currBin(getSlot(startTime)),
      nextYearStart(((startTime >> (logBinSize + logNumBins))+1)
		    *binSize*numBins) {
    bins=(EventType *) malloc(sizeof(EventType)*numBins);
    std::cerr << "New queue core. Bins:" << numBins << ", nextYearstart:";
    std::cerr << nextYearStart << ", starttime:" << startTime << "\n";
    numEvents=0;
  }

  /* FIXME: */

  ~EvQCore() {
    for (unsigned i=0; i<numBins; ++i) {
      while (!bins[i].empty()) bins[i].pop().del(); 
    }
  }

  void resize(unsigned logBinSize, unsigned logNumBins) {
    unsigned oldBins=numBins; 
    numBins=(1 << logNumBins);
    binSize=(1 << logBinSize);
    logTableSize=logNumBins;
    divideShift=logBinSize;
    moduloMask=(binSize*numBins-1);

      if (numBins>oldBins) { 
      bins=(EventType *) realloc(bins, sizeof(EventType)*numBins);
      /* For clarity, let's put this inside the loop: */
      for (unsigned i=oldBins; i<numBins; ++i) bins[i].next=0;
    }

    if (numEvents) {
      nextYearStart=((bins[currBin].time >> (logBinSize + logNumBins))+1)
	*binSize*numBins ;
      /* To fix everything, let us explicitly move the head of the 
       * queue, without using the ops for the whole queue. */
      unsigned oldCurr=currBin;
      currBin=getSlot(bins[currBin]);
      bins[currBin].push(bins[oldCurr].pop());
    }
 
    for (unsigned i=0; i<oldBins; ++i) {
      EventType * curr=&bins[i];
      unsigned newSlot;
      while (!curr->empty()) {
	newSlot=getSlot(*curr);
	if (newSlot != i) {
	  //std::cerr << "+";
	  bins[newSlot].push(curr->pop());
	} else {
	  //std::cerr << "-";
	  curr=curr->next;
	}
      }
    }

    if (numBins<oldBins) { 
      bins=(EventType *) realloc(bins, numBins*sizeof(EventType));
    }
  }
  
  void prefetch(const TimeType time) const {
#ifdef GNU_PREFETCH
    __builtin_prefetch(&(bins[getSlot(time)]), 0, 0);
#endif
  }

  /**
   * This is rather tricky. We push an _EventBase_ containing a pointer
   * to the real event. 
   */

  unsigned push(EventType subject) {
    unsigned bin=getSlot(subject);
    /* If the queue is empty or this shall be the new head, update: */
    if (numEvents==0 || subject<bins[currBin]) {
      currBin=bin;
      nextYearStart=((((TimeType) subject) >> (logTableSize + divideShift))+1)
	*binSize*numBins;
    }
    ++numEvents;
    return bins[bin].push(subject);
  }

  EventType pop(unsigned int & probeLenAcc, unsigned int & numFutEvsAcc) {
    if (numEvents==0) {
      return EventType();
    } 

    --numEvents;

    EventType retval=bins[currBin].pop();
    if (numEvents) {
      while (true) {
	if (!bins[currBin].empty()) {
	  if (bins[currBin] < nextYearStart) {
	    //std::cerr << "\n";
	    break; 
	  } else {
	    ++numFutEvsAcc;
	  }
	}
	++probeLenAcc;
	if (++currBin == numBins) {
	  currBin=0;
	  nextYearStart+=(moduloMask+1);
	  // std::cerr << "New year, next starting at:" 
	  //           << nextYearStart << "\n";
	}
	//std::cerr << "#";
      }
      bins[currBin].prefetch();
    }
    return retval;
  }

  void remove(const EventType subject) { /* Const might seem strange :) */
    unsigned bin=getSlot(subject);
    EventType * curr=bins[bin];
    while (*curr != subject) {
      curr=curr->next;
      assert(curr);
    }
    *curr=*(curr->next);
  }
  
  /**
   * There is not a method for querying the minimum time, because we 
   * cannot advance the currBin pointer beyond the last event 
   * popped: another with the same time stamp could be pushed soon.
   */
  TimeType getCurrTime() const {return bins[currBin];} 
    
  TimeType getYearLength() const {return moduloMask+1;}
  unsigned getLogBinSize() const {return divideShift;}
  unsigned getLogNumBins() const {return logTableSize;}

  unsigned getNumEvents() const {return numEvents;}

  unsigned getNumBins() const {return numBins;}
};
   
/**
 * A dynamic calendar queue implementation inpired by articles by ? and ?.
 * The basic assumption made is that the highest density of events is
 * found at the current time. This should be approximately true
 * for any event-generating mechanisms with no expilicit time dependence. 
 * 
 * In order to avoid linked list traversal, we aim at having approximately
 * two bins per event at current time. We use exclusively costs associated 
 * with pop operations in order to trigger requeue operations. The metrics
 * gathered are: 
 * 
 * 1. The density D of events belonging to the current year. 
 *
 * 2. The average number of events not belonging to current year encountered
 *    during probe: F.
 * 
 * The D is used to calculate the optimal bin size, whereas F is used for the
 * optimal year length.
 *
 *
 */

template<typename TimeType> 
class EvQueue { 
public:
  typedef typename EvQCore<TimeType>::EventType Event;
  typedef Event * EventPtr;
private:
  EvQCore<TimeType> queue;
  unsigned int popProbeLenSum; 
  unsigned int popFutureEventSum;
  unsigned int popCounter;
  
public:
  EvQueue(TimeType startTime=0, unsigned initLogNumEvents=0): 
    queue(0, initLogNumEvents+1, startTime), popProbeLenSum(0), 
    popFutureEventSum(0), popCounter(0) {}
  
  unsigned push(EventPtr newEvent, TimeType time) {
    queue.push(Event(time, newEvent)); 
    /* Not interested in the return value (op cost) */
    return queue.getNumEvents();
  }

  EventPtr pop(TimeType & time) {
    /* Do the popping, updating accumulors */
    Event popped=queue.pop(popProbeLenSum, popFutureEventSum);
    time=popped.time;
    popCounter++;

    if (__builtin_expect(popCounter == queue.getNumBins(),0)) { /* Time to check */      
      //std::cerr << "Checking for resize. Probe len avg:";
      //std::cerr << ((float) popProbeLenSum/queue.getNumBins()) << ", future avg:";
      //std::cerr << ((float) popFutureEventSum/queue.getNumBins()) << "\n"; 
            
      int numBinsLogChange=0;
      int yearLenLogChange=0;
      int binSizeLogChange=0;

      /* Bin size -loop. */
      while ((popProbeLenSum >> (queue.getLogNumBins() -1
				 + binSizeLogChange)) == 0) {
	//std::cerr << "#"; 
	binSizeLogChange--;
      }
      //std::cerr << "Here";
      /* Up: */
      while ((popProbeLenSum >> (queue.getLogNumBins() -1
				 + binSizeLogChange)) > 3) { 
	binSizeLogChange++;
      }

      //std::cerr << "There. Bin size log change:" << binSizeLogChange 
      //	<< "\n";

      /* Year length -loop. Down: */
      if (popFutureEventSum != 0) {
	while ((popFutureEventSum >> (queue.getLogNumBins()
				      -3+yearLenLogChange)) == 0) {
	  yearLenLogChange--;
	}
	while ((popFutureEventSum >> (queue.getLogNumBins()
				      -3+yearLenLogChange)) > 3) {
	  // std::cerr << "Y len loop. Comp val:"
	  //    << (queue.getLogNumBins()-2+yearLenLogChange) << "\n";
	  yearLenLogChange++;
	}
	//std::cerr << "Y len log ch:" << yearLenLogChange << ",bin sz log ch:"
	//  << binSizeLogChange << "\n";
	numBinsLogChange=yearLenLogChange-binSizeLogChange;
      } 

      if (binSizeLogChange || numBinsLogChange) {
	std::cerr << "Calqueue resize. Bin size:" 
	  << (1 << (queue.getLogBinSize()+binSizeLogChange))
	  << ", numBins:"
	  << (1 << (queue.getLogNumBins()+numBinsLogChange)) << "\n"; 
	queue.resize(queue.getLogBinSize()+binSizeLogChange,
		     queue.getLogNumBins()+numBinsLogChange);
	//std::cerr << "Done.";
      }
      /* In any case: */
      popProbeLenSum=0;
      popFutureEventSum=0;
      popCounter=0;
    }
    return popped.next;     
  }
  
  bool remove(EventPtr subject, TimeType time) {
    return queue.remove(Event(time, subject));
  }

  unsigned getNumEvents() {return queue.getNumEvents();}
};
  

  
#endif





