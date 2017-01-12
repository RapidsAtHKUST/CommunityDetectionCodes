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


#ifndef PRQUEUE_H
#define PRQUEUE_H

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

/**
 * An useful superclass for event implemetations. 
 */

template<typename SubclassType> 
class CalQEvent {  
private:
  SubclassType * next;
protected: /* For making it possible to set time */
  unsigned long int time;
public:
  CalQEvent(unsigned long int timeScheduled=0)
    :time(timeScheduled), next(0) {} /* In fact, null not really necessary */
  unsigned long int getTime() const {return time;}
  SubclassType * getNext() const {return next;}
  void setNext(SubclassType * ptrTarget) {next=ptrTarget;}
};

  

template<typename EventType> 
struct EventTypeTraits {
  typedef unsigned long int TimeType;
  typedef EventType * EventPtr;
  typedef EventType * const ConstEventPtr; //pointer to const 
  static TimeType getEventTime(const ConstEventPtr subject) {
    return subject->getTime();
  }
  static EventPtr getNext(const ConstEventPtr subject) {
    return subject->getNext();
  }
  static void setNext(const EventPtr subject, const ConstEventPtr next) {
    subject->setNext(next);
  }
  static void prefetch(const ConstEventPtr subject) {};  
};

template<typename EventType>
class EventList {
  typedef EventTypeTraits<EventType> MyTraits;
  typedef MyTraits::EventPtr EventPtr;
  typedef MyTraits::ConstEventPtr ConstEventPtr;  
  typedef MyTraits::TimeType TimeType;
private:
  EventPtr root;
public:  

  EventList(): root(0) {};   

  ~EventList() {    
    EventPtr first;
    while (!isEmpty()) {
      first=pop();
      delete first;
    }
  }
  
  /*
   * Pushes the object poisted by the subject down the queue to the
   * correct place. Returns the number of dereference 
   * operations performed, that is, empty list=0 etc.
   */

  /* Old version. This implementation does not need such a heavy 
   * solution. */
  //  unsigned int push(const EventPtr subject) {   
//      unsigned int cost=0; 
//      assert(subject != 0);
//      if ((root == 0) || 
//  	((cost++), // A side effect, maintaining the nice short-circuit 
//  	 (MyTraits::getEventTime(root) >= MyTraits::getEventTime(subject)))) { 
//        // Beginning of the queue      
//        assert(root != subject); // Do not reput events.  
//        MyTraits::setNext(subject, root); // null if needed
//        root=subject;
//      } else {
//        EventPtr prevToCurr=root;
//        while ((MyTraits::getNext(prevToCurr) != 0) && 
//  	     ((cost++), // Another size effect. Ingenious but ugly.
//  	      (MyTraits::getEventTime(MyTraits::getNext(prevToCurr)) 
//  	       < MyTraits::getEventTime(subject)))) {
//  	assert(MyTraits::getEventTime(prevToCurr) 
//  	       <= MyTraits::getEventTime(MyTraits::getNext(prevToCurr)));
//  	prevToCurr=MyTraits::getNext(prevToCurr);
//        }
//        assert(MyTraits::getNext(prevToCurr) != subject); 
//        // Do not reput events.
//        MyTraits::setNext(subject, MyTraits::getNext(prevToCurr)); 
//        // Null if needed.
//        MyTraits::setNext(prevToCurr, subject);
//      }
//      return cost;
//    }     

  void push(const EventPtr subject) {   
    assert(subject != 0);
    if ((root == 0) || 
	(MyTraits::getEventTime(root) >= MyTraits::getEventTime(subject))) { 
      // Beginning of the queue      
      assert(root != subject); // Do not reput events.  
      MyTraits::setNext(subject, root); // null if needed
      root=subject;
    } else {
      EventPtr prevToCurr=root;
      while ((MyTraits::getNext(prevToCurr) != 0) && 
	     (MyTraits::getEventTime(MyTraits::getNext(prevToCurr))
	      < MyTraits::getEventTime(subject))) {
	assert(MyTraits::getEventTime(prevToCurr) 
	       <= MyTraits::getEventTime(MyTraits::getNext(prevToCurr)));
	prevToCurr=MyTraits::getNext(prevToCurr);
      }
      assert(MyTraits::getNext(prevToCurr) != subject); 
      // Do not reput events.
      MyTraits::setNext(subject, MyTraits::getNext(prevToCurr)); 
      // Null if needed.
      MyTraits::setNext(prevToCurr, subject);
    }
  }   

  EventPtr pop() { 
    // No cost reference, as practically constant, at least when included
    // in CQ.
    EventPtr retval=root;
    if (retval != 0) {      
      root=MyTraits::getNext(root); // Null if needed
    }
    return retval;
  }

  TimeType getMinTime() {
    assert(root != 0); // If crash, own fault.
    return MyTraits::getEventTime(root);
  }

  bool isEmpty() {return (root==0);}

  /** 
   * Removes the element pointed to by the argument, returning whether
   * it was found. Not really intended for checking, so the event order
   * is not utilized, albeit checked in assertion.
   */

  bool remove(EventPtr subject) {
    if (root == 0) {
      return false;
    } 
    if (root == subject) {
      root=MyTraits::getNext(root);
      return true;
    }    
    EventPtr prevToCurr=root;
    while (MyTraits::getNext(prevToCurr) != 0) {
      assert (MyTraits::getEventTime(prevToCurr) 
	      <= MyTraits::getEventTime(MyTraits::getNext(prevToCurr)));
      if (MyTraits::getNext(prevToCurr) == subject) {
	MyTraits::setNext(prevToCurr,MyTraits::getNext(subject));
	return true;
      }
    }    
    // Not found.
    return false;
  }
  void prefetch() {__builtin_prefetch(root);}

};    
 
/** 
 * Funtional part of a calendar queue for the given event type. Time type
 * is assumed to be integral and year length less that maximum value
 * of unsigned int (can be assumed to be >= 2^32 = 4G
 */

template<typename EventType> class CalQCore {
private: 
  typedef EventTypeTraits<EventType> MyTraits;
  typedef MyTraits::EventPtr EventPtr;
  typedef MyTraits::TimeType TimeType;
  
  /* WHEN ADDING TO BELOW, PLEASE REMEMBER TO MODIFY THE ASSIGNMENT 
   * OPERATOR, ALSO*/
  EventList<EventType> * bins; // The root array
  
  unsigned int binSize; 
  unsigned int numBins;

  /* ...and all others redundant with respect to these: */
  
  unsigned int divideShift; // log2 binSize;
  unsigned int moduloMask; // yearLength-1;
  unsigned int logTableSize; 

  /* State variables: */
  
  unsigned int currBin;
  TimeType nextYearStart;
  unsigned int numEvents;
  TimeType lastPopped;
    
  unsigned int getSlot(const TimeType time) const { 
    return (time & moduloMask) >> divideShift;
  }

  CalQCore() {};

public:
  CalQCore(unsigned char logBinSize, // To enforce pow2 -rule 
	   unsigned char logNumBins, 
	   TimeType startTime=0) {
    logTableSize=logNumBins;
    binSize = 1 << logBinSize;
    numBins = 1 << logNumBins;
    divideShift=logBinSize;
    moduloMask=binSize*numBins-1; // Year length-1
    lastPopped=startTime;
    currBin=getSlot(startTime);
    nextYearStart=((startTime >> (logBinSize + logNumBins))+1)*binSize*numBins;
    bins=new EventList<EventType>[numBins];
    std::cerr << "New queue core. Bins:" << numBins << ", nextYearstart:";
    std::cerr << nextYearStart << ", starttime:" << lastPopped << "\n";
    numEvents=0;
  }

  ~CalQCore() {
    delete[] bins;
  }

  CalQCore<EventType>& operator=(CalQCore<EventType>& src) {
    delete[] bins; 
    logTableSize=src.logTableSize;
    binSize=src.binSize;
    numBins=src.numBins;
    divideShift=src.divideShift;
    moduloMask=src.moduloMask;
    currBin=src.currBin;
    nextYearStart=src.nextYearStart;
    numEvents=src.numEvents;
    lastPopped=src.lastPopped;
    bins=src.bins;
    src.bins=0; /* So that we won't delete it. */
    return *this;
  } 

  /**
   * Pushes an event to the queue. We only enforce causality rules here in
   * the assertion. Of course, the causality can be lifted by setting
   * the current bin to the one where a new uncausal one is put and
   * adjusting the lastPopped. This might be useful for some kind of 
   * "lazy" ordering.
   */
  
  void push(const EventPtr subject) {
    if (!(MyTraits::getEventTime(subject) >= lastPopped)) {
      std::cerr << "WtF? " << MyTraits::getEventTime(subject) << "," << lastPopped;
      assert(false);
    }
    numEvents++;
    /* Is this Lisp or what?:*/
    bins[getSlot(MyTraits::getEventTime(subject))].push(subject); 
  }

  EventPtr pop(unsigned int & probeLenAcc, unsigned int & numFutEvsAcc) {
    if (numEvents==0) {
      return 0;
    } 

    numEvents--;

    while (true) {
      if (!bins[currBin].isEmpty()) {
	if (bins[currBin].getMinTime() < nextYearStart) {
	  assert(lastPopped <= bins[currBin].getMinTime());
	  lastPopped=bins[currBin].getMinTime();
	  return bins[currBin].pop();
	} else { /* Wrong year */
	  numFutEvsAcc++;
	}
      }
      /* State: either bin empty or only contains event(s) of wrong year */
      probeLenAcc++;
      /* PREFETCH */
      //__builtin_prefetch(&bins[(currBin+PREFETCH_DIST*2) & (numBins-1)]);
      //bins[(currBin+PREFETCH_DIST) & (numBins-1)].prefetch();
      /* \PREFETCH*/
      if (++currBin == numBins) {
	currBin=0;
	nextYearStart+=(moduloMask+1);
	//std::cerr << "New year, next starting at:" << nextYearStart << "\n";
      }
    }
  }

  bool remove(const EventPtr subject) { /* Const might seem strange :) */
    return bins[getSlot(getEventTime(subject))].remove(subject);
  }

  /**
   * Eats everything contained in the source. The source is left empty.
   * Because of powers of 2 everywhere, the cache coherency might be nice.
   */

  void consume(CalQCore<EventType> & source) {
    unsigned int i=0;
    for (i=0; i<source.getNumBins(); i++) 
      while (!source.bins[i].isEmpty()) push(source.bins[i].pop());
    //std::cerr << "Events:" << numEvents << "\n";
  }      


  /**
   * There is not a method for querying the minimum time, because we 
   * cannot advance the currBin pointer beyond the last event 
   * popped: another with the same time stamp could be pushed soon.
   */
  TimeType getCurrTime() const {return lastPopped;} 
    
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

template<typename EventType> 
class MyCalQueue { 
  typedef EventTypeTraits<EventType>::TimeType TimeType;
  typedef EventTypeTraits<EventType>::EventPtr EventPtr;
private:
  CalQCore<EventType> queue;
  unsigned int popProbeLenSum; 
  unsigned int popFutureEventSum;
  unsigned int popCounter;
  
public:
  MyCalQueue(TimeType startTime=0, unsigned initLogNumEvents=0): 
    queue(0, initLogNumEvents+1, startTime), popProbeLenSum(0), 
    popFutureEventSum(0), popCounter(0) {}
  
  unsigned int push(EventPtr newEvent) {
    queue.push(newEvent); /* Not interested in the return value (op cost) */
    return queue.getNumEvents();
  }

  EventPtr pop() {
    /* Do the popping, updating accumulors */
    EventPtr retval=queue.pop(popProbeLenSum, popFutureEventSum);
    popCounter++;

    if (__builtin_expect(popCounter == queue.getNumBins(),0)) { /* Time to check */      
      std::cerr << "Checking for resize. Probe len avg:";
      std::cerr << ((float) popProbeLenSum/queue.getNumBins()) << ", future avg:";
      std::cerr << ((float) popFutureEventSum/queue.getNumBins()) << "\n"; 
            
      int numBinsLogChange=0;
      int yearLenLogChange=0;
      int binSizeLogChange=0;

      /* Bin size -loop */
      while ((popProbeLenSum >> (queue.getLogNumBins() 
				 + binSizeLogChange)) == 0) { 
	binSizeLogChange--;
      }
      /* Up: */
      while ((popProbeLenSum >> (queue.getLogNumBins() 
				 + binSizeLogChange)) > 3) { 
	binSizeLogChange++;
      }

      /* Year length -loop. Down: */
      while ((popFutureEventSum >> (queue.getLogNumBins()
				    -2+yearLenLogChange)) == 0) {
	yearLenLogChange--;
      }
      while ((popFutureEventSum >> (queue.getLogNumBins()
				    -2+yearLenLogChange)) > 3) {
	std::cerr << "Y len loop. Comp val:"
		  << (queue.getLogNumBins()-2+yearLenLogChange) << "\n";
	yearLenLogChange++;
      }
      std::cerr << "Y len log ch:" << yearLenLogChange << ",bin sz log ch:"
		<< binSizeLogChange << "\n";
      numBinsLogChange=yearLenLogChange-binSizeLogChange;
      
      if (binSizeLogChange || numBinsLogChange) {
	std::cerr << "Calqueue resize. Bin size:" 
		  << (1 << (queue.getLogBinSize()+binSizeLogChange))
		  << ", numBins:"
		  << (1 << (queue.getLogNumBins()+numBinsLogChange)) << "\n"; 
	CalQCore<EventType> newQueue(queue.getLogBinSize()+binSizeLogChange, 
				     queue.getLogNumBins()+numBinsLogChange, 
				     queue.getCurrTime());
	newQueue.consume(queue);
	queue=newQueue; /* Automagic deletion of bins. */
      }
      /* In any case: */
      popProbeLenSum=0;
      popFutureEventSum=0;
      popCounter=0;
    }
    return retval;     
  }
  bool remove(EventPtr subject) {return queue.remove(subject);}

  unsigned getNumEvents() {return queue.getNumEvents();}
};
  

  
#endif





